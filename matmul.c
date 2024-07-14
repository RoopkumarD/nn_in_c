#include <stdio.h>

/*
Did this to clarify that, n dimensional arrays multiplication
is nothing new. It is just matrix multiplication of matrix of
shape at last two ndims.

Think of tensor as packing of matrix (2 dim) into other places.
thus we get all the matrix and multiply with all the other places
matrix of other tensor.

It is kind of like multiplying polynomial: (A + B) * (C + D) = A * (C + D) + B *
(C + D)
*/

void simpleMatMul(int *mat1, int rows1, int cols1, int *mat2, int rows2,
                  int cols2) {
    int temp[rows1][cols2];

    for (int m = 0; m < rows1; m++) {
        for (int n = 0; n < cols2; n++) {
            temp[m][n] = 0;
            for (int l = 0; l < cols1; l++) {
                // temp[m][n] += tens1[i][j][m][l] * tens2[i][j][l][n];
                temp[m][n] += mat1[m * cols1 + l] * mat2[l * cols2 + n];
            }
        }
    }

    printf("[");
    for (int m = 0; m < rows1; m++) {
        if (m == 0) {
            printf("[");
        } else {
            printf(" [");
        }
        for (int n = 0; n < cols2; n++) {
            if (n == cols2 - 1) {
                printf("%d", temp[m][n]);
            } else {
                printf("%d, ", temp[m][n]);
            }
        }
        if (m != rows1 - 1) {
            printf("], ");
            printf("\n");
        } else {
            printf("]");
        }
    }
    printf("]\n");
    printf("\n");
    return;
}

// without broadcasting
int main(void) {
    int ndim = 4;

    int tens1[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    int t1[4] = {1, 2, 2, 3};
    int ts1[ndim];
    ts1[ndim - 1] = 1;
    for (int i = ndim - 2; i > -1; i--) {
        ts1[i] = ts1[i + 1] * t1[i + 1];
    }

    int tens2[6] = {0, 1, 2, 3, 4, 5};
    int t2[4] = {1, 2, 3, 1};
    int ts2[ndim];
    ts2[ndim - 1] = 1;
    for (int i = ndim - 2; i > -1; i--) {
        ts2[i] = ts2[i + 1] * t2[i + 1];
    }

    int rows = 2;
    int cols = 1;
    int internal = 3;

    int total_elems = 1;
    int loop_ndim = 2;
    int loop_shape[loop_ndim];
    int loop_stride[loop_ndim];
    for (int i = 0; i < loop_ndim; i++) {
        loop_shape[i] = t1[i];
        total_elems *= t1[i];
    }
    loop_stride[loop_ndim - 1] = 1;
    for (int i = loop_ndim - 2; i > -1; i--) {
        loop_stride[i] = loop_stride[i + 1] * loop_shape[i + 1];
    }

    for (int i = 0; i < total_elems; i++) {
        int temp = i;
        int curr_loop_idx[loop_ndim];
        printf("For packing ");
        for (int j = 0; j < loop_ndim; j++) {
            curr_loop_idx[j] = temp / loop_stride[j];
            temp %= loop_stride[j];
            printf("%d ", curr_loop_idx[j]);
        }
        printf("\n");

        int mat1stride = 0;
        int mat2stride = 0;

        for (int j = 0; j < loop_ndim; j++) {
            mat1stride += curr_loop_idx[j] * ts1[j];
            mat2stride += curr_loop_idx[j] * ts2[j];
        }

        int *mat1 = tens1 + mat1stride;
        int *mat2 = tens2 + mat2stride;

        simpleMatMul(mat1, t1[ndim - 2], t1[ndim - 1], mat2, t2[ndim - 2],
                     t2[ndim - 1]);
    }

    return 0;
}

int main2(void) {
    int tens1[1][2][2][3] = {
        {{{0, 1, 2}, {3, 4, 5}}, {{6, 7, 8}, {9, 10, 11}}}};
    int tens2[1][2][3][1] = {{{{0}, {1}, {2}}, {{3}, {4}, {5}}}};

    int rows = 2;
    int cols = 1;
    int internal = 3;

    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 2; j++) {
            printf("For packing %d, %d\n", i, j);
            int temp[rows][cols];

            for (int m = 0; m < rows; m++) {
                for (int n = 0; n < cols; n++) {
                    temp[m][n] = 0;
                    for (int l = 0; l < internal; l++) {
                        temp[m][n] += tens1[i][j][m][l] * tens2[i][j][l][n];
                    }
                }
            }

            printf("[");
            for (int m = 0; m < rows; m++) {
                if (m == 0) {
                    printf("[");
                } else {
                    printf(" [");
                }
                for (int n = 0; n < cols; n++) {
                    if (n == cols - 1) {
                        printf("%d", temp[m][n]);
                    } else {
                        printf("%d, ", temp[m][n]);
                    }
                }
                if (m != rows - 1) {
                    printf("], ");
                    printf("\n");
                } else {
                    printf("]");
                }
            }
            printf("]\n");
            printf("\n");
        }
    }

    return 0;
}
