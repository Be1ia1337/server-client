all:
	gcc client.c -o client
	gcc -pthread server.c -o server
