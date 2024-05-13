#include <math.h>
#include <stdio.h>
#include "mat.h"
#include "utils.h"

double cost_function(Matrix *activation, Matrix *train_y, int train_x_length);
void feed_forward(
	Matrix *train_x,
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
float learn_rate = 5;

int main(void) {
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
        Matrix *zs[compute_layers];
        Matrix *activations[num_layers];
        Matrix *delta[compute_layers];
        Matrix *delta_weights[compute_layers];

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
		matrix_zero_init(acti);
		activations[i+1] = acti;

		Matrix *del = Matrix_create(layers[i+1], training_length);
		if (del == NULL) {
			puts("Failed to create delta matrix");
			retval = 1;
			goto cleanup;
		}
		matrix_zero_init(del);
		delta[i] = del;

		Matrix *del_w = Matrix_create(layers[i+1], layers[i]);
		if (del_w == NULL) {
			puts("Failed to create delta weights matrix");
			retval = 1;
			goto cleanup;
		}
		matrix_zero_init(del_w);
		delta_weights[i] = del_w;

		Matrix *zsm = Matrix_create(layers[i+1], training_length);
		if (zsm == NULL) {
			puts("Failed to create z matrix");
			retval = 1;
			goto cleanup;
		}
		matrix_zero_init(zsm);
		zs[i] = zsm;
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
	X_mat->transpose = 1;
	Y_mat->transpose = 1;

	// first matrix of activations
	/*
	activations[0] = Matrix_create(X_mat->rows, X_mat->cols);
	if (activations[0] == NULL) {
		puts("Failed to create matrix for activations[0]");
		retval = 1;
		goto cleanup;
	}
	matrix_copy(X_mat, activations[0]);
	*/
	activations[0] = X_mat;
	matrix_print(activations[0]);

	// Now training the model
	for (int i = 0; i < epoch; i++) {
		for (int j = 0; j < compute_layers; j++) {
			// z = np.dot(w, z) + b
			int able_to_multiply = matrix_mul(weights[j], activations[j], zs[j]);
			if (able_to_multiply == 1) {
				puts("mat1:");
				matrix_print(weights[j]);
				puts("mat2:");
				matrix_print(activations[j]);
				retval = 1;
				goto cleanup;
			}
			matrix_add(zs[j], bias[j], zs[j]);
			// z = sigmoid(z)
			sigmoid(zs[j], activations[j+1]);
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
		matrix_scalar_mul(Y_mat, -1);
		matrix_add(activations[num_layers-1], Y_mat, delta[compute_layers-1]);
		matrix_scalar_mul(Y_mat, -1);
		sigmoid_prime(activations[num_layers-1], zs[compute_layers-1]);
		matrix_multiply(delta[compute_layers-1], zs[compute_layers-1], delta[compute_layers-1]);

		double weighted_learn_rate = -1 * learn_rate/training_length;
		double restore_value = -1 * training_length/learn_rate;

		// updating bias
		// delta_b[-1] = np.sum(delta, axis=1).reshape((-1, 1))
		matrix_scalar_mul(delta[compute_layers-1], weighted_learn_rate);
		// b - ((self.learn_rate / len(training_x)) * db)
		int total_elem = delta[compute_layers-1]->cols * delta[compute_layers-1]->rows;
		for (int k = 0; k < total_elem; k++) {
			bias[compute_layers-1]->data[k/delta[compute_layers-1]->cols] 
				+= delta[compute_layers-1]->data[k];
		}
		matrix_scalar_mul(delta[compute_layers-1], restore_value);

		// updating weights
		// delta_w[-1] = np.dot(delta, activations[-2].transpose())
		matrix_transpose(activations[num_layers-2]);
		matrix_mul(delta[compute_layers-1], activations[num_layers-2], delta_weights[compute_layers-1]);
		matrix_transpose(activations[num_layers-2]);
		// w - ((self.learn_rate / len(training_x)) * dw)
		matrix_scalar_mul(delta_weights[compute_layers-1], weighted_learn_rate);
		total_elem = delta_weights[compute_layers-1]->cols * delta_weights[compute_layers-1]->rows;
		for (int k = 0; k < total_elem; k++) {
			weights[compute_layers-1]->data[k] += delta_weights[compute_layers-1]->data[k];
		}

		for (int m = 2; m < num_layers; m++) {
			/*
			# first getting the delta
			delta = np.dot(self.weights[-i + 1].transpose(), delta) * sigmoid_prime(
		    	    zs[-i]
		    	)
			*/
			matrix_transpose(weights[compute_layers-m+1]);
			matrix_mul(weights[compute_layers-m+1], delta[compute_layers-m+1], delta[compute_layers-m]);
			matrix_transpose(weights[compute_layers-m+1]);
			sigmoid_prime(activations[num_layers-m], zs[compute_layers-m]);
			matrix_multiply(delta[compute_layers-m], zs[compute_layers-m], delta[compute_layers-m]);
			
			// updating bias
			// delta_b[-i] = np.sum(delta, axis=1, keepdims=True)
			matrix_scalar_mul(delta[compute_layers-m], weighted_learn_rate);
			// b - ((self.learn_rate / len(training_x)) * db)
			int total_elem = delta[compute_layers-m]->cols * delta[compute_layers-m]->rows;
			for (int k = 0; k < total_elem; k++) {
				bias[compute_layers-m]->data[k/delta[compute_layers-m]->cols] 
					+= delta[compute_layers-m]->data[k];
			}
			matrix_scalar_mul(delta[compute_layers-m], restore_value);

			// updating weights
			// delta_w[-i] = np.dot(delta, activations[-i - 1].transpose())
			matrix_transpose(activations[num_layers-m-1]);
			matrix_mul(delta[compute_layers-m], activations[num_layers-m-1], delta_weights[compute_layers-m]);
			matrix_transpose(activations[num_layers-m-1]);
			// w - ((self.learn_rate / len(training_x)) * dw)
			matrix_scalar_mul(delta_weights[compute_layers-m], weighted_learn_rate);
			total_elem = delta_weights[compute_layers-m]->cols * delta_weights[compute_layers-m]->rows;
			for (int k = 0; k < total_elem; k++) {
				weights[compute_layers-m]->data[k] += delta_weights[compute_layers-m]->data[k];
			}
		}
	}
	
	// Then feed forward to check the result
	feed_forward(X_mat, weights, bias, num_layers);

	// above goto will jump to this line for execution
	// if goto is not invoked, still below will process as normally
	// basically goto just jumps at where to continue executing
	cleanup:
	// freeing all the memory asked
	matrix_free(X_mat);
	matrix_free(Y_mat);
	// matrix_free(activations[0]);
	for (int i = 0; i < compute_layers; i++) {
		matrix_free(weights[i]);
		matrix_free(bias[i]);
		matrix_free(activations[i+1]);
		matrix_free(zs[i]);
		matrix_free(delta[i]);
		matrix_free(delta_weights[i]);
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
	Matrix *weights[],
	Matrix *bias[],
	int num_layer
) {
        Matrix *zs[num_layer-1];
        Matrix *activations[num_layers];
	for (int i = 0; i < num_layer-1; i++) {
		Matrix *acti = Matrix_create(layers[i+1], training_length);
		if (acti == NULL) {
			puts("Failed to create activation matrix for feedforward");
			goto cleanup;
		}
		activations[i+1] = acti;

		Matrix *zsm = Matrix_create(layers[i+1], training_length);
		if (zsm == NULL) {
			puts("Failed to create z matrix for feedforward");
			goto cleanup;
		}
		zs[i] = zsm;
	}

	activations[0] = train_x;

	for (int j = 0; j < num_layer-1; j++) {
		// val = sigmoid(np.dot(W, x) + b)
		int able_to_multiply = matrix_mul(weights[j], activations[j], zs[j]);
		if (able_to_multiply == 1) {
			puts("mat1:");
			matrix_print(weights[j]);
			puts("mat2:");
			matrix_print(activations[j]);
			goto cleanup;
		}
		matrix_add(zs[j], bias[j], zs[j]);
		// z = sigmoid(z)
		sigmoid(zs[j], activations[j+1]);
	}

	puts("Result of Feed Forward is:");
	matrix_print(activations[num_layer-1]);

	cleanup:
	for (int i = 0; i < num_layer-1; i++) {
		matrix_free(activations[i+1]);
		matrix_free(zs[i]);
	}

	return;
}
