as -o foo.o foo.s
cc -o xo x.c foo.o
cc -o xn -i x.c foo.o
cc -o xoz -z x.c foo.o
cc -o xnz -i -z x.c foo.o
