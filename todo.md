- Fix the segev and understand why cost fn is giving low value whereas it should
  have given alot cost value as most of the final activation value is 1.

- check if there a way to include stack trace when error occurs

- Lastly implement mnist in c and implement all the methods of the textbook

- Search "Neural Network in C" in github and check how did they do all those
  - https://github.com/arpitingle/mnist-web
  - https://github.com/100/Cranium
  - https://github.com/HIPS/Probabilistic-Backpropagation
    See autograd and automatic differentiation
  - https://github.com/save-buffer/gigagrad
  - https://github.com/pjreddie/dl-hw0
  - Search deep learning with language:c filter to see gems
  - read c-api section of https://numpy.org/doc/stable/user/basics.html

- Check for error handling in c, check if there any functions like
  python has where throwing error will also include which line error has been thrown
