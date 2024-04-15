CC=g++
CFLAGS=-Wall -std=c++98

all: client server

client: client.o
	$(CC) $(CFLAGS) -o client client.o

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

clean:
	rm -f *.o client server

fclean: clean
	rm -f client server

re: fclean all
