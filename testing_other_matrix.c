#include <stdlib.h>
#include "mat.h"

double arr1[3][3] = {
	{1,2,3},
	{4,5,6},
	{7,8,9},
};

double arr2[3][1] = {
	{10},
	{20},
	{30},
};

int main(void) {
	Matrix *mat1 = Matrix_create(3, 3);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mat1->data[i * 3 + j] = arr1[i][j];
		}
	}

	Matrix *mat2 = Matrix_create(3, 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 1; j++) {
			mat2->data[i] = arr2[i][j];
		}
	}

	Matrix *mat3 = Matrix_create(3, 1);

	matrix_mul(mat1, mat2, mat3);

	matrix_print(mat3);

	matrix_free(mat1);
	matrix_free(mat2);
	matrix_free(mat3);

	return 0;
}
