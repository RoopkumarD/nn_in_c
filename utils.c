#include "mat.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// sigmoid(mat1) = mat2
void sigmoid(Matrix *mat1, Matrix *mat2) {
	if (mat1 == NULL) {
		puts("Address mat1 is NULL");
		return;
	}
	if (mat2 == NULL) {
		puts("Address mat2 is NULL");
		return;
	}

	// checking if both matrix size are same or not
	if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
		puts("Matrix shape are not equal");
		return;
	}

	int total_elem = mat1->rows * mat1->cols;
	for (int i = 0; i < total_elem; i++) {
		double denominator = exp(-1 * mat1->data[i]) + 1;
		mat2->data[i] = 1 / denominator;
	}

	return;
}

// sigmoid_prime(mat1) = mat2
// mat1 should be sigmoid(mat) = mat1
// to calculate sigmoid_prime of mat
void sigmoid_prime(Matrix *mat1, Matrix *mat2) {
	if (mat1 == NULL) {
		puts("Address mat1 is NULL");
		return;
	}
	if (mat2 == NULL) {
		puts("Address mat2 is NULL");
		return;
	}

	// checking if both matrix size are same or not
	if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
		puts("Matrix shape are not equal");
		return;
	}

	int total_elem = mat1->rows * mat1->cols;
	for (int i = 0; i < total_elem; i++) {
		double s = mat1->data[i];
		mat2->data[i] = s * (1-s);
	}

	return;
}
