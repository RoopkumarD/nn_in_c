- To do these first:
  - Understand about memory fragmentation, caching, etc and how it affects performance as
    i thought of [this](https://chatgpt.com/share/66b07a73-2eb4-42b7-b208-81dd11b16ed4)
    way to define Matrix struct. Here, i am using flexible array whose length is known
    at run time. Understand it's implication and tradeoff between this and double pointer
    data.

  - At line 126 of `shallow_nn_train.c`, find faster way to copy whole column to other
    matrix. Also at 166, to copy 1 to last row faster.
    Understand about memcpy in glibc, and then read about articles at hacker news about
    problems and stuff about memcpy.

  - Understand about continuous memory and how it is good, refer glibc reference in
    `library_docs` folder.
    But still isn't malloc giving continous memory, as allocating one after other giving
    continous memory. Check this!!!

  - Implement all methods of textbook


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
