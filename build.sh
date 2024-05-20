gcc -c mat.c -o mat.o -g
gcc -c utils.c -o utils.o -g
gcc -c shallow_nn_vect.c -o shallow_nn_vect.o -g
gcc -c shallow_nn.c -o shallow_nn.o -g

gcc mat.o shallow_nn.o utils.o -o shallow_nn.out -lm -g
gcc mat.o shallow_nn_vect.o utils.o -o shallow_nn_vect.out -lm -g

rm *.o
