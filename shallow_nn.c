#include <stdio.h>
#include "mat.h"
#include "utils.h"

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

int epoch = 100;

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
	activations[0] = Matrix_create(X_mat->rows, X_mat->cols);
	if (activations[0] == NULL) {
		puts("Failed to create matrix for activations[0]");
		retval = 1;
		goto cleanup;
	}
	matrix_copy(X_mat, activations[0]);

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
			}
			matrix_add(zs[j], bias[j], zs[j]);
			// z = sigmoid(z)
			sigmoid(zs[j], activations[j+1]);
		}
		/*
		Future:
		Think of transpose without creating new matrix for it
		Create static array pointer for delta and delta_w
		```python
		delta = (activations[-1] - training_y) * sigmoid_prime(zs[-1])
		--- Here rather than creating another matrix for sum
		--- Just subtract bias[i] one column at a time
		delta_b[-1] = np.sum(delta, axis=1).reshape((-1, 1))
		delta_w[-1] = np.dot(delta, activations[-2].transpose())
		
		--- reverse iteration from second last layert to layer after input layer
		for i in range(2, self.num_layers):
		    delta = np.dot(self.weights[-i + 1].transpose(), delta) * sigmoid_prime(
		        zs[-i]
		    )
		    delta_b[-i] = np.sum(delta, axis=1).reshape((-1, 1))
		    delta_w[-i] = np.dot(delta, activations[-i - 1].transpose())
		
		self.weights = [
		    w - ((self.learn_rate / len(training_x)) * dw)
		    for w, dw in zip(self.weights, delta_w)
		]
		self.bias = [
		    b - ((self.learn_rate / len(training_x)) * db)
		    for b, db in zip(self.bias, delta_b)
		]
		print(
		    f"Epoch {ep} -> Cost {np.sum((activations[-1] - training_y) ** 2) * 0.5/len(training_x)}"
		)
		```
		*/
	}
	
	// Then feed forward to check the result

	// above goto will jump to this line for execution
	// if goto is not invoked, still below will process as normally
	// basically goto just jumps at where to continue executing
	cleanup:
	// freeing all the memory asked
	matrix_free(X_mat);
	matrix_free(Y_mat);
	matrix_free(activations[0]);
	for (int i = 0; i < compute_layers; i++) {
		matrix_free(weights[i]);
		matrix_free(bias[i]);
		matrix_free(activations[i+1]);
		matrix_free(zs[i]);
	}

	return retval;
}

// feed forward function
