#include <stdbool.h>
#include <stdio.h>

#ifndef TENSOR_H
#define TENSOR_H 1

int glb_init();
void glb_free();

/* clang-format off */
typedef struct {
    int     ndim;
    int     total_size;
    int     *shape;
    int     *strides;
    float   *data;
    size_t  glb_idxs[3];
} tensor;
/* clang-format on */

typedef struct _variable variable;

typedef void (*operator_func)(variable *parent[2], tensor *res);
typedef void (*operator_backprop)(variable *);

/* clang-format off */
struct _variable {
    // datas
    tensor                  *tens;
    tensor                  *gradient;

    // operation
    variable                *parents[2];
    operator_func           operation;
    operator_backprop       backprop;

    // extra debugging stuff
    // char                    *label;

    // memory stuff
    size_t                  glb_idx;
};
/* clang-format on */

typedef struct {
    size_t length;
    variable *sorted_operations[];
} oa;

#define FLEXALIGNOF(type) (sizeof(type) & ~(sizeof(type) - 1))
#define FLEXSIZEOF(type, member, n)                                            \
    ((offsetof(type, member) + FLEXALIGNOF(type) - 1 + (n)) &                  \
     ~(FLEXALIGNOF(type) - 1))

tensor *create_tensor(int *ndim_shape, int ndim);
variable *create_variable(int *ndim_shape, int ndim, bool requires_grad);
void free_tensor(tensor *tens);
void free_variable(variable *var);

void tensor_print(tensor *tens);
void tensor_transpose(tensor *tens);

variable *tensor_mm(variable *tens1, variable *tens2);

variable *tensor_add(variable *tens1, variable *tens2);
variable *tensor_sub(variable *tens1, variable *tens2);
variable *tensor_hm(variable *tens1, variable *tens2);

variable *tensor_sigmoid(variable *tens);
variable *tensor_square(variable *tens);

// loss function
variable *tensor_mse(variable *tens1, variable *tens2);

void random_initialisation(variable *mat);
void zero_init(variable *mat);

oa *topo_sort(variable *root);

void backward_diff(oa *sorted_ops);
void resolve_expression(oa *sorted_ops);

#endif // !TENSOR_H
