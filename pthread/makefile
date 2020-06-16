all: run
.PHONY: all
CC := gcc

run: ex7.o
	$(CC) -o $@ $< -lpthread

%.o: %.c
	$(CC) -c $< -lpthread

.PHONY: clean
clean:
	-$(RM) run *.o *.log
