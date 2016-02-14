all: server.o client.o

server.o: server.c
	gcc -Wall -o server server.c -lpthread

client.o: client.c
	gcc -Wall -o client client.c -lpthread

clean:
	rm *.o server client