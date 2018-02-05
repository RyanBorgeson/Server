#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NONE					0
#define ACK						4
#define FILE_REQUEST 			1
#define FILE_RESPONSE 			2
#define ERROR					3

#define CHECKSUM_SIZE   		15

#define WINDOW_SIZE 			5
#define WINDOW_DATA_SIZE		1024


struct Packet {
	uint16_t id;
	uint16_t type;
	uint16_t totalPackets;
	int totalBytes;
	char data[1024];
	unsigned char checksum;
};

int comparePackets(const void * p1, const void * p2);

unsigned char checksum(struct Packet packet);

int isValidPacket(struct Packet p);