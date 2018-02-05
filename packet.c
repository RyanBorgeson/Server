
#include "packet.h"

int comparePackets(const void * p1, const void * p2) {
	struct Packet *a = (struct Packet *)p1;
	struct Packet *b = (struct Packet *)p2;
	if (a->id < b->id)
		return -1;
	else if (a->id > b->id)
		return 1;
	else
		return 0;
}

unsigned char checksum(struct Packet packet) {
	unsigned char *ptr = (unsigned char*)&packet;
	int i;
	unsigned char checksum;

	for (i = 0, checksum = 0; i < sizeof(struct Packet) - 4; i++) {
		checksum += ptr[i];
	}

	return checksum;
}

int isValidPacket(struct Packet p) {
	return checksum(p) == p.checksum && p.type != NONE;
}

int packetExists(struct Packet * list, struct Packet packet, int count)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (list[i].id == packet.id && list[i].type != NONE)
			return 1;
	}
	return 0;
}