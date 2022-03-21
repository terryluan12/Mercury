CC=gcc
CFLAGS=-g
all: server client
server: server.c
		gcc -pthread -o server server.c helper.c
client: client.c
		gcc -o client client.c commands.c helper.c
clean:
	rm -f *.o server client
