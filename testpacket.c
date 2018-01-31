#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"

unsigned char* packetToHex(struct Packet *p_pk);

int main() {


	struct Packet pk;

	pk.id = 1;
	strcpy(pk.data, "Test packet");
	strcpy(pk.checksum, "TESTTESTTE");

	printf("Size: %d\n", sizeof(pk));
	printf("%d\n", &pk);
	printf("Data:\n");

	
	//struct Packet* p_pk = &pk;
	//unsigned char* charPtr = (unsigned char*)p_pk;

	//int i = 0;
	//for (i = 0; i < sizeof(pk); i++) {
	//	printf("%d: %02x\n", i, charPtr[i]);
	//}

	unsigned char* test = packetToHex(&pk);

	printf("%c", test);	

	return 0;
}

unsigned char* packetToHex(struct Packet *p_pk) {
	unsigned char* pp_pk = (unsigned char*)p_pk;
	return pp_pk;
}