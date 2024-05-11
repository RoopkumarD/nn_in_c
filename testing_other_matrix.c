#include <stdlib.h>
#include <stdio.h>

typedef struct {
	int rows;
	int cols;
	int transpose;
	double *data;
} Matrix;

void print_matrix(Matrix *mat);

int main(void) {
	Matrix *mat = (Matrix *)malloc(sizeof(Matrix));
	if (mat == NULL) {
		puts("Memory allocation for matrix failed");
		return 1;
	}

	mat->rows = 3;
	mat->cols = 4;
	mat->transpose = 0;
	mat->data = (double *)malloc((mat->rows * mat->cols) * sizeof(double));
	if (mat->data == NULL) {
		puts("Memory allocation for matrix data failed");
		free(mat);
		return 1;
	}

	int final_pos = mat->rows * mat->cols + 1;
	for (int i = 1; i < final_pos; i++) {
		mat->data[i-1] = i;
	}

	printf("Printing the matrix without transpose\n");
	print_matrix(mat);

	printf("Printing the matrix with transpose\n");
	mat->transpose = 1;
	print_matrix(mat);

	free(mat->data);
	free(mat);

	return 0;
}

void print_matrix(Matrix *mat) {
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
			printf("%f\t", mat->data[(1-m) * (i * cols + j) + m * (i + j * rows)]);
		}
		printf("\n");
	}

	return;
}
