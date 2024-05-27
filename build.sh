#!/bin/bash

# Compile each source file to an object file
gcc -c mat/mat.c -o mat.o -g
gcc -c mat/mat_io.c -o mat_io.o -g
gcc -c utils.c -o utils.o -g
gcc -c shallow_nn_train.c -o shallow_nn_train.o -g
gcc -c shallow_nn_cost.c -o shallow_nn_cost.o -g
gcc -c shallow_nn_vect.c -o shallow_nn_vect.o -g

# Link all object files into the final executable
gcc mat.o mat_io.o utils.o shallow_nn_train.o shallow_nn_cost.o shallow_nn_vect.o -o shallow_nn_vect.out -lm -g

# Cleanup
rm *.o
