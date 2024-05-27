// used gaurds as importing utils.h in shallow.c
// states matrix typedef redefination
#ifndef MAT_H
#define MAT_H 1

typedef struct {
  int rows;
  int cols;
  int transpose;
  double *data;
} Matrix;

#define matrix_transpose(mat) (mat->transpose = (mat->transpose == 0) ? 1 : 0);

// here to set rows -> send SET_MATRIX_DIMENSIONS(mat, )
// thus it will concaenate rows + nothing = rows
// if send SET_MATRIX_DIMENSIONS(mat, "")
// then it will concaenate rows + "" = rows""
// if send SET_MATRIX_DIMENSIONS(mat, "1")
// then it will concaenate rows + "1" = rows"1"
#define SET_MATRIX_DIMENSIONS(mat, num)                                        \
  int rows##num, cols##num;                                                    \
  int mi##num, nj##num;                                                        \
  int m##num = mat->transpose;                                                 \
  if (m##num == 0) {                                                           \
    rows##num = mat->rows;                                                     \
    cols##num = mat->cols;                                                     \
    mi##num = cols##num;                                                       \
    nj##num = 1;                                                               \
  } else if (m##num == 1) {                                                    \
    rows##num = mat->cols;                                                     \
    cols##num = mat->rows;                                                     \
    mi##num = 1;                                                               \
    nj##num = rows##num;                                                       \
  }

Matrix *Matrix_create(int rows, int cols);

void matrix_free(Matrix *mat);

int matrix_print(Matrix *mat);

// mat1 * mat2 = res
int matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res);

int matrix_scalar_mul(Matrix *mat, double num);

int matrix_scalar_add(Matrix *mat, double num);

int random_initialisation(Matrix *mat);

int matrix_zero_init(Matrix *mat);

// copying mat1 into mat2
int matrix_copy(Matrix *mat1, Matrix *mat2);

int matrix_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coefficient,
               float mat2_coefficient);
int matrix_row_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coeff,
                   float mat2_coeff);
int matrix_col_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coeff,
                   float mat2_coeff);

int matrix_multiply(Matrix *mat1, Matrix *mat2, Matrix *mat3);

#endif // !MAT_H
