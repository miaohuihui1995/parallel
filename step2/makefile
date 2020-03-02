all: run

clean:
	-rm run step2.o

run: step2.o
	gcc -o run step2.o -lm

step2.o: step2.c
	gcc -c step2.c -lm -Wall -Werror
