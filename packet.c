
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