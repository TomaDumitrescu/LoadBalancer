CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server

.PHONY: build clean

build: ./app

load_balancer:
	gcc *.h *.c -o app

clean:
	rm -f *.o app *.h.gch
