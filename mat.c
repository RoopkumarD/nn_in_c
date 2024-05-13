#include <stdio.h>
#include <stdlib.h>
#include "mat.h"

Matrix *Matrix_create(int rows, int cols) {
	Matrix *mat = (Matrix *)malloc(sizeof(Matrix));
	if (mat == NULL) {
		puts("Memory allocation for matrix failed");
		return NULL;
	}

	mat->rows = rows;
	mat->cols = cols;
	mat->transpose = 0;
	mat->data = (double *)malloc((rows * cols) * sizeof(double));
	if (mat->data == NULL) {
		puts("Memory allocation failed for mat->data");
		free(mat);
		return NULL;
	}

	return mat;
}

void matrix_free(Matrix *mat) {
	if (mat == NULL) {
		return;
	}

	free(mat->data);
	free(mat);

	return;
}

void matrix_print(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	int rows, cols;
	int m = mat->transpose;
	if (m == 0) {
		rows = mat->rows;
		cols = mat->cols;
	} else if (m == 1) {
		rows = mat->cols;
		cols = mat->rows;
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%f\t", mat->data[(1-m)*(i * cols + j) + m * (i + j * rows)]);
		}
		printf("\n");
	}

	return;
}

// mat1 * mat2 = res
int matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res) {
	if ((mat1 == NULL) || (mat2 == NULL) || (res == NULL)) {
		puts("Address to one of the mat given is NULL");
		return 1;
	}

	// for mat1
	int rows1, cols1;
	int m1 = mat1->transpose;
	if (m1 == 0) {
		rows1 = mat1->rows;
		cols1 = mat1->cols;
	} else if (m1 == 1) {
		rows1 = mat1->cols;
		cols1 = mat1->rows;
	}

	// for mat2
	int rows2, cols2;
	int m2 = mat2->transpose;
	if (m2 == 0) {
		rows2 = mat2->rows;
		cols2 = mat2->cols;
	} else if (m2 == 1) {
		rows2 = mat2->cols;
		cols2 = mat2->rows;
	}

	if (cols1 != rows2) {
		printf("Invalid dimension: cols1 != rows2 -> %d != %d\n", cols1, rows2);
		return 1;
	} else if ((res->rows != rows1) || (res->cols != cols2)) {
		printf("res dimension should be: %d %d but got %d %d\n", rows1, cols2, res->rows, res->cols);
		return 1;
	}

	for (int i = 0; i < rows1; i++) {
		for (int j = 0; j < cols2; j++) {
			res->data[i * cols2 + j] = 0;
			for (int k = 0; k < cols1; k++) {
				res->data[i * cols2 + j] += 
					mat1->data[(1-m1) * (i * cols1 + k) + m1 * (i + k * rows1)] * 
					mat2->data[(1-m2) * (k * cols2 + j) + m2 * (k + j * rows2)];
				//res->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
			}
		}
	}

	return 0;
}

void matrix_scalar_mul(Matrix *mat, double num) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	int total_elem = mat->rows * mat->cols;
	for (int i = 0; i < total_elem; i++) {
		mat->data[i] *= num;
	}

	return;
}

void matrix_scalar_add(Matrix *mat, double num) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	int total_elem = mat->rows * mat->cols;
	for (int i = 0; i < total_elem; i++) {
		mat->data[i] += num;
	}

	return;
}

void random_initialisation(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	int total_elem = mat->rows * mat->cols;
	for (int i = 0; i < total_elem; i++) {
		mat->data[i] = (double)rand() / RAND_MAX;
	}

	return;
}

void matrix_zero_init(Matrix *mat) {
	if (mat == NULL) {
		puts("Address to matrix is NULL");
		return;
	}

	int total_elem = mat->rows * mat->cols;
	for (int i = 0; i < total_elem; i++) {
		mat->data[i] = 0;
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

	mat2->transpose = mat1->transpose;

	int total_elem = mat1->rows * mat1->cols;
	for (int i = 0; i < total_elem; i++) {
		mat2->data[i] = mat1->data[i];
	}

	return;
}

// mat1 + mat2 = mat3
void matrix_add(Matrix *mat1, Matrix *mat2, Matrix *mat3) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return;
	} else if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return;
	} else if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return;
	}

	if (
		((mat1->cols == mat2->cols) && (mat1->cols == mat3->cols)) && 
		((mat1->rows == mat2->rows) && (mat1->rows == mat3->rows))
	) {
		int total_elem = mat1->rows * mat1->cols;
		for (int i = 0; i < total_elem; i++) {
			mat3->data[i] = mat1->data[i] + mat2->data[i];
		}
	}
	else if ((mat1->cols == mat2->cols) && (mat1->cols == mat3->cols)) {
		// now checking which one has row = 1 and other row has same
		// number as that of mat3
		if ((mat1->rows == 1) && (mat2->rows == mat3->rows)) {
			int total_elem = mat2->rows * mat2->cols;
			for (int i = 0; i < total_elem; i++) {
				mat3->data[i] = mat1->data[i % mat1->cols] + mat2->data[i];
			}
		} else if ((mat2->rows == 1) && (mat1->rows == mat3->rows)) {
			int total_elem = mat1->rows * mat1->cols;
			for (int i = 0; i < total_elem; i++) {
				mat3->data[i] = mat1->data[i] + mat2->data[i % mat2->cols];
			}
		} else {
			puts("Matrix shape are not equal");
		}
	}
	else if ((mat1->rows == mat2->rows) && (mat1->rows == mat3->rows)) {
		// now checking which one has cols = 1 and other cols has same
		// number as that of mat3
		if ((mat1->cols == 1) && (mat2->cols == mat3->cols)) {
			int total_elem = mat2->rows * mat2->cols;
			for (int i = 0; i < total_elem; i++) {
				mat3->data[i] = mat1->data[i / mat2->cols] + mat2->data[i];
			}
		} else if ((mat2->cols == 1) && (mat1->cols == mat3->cols)) {
			int total_elem = mat1->rows * mat1->cols;
			for (int i = 0; i < total_elem; i++) {
				mat3->data[i] = mat1->data[i] + mat2->data[i / mat1->cols];
			}
		} else {
			puts("Matrix shape are not equal");
		}
	}
	else {
		puts("Matrix shape are not equal");
	}

	return;
}

// mat1 * mat2 = mat3
void matrix_multiply(Matrix *mat1, Matrix *mat2, Matrix *mat3) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return;
	} else if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return;
	} else if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return;
	}

	if (
		((mat1->cols == mat2->cols) && (mat1->cols == mat3->cols)) && 
		((mat1->rows == mat2->rows) && (mat1->rows == mat3->rows))
	) {
		int total_elem = mat1->rows * mat1->cols;
		for (int i = 0; i < total_elem; i++) {
			mat3->data[i] = mat1->data[i] * mat2->data[i];
		}
	} else {
		puts("Matrix shape are not equal");
	}

	return;
}

void matrix_transpose(Matrix *mat) {
	if (mat == NULL) {
		puts("Mat address is NULL");
		return;
	}

	if (mat->transpose == 0) {
		mat->transpose = 1;
	} else {
		mat->transpose = 0;
	}

	return;
}
