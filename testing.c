#include "tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void fill_arr(float *mat, int *shape, int ndim) {
    int size = 1;
    for (int i = 0; i < ndim; i++) {
        size *= shape[i];
    }
    for (int i = 0; i < size; i++) {
        mat[i] = i;
    }
    return;
}

int main(void) {
    if (glb_init() == -1) {
        return -1;
    }
    atexit(glb_free);

    int ashape[] = {2, 2, 1, 3};
    int andim = sizeof(ashape) / sizeof(ashape[0]);
    variable *a = create_variable(ashape, andim, true);
    a->label = "a";
    fill_arr(a->tens->data, ashape, andim);

    int bshape[] = {1, 3, 3};
    int bndim = sizeof(bshape) / sizeof(bshape[0]);
    variable *b = create_variable(bshape, bndim, true);
    b->label = "b";
    fill_arr(b->tens->data, bshape, bndim);

    int dshape[] = {1, 3, 4};
    int dndim = sizeof(dshape) / sizeof(dshape[0]);
    variable *d = create_variable(dshape, dndim, true);
    d->label = "d";
    fill_arr(d->tens->data, dshape, dndim);

    int kshape[] = {1, 3, 3};
    int kndim = sizeof(kshape) / sizeof(kshape[0]);
    variable *k = create_variable(kshape, kndim, true);
    k->label = "k";
    fill_arr(k->tens->data, kshape, kndim);

    int nshape[] = {1, 4, 1};
    int nndim = sizeof(nshape) / sizeof(nshape[0]);
    variable *n = create_variable(nshape, nndim, true);
    n->label = "n";
    fill_arr(n->tens->data, nshape, nndim);

    variable *c = tensor_hm(a, b);
    c->label = "c";

    variable *e = tensor_mm(c, d);
    e->label = "e";

    variable *l = tensor_mm(k, e);
    l->label = "l";

    variable *o = tensor_mm(e, n);
    o->label = "o";

    variable *m = tensor_square(l);
    m->label = "m";

    variable *t = tensor_add(m, o);
    t->label = "t";

    // setting result gradient to one
    for (int i = 0; i < t->tens->total_size; i++) {
        t->gradient->data[i] = 1;
    }
    // setting rest of the gradient to zero here than
    // in backprop as need to accumulate
    memset(a->gradient->data, 0, sizeof(float) * a->gradient->total_size);
    memset(b->gradient->data, 0, sizeof(float) * b->gradient->total_size);
    memset(d->gradient->data, 0, sizeof(float) * d->gradient->total_size);
    memset(k->gradient->data, 0, sizeof(float) * k->gradient->total_size);
    memset(n->gradient->data, 0, sizeof(float) * n->gradient->total_size);
    memset(c->gradient->data, 0, sizeof(float) * c->gradient->total_size);
    memset(e->gradient->data, 0, sizeof(float) * e->gradient->total_size);
    memset(o->gradient->data, 0, sizeof(float) * o->gradient->total_size);
    memset(l->gradient->data, 0, sizeof(float) * l->gradient->total_size);
    memset(m->gradient->data, 0, sizeof(float) * m->gradient->total_size);

    oa *sorted_ops = topo_sort(t);
    resolve_expression(sorted_ops);
    backward_diff(sorted_ops);

    memset(k->gradient->data, 0, sizeof(float) * k->gradient->total_size);
    memset(e->gradient->data, 0, sizeof(float) * e->gradient->total_size);
    l->backprop(l);

    printf("gradient tensor: %s\n", a->label);
    tensor_print(a->gradient);
    printf("=======================================\n");
    printf("gradient tensor: %s\n", b->label);
    tensor_print(b->gradient);
    printf("=======================================\n");
    printf("gradient tensor: %s\n", d->label);
    tensor_print(d->gradient);
    printf("=======================================\n");
    printf("gradien tensor: %s\n", k->label);
    tensor_print(k->gradient);
    printf("=======================================\n");
    printf("gradient tensor: %s\n", n->label);
    tensor_print(n->gradient);
    printf("=======================================\n");
    for (size_t i = 0; i < sorted_ops->length; i++) {
        printf("gradient tensor: %s\n",
               sorted_ops->sorted_operations[i]->label);
        tensor_print(sorted_ops->sorted_operations[i]->gradient);
        printf("=======================================\n");
    }

    free(sorted_ops);

    return 0;
}
