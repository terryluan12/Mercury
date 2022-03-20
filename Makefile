CC=gcc
CFLAGS=-g
all: server client
server: ./server/server.c
		gcc -o server ./server/server.c
client: ./client/client.c
		gcc -o client ./client/client.c
clean:
	rm -f *.o server client

TODO FIGURE THIS OUT