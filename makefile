LDFLAGS = -ggdb `pkg-config --libs glib-2.0` -pthread
CFLAGS = -ggdb `pkg-config --cflags glib-2.0` -pthread

all: client server

server: server.o
	gcc -o $@ server.o $(LDFLAGS)

client: client.o 
	gcc -o $@ client.o $(LDFLAGS)

server.o: server.c server.h comando.h
	gcc -c server.c $(CFLAGS)

client.o: client.c comando.h
	gcc -c client.c $(CFLAGS)

clean:
	rm -f ./*.o client server
