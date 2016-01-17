# Blackbird Bitcoin Arbitrage Makefile

CC = g++
CFLAGS = -c -g -Wall -O2

EXEC = blackbird
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: EXEC

EXEC: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) -lcrypto -ljansson -lcurl -lmysqlclient -g

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clearscreen:
	clear

clean:
	rm -rf core $(OBJECTS)
