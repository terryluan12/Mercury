CC=gcc
CFLAGS=-pthread -o 
DIRS = src
TARGET=target

HELP_FILES=src/*_helper.c
all: target server client

$(TARGET): 
		mkdir -p $(TARGET)
server: $(DIRS)/server.c
		$(CC) $(CFLAGS) $(TARGET)/$@.exe $? $(HELP_FILES) 
client: $(DIRS)/client.c
		$(CC) $(CFLAGS) $(TARGET)/$@.exe $? $(HELP_FILES)
clean:
	rm -f $(TARGET)
