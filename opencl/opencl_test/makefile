all: run
.PHONY: all
CC := g++

run: main.o
	$(CC) -o $@ $< -lOpenCL -lm -Wall -Werror

%.o: %.c
	$(CC) -c $< -lOpenCL -lm -Wall -Werror

.PHONY: work
work:
	./run

.PHONY: clean
clean:
	-$(RM) run ocl_log.txt *.o
