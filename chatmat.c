#include <stdio.h>
#include <stdlib.h>
#include "chatmat.h"

// mat1 + mat2 = mat3
void matrix_add(Matrix *mat1, Matrix *mat2, Matrix *mat3) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return;
	}
	if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return;
	}
	if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return;
	}

	// checking if all matrix size are same or not
	if ((mat1->cols != mat2->cols) || (mat1->cols != mat3->cols)) {
		puts("Matrix shape are not equal");
		return;
	}
	else if ((mat1->rows != mat2->rows) || (mat1->rows != mat3->rows)) {
		puts("Matrix shape are not equal");
		return;
	}

	for (int i = 0; i < mat1->rows; i++) {
		for (int j = 0; j < mat1->cols; j++) {
			mat3->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
		}
	}

	return;
}

// copying mat1 into mat2
void matrix_copy(Matrix *mat1, Matrix *mat2) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return;
	}
	if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return;
	}

	// checking if both matrix size are same or not
	if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
		puts("Matrix shape are not equal");
		return;
	}

	for (int i = 0; i < mat1->rows; i++) {
		for (int j = 0; j < mat1->cols; j++) {
			mat2->data[i][j] = mat1->data[i][j];
		}
	}

	return;
}

Matrix *matrix_transpose(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return NULL;
	}

	Matrix *mat_trans = NULL;

	if (mat->rows == mat->cols) {
		for (int i = 0; i < mat->rows; i++) {
			for (int j = i+1; j < mat->cols; j++) {
				double temp = mat->data[i][j];
				mat->data[i][j] = mat->data[j][i];
				mat->data[j][i] = temp;
			}
		}
		mat_trans = mat;
	} else {
		int new_rows = mat->cols;
		int new_cols = mat->rows;

		mat_trans = Matrix_create(new_rows, new_cols);
		if (mat_trans == NULL) {
			matrix_free(mat);
			return NULL;
		}

		mat_trans->rows = new_rows;
		mat_trans->cols = new_cols;

		for (int i = 0; i < new_rows; i++) {
			for (int j = 0; j < new_cols; j++) {
				mat_trans->data[i][j] = mat->data[j][i];
			}
		}

		matrix_free(mat);
	}

	return mat_trans;
}

void matrix_zero_init(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		for (int j = 0; j < mat->cols; j++) {
			mat->data[i][j] = 0;
		}
	}
	return;
}

void random_initialisation(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		for (int j = 0; j < mat->cols; j++) {
			mat->data[i][j] = (double)rand() / RAND_MAX;
		}
	}
	return;
}

void matrix_scalar_add(Matrix *mat, double num) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		for (int j = 0; j < mat->cols; j++) {
			mat->data[i][j] += num;
		}
	}
	return;
}

void matrix_scalar_mul(Matrix *mat, double num) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	for (int i = 0; i < mat->rows; i++) {
		for (int j = 0; j < mat->cols; j++) {
			mat->data[i][j] *= num;
		}
	}
	return;
}

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

Matrix *Matrix_create(int rows, int cols) {
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

// mat1 * mat2
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
