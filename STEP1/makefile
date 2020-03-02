all: run

clean:
	-rm run STEP1.o

run: STEP1.o
	gcc -o run STEP1.o

STEP1.o: STEP1.c
	gcc -c STEP1.c -Wall -Werror
