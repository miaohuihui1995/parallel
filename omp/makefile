all: run0 run1 run2
.PHONY: all

run0: ex0.o
	gcc -o $@ $< -fopenmp

run1: ex1.o
	gcc -o $@ $< -fopenmp

run2: ex2.o
	gcc -o $@ $< -fopenmp

%.o: %.c
	gcc -c $< -fopenmp

.PHONY: clean
clean:
	-$(RM) run0 run1 run2 *.o
