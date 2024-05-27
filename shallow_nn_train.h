#ifndef SHALLOW_NN_TRAIN
#define SHALLOW_NN_TRAIN 1

#include "mat/mat.h"

// extra 1 value node to multiply AX as ax + b*1
#define EXTRA_ONE 1
#define EPOCH 1000
#define LEARN_RATE 5

int gradient_descent(Matrix *X_train, Matrix *Y_train, int num_layers,
                     Matrix *wb[num_layers - 1], int layers[num_layers],
                     int training_length);

#endif // !SHALLOW_NN_TRAIN
