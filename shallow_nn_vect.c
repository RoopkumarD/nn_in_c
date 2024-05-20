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
	Matrix *wb[],
	int num_layer
);

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define NUM_ROWS(array_2d) ARRAY_LEN(array_2d)
#define NUM_COLS(array_2d) ARRAY_LEN(array_2d[0])

// extra 1 value node to multiply AX as ax + b
# define EXTRA_ONE 1
# define EPOCH 1000
# define LEARN_RATE 5

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

int main(void) {
	srand(time(NULL));

	int retval = 0;

	Matrix *X_mat = NULL;
	Matrix *Y_mat = NULL;
	Matrix *wb[num_layers-1];
	int compute_layers = num_layers - 1;

        Matrix *activations[num_layers];

	// initialising random weights, bias, activations and zs
	Matrix *last_weights_bias = Matrix_create(layers[num_layers-1], layers[num_layers-2] + 1);
	if (last_weights_bias == NULL) {
		puts("Failed to create weight matrix");
		retval = 1;
		goto cleanup;
	}
	random_initialisation(last_weights_bias);
	int total_elem = last_weights_bias->rows * last_weights_bias->cols;
	for (int i = 0; i < total_elem; i++) {
		last_weights_bias->data[i] = 1;
	}
	wb[compute_layers-1] = last_weights_bias;

	Matrix *acti = Matrix_create(layers[num_layers-1] + EXTRA_ONE, training_length);
	if (acti == NULL) {
		puts("Failed to create activation matrix");
		retval = 1;
		goto cleanup;
	}
	activations[num_layers-1] = acti;

	for (int i = 0; i < compute_layers-1; i++) {
		// cols +1 as storing bias there
		Matrix *weights_bias = Matrix_create(layers[i+1], layers[i] + 1);
		if (weights_bias == NULL) {
			puts("Failed to create weight matrix");
			retval = 1;
			goto cleanup;
		}
		random_initialisation(weights_bias);
		wb[i] = weights_bias;

		Matrix *acti = Matrix_create(layers[i+1] + EXTRA_ONE, training_length);
		if (acti == NULL) {
			puts("Failed to create activation matrix");
			retval = 1;
			goto cleanup;
		}
		activations[i+1] = acti;
	}

	// zs for temp storage while matrix multiplication
	int highest_node_num = 0;
	for (int m = 1; m < num_layers; m++) {
		if (highest_node_num < layers[m]) {
			highest_node_num = layers[m];
		}
	}
	Matrix *zs = Matrix_create(highest_node_num, training_length);
	if (zs == NULL) {
		puts("Failed to create zs matrix");
		retval = 1;
		goto cleanup;
	}
	Matrix *delta = Matrix_create(highest_node_num, training_length);
	if (delta == NULL) {
		puts("Failed to create delta matrix");
		retval = 1;
		goto cleanup;
	}
	Matrix *delta_mul = Matrix_create(highest_node_num, training_length);
	if (delta_mul == NULL) {
		puts("Failed to create delta_mul matrix");
		retval = 1;
		goto cleanup;
	}
	// for that bias term we add extra row and col
	Matrix *delta_wb = Matrix_create(highest_node_num + 1, highest_node_num + 1);
	if (delta_wb == NULL) {
		puts("Failed to create delta_weights matrix");
		retval = 1;
		goto cleanup;
	}

	// converting X and y into matrix form
	int X_rows = NUM_ROWS(X);
	int X_cols = NUM_COLS(X);
	int extra_x_cols = X_cols + EXTRA_ONE;
	X_mat = Matrix_create(X_rows, extra_x_cols);
	if (X_mat == NULL) {
		puts("Failed to create matrix for X");
		retval = 1;
		goto cleanup;
	}
	for (int i = 0; i < X_rows; i++) {
		for (int j = 0; j < X_cols; j++) {
			X_mat->data[i * extra_x_cols + j] = X[i][j];
		}
	}
	for (int i = 0; i < X_rows; i++) {
		X_mat->data[i * extra_x_cols + X_cols] = 1;
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

	/*
	What i did here is converted the whole AX + B operation in feedforward
	to MX one operation where M = [A B]
	For example:
	[w1 w2][x1  + [b1]
		x2]

	can also be written as
	[w1 w2 b1][x1
		   x2]

	Thus with above, in each step i am adding extra 1 to each activations
	except the last one as it is wasteful. By doing above, in backprop
	rather than doing two loops to update weights and bias separately
	we can update them at once. Refer below
	*/
	// training
	for (int i = 0; i < EPOCH; i++) {
		for (int j = 0; j < compute_layers; j++) {
			// z = np.dot(w, z) + b
			zs->rows = layers[j+1];
			int failure = matrix_mul(wb[j], activations[j], zs);
			if (failure == 1) {
				puts("Wasn't able to multiply matrix");
				puts("Location: In feedforward of training");
				retval = 2;
				goto cleanup;
			}
			int temp = activations[j+1]->rows; 
			activations[j+1]->rows = activations[j+1]->rows - 1;
			// z = sigmoid(z)
			failure = sigmoid(zs, activations[j+1]);
			if (failure == 1) {
				printf("Wasn't able to get sigmoid at layer %d, epoch %d\n", j, i);
				puts("Location: In sigmoid of training");
				retval = 2;
				goto cleanup;
			}
			activations[j+1]->rows = temp;

			if (j != compute_layers - 1) {
				// for last activations there is no need to waste
				// setting value equal to 1
				int start_posn = (activations[j+1]->rows-1) * activations[j+1]->cols;
				int end_posn = start_posn + activations[j+1]->cols;
				for (int m = start_posn; m < end_posn; m++) {
					activations[j+1]->data[m] = 1;
				}
			}
		}
		// printing the cost at this epoch
		activations[num_layers-1]->rows -= 1;
		double cost_value = cost_function(activations[num_layers-1], Y_mat, training_length);
		printf("Epoch %d -> Cost %f\n", i, cost_value);

		// last layer delta
		// delta = (activations[-1] - training_y) * sigmoid_prime(zs[-1])
		delta->rows = activations[num_layers-1]->rows;
		int failure = matrix_add(activations[num_layers-1], Y_mat, delta, 1, -1);
		if (failure == 1) {
			printf("Failed to add activations[%d] and Y_mat\n", num_layers-1);
			puts("Location: Training -> last layer delta");
			retval = 3;
			goto cleanup;
		}
		zs->rows = activations[num_layers-1]->rows;
		sigmoid_prime(activations[num_layers-1], zs);
		matrix_multiply(delta, zs, delta);
		activations[num_layers-1]->rows += 1;

		double weighted_learn_rate = -1 * ((double)LEARN_RATE/training_length);

		// updating wb
		// delta_wb[-1] = np.dot(delta, activations[-2].transpose())
		/*
		Cause matrix multiplication of delta with activations containing
		1 at the last column will result in matrix where if columns_num = n
		then n-1 column with all rows will be of delta_w and last column is for
		delta_b.

		Take any number of training length and see that all the deltas are added
		together for each training case.
		*/
		matrix_transpose(activations[num_layers-2]);
		delta_wb->rows = wb[compute_layers-1]->rows;
		delta_wb->cols = wb[compute_layers-1]->cols;
		failure = matrix_mul(delta, activations[num_layers-2], delta_wb);
		if (failure == 1) {
			printf("Wasn't able to mat_mul delta and activations[%d]\n", num_layers-2);
			puts("Location: In last layer delta_wb of training");
			retval = 3;
			goto cleanup;
		}
		matrix_transpose(activations[num_layers-2]);
		// wb - ((self.learn_rate / len(training_x)) * dwb)
		int total_elem = delta_wb->cols * delta_wb->rows;
		for (int k = 0; k < total_elem; k++) {
			wb[compute_layers-1]->data[k] += (delta_wb->data[k] * weighted_learn_rate);
		}

		for (int m = 2; m < num_layers; m++) {
			// getting delta of previous layer
			matrix_transpose(wb[compute_layers-m+1]);
			// cols because above transpose
			int temp = wb[compute_layers-m+1]->cols;
			wb[compute_layers-m+1]->cols -= 1;
			delta_mul->rows = wb[compute_layers-m+1]->cols;
			int failure = matrix_mul(wb[compute_layers-m+1], delta, delta_mul);
			if (failure == 1) {
				printf("Wasn't able to mat_mul wb[%d] and delta\n", compute_layers-m+1);
				printf("Location: In delta of layer %d of training\n", compute_layers-m);
				retval = 3;
				goto cleanup;
			}
			wb[compute_layers-m+1]->cols = temp;
			matrix_transpose(wb[compute_layers-m+1]);

			temp = activations[num_layers-m]->rows; 
			activations[num_layers-m]->rows = activations[num_layers-m]->rows - 1;
			zs->rows = activations[num_layers-m]->rows;
			failure = sigmoid_prime(activations[num_layers-m], zs);
			if (failure == 1) {
				puts("Wasn't able to get sigmoid prime");
				puts("Location: In sigmoid_prime of delta loop");
				retval = 2;
				goto cleanup;
			}
			activations[num_layers-m]->rows = temp;

			matrix_multiply(delta_mul, zs, delta_mul);

			// updating wb
			// delta_wb[-i] = np.dot(delta, activations[-i - 1].transpose())
			matrix_transpose(activations[num_layers-m-1]);
			delta_wb->rows = wb[compute_layers-m]->rows;
			delta_wb->cols = wb[compute_layers-m]->cols;
			failure = matrix_mul(delta_mul, activations[num_layers-m-1], delta_wb);
			if (failure == 1) {
				printf("Wasn't able to mat_mul delta and activations[%d]\n", num_layers-m-1);
				puts("Location: In layer loop delta_wb of training");
				retval = 3;
				goto cleanup;
			}
			matrix_transpose(activations[num_layers-m-1]);
			// wb - ((self.learn_rate / len(training_x)) * dwb)
			total_elem = delta_wb->cols * delta_wb->rows;
			for (int k = 0; k < total_elem; k++) {
				wb[compute_layers-m]->data[k] += (delta_wb->data[k] * weighted_learn_rate);
			}

			// copying delta_mul to delta
			delta->rows = delta_mul->rows;
			matrix_copy(delta_mul, delta);
		}
	}
	
	// Then feed forward to check the result
	feed_forward(X_mat, training_length, wb, num_layers);

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
	matrix_free(delta_wb);
	for (int i = 0; i < compute_layers; i++) {
		matrix_free(wb[i]);
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
	Matrix *wb[],
	int num_layer
) {
	if (train_x == NULL) {
		puts("train_x is NULL");
		goto cleanup;
	}

	int highest_node_num = 0;
	for (int m = 1; m < num_layers; m++) {
		if (highest_node_num < layers[m]) {
			highest_node_num = layers[m];
		}
	}
	Matrix *zs = Matrix_create(highest_node_num + EXTRA_ONE, training_length);
	if (zs == NULL) {
		puts("Failed to create zs matrix for feedforward");
		goto cleanup;
	}
	Matrix *activations = Matrix_create(highest_node_num + EXTRA_ONE, training_length);
	if (activations == NULL) {
		puts("Failed to create activations matrix for feedforward");
		goto cleanup;
	}

	// setting activation to train_x
	// need to worry about transpose
	SET_MATRIX_DIMENSIONS(train_x, )

	if (activations->cols != cols) {
		puts("Bro send data in (input row) * (obs cols) format");
		goto cleanup;
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			activations->data[i * cols + j] = train_x->data[i * mi + j * nj];
		}
	}
	activations->rows = rows;

	// don't have to do this as i am sending already attached 1 row train_x
	/*
	int start_posn = (activations->rows-1) * activations->cols;
	int end_posn = start_posn + activations->cols;
	for (int m = start_posn; m < end_posn; m++) {
		activations->data[m] = 1;
	}
	*/

	for (int j = 0; j < num_layer-1; j++) {
		// val = sigmoid(np.dot(W, x) + b)

		zs->rows = layers[j+1];
		int failure = matrix_mul(wb[j], activations, zs);
		if (failure == 1) {
			printf("Wasn't able to mat_mul wb[%d] and activations\n", j);
			printf("Location: Feedforward function loop\n");
			goto cleanup;
		}
		activations->rows = zs->rows;
		// z = sigmoid(z)
		failure = sigmoid(zs, activations);
		if (failure == 1) {
			puts("Wasn't able to get sigmoid");
			puts("Location: In sigmoid of feedforward function loop");
			goto cleanup;
		}
		activations->rows += EXTRA_ONE;

		if (j != num_layers - 2) {
			// for last activations there is no need to waste
			// setting value equal to 1
			int start_posn = (activations->rows-1) * activations->cols;
			int end_posn = start_posn + activations->cols;
			for (int m = start_posn; m < end_posn; m++) {
				activations->data[m] = 1;
			}
		}
	}

	activations->rows -= 1;
	puts("Result of Feed Forward is:");
	matrix_print(activations);

	cleanup:
	matrix_free(activations);
	matrix_free(zs);

	return;
}
