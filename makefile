all:
	gcc -Wall server.c com.c -o server
	gcc -Wall client.c com.c -o client
	gcc -Wall testpacket.c -o test