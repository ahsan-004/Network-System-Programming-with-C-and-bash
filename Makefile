CC= gcc
CFLAGS= -Wall

all: client server

client: client.c
	$(CC) $(CFLAGS) client.c -o client 
server: server.c
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm -rf *.o client server
install:
	cp hims.sh /etc/init.d/hims.sh