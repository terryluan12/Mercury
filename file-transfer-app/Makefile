CC=gcc
CFLAGS=-o

DIRS = src
TARGET=target


all: $(TARGET) server client

$(TARGET): 
		mkdir -p $(TARGET)
server: $(DIRS)/server.c
		$(CC) $(CFLAGS) $(TARGET)/$@ $?
client: $(DIRS)/client.c
		$(CC) $(CFLAGS) $(TARGET)/$@ $?
clean:
	rm -f $(TARGET)
