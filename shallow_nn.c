#include <stdio.h>
#include "chatmat.h"

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
	// first initialising random weights and bias
	
	// weights initialisation
	for (int i = 0; i < num_layers-1; i++) {
		// first create a matrix for weight
		Matrix *w = matrix_create(layers[i+1], layers[i]);
		if (w == NULL) {
			puts("Failed to create weight matrix");
			// freeing previously created matrix
			for (int j = 0; j < i; j++) {
				matrix_free(weights[j]);
			}
			return 1;
		}
		random_initialisation(w);
		weights[i] = w;
	}

	// Weight pointer array size
	int weight_pointer_arr_size = sizeof(weights) / sizeof(weights[0]);

	// bias initialisation
	for (int i = 0; i < num_layers - 1; i++) {
		// first create a matrix for weight
		Matrix *b = matrix_create(layers[i+1], 1);
		if (b == NULL) {
			puts("Failed to create bias matrix");
			// freeing previously created matrix
			for (int j = 0; j < i; j++) {
				matrix_free(bias[j]);
			}
			// also freeing previously created weights
			for (int i = 0; i < weight_pointer_arr_size; i++) {
				matrix_free(weights[i]);
			}
			return 1;
		}
		matrix_zero_init(b);
		bias[i] = b;
	}

	// Bias pointer array size
	int bias_pointer_arr_size = sizeof(bias) / sizeof(bias[0]);


	// converting X and y into matrix form
	int X_rows = sizeof(X) / sizeof(X[0]);
	int X_cols = sizeof(X[0]) / sizeof(X[0][0]);
	Matrix *X_mat = matrix_create(X_rows, X_cols);
	if (X_mat == NULL) {
		puts("Failed to create matrix for X");
		// freeing above created weights and bias array
		for (int i = 0; i < weight_pointer_arr_size; i++) {
			matrix_free(weights[i]);
		}
		for (int i = 0; i < bias_pointer_arr_size; i++) {
			matrix_free(bias[i]);
		}
		return 1;
	}
	for (int i = 0; i < X_rows; i++) {
		for (int j = 0; j < X_cols; j++) {
			X_mat->data[i][j] = X[i][j];
		}
	}
	// since y is 1d arr
	int y_size = sizeof(Y) / sizeof(Y[0]);
	Matrix *Y_mat = matrix_create(1, y_size);
	if (Y_mat == NULL) {
		puts("Failed to create matrix for Y");
		// freeing previously created X_mat
		matrix_free(X_mat);
		// freeing above created weights and bias array
		for (int i = 0; i < weight_pointer_arr_size; i++) {
			matrix_free(weights[i]);
		}
		for (int i = 0; i < bias_pointer_arr_size; i++) {
			matrix_free(bias[i]);
		}
		return 1;
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

	// Now training the model
	
	// Then feed forward to check the result

	// freeing all the memory asked
	matrix_free(X_mat);
	matrix_free(Y_mat);
	for (int i = 0; i < weight_pointer_arr_size; i++) {
		matrix_free(weights[i]);
	}
	for (int i = 0; i < bias_pointer_arr_size; i++) {
		matrix_free(bias[i]);
	}
	return 0;
}
