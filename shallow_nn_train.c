#include "shallow_nn_train.h"
#include "activations.h"
#include "shallow_nn_cost.h"
#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
Explanation of functions at `shallow_nn_train.md`
*/

int stochastic_gd(Matrix *X_train, Matrix *Y_train, int num_layers,
                  Matrix *wb[num_layers - 1], int layers[num_layers],
                  int training_length, int batch_length) {
  assert(batch_length > 1 && batch_length <= training_length);
  int retval = 0;
  unsigned int arr[training_length];
  Matrix *activations[num_layers];
  // here, i am getting total batches number as training_length / batch_length
  // + 1 -> as last batch will contain remainder of training cases which in
  // total is not equal to training_length.
  int batch_num = ceil((double)training_length / batch_length);
  int remainder_batch_len = training_length % batch_length;
  int temp_batch_length = batch_length;
  int compute_layers = num_layers - 1;

  Matrix *y_cost = Matrix_create(1, batch_length);
  if (y_cost == NULL) {
    puts("Failed to create y_cost matrix");
    retval = 1;
    goto cleanup;
  }

  for (int i = 0; i < num_layers; i++) {
    activations[i] = Matrix_create(layers[i] + EXTRA_ONE, batch_length);
    if (activations[i] == NULL) {
      puts("Failed to create activation matrix");
      retval = 1;
      goto cleanup;
    }
  }

  // zs for temp storage while matrix multiplication
  int highest_node_num = 0;
  for (int m = 0; m < num_layers; m++) {
    if (highest_node_num < layers[m]) {
      highest_node_num = layers[m];
    }
  }
  Matrix *zs = Matrix_create(highest_node_num, batch_length);
  if (zs == NULL) {
    puts("Failed to create zs matrix");
    retval = 1;
    goto cleanup;
  }
  Matrix *delta = Matrix_create(highest_node_num, batch_length);
  if (delta == NULL) {
    puts("Failed to create delta matrix");
    retval = 1;
    goto cleanup;
  }
  Matrix *delta_mul = Matrix_create(highest_node_num, batch_length);
  if (delta_mul == NULL) {
    puts("Failed to create delta_mul matrix");
    retval = 1;
    goto cleanup;
  }
  int max_wb_rows = 0, max_wb_cols = 0;
  for (int z = 0; z < compute_layers; z++) {
    if (wb[z]->rows > max_wb_rows) {
      max_wb_rows = wb[z]->rows;
    }
    if (wb[z]->cols > max_wb_cols) {
      max_wb_cols = wb[z]->cols;
    }
  }
  Matrix *delta_wb = Matrix_create(max_wb_rows, max_wb_cols);
  if (delta_wb == NULL) {
    puts("Failed to create delta_weights matrix");
    retval = 1;
    goto cleanup;
  }

  // checking for if any matrix defined above is
  // NULL or not as this way, i don't have to check in any
  // below matrix function. This makes sense as below i am
  // not assigning the result to anything just touching
  // data field of Matrix, just there can't be NULL in future
  // for above
  // Since most of above are checked if they are NULL or not
  // let's check for X_mat
  if (X_train == NULL) {
    fprintf(stderr, "%s: X_train is NULL\n", __func__);
    retval = 1;
    goto cleanup;
  }

  // training
  for (int i = 0; i < EPOCH; i++) {
    // it will set arr to it's index value and then shuffle it
    shuffle(arr, training_length);
    double total_avg_cost_across_batch = 0;

    // creating each batch, and then doing below stuff
    for (int b = 0; b < batch_num; b++) {
      // copying batch in activations[0]
      int start_posn = b * batch_length;
      int end_posn = (b + 1) * batch_length;

      // case where last batch contains total obs <= batch_length
      // as used ceil above
      if ((b == batch_num - 1) && (remainder_batch_len != 0)) {
        batch_length = remainder_batch_len;
        y_cost->cols = batch_length;
        for (int ki = 0; ki < num_layers; ki++) {
          activations[ki]->cols = batch_length;
        }
        zs->cols = batch_length;
        delta->cols = batch_length;
        delta_mul->cols = batch_length;
        end_posn = training_length;
      }

      for (int m = start_posn; m < end_posn; m++) {
        // since both activations and X are in form such
        // that obs are in cols and rows are dimensions
        // activations[0] -> m = 0 and X -> m = 1 as transposed
        int mcol = m - start_posn;
        for (int mrow = 0; mrow < layers[0] + EXTRA_ONE; mrow++) {
          activations[0]->data[mcol + mrow * batch_length] =
              X_train->data[arr[m] + mrow * training_length];
        }
        // and copying Y to y_cost
        y_cost->data[mcol] = Y_train->data[arr[m]];
      }

      // after copying, starting training one time for this batch
      for (int j = 0; j < compute_layers; j++) {
        // z = np.dot(w, z) + b
        zs->rows = layers[j + 1];
        int failure = matrix_mul(wb[j], activations[j], zs);
        if (failure != 0) {
          printf("Wasn't able to matrix_mul wb[%d] and activations[%d]\n", j,
                 j);
          puts("Location: In feedforward of training");
          retval = 2;
          goto cleanup;
        }
        int temp = activations[j + 1]->rows;
        activations[j + 1]->rows = activations[j + 1]->rows - 1;
        // z = sigmoid(z)
        failure = sigmoid(zs, activations[j + 1]);
        if (failure != 0) {
          printf("Wasn't able to get sigmoid of zs and activations[%d]\n",
                 j + 1);
          printf("Location: In sigmoid of training at layer %d, epoch %d\n",
                 j + 1, i);
          retval = 2;
          goto cleanup;
        }
        activations[j + 1]->rows = temp;

        if (j != compute_layers - 1) {
          // for last activations there is no need to waste
          // setting value equal to 1
          int start_posn =
              (activations[j + 1]->rows - 1) * activations[j + 1]->cols;
          int end_posn = start_posn + activations[j + 1]->cols;
          for (int m = start_posn; m < end_posn; m++) {
            activations[j + 1]->data[m] = 1;
          }
        }
      }
      // printing the cost at this epoch
      activations[num_layers - 1]->rows -= 1;
      double cost_value =
          sqr_cost_function(activations[num_layers - 1], y_cost, batch_length);
      total_avg_cost_across_batch += cost_value;

      // last layer delta
      // delta = (activations[-1] - training_y) * sigmoid_prime(zs[-1])
      delta->rows = activations[num_layers - 1]->rows;
      int failure =
          matrix_add(activations[num_layers - 1], y_cost, delta, 1, -1);
      if (failure != 0) {
        printf("Failed to add activations[%d] and Y_mat\n", num_layers - 1);
        puts("Location: Training -> last layer delta");
        retval = 3;
        goto cleanup;
      }
      zs->rows = activations[num_layers - 1]->rows;
      sigmoid_prime(activations[num_layers - 1], zs);
      failure = matrix_multiply(delta, zs, delta);
      if (failure != 0) {
        printf("Failed to matrix_multiply delta and zs\n");
        puts("Location: Training -> last layer delta");
        retval = 3;
        goto cleanup;
      }
      activations[num_layers - 1]->rows += 1;

      double weighted_learn_rate = -1 * ((double)LEARN_RATE / batch_length);

      matrix_transpose(activations[num_layers - 2]);
      delta_wb->rows = wb[compute_layers - 1]->rows;
      delta_wb->cols = wb[compute_layers - 1]->cols;
      failure = matrix_mul(delta, activations[num_layers - 2], delta_wb);
      if (failure != 0) {
        printf("Wasn't able to mat_mul delta and activations[%d]\n",
               num_layers - 2);
        puts("Location: In last layer delta_wb of training");
        retval = 3;
        goto cleanup;
      }
      matrix_transpose(activations[num_layers - 2]);
      // wb - ((self.learn_rate / len(training_x)) * dwb)
      int total_elem = delta_wb->cols * delta_wb->rows;
      for (int k = 0; k < total_elem; k++) {
        wb[compute_layers - 1]->data[k] +=
            (delta_wb->data[k] * weighted_learn_rate);
      }

      for (int m = 2; m < num_layers; m++) {
        // getting delta of previous layer
        matrix_transpose(wb[compute_layers - m + 1]);
        // cols because above transpose
        int temp = wb[compute_layers - m + 1]->cols;
        wb[compute_layers - m + 1]->cols -= 1;
        delta_mul->rows = wb[compute_layers - m + 1]->cols;
        int failure = matrix_mul(wb[compute_layers - m + 1], delta, delta_mul);
        if (failure != 0) {
          printf("Wasn't able to mat_mul wb[%d] and delta\n",
                 compute_layers - m + 1);
          printf("Location: In delta of layer %d of training\n",
                 compute_layers - m);
          retval = 3;
          goto cleanup;
        }
        wb[compute_layers - m + 1]->cols = temp;
        matrix_transpose(wb[compute_layers - m + 1]);

        temp = activations[num_layers - m]->rows;
        activations[num_layers - m]->rows =
            activations[num_layers - m]->rows - 1;
        zs->rows = activations[num_layers - m]->rows;
        failure = sigmoid_prime(activations[num_layers - m], zs);
        if (failure != 0) {
          puts("Wasn't able to get sigmoid prime");
          puts("Location: In sigmoid_prime of delta loop");
          retval = 2;
          goto cleanup;
        }
        activations[num_layers - m]->rows = temp;

        failure = matrix_multiply(delta_mul, zs, delta_mul);
        if (failure != 0) {
          puts("Wasn't able to matrix_multiply delta_mul and zs");
          puts("Location: At matrix_multiply of delta loop");
          retval = 2;
          goto cleanup;
        }

        // updating wb
        // delta_wb[-i] = np.dot(delta, activations[-i - 1].transpose())
        matrix_transpose(activations[num_layers - m - 1]);
        delta_wb->rows = wb[compute_layers - m]->rows;
        delta_wb->cols = wb[compute_layers - m]->cols;
        failure =
            matrix_mul(delta_mul, activations[num_layers - m - 1], delta_wb);
        if (failure != 0) {
          printf("Wasn't able to mat_mul delta and activations[%d]\n",
                 num_layers - m - 1);
          puts("Location: In layer loop delta_wb of training");
          retval = 3;
          goto cleanup;
        }
        matrix_transpose(activations[num_layers - m - 1]);
        // wb - ((self.learn_rate / len(training_x)) * dwb)
        total_elem = delta_wb->cols * delta_wb->rows;
        for (int k = 0; k < total_elem; k++) {
          wb[compute_layers - m]->data[k] +=
              (delta_wb->data[k] * weighted_learn_rate);
        }

        // copying delta_mul to delta
        delta->rows = delta_mul->rows;
        failure = matrix_copy(delta_mul, delta);
        if (failure != 0) {
          puts("Couldn't copy delta_mul to delta");
          puts("Location: At end of delta loop");
          retval = 10;
          goto cleanup;
        }
      }

      // resetting
      if ((b == batch_num - 1) && (remainder_batch_len != 0)) {
        batch_length = temp_batch_length;
        y_cost->cols = batch_length;
        for (int ki = 0; ki < num_layers; ki++) {
          activations[ki]->cols = batch_length;
        }
        zs->cols = batch_length;
        delta->cols = batch_length;
        delta_mul->cols = batch_length;
      }
    }

    // after above all batch training printing total_avg_cost_across_batch
    total_avg_cost_across_batch /= batch_num;
    printf("Epoch %d -> Cost %f\n", i, total_avg_cost_across_batch);
  }

cleanup:
  matrix_free(y_cost);
  matrix_free(zs);
  matrix_free(delta);
  matrix_free(delta_mul);
  matrix_free(delta_wb);
  for (int i = 0; i < num_layers; i++) {
    matrix_free(activations[i]);
  }

  return retval;
}

int gradient_descent(Matrix *X_train, Matrix *Y_train, int num_layers,
                     Matrix *wb[num_layers - 1], int layers[num_layers],
                     int training_length) {
  int retval = 0;
  int compute_layers = num_layers - 1;

  Matrix *activations[num_layers];
  Matrix *acti =
      Matrix_create(layers[num_layers - 1] + EXTRA_ONE, training_length);
  if (acti == NULL) {
    puts("Failed to create activation matrix");
    retval = 1;
    goto cleanup;
  }
  activations[num_layers - 1] = acti;
  for (int i = 0; i < compute_layers - 1; i++) {
    Matrix *acti = Matrix_create(layers[i + 1] + EXTRA_ONE, training_length);
    if (acti == NULL) {
      puts("Failed to create activation matrix");
      retval = 1;
      goto cleanup;
    }
    activations[i + 1] = acti;
  }

  // zs for temp storage while matrix multiplication
  int highest_node_num = 0;
  for (int m = 1; m < num_layers; m++) {
    if (highest_node_num < layers[m]) {
      highest_node_num = layers[m];
    }
  }
  Matrix *zs = Matrix_create(highest_node_num, training_length);
  if (zs == NULL) {
    puts("Failed to create zs matrix");
    retval = 1;
    goto cleanup;
  }
  Matrix *delta = Matrix_create(highest_node_num, training_length);
  if (delta == NULL) {
    puts("Failed to create delta matrix");
    retval = 1;
    goto cleanup;
  }
  Matrix *delta_mul = Matrix_create(highest_node_num, training_length);
  if (delta_mul == NULL) {
    puts("Failed to create delta_mul matrix");
    retval = 1;
    goto cleanup;
  }
  int max_wb_rows = 0, max_wb_cols = 0;
  for (int z = 0; z < compute_layers; z++) {
    if (wb[z]->rows > max_wb_rows) {
      max_wb_rows = wb[z]->rows;
    }
    if (wb[z]->cols > max_wb_cols) {
      max_wb_cols = wb[z]->cols;
    }
  }
  Matrix *delta_wb = Matrix_create(max_wb_rows, max_wb_cols);
  if (delta_wb == NULL) {
    puts("Failed to create delta_weights matrix");
    retval = 1;
    goto cleanup;
  }

  // first matrix of activations
  activations[0] = X_train;

  /*
  What i did here is converted the whole AX + B operation in feedforward
  to MX one operation where M = [A B]
  For example:
  [w1 w2][x1  + [b1]
          x2]

  can also be written as
  [w1 w2 b1][x1
             x2]

  Thus with above, in each step i am adding extra 1 to each activations
  except the last one as it is wasteful. By doing above, in backprop
  rather than doing two loops to update weights and bias separately
  we can update them at once. Refer below
  */
  // training
  for (int i = 0; i < EPOCH; i++) {
    for (int j = 0; j < compute_layers; j++) {
      // z = np.dot(w, z) + b
      zs->rows = layers[j + 1];
      int failure = matrix_mul(wb[j], activations[j], zs);
      if (failure == 1) {
        puts("Wasn't able to multiply matrix");
        puts("Location: In feedforward of training");
        retval = 2;
        goto cleanup;
      }
      int temp = activations[j + 1]->rows;
      activations[j + 1]->rows = activations[j + 1]->rows - 1;
      // z = sigmoid(z)
      failure = sigmoid(zs, activations[j + 1]);
      if (failure == 1) {
        printf("Wasn't able to get sigmoid at layer %d, epoch %d\n", j, i);
        puts("Location: In sigmoid of training");
        retval = 2;
        goto cleanup;
      }
      activations[j + 1]->rows = temp;

      if (j != compute_layers - 1) {
        // for last activations there is no need to waste
        // setting value equal to 1
        int start_posn =
            (activations[j + 1]->rows - 1) * activations[j + 1]->cols;
        int end_posn = start_posn + activations[j + 1]->cols;
        for (int m = start_posn; m < end_posn; m++) {
          activations[j + 1]->data[m] = 1;
        }
      }
    }
    // printing the cost at this epoch
    activations[num_layers - 1]->rows -= 1;
    double cost_value = sqr_cost_function(activations[num_layers - 1], Y_train,
                                          training_length);
    printf("Epoch %d -> Cost %f\n", i, cost_value);

    // last layer delta
    // delta = (activations[-1] - training_y) * sigmoid_prime(zs[-1])
    delta->rows = activations[num_layers - 1]->rows;
    int failure =
        matrix_add(activations[num_layers - 1], Y_train, delta, 1, -1);
    if (failure == 1) {
      printf("Failed to add activations[%d] and Y_mat\n", num_layers - 1);
      puts("Location: Training -> last layer delta");
      retval = 3;
      goto cleanup;
    }
    zs->rows = activations[num_layers - 1]->rows;
    sigmoid_prime(activations[num_layers - 1], zs);
    matrix_multiply(delta, zs, delta);
    activations[num_layers - 1]->rows += 1;

    double weighted_learn_rate = -1 * ((double)LEARN_RATE / training_length);

    // updating wb
    // delta_wb[-1] = np.dot(delta, activations[-2].transpose())
    /*
    Cause matrix multiplication of delta with activations containing
    1 at the last column will result in matrix where if columns_num = n
    then n-1 column with all rows will be of delta_w and last column is for
    delta_b.

    Take any number of training length and see that all the deltas are added
    together for each training case.
    */
    matrix_transpose(activations[num_layers - 2]);
    delta_wb->rows = wb[compute_layers - 1]->rows;
    delta_wb->cols = wb[compute_layers - 1]->cols;
    failure = matrix_mul(delta, activations[num_layers - 2], delta_wb);
    if (failure == 1) {
      printf("Wasn't able to mat_mul delta and activations[%d]\n",
             num_layers - 2);
      puts("Location: In last layer delta_wb of training");
      retval = 3;
      goto cleanup;
    }
    matrix_transpose(activations[num_layers - 2]);
    // wb - ((self.learn_rate / len(training_x)) * dwb)
    int total_elem = delta_wb->cols * delta_wb->rows;
    for (int k = 0; k < total_elem; k++) {
      wb[compute_layers - 1]->data[k] +=
          (delta_wb->data[k] * weighted_learn_rate);
    }

    for (int m = 2; m < num_layers; m++) {
      // getting delta of previous layer
      matrix_transpose(wb[compute_layers - m + 1]);
      // cols because above transpose
      int temp = wb[compute_layers - m + 1]->cols;
      wb[compute_layers - m + 1]->cols -= 1;
      delta_mul->rows = wb[compute_layers - m + 1]->cols;
      int failure = matrix_mul(wb[compute_layers - m + 1], delta, delta_mul);
      if (failure == 1) {
        printf("Wasn't able to mat_mul wb[%d] and delta\n",
               compute_layers - m + 1);
        printf("Location: In delta of layer %d of training\n",
               compute_layers - m);
        retval = 3;
        goto cleanup;
      }
      wb[compute_layers - m + 1]->cols = temp;
      matrix_transpose(wb[compute_layers - m + 1]);

      temp = activations[num_layers - m]->rows;
      activations[num_layers - m]->rows = activations[num_layers - m]->rows - 1;
      zs->rows = activations[num_layers - m]->rows;
      failure = sigmoid_prime(activations[num_layers - m], zs);
      if (failure == 1) {
        puts("Wasn't able to get sigmoid prime");
        puts("Location: In sigmoid_prime of delta loop");
        retval = 2;
        goto cleanup;
      }
      activations[num_layers - m]->rows = temp;

      matrix_multiply(delta_mul, zs, delta_mul);

      // updating wb
      // delta_wb[-i] = np.dot(delta, activations[-i - 1].transpose())
      matrix_transpose(activations[num_layers - m - 1]);
      delta_wb->rows = wb[compute_layers - m]->rows;
      delta_wb->cols = wb[compute_layers - m]->cols;
      failure =
          matrix_mul(delta_mul, activations[num_layers - m - 1], delta_wb);
      if (failure == 1) {
        printf("Wasn't able to mat_mul delta and activations[%d]\n",
               num_layers - m - 1);
        puts("Location: In layer loop delta_wb of training");
        retval = 3;
        goto cleanup;
      }
      matrix_transpose(activations[num_layers - m - 1]);
      // wb - ((self.learn_rate / len(training_x)) * dwb)
      total_elem = delta_wb->cols * delta_wb->rows;
      for (int k = 0; k < total_elem; k++) {
        wb[compute_layers - m]->data[k] +=
            (delta_wb->data[k] * weighted_learn_rate);
      }

      // copying delta_mul to delta
      delta->rows = delta_mul->rows;
      matrix_copy(delta_mul, delta);
    }
  }

cleanup:
  matrix_free(zs);
  matrix_free(delta);
  matrix_free(delta_mul);
  matrix_free(delta_wb);
  // don't have to free first activations
  // as that is X_train, which will be freed
  // in main function
  for (int i = 0; i < compute_layers; i++) {
    matrix_free(activations[i + 1]);
  }

  return retval;
}
