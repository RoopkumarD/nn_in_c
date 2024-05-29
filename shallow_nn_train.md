There are currently two functions in `shallow_nn_train.c` file, one is for stochastic
gradient descent and other is normal gradient descent. As you can see that stochastic
gradient descent is not that different from normal gradient descent it's just the
preparation of batch which is going to be trained.

You can also consider it as, in stochastic gradient descent function, we prepare each
batch for training and then call normal gradient descent function with that batch as
`X_train`.

So i will explain the gradient descent function:

First we prepare all the matrix which are needed while training, as since we are using
goto so i can't directly jump after the line where variable is declared. So declaring
everything first.

Then in feedforward of training which is the first loop to get all the activations values
for training, we could have first matrix multiplied weights with input and then added the
bias term later. But when i look at it, i can just create another matrix with name wb
where weights are in usual position, just that bias of each node are at last column of wb.
Thus when we multiply wb * input where input have last row as value 1.

Thus we get whole operation of above at once, thus reducing the function calling overhead
of `matrix_col_add`.

Example:

If weights, bias and input are:

W = [a1, a2
     a3, a4]

B = [b1
     b2]

X = [x1
     x2]

Here, normally:

WX + B = [a1x1 + a2x2 + b1
          a3x1 + a4x2 + b2]

we can do above thing at once if we define:

WB = [a1, a2, b1
      a3, a4, b2]

X = [x1
     x2
     1]

Thus,

WB * X = [a1x1 + a2x2 + b1
          a3x1 + a4x2 + b2]

Here as you have observed that, we have to add ones last row. So that is the overhead i
have to deal with all activations as you see in the code. I can't make it so that
activation(WB * X) results in activation vector with last row have one (if you can please
riase an issue at github repo).

And rest of the training is how we normally train with backpropogation, with just having
wb so rather than having delta_b and delta_w, we combine them at once.
