all: run

clean:
	-rm run step1.o

run: step1.o
	gcc -o run step1.o

step1.o: step1.c
	gcc -c step1.c -Wall -Werror
