all: run

clean:
	-rm run main.o

run: main.o
	gcc -o $@ $<

%.o: %.c
	gcc -c main.c -Wall -Werror
