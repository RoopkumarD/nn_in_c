#include "tensor.h"
#include "err_helper.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Using global array to store all the pointer, such that
don't have to bubble up NULL to parent caller and can exit
in the function where calloc has failed

by calling gpl free which frees all the memory allocated

This also imposes another restriction which is to free all
intermediate/temp memory and not store in this array
because all the pointer stored in this array will exists till
end of the process

Usage: glb_init at start of main function of process and then
register glb_free to atexit function
which is just something which executes the function registered
whenever returned from main or using exit to terminate process
sigkill, abort and stuff won't call function registered in atexit
though it doesn't matter as at those cases os will reclaim the memory
*/
#define GPAL 256

typedef struct {
    void **gptracc;
    size_t length;
    size_t used_length;
} gpa;

gpa _global_ptr_accumulator = {.gptracc = NULL, .used_length = 0, .length = 0};

int glb_init() {
    void **temp = malloc(GPAL * sizeof(void *));
    if (temp == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr,
                "%s: mem allocation for _global_ptr_accumulator failed\n",
                __func__);
        return -1;
    }
    _global_ptr_accumulator.gptracc = temp;
    _global_ptr_accumulator.length = GPAL;
    _global_ptr_accumulator.used_length = 0;
    return 0;
}

void glb_free() {
    for (size_t i = 0; i < _global_ptr_accumulator.used_length; i++) {
        free_variable(_global_ptr_accumulator.gptracc[i]);
    }
    free(_global_ptr_accumulator.gptracc);
    return;
}

int glb_reallocate() {
    size_t new_size = _global_ptr_accumulator.length + GPAL;
    void **temp =
        realloc(_global_ptr_accumulator.gptracc, new_size * sizeof(void *));
    if (temp == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr, "%s: mem realloc for _global_ptr_accumulator failed\n",
                __func__);
        return -1;
    }
    _global_ptr_accumulator.gptracc = temp;
    _global_ptr_accumulator.length = _global_ptr_accumulator.length + GPAL;

    return 0;
}

/*
Adding all the pointer whose lifetime is till end of process
*/
void glb_addptr(void *tens) {
    if (_global_ptr_accumulator.used_length == _global_ptr_accumulator.length) {
        int result = glb_reallocate();
        if (result == -1) {
            free_variable(tens);
            exit(EXIT_FAILURE);
        }
    }
    _global_ptr_accumulator.gptracc[_global_ptr_accumulator.used_length] = tens;
    _global_ptr_accumulator.used_length++;
    return;
}

/*
Returns NULL so caller is responsible for calling exit()
if got NULL
*/
tensor *create_tensor(int *ndim_shape, int ndim) {
    if (ndim <= 0) {
        LINE_FILE_PRINT(1);
        fprintf(stderr, "%s: Negative or Zero ndim value = %d\n", __func__,
                ndim);
        return NULL;
    }

    int total_elems = 1;
    for (int i = 0; i < ndim; i++) {
        if (ndim_shape[i] <= 0) {
            LINE_FILE_PRINT(1);
            fprintf(stderr,
                    "%s: Negative or Zero dimension val=%d at index=%d\n",
                    __func__, ndim_shape[i], i);
            return NULL;
        }
        total_elems *= ndim_shape[i];
    }

    int *combined_ndims = NULL;
    // int *ten_ndim_shape = NULL, *ten_stride = NULL;
    float *data_storage = NULL;
    tensor *tens = NULL;

    combined_ndims = malloc(2 * ndim * sizeof(int));
    if (combined_ndims == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr, "%s: malloc returned NULL for shape allocation\n",
                __func__);
        goto cleanup;
    }
    int *ten_ndim_shape = combined_ndims;
    int *ten_stride = combined_ndims + ndim;
    // ten_ndim_shape = malloc(ndim * sizeof(int));
    // if (ten_ndim_shape == NULL) {
    //     LINE_FILE_PRINT(2);
    //     fprintf(stderr, "%s: malloc returned NULL for shape allocation\n",
    //             __func__);
    //     goto cleanup;
    // }
    // ten_stride = malloc(ndim * sizeof(int));
    // if (ten_stride == NULL) {
    //     LINE_FILE_PRINT(2);
    //     fprintf(stderr, "%s: malloc returned NULL for stride allocation\n",
    //             __func__);
    //     goto cleanup;
    // }
    data_storage = calloc(total_elems, sizeof(float));
    if (data_storage == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr, "%s: calloc returned NULL for data allocation\n",
                __func__);
        goto cleanup;
    }
    tens = malloc(sizeof(tensor));
    if (tens == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr,
                "%s: malloc returned NULL for tensor struct allocation\n",
                __func__);
        goto cleanup;
    }

    tens->ndim = ndim;
    tens->total_size = total_elems;
    tens->shape = ten_ndim_shape;
    tens->strides = ten_stride;
    tens->data = data_storage;

    memcpy(ten_ndim_shape, ndim_shape, ndim * sizeof(int));
    // since {depth, row, col} is indexes so stride will be
    // {row * col, col, 1}
    ten_stride[ndim - 1] = 1;
    for (int i = ndim - 2; i > -1; i--) {
        ten_stride[i] = ten_stride[i + 1] * ten_ndim_shape[i + 1];
    }

    return tens;

cleanup:
    free(combined_ndims);
    // free(ten_ndim_shape);
    // free(ten_stride);
    free(data_storage);
    free(tens);

    return NULL;
}

void free_tensor(tensor *tens) {
    // need to have otherwise sigfault for accessing member of NULL
    if (tens == NULL) {
        return;
    }
    free(tens->shape); // as combined_ndims in above allocation
    // free(tens->shape);
    // free(tens->strides);
    free(tens->data);
    free(tens);
    return;
}

variable *create_variable(int *ndim_shape, int ndim, bool requires_grad) {
    tensor *tens = NULL, *grad = NULL;
    variable *ret_var = NULL;
    tens = create_tensor(ndim_shape, ndim);
    if (tens == NULL) {
        goto cleanup;
    }

    if (requires_grad == true) {
        grad = create_tensor(ndim_shape, ndim);
        if (grad == NULL) {
            goto cleanup;
        }
    }

    ret_var = malloc(sizeof(variable));
    if (ret_var == NULL) {
        LINE_FILE_PRINT(2);
        fprintf(stderr,
                "%s: malloc returned NULL for variable struct allocation\n",
                __func__);
        goto cleanup;
    }
    ret_var->tens = tens;
    ret_var->gradient = grad;
    ret_var->parents[0] = NULL;
    ret_var->parents[1] = NULL;
    ret_var->operation = NULL;
    ret_var->backprop = NULL;

    if (_global_ptr_accumulator.gptracc != NULL) {
        glb_addptr(ret_var);
    }

    return ret_var;

cleanup:
    free_tensor(tens);
    free_tensor(grad);
    free(ret_var);

    exit(EXIT_FAILURE);
}

void free_variable(variable *var) {
    if (var == NULL) {
        return;
    }
    free_tensor(var->tens);
    free_tensor(var->gradient);
    free(var);
    return;
}

// future todo: create state machine
void tensor_print(tensor *tens) {
    int ndim = tens->ndim;
    int *shape = tens->shape;
    int *strides = tens->strides;
    int total_elem = tens->total_size;

    int curr_idx[ndim];
    for (int j = 0; j < ndim; j++) {
        curr_idx[j] = -1;
    }

    for (int i = 0; i < total_elem; i++) {
        int temp = i;
        int idx_temp[ndim];
        for (int j = 0; j < ndim; j++) {
            int dim = temp / strides[j];
            idx_temp[j] = (dim != curr_idx[j]) ? 1 : 0;
            curr_idx[j] = dim;
            temp %= strides[j];
        }
        for (int j = 0; j < ndim; j++) {
            if (curr_idx[j] == 0 && idx_temp[j] == 1) {
                printf("[");
            } else if (curr_idx[ndim - 1] == 0) {
                printf(" ");
            }
        }

        printf("%f", tens->data[i]);

        for (int j = ndim - 1; j > -1; j--) {
            if (curr_idx[j] == shape[j] - 1) {
                printf("]");
            } else {
                printf(", ");
                int prev = 1;
                for (int m = ndim - 1; m > -1; m--) {
                    if (curr_idx[m] == shape[m] - 1 && prev == 1) {
                        printf("\n");
                        prev = 1;
                    } else {
                        prev = 0;
                    }
                }
                break;
            }
        }
        for (int j = 0; j < ndim; j++) {
            if (shape[j] == 1) {
                int check = 1;
                for (int m = j; m < ndim; m++) {
                    if (curr_idx[m] != shape[m] - 1) {
                        check = 0;
                        break;
                    }
                }
                if (check == 1) {
                    curr_idx[j] = -1;
                }
            }
        }
    }
    printf("\n");

    return;
}

void __matmul__(float *data1, int row1, int col1, float *data2, int row2,
                int col2, float *res) {
    int res_row = row1, res_col = col2;

    for (int i = 0; i < res_row; i++) {
        for (int j = 0; j < res_col; j++) {
            float temp = 0;
            for (int k = 0; k < col1; k++) {
                temp += data1[i * col1 + k] * data2[k * col2 + j];
            }
            res[i * res_col + j] = temp;
        }
    }

    return;
}

// tensor transpose
// future todo: create it as mathematical operation
// as if used in between some function composition
// it needs to maintain it's gradient then
void tensor_transpose(tensor *tens) {
    int *shape = tens->shape;
    int new_shape[tens->ndim];
    int new_strides[tens->ndim];

    new_shape[tens->ndim - 1] = shape[0];
    new_strides[tens->ndim - 1] = 1;
    for (int i = tens->ndim - 2; i > -1; i--) {
        new_shape[i] = shape[tens->ndim - i - 1];
        new_strides[i] = new_strides[i + 1] * new_shape[i + 1];
    }

    // temporary memory to store; later copied
    float temp_data[tens->total_size];

    for (int j = 0; j < tens->total_size; j++) {
        int temp_idx = j;
        size_t offset = 0;
        for (int k = 0; k < tens->ndim; k++) {
            offset += ((temp_idx / new_strides[k]) *
                       tens->strides[tens->ndim - k - 1]);
            temp_idx %= new_strides[k];
        }
        temp_data[j] = tens->data[offset];
    }

    // copying the transposed matrix to original
    memcpy(tens->data, temp_data, sizeof(float) * tens->total_size);
    memcpy(tens->shape, new_shape, sizeof(int) * tens->ndim);
    memcpy(tens->strides, new_strides, sizeof(int) * tens->ndim);

    return;
}

/*
Broadcasting:

Now would be great time to talk about broadcasting
Here, normally i would have just duplicated the data for those
tensor which are small in dimesnion compared to other. But i hate
creating new memory to accomodate duplicate data.

So i had an idea, after looking at loops can i just find the element
by not adding the term for i,j,k,... for that small tensor as not adding
is equivalent to 0 * stride of that dim. refer to broadcasting.py where
i used this trick

===========================================

Tensor BMM (Brocasted Matrix multiplication):

First remove all the thoughts about anything and see tensor as packing of
2d matrix in the rest n-2 dimensional space.

For example:
(1,2,3,3) is just 2d matrix of shape 3x3 packed in rest of the dimension
here 1 * 2 -> 2 which means there are two matrix of shape 3x3 in the tensor

and tensor bmm is just matrix multiplication of all the matrix in
that tensor to other matrix in other tensor according to index.

if tens1 -> (1,2,2,3) then let 1mat0 be at (0,0) and 1mat1 be at (0,1)
if tens2 -> (1,1,3,1) then let 2mat0 be at (0,0)

then resultant tensor after broadcasting will have shape -> (1,2,2,1)
where at (0,0) will have matrix of shape (2,1) where the matrix is derived
from 1mat0 @ 2mat0

and at (0,1) will have matrix of shape (2,1) where the matrix is derived
from 1mat1 @ 2mat0

src from where i understood this view: https://nrehiew.github.io/blog/pytorch/
*/
void _tensor_mm(variable *parent[2], tensor *res) {
    tensor *tens1 = parent[0]->tens;
    tensor *tens2 = parent[1]->tens;

    int row = res->shape[res->ndim - 2];
    int col = res->shape[res->ndim - 1];
    int internal = tens1->shape[tens1->ndim - 1];

    // for 2 dimensional array, no need to go through
    // loops and waste time
    if (res->ndim == 2) {
        __matmul__(tens1->data, row, internal, tens2->data, internal, col,
                   res->data);
        return;
    }

    // broadcasting so that both have same number of ndim
    // as later on i don't have to deal with if either has
    // less ndim or not
    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    /*
    Did below way because i want to iterate for each
    dimension from 0 to n-2 in linear way as need to multiply
    matrix at each packing in the tensor and after looking
    at for (int i ...) { for (int j...) {...}}
    it is same as looking at data at tensor with shape and stride
    */
    int loop_strides = res->strides[res->ndim - 3];
    int loop_ndim = res->ndim - 2;
    for (int i = 0; i < res->total_size; i += loop_strides) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < loop_ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *mat1 = tens1->data + mat1stride;
        float *mat2 = tens2->data + mat2stride;
        float *rm = res->data + rm_stride;

        __matmul__(mat1, row, internal, mat2, internal, col, rm);
    }

    return;
}

/*
Matrix_transpose(mat1);
matrix *par[2] = {mat1->mat, res->gradient};
_matrix_mul(par, mat2->gradient);
Matrix_transpose(mat1);

Matrix_transpose(mat2);
matrix *pare[2] = {res->gradient, mat2->mat};
_matrix_mul(pare, mat1->gradient);
Matrix_transpose(mat2);

for tensor -> since mm is batched matrix multiplication
then each element derivative is matrix multiply derivative
and loop won't contribute to derivative

also += than = because need to accumulate gradients
*/
void _tensor_mm_backprop(variable *result) {
    variable *var_tens1 = result->parents[0];
    variable *var_tens2 = result->parents[1];

    tensor *tens1 = var_tens1->tens;
    tensor *tens2 = var_tens2->tens;
    tensor *res = result->tens;

    tensor *grad_tens1 = var_tens1->gradient;
    tensor *grad_tens2 = var_tens2->gradient;

    if (grad_tens1 == NULL && grad_tens2 == NULL) {
        return;
    }

    tensor *grad_res = result->gradient;

    // for 2 dimensional array, no need to go through
    // loops and waste time
    int row1 =
        (grad_tens1 != NULL) ? grad_tens1->shape[grad_tens1->ndim - 2] : -1;
    int col1 =
        (grad_tens1 != NULL) ? grad_tens1->shape[grad_tens1->ndim - 1] : -1;
    int internal1 = grad_res->shape[grad_res->ndim - 1];

    int row2 =
        (grad_tens2 != NULL) ? grad_tens2->shape[grad_tens2->ndim - 2] : -1;
    int col2 =
        (grad_tens2 != NULL) ? grad_tens2->shape[grad_tens2->ndim - 1] : -1;
    int internal2 = grad_res->shape[grad_res->ndim - 2];

    if (res->ndim == 2) {
        if (grad_tens1 != NULL) {
            for (int i = 0; i < row1; i++) {
                for (int j = 0; j < col1; j++) {
                    float temp = 0;
                    for (int k = 0; k < internal1; k++) {
                        temp += grad_res->data[i * internal1 + k] *
                                tens2->data[j * internal1 + k];
                    }
                    grad_tens1->data[i * col1 + j] += temp;
                }
            }
        }
        if (grad_tens2 != NULL) {
            for (int i = 0; i < row2; i++) {
                for (int j = 0; j < col2; j++) {
                    float temp = 0;
                    for (int k = 0; k < internal2; k++) {
                        temp += tens1->data[k * row2 + i] *
                                grad_res->data[k * col2 + j];
                    }
                    grad_tens2->data[i * col2 + j] += temp;
                }
            }
        }
        return;
    }

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    int loop_strides = res->strides[res->ndim - 3];
    int loop_ndim = res->ndim - 2;
    for (int i = 0; i < res->total_size; i += loop_strides) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < loop_ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *mat1 = tens1->data + mat1stride;
        float *mat2 = tens2->data + mat2stride;
        float *rm = grad_res->data + rm_stride;

        if (grad_tens1 != NULL) {
            float *grad_mat1 = grad_tens1->data + mat1stride;
            for (int i = 0; i < row1; i++) {
                for (int j = 0; j < col1; j++) {
                    float temp = 0;
                    for (int k = 0; k < internal1; k++) {
                        temp += rm[i * internal1 + k] * mat2[j * internal1 + k];
                    }
                    grad_mat1[i * col1 + j] += temp;
                }
            }
        }
        if (grad_tens2 != NULL) {
            float *grad_mat2 = grad_tens2->data + mat2stride;
            for (int i = 0; i < row2; i++) {
                for (int j = 0; j < col2; j++) {
                    float temp = 0;
                    for (int k = 0; k < internal2; k++) {
                        temp += mat1[k * row2 + i] * rm[k * col2 + j];
                    }
                    grad_mat2[i * col2 + j] += temp;
                }
            }
        }
    }

    return;
}

variable *tensor_mm(variable *tens1, variable *tens2) {
    int tens1_ndim = tens1->tens->ndim;
    int *tens1_shape = tens1->tens->shape;

    int tens2_ndim = tens2->tens->ndim;
    int *tens2_shape = tens2->tens->shape;

    if (tens1_ndim < 2 || tens2_ndim < 2) {
        LINE_FILE_PRINT(1);
        fprintf(stderr,
                "%s: tensor dimensions is less than 2 for dot_product; tens1 "
                "-> %d, tens2 -> %d\n",
                __func__, tens1_ndim, tens2_ndim);
        exit(EXIT_FAILURE);
    }

    if (tens1_shape[tens1_ndim - 1] != tens2_shape[tens2_ndim - 2]) {
        LINE_FILE_PRINT(1);
        fprintf(stderr, "%s: Invalid dimension: cols != rows -> %d != %d\n",
                __func__, tens1_shape[tens1_ndim - 1],
                tens2_shape[tens2_ndim - 2]);
        exit(EXIT_FAILURE);
    }

    // broadcasting for final result dimensions
    int res_ndim = (tens1_ndim > tens2_ndim) ? tens1_ndim : tens2_ndim;
    int res_shape[res_ndim];

    int tens1_diff = res_ndim - tens1_ndim;
    int tens2_diff = res_ndim - tens2_ndim;

    for (int i = 0; i < res_ndim - 2; i++) {
        int tens1_curr_shape =
            (i >= tens1_diff) ? tens1_shape[i - tens1_diff] : 1;
        int tens2_curr_shape =
            (i >= tens2_diff) ? tens2_shape[i - tens2_diff] : 1;

        // the reason for this
        // valid -> (1,2,3,4) @ (2, 4, 3) => as number of matrix is equal in
        // both side
        //
        // invalid -> (1,2,3,4) @ (3, 4, 3) => left side has 2 mat
        // whereas right side has 3 mat and we can't multiply both as where
        // would be map the last mat to multiply now comparing and broadcasting
        //
        // for n-2 dimensions
        if (tens1_curr_shape != 1 && tens2_curr_shape != 1 &&
            tens1_curr_shape != tens2_curr_shape) {
            fprintf(stderr,
                    "%s: Invalid dimension: tens1_shape[%d] == %d and "
                    "tens2_shape[%d] == %d\n",
                    __func__, i, tens1_curr_shape, i, tens2_curr_shape);
            exit(EXIT_FAILURE);
        }

        res_shape[i] = (tens1_curr_shape > tens2_curr_shape) ? tens1_curr_shape
                                                             : tens2_curr_shape;
    }
    // for rest of the two dimensions
    res_shape[res_ndim - 1] = tens2_shape[tens2_ndim - 1];
    res_shape[res_ndim - 2] = tens1_shape[tens1_ndim - 2];

    variable *res = create_variable(res_shape, res_ndim, true);
    res->parents[0] = tens1;
    res->parents[1] = tens2;
    res->operation = _tensor_mm;
    res->backprop = _tensor_mm_backprop;

    return res;
}

void _tensor_add(variable *parent[2], tensor *res) {
    tensor *tens1 = parent[0]->tens;
    tensor *tens2 = parent[1]->tens;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *tens1_val = tens1->data + mat1stride;
        float *tens2_val = tens2->data + mat2stride;
        float *rm = res->data + rm_stride;

        rm[0] = tens1_val[0] + tens2_val[0];
    }

    return;
}

void _tensor_add_backprop(variable *result) {
    variable *var_tens1 = result->parents[0];
    variable *var_tens2 = result->parents[1];

    tensor *tens1 = var_tens1->tens;
    tensor *tens2 = var_tens2->tens;
    tensor *res = result->tens;

    tensor *grad_tens1 = var_tens1->gradient;
    tensor *grad_tens2 = var_tens2->gradient;

    if (grad_tens1 == NULL && grad_tens2 == NULL) {
        return;
    }

    tensor *grad_res = result->gradient;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *rm = grad_res->data + rm_stride;

        if (grad_tens1 != NULL) {
            float *grad_mat1 = grad_tens1->data + mat1stride;
            grad_mat1[0] += rm[0];
        }
        if (grad_tens2 != NULL) {
            float *grad_mat2 = grad_tens2->data + mat2stride;
            grad_mat2[0] += rm[0];
        }
    }

    return;
}

variable *tensor_add(variable *tens1, variable *tens2) {
    int tens1_ndim = tens1->tens->ndim;
    int *tens1_shape = tens1->tens->shape;

    int tens2_ndim = tens2->tens->ndim;
    int *tens2_shape = tens2->tens->shape;

    // broadcasting for final result dimensions
    int res_ndim = (tens1_ndim > tens2_ndim) ? tens1_ndim : tens2_ndim;
    int res_shape[res_ndim];

    int tens1_diff = res_ndim - tens1_ndim;
    int tens2_diff = res_ndim - tens2_ndim;

    for (int i = 0; i < res_ndim; i++) {
        int tens1_curr_shape =
            (i >= tens1_diff) ? tens1_shape[i - tens1_diff] : 1;
        int tens2_curr_shape =
            (i >= tens2_diff) ? tens2_shape[i - tens2_diff] : 1;

        if (tens1_curr_shape != 1 && tens2_curr_shape != 1 &&
            tens1_curr_shape != tens2_curr_shape) {
            fprintf(stderr,
                    "%s: Invalid dimension: tens1_shape[%d] == %d and "
                    "tens2_shape[%d] == %d\n",
                    __func__, i, tens1_curr_shape, i, tens2_curr_shape);
            exit(EXIT_FAILURE);
        }

        res_shape[i] = (tens1_curr_shape > tens2_curr_shape) ? tens1_curr_shape
                                                             : tens2_curr_shape;
    }

    variable *res = create_variable(res_shape, res_ndim, true);
    res->parents[0] = tens1;
    res->parents[1] = tens2;
    res->operation = _tensor_add;
    res->backprop = _tensor_add_backprop;

    return res;
}

void _tensor_sub(variable *parent[2], tensor *res) {
    tensor *tens1 = parent[0]->tens;
    tensor *tens2 = parent[1]->tens;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *tens1_val = tens1->data + mat1stride;
        float *tens2_val = tens2->data + mat2stride;
        float *rm = res->data + rm_stride;

        rm[0] = tens1_val[0] - tens2_val[0];
    }

    return;
}

void _tensor_sub_backprop(variable *result) {
    variable *var_tens1 = result->parents[0];
    variable *var_tens2 = result->parents[1];

    tensor *tens1 = var_tens1->tens;
    tensor *tens2 = var_tens2->tens;
    tensor *res = result->tens;

    tensor *grad_tens1 = var_tens1->gradient;
    tensor *grad_tens2 = var_tens2->gradient;

    if (grad_tens1 == NULL && grad_tens2 == NULL) {
        return;
    }

    tensor *grad_res = result->gradient;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *rm = grad_res->data + rm_stride;

        if (grad_tens1 != NULL) {
            float *grad_mat1 = grad_tens1->data + mat1stride;
            grad_mat1[0] += rm[0];
        }
        if (grad_tens2 != NULL) {
            float *grad_mat2 = grad_tens2->data + mat2stride;
            grad_mat2[0] += rm[0] * -1;
        }
    }

    return;
}

variable *tensor_sub(variable *tens1, variable *tens2) {
    int tens1_ndim = tens1->tens->ndim;
    int *tens1_shape = tens1->tens->shape;

    int tens2_ndim = tens2->tens->ndim;
    int *tens2_shape = tens2->tens->shape;

    // broadcasting for final result dimensions
    int res_ndim = (tens1_ndim > tens2_ndim) ? tens1_ndim : tens2_ndim;
    int res_shape[res_ndim];

    int tens1_diff = res_ndim - tens1_ndim;
    int tens2_diff = res_ndim - tens2_ndim;

    for (int i = 0; i < res_ndim; i++) {
        int tens1_curr_shape =
            (i >= tens1_diff) ? tens1_shape[i - tens1_diff] : 1;
        int tens2_curr_shape =
            (i >= tens2_diff) ? tens2_shape[i - tens2_diff] : 1;

        if (tens1_curr_shape != 1 && tens2_curr_shape != 1 &&
            tens1_curr_shape != tens2_curr_shape) {
            fprintf(stderr,
                    "%s: Invalid dimension: tens1_shape[%d] == %d and "
                    "tens2_shape[%d] == %d\n",
                    __func__, i, tens1_curr_shape, i, tens2_curr_shape);
            exit(EXIT_FAILURE);
        }

        res_shape[i] = (tens1_curr_shape > tens2_curr_shape) ? tens1_curr_shape
                                                             : tens2_curr_shape;
    }

    variable *res = create_variable(res_shape, res_ndim, true);
    res->parents[0] = tens1;
    res->parents[1] = tens2;
    res->operation = _tensor_sub;
    res->backprop = _tensor_sub_backprop;

    return res;
}

// hadamard multiplication
void _tensor_hm(variable *parent[2], tensor *res) {
    tensor *tens1 = parent[0]->tens;
    tensor *tens2 = parent[1]->tens;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *tens1_val = tens1->data + mat1stride;
        float *tens2_val = tens2->data + mat2stride;
        float *rm = res->data + rm_stride;

        rm[0] = tens1_val[0] * tens2_val[0];
    }

    return;
}

void _tensor_hm_backprop(variable *result) {
    variable *var_tens1 = result->parents[0];
    variable *var_tens2 = result->parents[1];

    tensor *tens1 = var_tens1->tens;
    tensor *tens2 = var_tens2->tens;
    tensor *res = result->tens;

    tensor *grad_tens1 = var_tens1->gradient;
    tensor *grad_tens2 = var_tens2->gradient;

    if (grad_tens1 == NULL && grad_tens2 == NULL) {
        return;
    }

    tensor *grad_res = result->gradient;

    int tens1_shape[res->ndim];
    int tens1_diff = res->ndim - tens1->ndim;
    int tens2_shape[res->ndim];
    int tens2_diff = res->ndim - tens2->ndim;

    for (int i = 0; i < res->ndim; i++) {
        tens1_shape[i] = (i >= tens1_diff) ? tens1->shape[i - tens1_diff] : 1;
        tens2_shape[i] = (i >= tens2_diff) ? tens2->shape[i - tens2_diff] : 1;
    }

    int tens1_stride[res->ndim];
    tens1_stride[res->ndim - 1] = 1;
    int tens2_stride[res->ndim];
    tens2_stride[res->ndim - 1] = 1;

    for (int i = res->ndim - 2; i > -1; i--) {
        tens1_stride[i] = tens1_stride[i + 1] * tens1_shape[i + 1];
        tens2_stride[i] = tens2_stride[i + 1] * tens2_shape[i + 1];
    }

    for (int i = 0; i < res->total_size; i++) {
        int temp = i;
        int mat1stride = 0;
        int mat2stride = 0;
        int rm_stride = 0;

        for (int j = 0; j < res->ndim; j++) {
            int curr_loop_idx = temp / res->strides[j];
            mat1stride +=
                (tens1_shape[j] != 1) ? curr_loop_idx * tens1_stride[j] : 0;
            mat2stride +=
                (tens2_shape[j] != 1) ? curr_loop_idx * tens2_stride[j] : 0;
            rm_stride += curr_loop_idx * res->strides[j];
            temp %= res->strides[j];
        }

        float *mat1 = tens1->data + mat1stride;
        float *mat2 = tens2->data + mat2stride;
        float *rm = grad_res->data + rm_stride;

        if (grad_tens1 != NULL) {
            float *grad_mat1 = grad_tens1->data + mat1stride;
            grad_mat1[0] += rm[0] * mat2[0];
        }
        if (grad_tens2 != NULL) {
            float *grad_mat2 = grad_tens2->data + mat2stride;
            grad_mat2[0] += rm[0] * mat1[0];
        }
    }

    return;
}

variable *tensor_hm(variable *tens1, variable *tens2) {
    int tens1_ndim = tens1->tens->ndim;
    int *tens1_shape = tens1->tens->shape;

    int tens2_ndim = tens2->tens->ndim;
    int *tens2_shape = tens2->tens->shape;

    // broadcasting for final result dimensions
    int res_ndim = (tens1_ndim > tens2_ndim) ? tens1_ndim : tens2_ndim;
    int res_shape[res_ndim];

    int tens1_diff = res_ndim - tens1_ndim;
    int tens2_diff = res_ndim - tens2_ndim;

    for (int i = 0; i < res_ndim; i++) {
        int tens1_curr_shape =
            (i >= tens1_diff) ? tens1_shape[i - tens1_diff] : 1;
        int tens2_curr_shape =
            (i >= tens2_diff) ? tens2_shape[i - tens2_diff] : 1;

        if (tens1_curr_shape != 1 && tens2_curr_shape != 1 &&
            tens1_curr_shape != tens2_curr_shape) {
            fprintf(stderr,
                    "%s: Invalid dimension: tens1_shape[%d] == %d and "
                    "tens2_shape[%d] == %d\n",
                    __func__, i, tens1_curr_shape, i, tens2_curr_shape);
            exit(EXIT_FAILURE);
        }

        res_shape[i] = (tens1_curr_shape > tens2_curr_shape) ? tens1_curr_shape
                                                             : tens2_curr_shape;
    }

    variable *res = create_variable(res_shape, res_ndim, true);
    res->parents[0] = tens1;
    res->parents[1] = tens2;
    res->operation = _tensor_hm;
    res->backprop = _tensor_hm_backprop;

    return res;
}

void _tensor_sigmoid(variable *parent[2], tensor *res) {
    tensor *tens = parent[0]->tens;

    for (int i = 0; i < res->total_size; i++) {
        float denominator = exp(-1 * tens->data[i]) + 1;
        res->data[i] = 1 / denominator;
    }

    return;
}

void _tensor_sigmoid_backprop(variable *result) {
    variable *var_tens = result->parents[0];
    tensor *tens = var_tens->tens;
    tensor *res = result->tens;

    tensor *grad_tens = var_tens->gradient;
    tensor *grad_res = result->gradient;

    if (grad_tens == NULL) {
        return;
    }

    for (int i = 0; i < res->total_size; i++) {
        float s = res->data[i];
        grad_tens->data[i] += s * (1 - s) * grad_res->data[i];
    }

    return;
}

variable *tensor_sigmoid(variable *tens) {
    variable *res = create_variable(tens->tens->shape, tens->tens->ndim, true);
    res->parents[0] = tens;
    res->parents[1] = NULL;
    res->operation = _tensor_sigmoid;
    res->backprop = _tensor_sigmoid_backprop;

    return res;
}

void _tensor_square(variable *parent[2], tensor *res) {
    tensor *tens = parent[0]->tens;

    for (int i = 0; i < res->total_size; i++) {
        res->data[i] = pow(tens->data[i], 2);
    }

    return;
}

void _tensor_square_backprop(variable *result) {
    variable *var_tens = result->parents[0];
    tensor *tens = var_tens->tens;
    tensor *res = result->tens;

    tensor *grad_tens = var_tens->gradient;
    tensor *grad_res = result->gradient;

    if (grad_tens == NULL) {
        return;
    }

    for (int i = 0; i < res->total_size; i++) {
        grad_tens->data[i] += 2 * grad_res->data[i] * tens->data[i];
    }

    return;
}

variable *tensor_square(variable *tens) {
    variable *res = create_variable(tens->tens->shape, tens->tens->ndim, true);
    res->parents[0] = tens;
    res->parents[1] = NULL;
    res->operation = _tensor_square;
    res->backprop = _tensor_square_backprop;

    return res;
}

// matrix mse
void _tensor_mse(variable *parent[2], tensor *res) {
    tensor *y_true = parent[0]->tens;
    tensor *y_pred = parent[1]->tens;

    for (int i = 0; i < res->total_size; i++) {
        float temp = y_true->data[i] - y_pred->data[i];
        res->data[i] = pow(temp, 2);
    }

    return;
}

void _tensor_mse_backprop(variable *result) {
    variable *var_tens1 = result->parents[0];
    variable *var_tens2 = result->parents[1];

    tensor *tens1 = var_tens1->tens;
    tensor *tens2 = var_tens2->tens;
    tensor *res = result->tens;

    tensor *grad_tens1 = var_tens1->gradient;
    tensor *grad_tens2 = var_tens2->gradient;

    if (grad_tens1 == NULL && grad_tens2 == NULL) {
        return;
    }

    tensor *grad_res = result->gradient;

    for (int i = 0; i < res->total_size; i++) {
        float temp = 2 * (tens1->data[i] - tens2->data[i]) * grad_res->data[i];
        if (grad_tens1 != NULL) {
            grad_tens1->data[i] += temp;
        }
        if (grad_tens2 != NULL) {
            grad_tens2->data[i] += -1 * temp;
        }
    }

    return;
}

// loss function -> MSE
variable *tensor_mse(variable *tens1, variable *tens2) {
    int tens1_ndim = tens1->tens->ndim;
    int *tens1_shape = tens1->tens->shape;

    int tens2_ndim = tens2->tens->ndim;
    int *tens2_shape = tens2->tens->shape;

    if (tens1_ndim != tens2_ndim) {
        fprintf(stderr,
                "%s: Invalid number of dimension: tens1_ndim == %d and "
                "tens2_ndim == %d\n",
                __func__, tens1_ndim, tens2_ndim);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < tens1_ndim; i++) {
        if (tens1_shape[i] != tens2_shape[i]) {
            fprintf(stderr,
                    "%s: Invalid dimension: tens1_shape[%d] == %d and "
                    "tens2_shape[%d] == %d\n",
                    __func__, i, tens1_shape[i], i, tens2_shape[i]);
            exit(EXIT_FAILURE);
        }
    }

    variable *res = create_variable(tens1_shape, tens1_ndim, true);
    res->parents[0] = tens1;
    res->parents[1] = tens2;
    res->operation = _tensor_mse;
    res->backprop = _tensor_mse_backprop;

    return res;
}

void random_initialisation(variable *mat) {
    tensor *m = mat->tens;
    for (size_t i = 0; i < m->total_size; i++) {
        m->data[i] = ((double)rand() / (RAND_MAX + 1.0)) - 0.5;
    }

    return;
}

void zero_init(variable *mat) {
    tensor *m = mat->tens;
    for (size_t i = 0; i < m->total_size; i++) {
        m->data[i] = 0;
    }

    return;
}

/*
 * future todo:
 * This works and does topological sort because before adding
 * the variable to list, it first goes down more and more till
 * it reaches end and then it starts adding them
 *
 * tried to do with stack and while loop but failed as need to
 * figure out adding variable later like above
 *
 * the reason for above is if consider a graph like
 *
 * a * b = c
 * c @ d = e
 * k @ e = l
 * e @ n = o
 * sigmoid(l) = m
 * m + o = t
 *
 * here split node is e which is used in both l and o
 * now here, i want to accumulate gradient from all directions
 * then go below for e.
 *
 * thus i above is important as first is added a downwards to upwards
 * such that dependency in reverse sense is satisfied and this satisfies
 * my condition of getting the nodes before split node before and
 * getting nodes after split node after.
 */
void recursively_add(oa *sorted_ops, variable *root) {
    // so that i don't add same variable twice
    //
    // this happened to me when i used same variable for
    // other calculation related to same computation tree
    //
    // thus visiting the node twice and thus again not just
    // going through this node again but all the previous
    // node which made this node
    for (size_t i = 0; i < sorted_ops->length; i++) {
        if (sorted_ops->sorted_operations[i] == root) {
            return;
        }
    }

    variable *p1 = root->parents[0];
    variable *p2 = root->parents[1];
    if (p1 == NULL && p2 == NULL) {
        return;
    }

    if (p1 != NULL) {
        recursively_add(sorted_ops, p1);
    }
    if (p2 != NULL) {
        recursively_add(sorted_ops, p2);
    }

    sorted_ops->sorted_operations[sorted_ops->length++] = root;

    return;
}

// returned ptr is caller responsibility to free
oa *topo_sort(variable *root) {
    if (root == NULL) {
        fprintf(stderr, "%s: root arg is NULL\n", __func__);
        exit(EXIT_FAILURE);
    }

    size_t assumed_total_variables = _global_ptr_accumulator.used_length;
    oa *sorted_operations = malloc(FLEXSIZEOF(
        oa, sorted_operations, sizeof(variable *) * assumed_total_variables));
    if (sorted_operations == NULL) {
        fprintf(stderr,
                "%s: couldn't allocate space for sorted_operations oa\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    // setting everything to zero as used malloc
    memset(sorted_operations->sorted_operations, 0,
           assumed_total_variables * sizeof(variable *));
    sorted_operations->length = 0;

    recursively_add(sorted_operations, root);

    return sorted_operations;
}

void backward_diff(oa *sorted_ops) {
    if (sorted_ops == NULL) {
        fprintf(stderr, "%s: sorted_ops is NULL\n", __func__);
        exit(EXIT_FAILURE);
    }

    // setting intermediate variables gradient to be zero
    // cause in main code, can't do if i compose functions like this
    //    variable *z = tensor_sigmoid(
    //        tensor_add(tensor_mm(weights[j], activations[j]), bias[j]));
    // but this will only do for those in ops_list and only
    // non leaf variable are there
    // for leaf i have to do it myself

    // doing it in separate loop because i don't want to erase
    // accumulated gradients
    //
    // here -1 cause i don't want last layer 1's to be zero
    variable *last_var = sorted_ops->sorted_operations[sorted_ops->length - 1];
    for (int i = 0; i < last_var->gradient->total_size; i++) {
        last_var->gradient->data[i] = 1;
    }
    for (size_t i = 0; i < sorted_ops->length - 1; i++) {
        memset(sorted_ops->sorted_operations[i]->gradient->data, 0,
               sizeof(float) *
                   sorted_ops->sorted_operations[i]->gradient->total_size);
    }

    // int not size_t cause otherwise loop won't run
    // idk why but future todo:
    for (int i = sorted_ops->length - 1; i > -1; i--) {
        variable *res = sorted_ops->sorted_operations[i];
        res->backprop(res);
    }

    return;
}

void resolve_expression(oa *sorted_ops) {
    if (sorted_ops == NULL) {
        fprintf(stderr, "%s: sorted_ops is NULL\n", __func__);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < sorted_ops->length; i++) {
        variable *res = sorted_ops->sorted_operations[i];
        res->operation(res->parents, res->tens);
    }

    return;
}
