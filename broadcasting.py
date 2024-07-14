import numpy as np

np.random.seed(42)

broadcasted_dim = [4, 2, 3, 3]
stride = [0, 0, 0, 1]
for i in range(2, -1, -1):
    stride[i] = stride[i + 1] * broadcasted_dim[i + 1]

a = np.random.randn(1, 3, 3)
a_flattened = a.flatten()
a_new_shape = list(a.shape)
for i in range(len(broadcasted_dim) - len(a.shape)):  # first adding filler 1
    a_new_shape.insert(0, 1)

print("Original Tensor:")
print(a)
print("a shape ->", a.shape)
print("a new shape ->", a_new_shape)
print("")

print("Broadcasted Result:\n")
# for i in range(broadcasted_dim[0]):
#     for j in range(broadcasted_dim[1]):
#         print(a[i, j])

loop_ndim = 2
loop_shape = [broadcasted_dim[0], broadcasted_dim[1]]
loop_stride = [loop_shape[1], 1]
total_loop_elem = loop_shape[0] * loop_stride[0]

for i in range(total_loop_elem):
    curr_loop_idx = [0] * loop_ndim
    temp = i
    for j in range(loop_ndim):
        curr_loop_idx[j] = temp // loop_stride[j]
        temp %= loop_stride[j]

    end = 0
    for j in range(2):
        end += a_new_shape[-j] * stride[-j]

    a_flattened_final = 0
    a_final = [0] * loop_ndim
    for j in range(loop_ndim):
        a_flattened_final += curr_loop_idx[j] * stride[j] if a_new_shape[j] != 1 else 0
        a_final[j] = 0 if a_new_shape[j] == 1 else curr_loop_idx[j]

    print(curr_loop_idx, a_final)
    print("--------------------------")
    print("a", a[a_final])  # won't give correst result as there isn't extra dimension
    # to a. but it doesn't matter as bellow one is what i need
    print("--------------------------")
    print("a_flattened", a_flattened[a_flattened_final : a_flattened_final + end])
    print("--------------------------")

print(a[0])
