CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: scheduler

scheduler: src/main.c
	$(CC) -o scheduler src/main.c $(CFLAGS)

clean:
	rm -f src/*.o scheduler *.out
	
.PHONY: all clean