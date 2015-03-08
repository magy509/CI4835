all: client server

server: server.o
	gcc `pkg-config --cflags --libs glib-2.0` -o server server.o -pthread

client: client.o 
	gcc client.o -o client -pthread

server.o: server.c server.h
	gcc `pkg-config --cflags --libs glib-2.0` -c server.c server.h

client.o: client.c
	gcc client.c -c  

clean:
	rm -f *.o client server
