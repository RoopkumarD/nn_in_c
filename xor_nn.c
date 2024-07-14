#include "tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_shape(int *shape, int ndim) {
    printf("[");
    for (int i = 0; i < ndim; i++) {
        printf("%d", shape[i]);
        if (i != ndim - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    return;
}

double cost_function(variable *sq_res);

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define NUM_ROWS(array_2d) ARRAY_LEN(array_2d)
#define NUM_COLS(array_2d) ARRAY_LEN(array_2d[0])

#define EPOCH 1000
#define LEARN_RATE 3

float X[4][2] = {
    {0, 0},
    {1, 0},
    {0, 1},
    {1, 1},
};

float Y[4] = {0, 1, 1, 0};

int layers[] = {2, 2, 1};
int num_layers = sizeof(layers) / sizeof(int);
int training_length = ARRAY_LEN(X);

int main(void) {
    if (glb_init() == -1) {
        return -1;
    }
    atexit(glb_free);

    srand(time(NULL));

    int X_shape[] = {4, 2};
    int X_ndim = 2;
    variable *X_mat = create_variable(X_shape, X_ndim, false);
    memcpy(X_mat->tens->data, X, sizeof(float) * X_mat->tens->total_size);
    tensor_transpose(X_mat->tens);

    int Y_shape[] = {1, 4};
    int Y_ndim = 2;
    variable *Y_mat = create_variable(Y_shape, Y_ndim, false);
    memcpy(Y_mat->tens->data, Y, sizeof(float) * Y_mat->tens->total_size);

    variable *weights[num_layers - 1];
    variable *bias[num_layers - 1];
    variable *activations[num_layers];
    variable *sq_res = NULL;
    int compute_layers = num_layers - 1;

    // setting everything to NULL
    activations[0] = NULL;
    for (size_t i = 0; i < compute_layers; i++) {
        weights[i] = NULL;
        bias[i] = NULL;
        activations[i + 1] = NULL;
    }

    for (int i = 0; i < compute_layers; i++) {
        int w_shape[] = {layers[i + 1], layers[i]};
        int w_ndim = 2;
        variable *w = create_variable(w_shape, w_ndim, true);
        random_initialisation(w);
        weights[i] = w;

        int b_shape[] = {layers[i + 1], 1};
        int b_ndim = 2;
        variable *b = create_variable(b_shape, b_ndim, true);
        zero_init(b);
        bias[i] = b;
    }

    activations[0] = X_mat;

    // creating computation graph
    for (int j = 0; j < compute_layers; j++) {
        // z = np.dot(w, z) + b
        // z = sigmoid(z)
        variable *z = tensor_sigmoid(
            tensor_add(tensor_mm(weights[j], activations[j]), bias[j]));
        activations[j + 1] = z;
    }
    sq_res = tensor_mse(Y_mat, activations[num_layers - 1]);

    oa *ops_list = topo_sort(sq_res);

    // Now training the model
    for (int i = 0; i < EPOCH; i++) {
        resolve_expression(ops_list);

        // problem which is how to set
        // intermediate variable gradient to be zero
        for (size_t i = 0; i < compute_layers; i++) {
            memset(weights[i]->gradient->data, 0,
                   sizeof(float) * weights[i]->gradient->total_size);
            memset(bias[i]->gradient->data, 0,
                   sizeof(float) * weights[i]->gradient->total_size);
        }

        backward_diff(ops_list);

        double cost_value = cost_function(sq_res);
        printf("Epoch %d -> Cost %f\n", i, cost_value);

        // updating with gradients
        for (int i = 0; i < compute_layers; i++) {
            variable *w = weights[i];
            for (size_t m = 0; m < w->gradient->total_size; m++) {
                // printf("%f\n", w->gradient->data[m]);
                w->tens->data[m] -= w->gradient->data[m] * LEARN_RATE;
            }
            variable *b = bias[i];
            for (size_t m = 0; m < b->gradient->total_size; m++) {
                b->tens->data[m] -= b->gradient->data[m] * LEARN_RATE;
            }
        }
    }
    // feedforward
    resolve_expression(ops_list);

    tensor *inputs = X_mat->tens;
    tensor *y_true = Y_mat->tens;
    tensor *y_pred = activations[num_layers - 1]->tens;
    printf("---------------------------------------------------------------\n");
    printf("---------------------------------------------------------------\n");
    printf("\t\t\t\tXOR Fn\t\t\t\t\n");
    printf("---------------------------------------------------------------\n");
    printf("---------------------------------------------------------------\n");
    printf("\tx0\t|\tx1\t|\ty_true\t|\ty_pred\t\n");
    for (int i = 0; i < training_length; i++) {
        printf("\t%.2f\t|\t%.2f\t|\t%.2f\t|\t%.2f\t\n", inputs->data[i * 2],
               inputs->data[(i * 2) + 1], y_true->data[i], y_pred->data[i]);
    }

    free(ops_list);
    return 0;
}

// np.sum((activations[-1] - training_y) ** 2) * 0.5/len(training_x)
double cost_function(variable *sq_res) {
    double total_res = 0;

    tensor *sqmat = sq_res->tens;
    for (size_t i = 0; i < sqmat->total_size; i++) {
        total_res += sqmat->data[i];
    }

    total_res = total_res * 0.5;
    total_res = total_res / sqmat->shape[sqmat->ndim - 1];

    return total_res;
}
