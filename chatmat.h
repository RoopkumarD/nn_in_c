typedef struct {
	int rows;
	int cols;
	double **data;
} Matrix;

Matrix *matrix_create(int rows, int cols);

void matrix_free(Matrix *mat);

void matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res);
