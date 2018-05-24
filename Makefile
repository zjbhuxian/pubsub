EXE=server_main client_main
CC=g++
CCFLAG=-std=c++11
FLAG=-lhiredis -levent -lpthread -Wformat
OBJ=redis_publisher.o publisher.o redis_subscriber.o subscriber.o

all:$(EXE)

$(EXE):$(OBJ)
	$(CC) -o publisher redis_publisher.o publisher.o $(FLAG)
	$(CC) -o subscriber redis_subscriber.o subscriber.o $(FLAG)

redis_publisher.o:redis_publisher.h
	$(CC) -c redis_publisher.h redis_publisher.cpp $(CCFLAG)
redis_subscriber.o:redis_subscriber.h
	$(CC) -c redis_subscriber.h redis_subscriber.cpp $(CCFLAG)

publisher.o:publisher.cpp
	$(CC) -c publisher.cpp $(CCFLAG)

subscriber.o:subscriber.cpp
	$(CC) -c subscriber.cpp $(CCFLAG)
clean:
	rm publisher subscriber *.o
