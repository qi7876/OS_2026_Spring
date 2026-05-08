CC = gcc
CFLAGS = -Wall -Wextra -std=c11

.PHONY: all clean

all: a b

a: a.c
	$(CC) $(CFLAGS) a.c -o a

b: b.c
	$(CC) $(CFLAGS) b.c -o b

clean:
	rm -f a b
