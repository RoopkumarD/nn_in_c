gcc -c mat.c -o mat.o
gcc -c utils.c -o utils.o -lm
gcc -c shallow_nn.c -o shallow_nn.o

gcc mat.o shallow_nn.o utils.o -o shallow_nn.out -lm

rm *.o
