all: run

clean:
	-rm run step4.o

run: step4.o
	gcc -o run step4.o

step4.o: step4.c
	gcc -c step4.c -Wall -Werror
