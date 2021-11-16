all:
	gcc -Wall -c common.c
	gcc -Wall -c list.c
	gcc -Wall client.c common.o list.o -o client
	gcc -Wall server.c common.o list.o -o server

clean:
	rm common.o client server server-mt
