all: client server

server: server.o
	gcc server.o -o server -pthread

client: client.o 
	gcc client.o -o client -pthread

client.o: client.c
	gcc client.c -c  

server.o: server.c
	gcc server.c -c

clean:
	rm -f *.o client server