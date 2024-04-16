gcc -c chatmat.c -o chatmat.o
# gcc -c use.c -o use.o
gcc -c shallow_nn.c -o shallow_nn.o

# gcc chatmat.o use.o -o use_lib.out
gcc chatmat.o shallow_nn.o -o shallow_nn.out

rm *.o
