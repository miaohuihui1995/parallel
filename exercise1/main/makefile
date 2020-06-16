all: ma ck pr
.PHONY: all
CC := gcc
CFLAGS := -o
COMMANDS := -fopenmp -lm -Wall -Werror
EXEC := ma ck pr

ma: main.o
	$(CC) $(CFLAGS) $@ $< $(COMMANDS)

ck: check.o
	$(CC) $(CFLAGS) $@ $< $(COMMANDS)

pr: printdata.o
	$(CC) $(CFLAGS) $@ $< $(COMMANDS)

%.o: %.c
	$(CC) -c $< $(COMMANDS)

.PHONY: clean cleanobj cleandata
clean:
	-$(RM) $(EXEC) *.o
cleanobj:
	-$(RM) *.o
cleandata:
	-$(RM) *.bin
