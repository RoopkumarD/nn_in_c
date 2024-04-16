typedef struct {
	int rows;
	int cols;
	int transpose;
	double **data;
} Matrix;

void matrix_zero_init(Matrix *mat);
void random_initialisation(Matrix *mat);

void matrix_scalar_add(Matrix *mat, double num);
void matrix_scalar_mul(Matrix *mat, double num);

void matrix_print(Matrix *mat);

Matrix *matrix_create(int rows, int cols);

void matrix_free(Matrix *mat);

void matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res);
