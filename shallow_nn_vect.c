#include "mat/mat.h"
#include "mat/mat_io.h"
#include "shallow_nn_train.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void feed_forward(Matrix *train_x, int training_length, Matrix *wb[],
                  int num_layer);

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define NUM_ROWS(array_2d) ARRAY_LEN(array_2d)
#define NUM_COLS(array_2d) ARRAY_LEN(array_2d[0])

int layers[] = {2, 2, 1};
int num_layers = sizeof(layers) / sizeof(int);

int main(void) {
  srand(time(NULL));

  int retval = 0;

  Matrix *X_mat = NULL;
  Matrix *Y_mat = NULL;
  Matrix *wb[num_layers - 1];
  int compute_layers = num_layers - 1;

  for (int i = 0; i < compute_layers; i++) {
    // cols +1 as storing bias there
    Matrix *weights_bias = Matrix_create(layers[i + 1], layers[i] + 1);
    if (weights_bias == NULL) {
      puts("Failed to create weight matrix");
      retval = 1;
      goto cleanup;
    }
    random_initialisation(weights_bias);
    wb[i] = weights_bias;
  }

  int failure =
      read_csv("./csvs/test.csv", "Result", &X_mat, &Y_mat, EXTRA_ONE);
  if (failure != 0) {
    puts("Failed to read CSV!!!");
    retval = failure;
    goto cleanup;
  }

  int training_length = X_mat->rows;

  // now transposing X_mat and Y_mat
  matrix_transpose(X_mat);

  // training the model
  failure =
      gradient_descent(X_mat, Y_mat, num_layers, wb, layers, training_length);
  if (failure != 0) {
    puts("Failed to Train!!!");
    goto cleanup;
  }

  // Then feed forward to check the result
  feed_forward(X_mat, training_length, wb, num_layers);

cleanup:
  // freeing all the memory asked
  matrix_free(X_mat);
  matrix_free(Y_mat);
  for (int i = 0; i < compute_layers; i++) {
    matrix_free(wb[i]);
  }

  return retval;
}

void feed_forward(Matrix *train_x, int training_length, Matrix *wb[],
                  int num_layer) {
  if (train_x == NULL) {
    puts("train_x is NULL");
    goto cleanup;
  }

  int highest_node_num = 0;
  for (int m = 1; m < num_layers; m++) {
    if (highest_node_num < layers[m]) {
      highest_node_num = layers[m];
    }
  }
  Matrix *zs = Matrix_create(highest_node_num + EXTRA_ONE, training_length);
  if (zs == NULL) {
    puts("Failed to create zs matrix for feedforward");
    goto cleanup;
  }
  Matrix *activations =
      Matrix_create(highest_node_num + EXTRA_ONE, training_length);
  if (activations == NULL) {
    puts("Failed to create activations matrix for feedforward");
    goto cleanup;
  }

  // setting activation to train_x
  // need to worry about transpose
  SET_MATRIX_DIMENSIONS(train_x, )

  if (activations->cols != cols) {
    puts("Bro send data in (input row) * (obs cols) format");
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
      printf("Wasn't able to mat_mul wb[%d] and activations\n", j);
      printf("Location: Feedforward function loop\n");
      goto cleanup;
    }
    activations->rows = zs->rows;
    // z = sigmoid(z)
    failure = sigmoid(zs, activations);
    if (failure == 1) {
      puts("Wasn't able to get sigmoid");
      puts("Location: In sigmoid of feedforward function loop");
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
  matrix_print(activations);

cleanup:
  matrix_free(activations);
  matrix_free(zs);

  return;
}
