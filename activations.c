#include "activations.h"
#include "err_helper.h"
#include <math.h>
#include <stdio.h>

/*
DO NOTE THAT I AM NOT CHECKING NULL HERE, CAUSE THESE FUNCTION
DOESN'T ASSIGN STUFF, SO TO REMOVE REDUNDANT CHECKING. I HAVE
CHECKED IF MAT IS NULL OR NOT AT TOP WHENEVER I AM USING THESE
FUNCTIONS
*/

// sigmoid(mat1) = mat2
int sigmoid(Matrix *mat1, Matrix *mat2) {
  // checking if both matrix size are same or not
  if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "mat1: col: %d, row: %d\n", mat1->cols, mat1->rows);
    fprintf(stderr, "mat2: col: %d, row: %d\n", mat2->cols, mat2->rows);
    fprintf(stderr, "%s: Matrix shape are not equal\n", __func__);
    return 1;
  }

  int total_elem = mat1->rows * mat1->cols;
  for (int i = 0; i < total_elem; i++) {
    double denominator = exp(-1 * mat1->data[i]) + 1;
    mat2->data[i] = 1 / denominator;
  }

  return 0;
}

// sigmoid_prime(mat1) = mat2
// mat1 should be sigmoid(mat) = mat1
// to calculate sigmoid_prime of mat
int sigmoid_prime(Matrix *mat1, Matrix *mat2) {
  // checking if both matrix size are same or not
  if ((mat1->cols != mat2->cols) || (mat1->rows != mat2->rows)) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "mat1: col: %d, row: %d\n", mat1->cols, mat1->rows);
    fprintf(stderr, "mat2: col: %d, row: %d\n", mat2->cols, mat2->rows);
    fprintf(stderr, "%s: Matrix shape are not equal\n", __func__);
    return 1;
  }

  int total_elem = mat1->rows * mat1->cols;
  for (int i = 0; i < total_elem; i++) {
    double s = mat1->data[i];
    mat2->data[i] = s * (1 - s);
  }

  return 0;
}
