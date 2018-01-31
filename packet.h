
#define ACK				0
#define FILE_REQUEST 	1
#define FILE_RESPONSE 	2
#define ERROR			3


struct Packet {
	uint16_t id;
	uint16_t type;
	uint16_t totalPackets;
	int totalBytes;
	char data[1024];
	char checksum[10];
};
