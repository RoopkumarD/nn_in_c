gcc -c chatmat.c -o chatmat.o
gcc -c utils.c -o utils.o -lm
gcc -c shallow_nn.c -o shallow_nn.o

gcc chatmat.o shallow_nn.o utils.o -o shallow_nn.out -lm

rm *.o
