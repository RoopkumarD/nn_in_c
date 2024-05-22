#include <float.h>
#include <math.h>
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

	SET_MATRIX_DIMENSIONS(mat, )

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			printf("%f\t", mat->data[i * mi + j * nj]);
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
	SET_MATRIX_DIMENSIONS(mat1, 1)

	// for mat2
	SET_MATRIX_DIMENSIONS(mat2, 2)

	// for mat3
	SET_MATRIX_DIMENSIONS(res, 3)

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
				res->data[i * mi3 + j * nj3] += 
					mat1->data[i * mi1 + k * nj1] * 
					mat2->data[k * mi2 + j * nj2];
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
		mat->data[i] = ((double)rand() / (RAND_MAX+1.0)) - 0.5;
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

/*
* c1 * mat1 + c2 * mat2 = mat3
* mat1 and mat2 are of same shape
*/
int matrix_add(
	Matrix *mat1,
	Matrix *mat2,
	Matrix *mat3,
	float mat1_coefficient,
	float mat2_coefficient
) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return 1;
	} else if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return 1;
	} else if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return 1;
	}

	// for mat1
	SET_MATRIX_DIMENSIONS(mat1, 1)

	// for mat2
	SET_MATRIX_DIMENSIONS(mat2, 2)

	// for mat3
	SET_MATRIX_DIMENSIONS(mat3, 3)

	if (
		((cols1 != cols2) || (cols1 != cols3)) || 
		((rows1 != rows2) || (rows1 != rows3))
	) {
		puts("Matrix shape are not equal");
		return 2;
	}

	for (int i = 0; i < rows1; i++) {
		for (int j = 0; j < cols1; j++) {
			mat3->data[i * mi3 + j * nj3] = 
					mat1_coefficient * mat1->data[i * mi1 + j * nj1] +
					mat2_coefficient * mat2->data[i * mi2 + j * nj2];
		}
	}

	return 0;
}

/*
* Note:
* c1 * mat1 + c2 * mat2 = mat3
* mat1 and mat2 cols value are same
* mat1->rows == 1 and mat2->rows == any
* mat3->rows == mat2->rows
*/
int matrix_row_add(
	Matrix *mat1,
	Matrix *mat2,
	Matrix *mat3,
	float mat1_coeff,
	float mat2_coeff
) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return 1;
	} else if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return 1;
	} else if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return 1;
	}

	// for mat1
	SET_MATRIX_DIMENSIONS(mat1, 1)

	// for mat2
	SET_MATRIX_DIMENSIONS(mat2, 2)

	// for mat3
	SET_MATRIX_DIMENSIONS(mat3, 3)

	if ((cols1 != cols2) || (cols1 != cols3)) {
		puts("Matrix column value are not equal");
		return 2;
	} else if (rows1 != 1) {
		puts("mat1 rows value should be 1");
		return 2;
	} else if (rows2 != rows3) {
		puts("mat2 and mat3 rows should be same");
		return 2;
	}

	for (int i = 0; i < rows2; i++) {
		for (int j = 0; j < cols2; j++) {
			mat3->data[i * mi3 + j * nj3] = 
				// because view of mat3 and mat2 are transposed
				// if mat1 is transpose we are caring about which element
				// in currently right now
					mat1_coeff * mat1->data[(i * cols1 + j) % cols1] +
					mat2_coeff * mat2->data[i * mi2 + j * nj2];
		}
	}

	return 0;
}

/*
* Note:
* c1 * mat1 + c2 * mat2 = mat3
* mat1 and mat2 rows value are same
* mat1->cols == 1 and mat2->cols == any
* mat3->cols == mat2->cols
*/
int matrix_col_add(
	Matrix *mat1,
	Matrix *mat2,
	Matrix *mat3,
	float mat1_coeff,
	float mat2_coeff
) {
	if (mat1 == NULL) {
		puts("Address to mat1 is NULL");
		return 1;
	} else if (mat2 == NULL) {
		puts("Address to mat2 is NULL");
		return 1;
	} else if (mat3 == NULL) {
		puts("Address to mat3 is NULL");
		return 1;
	}

	// for mat1
	SET_MATRIX_DIMENSIONS(mat1, 1)

	// for mat2
	SET_MATRIX_DIMENSIONS(mat2, 2)

	// for mat3
	SET_MATRIX_DIMENSIONS(mat3, 3)

	if ((rows1 != rows2) || (rows1 != rows3)) {
		puts("Matrix rows value are not equal");
		return 2;
	} else if (cols1 != 1) {
		puts("mat1 cols value should be 1");
		return 2;
	} else if (cols2 != cols3) {
		puts("mat2 and mat3 cols should be same");
		return 2;
	}

	/*
	int total_elem = mat2->rows * mat2->cols;
	for (int i = 0; i < total_elem; i++) {
		mat3->data[i] = mat1->data[i / cols2] + mat2->data[i];
	}
	*/

	for (int i = 0; i < rows2; i++) {
		for (int j = 0; j < cols2; j++) {
			mat3->data[i * mi3 + j * nj3] = 
					mat1_coeff * mat1->data[(i * cols2 + j) / cols2] +
					mat2_coeff * mat2->data[i * mi2 + j * nj2];
		}
	}

	return 0;
}


/*
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
*/

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

	// for mat1
	SET_MATRIX_DIMENSIONS(mat1, 1)

	// for mat2
	SET_MATRIX_DIMENSIONS(mat2, 2)

	// for mat3
	SET_MATRIX_DIMENSIONS(mat3, 3)

	if (
		((cols1 != cols2) || (cols1 != cols3)) || 
		((rows1 != rows2) || (rows1 != rows3))
	) {
		puts("Matrix shape are not equal");
		return;
	}

	int total_elem = mat1->rows * mat1->cols;
	for (int i = 0; i < total_elem; i++) {
		mat3->data[i] = mat1->data[i] * mat2->data[i];
	}

	return;
}

int matrix_dump_csv(Matrix *mat, char *filename) {
	if (mat == NULL) {
		puts("Mat address is NULL");
		return 1;
	}

	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open %s\n", filename);
		return 2;
	}

	SET_MATRIX_DIMENSIONS(mat, )

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (j < cols - 1) {
				if (isnan(mat->data[i * mi + j * nj])) {
					fprintf(fp, ",");
				} else {
					fprintf(fp, "%.*f,", DBL_DIG, mat->data[i * mi + j * nj]);
				}
			} else {
				// if NAN don't print anything to file
				if (isnan(mat->data[i * mi + j * nj]) == 0) {
					fprintf(fp, "%.*f", DBL_DIG, mat->data[i * mi + j * nj]);
				}
			}
		}
		fprintf(fp, "\n");
	}

	if (ferror(fp)) {
		fprintf(stderr, "Err writing file: %s", filename);
		fclose(fp);
		return 3;
	}

	fclose(fp);

	return 0;
}
