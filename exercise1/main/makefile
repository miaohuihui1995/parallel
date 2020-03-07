all: run

clean:
	-rm run main.o

run: main.o
	gcc -o run main.o -lm -fopenmp

main.o: main.c
	gcc -c -fopenmp main.c -lm -Wall -Werror
