// used gaurds as importing utils.h in shallow.c
// states matrix typedef redefination
#ifndef MAT_H
#define MAT_H 1

/*
Rationale for using flexible array member is that, i only need to handle
one allocation rather than doing two allocation.
Also this seems fast compared to previous struct with *data when both compared
with time taken to do training in `shallow_train.c`
*/
typedef struct {
    int rows;
    int cols;
    int transpose;
    double data[];
} Matrix;

#define matrix_transpose(mat) (mat->transpose = (mat->transpose == 0) ? 1 : 0);

/*
Defining separate even though second one works with empty arguement because
having empty arguement is not a valid code. Still it works for my case, still
for the sake of not having problem later on, i will define both separately.
*/
#define SET_MATRIX_DIMENSIONS(mat)                                             \
    int rows, cols;                                                            \
    int mi, nj;                                                                \
    int m = mat->transpose;                                                    \
    if (m == 0) {                                                              \
        rows = mat->rows;                                                      \
        cols = mat->cols;                                                      \
        mi = cols;                                                             \
        nj = 1;                                                                \
    } else if (m == 1) {                                                       \
        rows = mat->cols;                                                      \
        cols = mat->rows;                                                      \
        mi = 1;                                                                \
        nj = rows;                                                             \
    }

#define SET_MATRIX_DIMENSIONS_WITH_NUM(mat, num)                               \
    int rows##num, cols##num;                                                  \
    int mi##num, nj##num;                                                      \
    int m##num = mat->transpose;                                               \
    if (m##num == 0) {                                                         \
        rows##num = mat->rows;                                                 \
        cols##num = mat->cols;                                                 \
        mi##num = cols##num;                                                   \
        nj##num = 1;                                                           \
    } else if (m##num == 1) {                                                  \
        rows##num = mat->cols;                                                 \
        cols##num = mat->rows;                                                 \
        mi##num = 1;                                                           \
        nj##num = rows##num;                                                   \
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
