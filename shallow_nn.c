#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mat.h"
#include "utils.h"

double cost_function(Matrix *activation, Matrix *train_y, int train_x_length);
void feed_forward(
	Matrix *train_x,
	int training_length,
	Matrix *weights[],
	Matrix *bias[],
	int num_layer
);

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define NUM_ROWS(array_2d) ARRAY_LEN(array_2d)
#define NUM_COLS(array_2d) ARRAY_LEN(array_2d[0])

int X[4][2] = {
	{0,0},
	{1,0},
	{0,1},
	{1,1},
};

int Y[4] = {0, 1, 1, 0};

int layers[] = {2, 2, 1};
int num_layers = sizeof(layers) / sizeof(int);
int training_length = ARRAY_LEN(X);

int epoch = 1000;
float learn_rate = 10;

int main(void) {
	srand(time(NULL));

	int retval = 0;

	Matrix *X_mat = NULL;
	Matrix *Y_mat = NULL;
	// because can't do dynamical sized array at file level
	Matrix *weights[num_layers-1];
	Matrix *bias[num_layers-1];
	int compute_layers = num_layers - 1;

	// array for training process
	/*
	Can't do this below because i get goto_into_protected_scope
	actually C99 allows dynamically sized array means allocating
	array by dynamic number like i did here. But i can't use goto
	to jump over this line. As C allocates the memory.

        Constraints:
        [...] A goto statement shall not jump from outside the scope of
	an identiﬁer having a variably modiﬁed type to inside the scope of
	that identiﬁer.

	inside the scope as after below line, we are into its' scope
        */
        Matrix *activations[num_layers];

	// initialising random weights, bias, activations and zs
	for (int i = 0; i < compute_layers; i++) {
		Matrix *w = Matrix_create(layers[i+1], layers[i]);
		if (w == NULL) {
			puts("Failed to create weight matrix");
			retval = 1;
			goto cleanup;
		}
		random_initialisation(w);
		weights[i] = w;

		Matrix *b = Matrix_create(layers[i+1], 1);
		if (b == NULL) {
			puts("Failed to create bias matrix");
			retval = 1;
			goto cleanup;
		}
		matrix_zero_init(b);
		bias[i] = b;

		Matrix *acti = Matrix_create(layers[i+1], training_length);
		if (acti == NULL) {
			puts("Failed to create activation matrix");
			retval = 1;
			goto cleanup;
		}
		activations[i+1] = acti;
	}

	// zs for temp storage while matrix multiplication
	int high_value_of_nodes = 0;
	for (int m = 1; m < num_layers; m++) {
		if (high_value_of_nodes < layers[m]) {
			high_value_of_nodes = layers[m];
		}
	}
	Matrix *zs = Matrix_create(high_value_of_nodes, training_length);
	if (zs == NULL) {
		puts("Failed to create zs matrix");
		retval = 1;
		goto cleanup;
	}
	Matrix *delta = Matrix_create(high_value_of_nodes, training_length);
	if (delta == NULL) {
		puts("Failed to create delta matrix");
		retval = 1;
		goto cleanup;
	}
	Matrix *delta_mul = Matrix_create(high_value_of_nodes, training_length);
	if (delta_mul == NULL) {
		puts("Failed to create delta_mul matrix");
		retval = 1;
		goto cleanup;
	}
	Matrix *delta_weights = Matrix_create(high_value_of_nodes, high_value_of_nodes);
	if (delta_weights == NULL) {
		puts("Failed to create delta_weights matrix");
		retval = 1;
		goto cleanup;
	}

	// converting X and y into matrix form
	int X_rows = NUM_ROWS(X);
	int X_cols = NUM_COLS(X);
	X_mat = Matrix_create(X_rows, X_cols);
	if (X_mat == NULL) {
		puts("Failed to create matrix for X");
		retval = 1;
		goto cleanup;
	}
	for (int i = 0; i < X_rows; i++) {
		for (int j = 0; j < X_cols; j++) {
			X_mat->data[i * X_cols + j] = X[i][j];
		}
	}
	// since y is 1d arr
	int y_size = ARRAY_LEN(Y);
	Y_mat = Matrix_create(1, y_size);
	if (Y_mat == NULL) {
		puts("Failed to create matrix for Y");
		retval = 1;
		goto cleanup;
	}
	for (int i = 0; i < y_size; i++) {
		Y_mat->data[i] = Y[i];
	}

	// now transposing X_mat and Y_mat
	matrix_transpose(X_mat);

	// first matrix of activations
	activations[0] = X_mat;

	// Now training the model
	for (int i = 0; i < epoch; i++) {
		for (int j = 0; j < compute_layers; j++) {
			// z = np.dot(w, z) + b
			zs->rows = weights[j]->rows;
			int able_to_multiply = matrix_mul(weights[j], activations[j], zs);
			if (able_to_multiply == 1) {
				puts("mat1:");
				matrix_print(weights[j]);
				puts("mat2:");
				matrix_print(activations[j]);
				retval = 1;
				goto cleanup;
			}
			int failure = matrix_col_add(bias[j], zs, zs, 1, 1);
			if (failure) {
				puts("Failed adding at line 173");
				retval = 1;
				goto cleanup;
			}
			// matrix_add(zs, bias[j], zs);
			// z = sigmoid(z)
			sigmoid(zs, activations[j+1]);
		}
		/*
		printing the current result
		print(
		    f"Epoch {ep} -> Cost {np.sum((activations[-1] - training_y) ** 2) * 0.5/len(training_x)}"
		)
		*/
		double cost_value = cost_function(activations[num_layers-1], Y_mat, training_length);
		printf("Epoch %d -> Cost %f\n", i, cost_value);

		// delta = (activations[-1] - training_y) * sigmoid_prime(zs[-1])
		delta->rows = activations[num_layers-1]->rows;
		int failure = matrix_add(activations[num_layers-1], Y_mat, delta, 1, -1);
		if (failure) {
			puts("Failed adding at line 194");
			retval = 1;
			goto cleanup;
		}
		zs->rows = activations[num_layers-1]->rows;
		sigmoid_prime(activations[num_layers-1], zs);
		matrix_multiply(delta, zs, delta);

		double weighted_learn_rate = -1 * learn_rate/training_length;
		// double restore_value = -1 * training_length/learn_rate;

		// updating bias
		// delta_b[-1] = np.sum(delta, axis=1).reshape((-1, 1))
		// matrix_scalar_mul(delta, weighted_learn_rate);
		// b - ((self.learn_rate / len(training_x)) * db)
		int total_elem = delta->cols * delta->rows;
		for (int k = 0; k < total_elem; k++) {
			bias[compute_layers-1]->data[k/delta->cols] += (delta->data[k] * weighted_learn_rate);
		}
		// matrix_scalar_mul(delta, restore_value);

		// updating weights
		// delta_w[-1] = np.dot(delta, activations[-2].transpose())
		matrix_transpose(activations[num_layers-2]);
		delta_weights->rows = weights[compute_layers-1]->rows;
		delta_weights->cols = weights[compute_layers-1]->cols;
		int able_to_multiply = matrix_mul(delta, activations[num_layers-2], delta_weights);
		if (able_to_multiply == 1) {
			puts("mat1:");
			matrix_print(delta);
			puts("mat2:");
			matrix_print(activations[num_layers-2]);
			puts("res:");
			matrix_print(delta_weights);
			retval = 1;
			goto cleanup;
		}
		matrix_transpose(activations[num_layers-2]);
		// w - ((self.learn_rate / len(training_x)) * dw)
		// matrix_scalar_mul(delta_weights, weighted_learn_rate);
		total_elem = delta_weights->cols * delta_weights->rows;
		for (int k = 0; k < total_elem; k++) {
			weights[compute_layers-1]->data[k] += (delta_weights->data[k] * weighted_learn_rate);
		}

		for (int m = 2; m < num_layers; m++) {
			/*
			# first getting the delta
			delta = np.dot(self.weights[-i + 1].transpose(), delta) * sigmoid_prime(
		    	    zs[-i]
		    	)
			*/
			matrix_transpose(weights[compute_layers-m+1]);
			// cols because above transpose
			delta_mul->rows = weights[compute_layers-m+1]->cols;
			int able_to_multiply = matrix_mul(weights[compute_layers-m+1], delta, delta_mul);
			if (able_to_multiply == 1) {
				puts("mat1:");
				matrix_print(weights[compute_layers-m+1]);
				puts("mat2:");
				matrix_print(delta);
				puts("res:");
				matrix_print(delta_mul);
				retval = 1;
				goto cleanup;
			}
			matrix_transpose(weights[compute_layers-m+1]);
			zs->rows = activations[num_layers-m]->rows;
			sigmoid_prime(activations[num_layers-m], zs);
			matrix_multiply(delta_mul, zs, delta_mul);
			
			// updating bias
			// delta_b[-i] = np.sum(delta, axis=1, keepdims=True)
			// matrix_scalar_mul(delta_mul, weighted_learn_rate);
			// b - ((self.learn_rate / len(training_x)) * db)
			int total_elem = delta_mul->cols * delta_mul->rows;
			for (int k = 0; k < total_elem; k++) {
				bias[compute_layers-m]->data[k/delta_mul->cols] += (delta_mul->data[k] * weighted_learn_rate);
			}
			// matrix_scalar_mul(delta_mul, restore_value);

			// updating weights
			// delta_w[-i] = np.dot(delta, activations[-i - 1].transpose())
			matrix_transpose(activations[num_layers-m-1]);
			delta_weights->rows = weights[compute_layers-m]->rows;
			delta_weights->cols = weights[compute_layers-m]->cols;
			matrix_mul(delta_mul, activations[num_layers-m-1], delta_weights);
			matrix_transpose(activations[num_layers-m-1]);
			// w - ((self.learn_rate / len(training_x)) * dw)
			// matrix_scalar_mul(delta_weights, weighted_learn_rate);
			total_elem = delta_weights->cols * delta_weights->rows;
			for (int k = 0; k < total_elem; k++) {
				weights[compute_layers-m]->data[k] += (delta_weights->data[k] * weighted_learn_rate);
			}

			// copying delta_mul to delta
			delta->rows = delta_mul->rows;
			matrix_copy(delta_mul, delta);
		}
	}
	
	// Then feed forward to check the result
	feed_forward(X_mat, training_length, weights, bias, num_layers);

	// above goto will jump to this line for execution
	// if goto is not invoked, still below will process as normally
	// basically goto just jumps at where to continue executing
	cleanup:
	// freeing all the memory asked
	matrix_free(X_mat);
	matrix_free(Y_mat);
	matrix_free(zs);
	matrix_free(delta);
	matrix_free(delta_mul);
	matrix_free(delta_weights);
	// matrix_free(activations[0]);
	for (int i = 0; i < compute_layers; i++) {
		matrix_free(weights[i]);
		matrix_free(bias[i]);
		matrix_free(activations[i+1]);
	}

	return retval;
}

// np.sum((activations[-1] - training_y) ** 2) * 0.5/len(training_x)
double cost_function(Matrix *activation, Matrix *train_y, int train_x_length) {
	double total_res = 0;

	int total_elem = activation->cols * activation->rows;
	for (int i = 0; i < total_elem; i++) {
		double curr_elem = activation->data[i] - train_y->data[i];
		curr_elem = pow(curr_elem, 2);
		total_res += curr_elem;
	}

	total_res = total_res * 0.5;
	total_res = total_res / train_x_length;

	return total_res;
}

void feed_forward(
	Matrix *train_x,
	int training_length,
	Matrix *weights[],
	Matrix *bias[],
	int num_layer
) {
	if (train_x == NULL) {
		puts("train_x is NULL");
		goto cleanup;
	}

	int high_value_of_nodes = 0;
	for (int m = 1; m < num_layers; m++) {
		if (high_value_of_nodes < layers[m]) {
			high_value_of_nodes = layers[m];
		}
	}
	Matrix *zs = Matrix_create(high_value_of_nodes, training_length);
	if (zs == NULL) {
		puts("Failed to create zs matrix for feedforward");
		goto cleanup;
	}
	Matrix *activations = Matrix_create(high_value_of_nodes, training_length);
	if (activations == NULL) {
		puts("Failed to create zs matrix for feedforward");
		goto cleanup;
	}

	// setting activation to train_x
	// need to worry about transpose
	int rows, cols;
	int m = train_x->transpose;
	if (m == 0) {
		rows = train_x->rows;
		cols = train_x->cols;
	} else if (m == 1) {
		rows = train_x->cols;
		cols = train_x->rows;
	}

	if (activations->cols != cols) {
		puts("Bro send data in (input row) * (obs cols) format");
		goto cleanup;
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			activations->data[i * cols + j] = 
				train_x->data[(1-m)*(i * cols + j) + m * (i + j * rows)];
		}
	}
	activations->rows = rows;

	for (int j = 0; j < num_layer-1; j++) {
		// val = sigmoid(np.dot(W, x) + b)
		zs->rows = weights[j]->rows;
		int able_to_multiply = matrix_mul(weights[j], activations, zs);
		if (able_to_multiply == 1) {
			puts("mat1:");
			matrix_print(weights[j]);
			puts("mat2:");
			matrix_print(activations);
			goto cleanup;
		}
		int failure = matrix_col_add(bias[j], zs, zs, 1, 1);
		if (failure) {
			puts("Failed adding at line 189");
			goto cleanup;
		}
		// matrix_add(zs, bias[j], zs);
		// z = sigmoid(z)
		activations->rows = zs->rows;
		sigmoid(zs, activations);
	}

	puts("Result of Feed Forward is:");
	matrix_print(activations);

	cleanup:
	matrix_free(activations);
	matrix_free(zs);

	return;
}
