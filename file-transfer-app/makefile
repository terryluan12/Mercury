CC=gcc
CFLAGS=-g
all: server deliver
server: server/server.c header.h
		gcc server/server.c -o server.exe
deliver: client/client.c header.h
		gcc client/client.c -o deliver.exe
clean:
	rm -f server.exe deliver.exe
