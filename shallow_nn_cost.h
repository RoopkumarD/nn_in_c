#ifndef SHALLOW_NN_COST
#define SHALLOW_NN_COST 1

#include "mat/mat.h"

double sqr_cost_function(Matrix *activation, Matrix *train_y,
                         int train_x_length);

#endif // !SHALLOW_NN_COST
