CC=g++
CFLAGS=-g -Wall
LDFLAGS=

BINARY_NAME=inverseMapping

CFLAGS  += `pkg-config --cflags opencv`
LDFLAGS += `pkg-config --libs opencv`


all:
	$(CC) $(CFLAGS) inverseMapping.cpp -o $(BINARY_NAME) $(LDFLAGS)


clean:
	-rm -rf $(BINARY_NAME)
