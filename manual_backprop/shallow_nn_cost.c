#include "shallow_nn_cost.h"
#include <math.h>

// np.sum((activations[-1] - training_y) ** 2) * 0.5/len(training_x)
double sqr_cost_function(Matrix *activation, Matrix *train_y,
                         int train_x_length) {
    double total_res = 0;

    int total_elem = activation->cols * activation->rows;
    for (int i = 0; i < total_elem; i++) {
        double curr_elem = activation->data[i] - train_y->data[i];
        curr_elem = pow(curr_elem, 2);
        total_res += curr_elem;
    }

    int denom = 2 * train_x_length;
    total_res = total_res / denom;

    return total_res;
}
