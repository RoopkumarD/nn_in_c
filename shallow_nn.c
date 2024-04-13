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
			return 1;
		}
		matrix_zero_init(b);
		bias[i] = b;
	}

	// Now training the model
	
	// Then feed forward to check the result

	return 0;
}
