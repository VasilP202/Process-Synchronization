CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LFLAGS=-lrt -lpthread

all:
	gcc $(CFLAGS) proj2.c -o proj2 $(LFLAGS)

clean:
	rm -f proj2 proj2.out