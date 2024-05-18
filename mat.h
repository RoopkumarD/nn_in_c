// used gaurds as importing utils.h in shallow.c
// states matrix typedef redefination
#ifndef MAT_H
#define MAT_H

typedef struct {
	int rows;
	int cols;
	int transpose;
	double *data;
} Matrix;

Matrix *Matrix_create(int rows, int cols);

void matrix_free(Matrix *mat);

void matrix_print(Matrix *mat);

// mat1 * mat2 = res
int matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res);

void matrix_scalar_mul(Matrix *mat, double num);

void matrix_scalar_add(Matrix *mat, double num);

void random_initialisation(Matrix *mat);

void matrix_zero_init(Matrix *mat);

// copying mat1 into mat2
void matrix_copy(Matrix *mat1, Matrix *mat2);

void matrix_add(Matrix *mat1, Matrix *mat2, Matrix *mat3);
void matrix_multiply(Matrix *mat1, Matrix *mat2, Matrix *mat3);

void matrix_transpose(Matrix *mat);

int matrix_dump_csv(Matrix *mat, char *filename);

#endif // !MAT_H
