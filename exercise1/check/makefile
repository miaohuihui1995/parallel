all: run

clean:
	-rm run check.o

run: check.o
	gcc -o run check.o -lm

check.o: check.c
	gcc -c check.c -lm -Wall -Werror
