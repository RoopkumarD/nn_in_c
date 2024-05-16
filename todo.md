- Separate all functions like training and stuff in another function

- About matrix multiplication with transpose
```c
// using ternary operator exp1 ? exp2 : exp3
// if exp1 true then exp2 else exp3
// need to see which one is faster
mat1->data[(m1 == 0) ? (i * mat1->cols + k) : (i + k * mat1->rows)] *
mat2->data[(m2 == 0) ? (k * mat2->cols + j) : (k + j * mat2->rows)];
```
is above faster than the integer operation or i should create external
if loop and let multiplication do it's thing
Need to look underneath at assembly and compare

- Create input .csv file and read from it

- Check for error handling in c, check if there any functions like
  python has where throwing error will also include which line error has been thrown

- Lastly implement mnist in c and implement all the methods of the textbook

- Search "Neural Network in C" in github and check how did they do all those
  - https://github.com/100/Cranium
  - https://github.com/HIPS/Probabilistic-Backpropagation
    See autograd and automatic differentiation
