all=server_main client_main
CC=g++
CFLAGS=-std=c++11
FLAG=-lhiredis -levent -lpthread -Wformat

server:publisher.cpp
	g++ $(CFLAGS) $(FLAG) $^ -o $@
client:subscriber.cpp
	g++ $(CFLAGS) $(FLAG) $^ -o $@
clean:
	rm server client
