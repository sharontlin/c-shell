GCC = gcc -g

all: mainshell.o
	$(GCC) -o mainshell.out mainshell.o

mainshell.o: mainshell.c
	$(GCC) -c mainshell.c

run: mainshell.out
	./mainshell.out

clean:
	rm -f *~ *.out *.o *.txt
