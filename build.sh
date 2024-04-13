gcc -c chatmat.c -o chatmat.o
gcc -c use.c -o use.o

gcc chatmat.o use.o -o use_lib.out

rm *.o
