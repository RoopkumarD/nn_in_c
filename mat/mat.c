#include "mat.h"
#include "../err_helper.h"
#include <stdio.h>
#include <stdlib.h>

/*
NOTE: HERE, I AM NOT CHECKING IF MATRIX POINTER IS NULL
OR NOT IN EACH FUNCTION WHICH ARE TAKING MATRIX AS POINTER TO IT.
BECAUSE WHERE EVER I AM USING THOSE FUNCTION, I AM CHECKING THEM
AT ONCE AHEAD SO TO REDUCE REDUNDANT INSTRUCTIONS.
SINCE THESE FUNCTION ARE DEALING WITH DATA FIELD OF MATRIX AND NOT
ASSIGNING OR RETURNING MATRIX POINTER. I CAN SAFELY NOT CHECK
NULL POINTER AGAIN AS THERE IS NO ASSIGNMENT.

DO ALWAYS CHECK IF MATRIX DEFINED IS NULL OR NOT, OTHERWISE SEG FAULT.
*/

Matrix *Matrix_create(int rows, int cols) {
  Matrix *mat = (Matrix *)malloc(sizeof(Matrix));
  if (mat == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "%s: Memory allocation for matrix failed\n", __func__);
    return NULL;
  }

  mat->rows = rows;
  mat->cols = cols;
  mat->transpose = 0;
  mat->data = (double *)malloc((rows * cols) * sizeof(double));
  if (mat->data == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "%s: Memory allocation failed for mat->data\n", __func__);
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

int matrix_print(Matrix *mat) {
  SET_MATRIX_DIMENSIONS(mat);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%f\t", mat->data[i * mi + j * nj]);
    }
    printf("\n");
  }

  return 0;
}

// mat1 * mat2 = res
int matrix_mul(Matrix *mat1, Matrix *mat2, Matrix *res) {
  // for mat1
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat1, 1);

  // for mat2
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat2, 2);

  // for mat3
  SET_MATRIX_DIMENSIONS_WITH_NUM(res, 3);

  if (cols1 != rows2) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: Invalid dimension: cols1 != rows2 -> %d != %d\n",
            __func__, cols1, rows2);
    return 1;
  } else if ((res->rows != rows1) || (res->cols != cols2)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: res dimension should be: %d %d but got %d %d\n",
            __func__, rows1, cols2, res->rows, res->cols);
    return 1;
  }

  for (int i = 0; i < rows1; i++) {
    for (int j = 0; j < cols2; j++) {
      res->data[i * cols2 + j] = 0;
      for (int k = 0; k < cols1; k++) {
        res->data[i * mi3 + j * nj3] +=
            mat1->data[i * mi1 + k * nj1] * mat2->data[k * mi2 + j * nj2];
        // res->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
      }
    }
  }

  return 0;
}

int matrix_scalar_mul(Matrix *mat, double num) {
  int total_elem = mat->rows * mat->cols;
  for (int i = 0; i < total_elem; i++) {
    mat->data[i] *= num;
  }

  return 0;
}

int matrix_scalar_add(Matrix *mat, double num) {
  int total_elem = mat->rows * mat->cols;
  for (int i = 0; i < total_elem; i++) {
    mat->data[i] += num;
  }

  return 0;
}

int random_initialisation(Matrix *mat) {
  int total_elem = mat->rows * mat->cols;
  for (int i = 0; i < total_elem; i++) {
    mat->data[i] = ((double)rand() / (RAND_MAX + 1.0)) - 0.5;
  }

  return 0;
}

int matrix_zero_init(Matrix *mat) {
  int total_elem = mat->rows * mat->cols;
  for (int i = 0; i < total_elem; i++) {
    mat->data[i] = 0;
  }

  return 0;
}

// copying mat1 into mat2
int matrix_copy(Matrix *mat1, Matrix *mat2) {
  // checking if both matrix size are same or not
  if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: mat1 cols: %d, rows: %d\n", __func__, mat1->cols,
            mat1->rows);
    fprintf(stderr, "%s: mat2 cols: %d, rows: %d\n", __func__, mat2->cols,
            mat2->rows);
    fprintf(stderr, "%s: Matrix shape are not equal\n", __func__);
    return 2;
  }

  mat2->transpose = mat1->transpose;

  int total_elem = mat1->rows * mat1->cols;
  for (int i = 0; i < total_elem; i++) {
    mat2->data[i] = mat1->data[i];
  }

  return 0;
}

/*
 * c1 * mat1 + c2 * mat2 = mat3
 * mat1 and mat2 are of same shape
 */
int matrix_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coefficient,
               float mat2_coefficient) {
  // for mat1
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat1, 1);

  // for mat2
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat2, 2);

  // for mat3
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat3, 3);

  if (((cols1 != cols2) || (cols1 != cols3)) ||
      ((rows1 != rows2) || (rows1 != rows3))) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "%s: mat1 cols: %d, rows: %d\n", __func__, cols1, rows1);
    fprintf(stderr, "%s: mat2 cols: %d, rows: %d\n", __func__, cols2, rows2);
    fprintf(stderr, "%s: mat3 cols: %d, rows: %d\n", __func__, cols3, rows3);
    fprintf(stderr, "%s: Matrix shape are not equal\n", __func__);
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
int matrix_row_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coeff,
                   float mat2_coeff) {
  // for mat1
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat1, 1);

  // for mat2
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat2, 2);

  // for mat3
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat3, 3);

  if ((cols1 != cols2) || (cols1 != cols3)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: cols1: %d, cols2: %d, cols3: %d\n", __func__, cols1,
            cols2, cols3);
    fprintf(stderr, "%s: Matrix column value are not equal\n", __func__);
    return 2;
  } else if (rows1 != 1) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: mat1 rows value should be 1\n", __func__);
    return 2;
  } else if (rows2 != rows3) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: row2: %d, row3: %d\n", __func__, rows2, rows3);
    fprintf(stderr, "%s: mat2 and mat3 rows should be same\n", __func__);
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
int matrix_col_add(Matrix *mat1, Matrix *mat2, Matrix *mat3, float mat1_coeff,
                   float mat2_coeff) {
  // for mat1
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat1, 1);

  // for mat2
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat2, 2);

  // for mat3
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat3, 3);

  if ((rows1 != rows2) || (rows1 != rows3)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: rows1: %d, rows2: %d, rows3: %d\n", __func__, rows1,
            rows2, rows3);
    fprintf(stderr, "%s: Matrix rows value are not equal\n", __func__);
    return 2;
  } else if (cols1 != 1) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: mat1 cols value should be 1\n", __func__);
    return 2;
  } else if (cols2 != cols3) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "%s: cols2: %d, cols3: %d\n", __func__, cols2, cols3);
    fprintf(stderr, "%s: mat2 and mat3 cols should be same\n", __func__);
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

// mat1 * mat2 = mat3
int matrix_multiply(Matrix *mat1, Matrix *mat2, Matrix *mat3) {
  // for mat1
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat1, 1);

  // for mat2
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat2, 2);

  // for mat3
  SET_MATRIX_DIMENSIONS_WITH_NUM(mat3, 3);

  if (((cols1 != cols2) || (cols1 != cols3)) ||
      ((rows1 != rows2) || (rows1 != rows3))) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "%s: mat1 cols: %d, rows: %d\n", __func__, cols1, rows1);
    fprintf(stderr, "%s: mat2 cols: %d, rows: %d\n", __func__, cols2, rows2);
    fprintf(stderr, "%s: mat3 cols: %d, rows: %d\n", __func__, cols3, rows3);
    fprintf(stderr, "%s: Matrix shape are not equal\n", __func__);
    return 2;
  }

  int total_elem = mat1->rows * mat1->cols;
  for (int i = 0; i < total_elem; i++) {
    mat3->data[i] = mat1->data[i] * mat2->data[i];
  }

  return 0;
}
