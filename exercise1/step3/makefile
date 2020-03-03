all: run

clean:
	-rm run step3.o

run: step3.o
	gcc -o run step3.o

step3.o: step3.c
	gcc -c step3.c -Wall -Werror
