#include <stdio.h>
#include "chatmat.h"

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

int epoch = 100;

Matrix *weights[] = {NULL, NULL};
Matrix *bias[] = {NULL, NULL};

int main(void) {
	int retval = 0;
	Matrix *X_mat = NULL;
	Matrix *Y_mat = NULL;
	int weight_pointer_arr_size = ARRAY_LEN(weights);
	int bias_pointer_arr_size = ARRAY_LEN(bias);
	/*
	int weight_pointer_arr_size = sizeof(weights) / sizeof(weights[0]);
	int bias_pointer_arr_size = sizeof(bias) / sizeof(bias[0]);
	*/

	// first initialising random weights and bias
	// weights initialisation
	for (int i = 0; i < num_layers-1; i++) {
		// first create a matrix for weight
		Matrix *w = matrix_create(layers[i+1], layers[i]);
		if (w == NULL) {
			puts("Failed to create weight matrix");
			retval = 1;
			goto cleanup;
		}
		random_initialisation(w);
		weights[i] = w;
	}

	// bias initialisation
	for (int i = 0; i < num_layers - 1; i++) {
		// first create a matrix for weight
		Matrix *b = matrix_create(layers[i+1], 1);
		if (b == NULL) {
			puts("Failed to create bias matrix");
			retval = 1;
			goto cleanup;
		}
		matrix_zero_init(b);
		bias[i] = b;
	}

	// converting X and y into matrix form
	int X_rows = NUM_ROWS(X);
	int X_cols = NUM_COLS(X);
	/*
	int X_rows = sizeof(X) / sizeof(X[0]);
	int X_cols = sizeof(X[0]) / sizeof(X[0][0]);
	*/
	X_mat = matrix_create(X_rows, X_cols);
	if (X_mat == NULL) {
		puts("Failed to create matrix for X");
		retval = 1;
		goto cleanup;
	}
	for (int i = 0; i < X_rows; i++) {
		for (int j = 0; j < X_cols; j++) {
			X_mat->data[i][j] = X[i][j];
		}
	}
	// since y is 1d arr
	int y_size = ARRAY_LEN(Y);
	// int y_size = sizeof(Y) / sizeof(Y[0]);
	Y_mat = matrix_create(1, y_size);
	if (Y_mat == NULL) {
		puts("Failed to create matrix for Y");
		retval = 1;
		goto cleanup;
	}
	for (int i = 0; i < y_size; i++) {
		Y_mat->data[0][i] = Y[i];
	}

	// now transposing X_mat and Y_mat
	// rather than creating new array for each, let's add new property to Matrix struct
	// where transpose = 1 means read the array in opposite wise if 0 then read normally
	// thus this way no need to extra more memory
	//
	// done with adding this property -> just add this thing in matrix_mul function
	// check this article -> https://numpy.org/doc/stable/dev/internals.html

	// Now training the model
	
	// Then feed forward to check the result

	// above goto will jump to this line for execution
	// if goto is not invoked, still below will process as normally
	// basically goto just jumps at where to continue executing
	cleanup:
	// freeing all the memory asked
	matrix_free(X_mat);
	matrix_free(Y_mat);
	for (int i = 0; i < weight_pointer_arr_size; i++) {
		matrix_free(weights[i]);
	}
	for (int i = 0; i < bias_pointer_arr_size; i++) {
		matrix_free(bias[i]);
	}

	return retval;
}
