all:
	gcc -Wall server.c packet.c -o server
	gcc -Wall client.c packet.c -o client