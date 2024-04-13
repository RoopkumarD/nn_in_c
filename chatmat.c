#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int rows;
	int cols;
	double **data;
} Matrix;

void matrix_print(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		for (int j = 0; j < mat->cols; j++) {
			printf("%f\t", mat->data[i][j]);
		}
		printf("\n");
	}

	return;
}

Matrix *matrix_create(int rows, int cols) {
	Matrix *mat = (Matrix *)malloc(sizeof(Matrix));
	if (mat == NULL) {
		puts("Memory allocation for matrix failed");
		return NULL;
	}

	mat->rows = rows;
	mat->cols = cols;
	mat->data = (double **)malloc(rows * sizeof(double *));
	if (mat->data == NULL) {
		puts("Memory allocation failed for mat->data");
		free(mat);
		return NULL;
	}

	for (int i = 0; i < rows; i++) {
		mat->data[i] = (double *)malloc(cols * sizeof(double));
		if (mat->data[i] == NULL) {
			puts("Memory allocation failed for mat->data");
			for (int j = 0; j < i; j++) {
				free(mat->data[j]);
			}
			free(mat->data);
			free(mat);
			return NULL;
		}
	}

	return mat;
}

void matrix_free(Matrix *mat) {
	if (mat == NULL) {
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		free(mat->data[i]);
	}

	free(mat->data);
	free(mat);

	return;
}

void matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res) {
	if (mat1->cols != mat2->rows) {
		puts("Invalid dimension for matrix multiplication");
		return;
	}

	for (int i = 0; i < mat1->rows; i++) {
		for (int j = 0; j < mat2->cols; j++) {
			res->data[i][j] = 0;
			for (int k = 0; k < mat1->cols; k++) {
				res->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
			}
		}
	}

	return;
}
