CC := gcc
CFLAGS := -o
COMMANDS := -lOpenCL -Wall -Werror

run: main.o
	$(CC) $(CFLAGS) $@ $< $(COMMANDS)

%.o: %.c
	$(CC) -c $< $(COMMANDS)

work:
	./run
clean:
	-$(RM) run *.o
