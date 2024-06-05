- To do these first:
  - At line 126 of `shallow_nn_train.c`, find faster way to copy whole column to other
    matrix. Also at 166, to copy 1 to last row faster.
    Understand about memcpy in glibc, and then read about articles at hacker news about
    problems and stuff about memcpy.

  - Understand about continuous memory and how it is good, refer glibc reference in
    `library_docs` folder.
    But still isn't malloc giving continous memory, as allocating one after other giving
    continous memory. Check this!!!
    Understand about caching in cpu

  - Implement all methods of textbook

  - Checkout glibc sysdependent implementation of memcpy and also checkout hacker news
    articles regarding it


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
