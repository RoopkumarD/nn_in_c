#include "activations.h"
#include "err_helper.h"
#include "mat/mat.h"
#include "mat/mat_io.h"
#include "shallow_nn_cost.h"
#include "shallow_nn_train.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void feed_forward(Matrix *train_x, Matrix *Y_mat, int training_length,
                  Matrix *wb[], int num_layer);

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define NUM_ROWS(array_2d) ARRAY_LEN(array_2d)
#define NUM_COLS(array_2d) ARRAY_LEN(array_2d[0])

int layers[] = {0, 2, 1};
int num_layers = sizeof(layers) / sizeof(int);

int main(void) {
  srand(42);

  int retval = 0;
  int failure = 0;
  int compute_layers = num_layers - 1;

  Matrix *X_mat = NULL;
  Matrix *Y_mat = NULL;

  Matrix *X_test = NULL;
  Matrix *Y_test = NULL;

  Matrix *wb[num_layers - 1];
  // so that later on if below reading csv failed
  // then it can gracefully free pointer which points
  // to NULL without throwing SIGARBT for freeing
  // corrupt memory -> because freeing some memory which
  // isn't there and it is not NULL, thus free throwing err
  for (int i = 0; i < compute_layers; i++) {
    wb[i] = NULL;
  }

  puts("Reading CSV");
  /*
  failure = read_csv("./csvs/mnist_train.csv", "0", &X_mat, &Y_mat, EXTRA_ONE);
  if (failure != 0) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to read CSV!!!\n");
    retval = failure;
    goto cleanup;
  }

  failure = store_mat_bin("./csvs/mnist_train_x.bin", X_mat);
  if (failure != 0) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to store CSV in bin\n");
    retval = failure;
    goto cleanup;
  }

  failure = store_mat_bin("./csvs/mnist_train_y.bin", Y_mat);
  if (failure != 0) {
  LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to store Y in bin\n");
    retval = failure;
    goto cleanup;
  }
  */

  // doing these so to not parse csv again and again
  failure = read_mat_bin("./csvs/mnist_train_x.bin", &X_mat);
  if (failure != 0) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to read CSV X in bin\n");
    retval = failure;
    goto cleanup;
  }

  failure = read_mat_bin("./csvs/mnist_train_y.bin", &Y_mat);
  if (failure != 0) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to read CSV Y in bin\n");
    retval = failure;
    goto cleanup;
  }

  /*
  failure = read_csv("./csvs/mnist_test.csv", "0", &X_test, &Y_test, EXTRA_ONE);
  if (failure != 0) {
    LINE_FILE_PRINT();
    fprint(stderr, "Failed to read test CSV!!!\n");
    retval = failure;
    goto cleanup;
  }
  */

  int training_length = X_mat->rows;
  layers[0] = X_mat->cols - EXTRA_ONE;

  for (int i = 0; i < compute_layers; i++) {
    // cols +1 as storing bias there
    Matrix *weights_bias = Matrix_create(layers[i + 1], layers[i] + EXTRA_ONE);
    if (weights_bias == NULL) {
      LINE_FILE_PRINT(2);
      fprintf(stderr, "Failed to create weight matrix\n");
      retval = 1;
      goto cleanup;
    }
    random_initialisation(weights_bias);
    wb[i] = weights_bias;
  }

  // now transposing X_mat
  matrix_transpose(X_mat);
  // matrix_transpose(X_test);

  // training the model
  puts("Training the Model");
  failure = stochastic_gd(X_mat, Y_mat, num_layers, wb, layers, training_length,
                          10000);
  if (failure != 0) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to Train!!!\n");
    goto cleanup;
  }

  /*
  // Then feed forward to check the result
  feed_forward(X_test, Y_test, training_length, wb, num_layers);
  */

cleanup:
  // freeing all the memory asked
  matrix_free(X_mat);
  matrix_free(Y_mat);
  for (int i = 0; i < compute_layers; i++) {
    matrix_free(wb[i]);
  }

  return retval;
}

void feed_forward(Matrix *train_x, Matrix *Y_mat, int training_length,
                  Matrix *wb[], int num_layer) {
  if (train_x == NULL) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "train_x == NULL\n");
    goto cleanup;
  }

  int highest_node_num = 0;
  for (int m = 0; m < num_layers; m++) {
    if (highest_node_num < layers[m]) {
      highest_node_num = layers[m];
    }
  }
  Matrix *zs = Matrix_create(highest_node_num, training_length);
  if (zs == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to create zs matrix for feedforward\n");
    goto cleanup;
  }
  Matrix *activations =
      Matrix_create(highest_node_num + EXTRA_ONE, training_length);
  if (activations == NULL) {
    LINE_FILE_PRINT(2);
    fprintf(stderr, "Failed to create activations matrix for feedforward\n");
    goto cleanup;
  }

  // setting activation to train_x
  // need to worry about transpose
  SET_MATRIX_DIMENSIONS(train_x);

  if (activations->cols != cols) {
    LINE_FILE_PRINT(1);
    fprintf(stderr, "Bro send data in (input row) * (obs cols) format\n");
    goto cleanup;
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      activations->data[i * cols + j] = train_x->data[i * mi + j * nj];
    }
  }
  activations->rows = rows;

  // don't have to do this as i am sending already attached 1 row train_x
  /*
  int start_posn = (activations->rows-1) * activations->cols;
  int end_posn = start_posn + activations->cols;
  for (int m = start_posn; m < end_posn; m++) {
          activations->data[m] = 1;
  }
  */

  for (int j = 0; j < num_layer - 1; j++) {
    // val = sigmoid(np.dot(W, x) + b)

    zs->rows = layers[j + 1];
    int failure = matrix_mul(wb[j], activations, zs);
    if (failure == 1) {
      LINE_FILE_PRINT(2);
      fprintf(stderr, "Wasn't able to mat_mul wb[%d] and activations\n", j);
      goto cleanup;
    }
    activations->rows = zs->rows;
    // z = sigmoid(z)
    failure = sigmoid(zs, activations);
    if (failure == 1) {
      LINE_FILE_PRINT(2);
      fprintf(stderr, "Wasn't able to get sigmoid\n");
      goto cleanup;
    }
    activations->rows += EXTRA_ONE;

    if (j != num_layers - 2) {
      // for last activations there is no need to waste
      // setting value equal to 1
      int start_posn = (activations->rows - 1) * activations->cols;
      int end_posn = start_posn + activations->cols;
      for (int m = start_posn; m < end_posn; m++) {
        activations->data[m] = 1;
      }
    }
  }

  activations->rows -= 1;
  puts("Result of Feed Forward is:");
  double cost_value = sqr_cost_function(activations, Y_mat, training_length);
  printf("Total Cost: %f\n", cost_value);

cleanup:
  matrix_free(activations);
  matrix_free(zs); // double free err in zs

  return;
}
