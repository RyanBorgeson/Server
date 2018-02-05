all:
	gcc -Wall server.c com.c packet.c -o server
	gcc -Wall client.c com.c packet.c -o client
	gcc -Wall testpacket.c -o test
	gcc -Wall packetlist.c -o packetlist