all: myELF

myELF: myELF.o
	gcc -g -Wall -m64 -o myELF myELF.o
myELF.o: myELF.c
	gcc -g -Wall -m64 -c -o myELF.o myELF.c
clean:
	rm myELF myELF.o