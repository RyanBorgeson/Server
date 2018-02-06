/**
 * Packet.c
 * Methods for handling packets such as creating packets,
 * calculating checksums, and checking for valid packets.
 * @author Ryan Borgeson
 * @date 2/5/2018
 */
#include "packet.h"

int comparePackets(const void * p1, const void * p2) {
	Packet *a = (Packet *)p1;
	Packet *b = (Packet *)p2;
	if (a->id < b->id)
		return -1;
	else if (a->id > b->id)
		return 1;
	else
		return 0;
}

unsigned char checksum(Packet packet) {
	unsigned char *ptr = (unsigned char*)&packet;
	int i;
	unsigned char checksum;

	for (i = 0, checksum = 0; i < sizeof(Packet) - 4; i++) {
		checksum += ptr[i];
	}

	return checksum;
}

int isValidPacket(Packet p) {
	return checksum(p) == p.checksum && p.type != NONE;
}

int packetExists(Packet * list, Packet packet, int count)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (list[i].id == packet.id && list[i].type != NONE)
			return 1;
	}
	return 0;
}


Packet createPacket(uint16_t id, uint16_t type, uint16_t totalPackets, int totalBytes, char * data) {
	struct Packet packet = (const struct Packet) { 0 };
	packet.id = id;
	packet.type = type;
	packet.totalPackets = totalPackets;
	packet.totalBytes = totalBytes;
	memcpy(packet.data, data, PACKET_DATA_SIZE);
	packet.checksum = (unsigned char)checksum(packet);
	return packet;
}

void sendPacket(Packet packet, int sockfd, struct sockaddr_in addr) {
	unsigned char *payload = (unsigned char*)&packet;
	sendto(sockfd, payload, sizeof(Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}

void clearWindow(Packet * packets, int length) {
	int c = 0;
	for (c = 0; c < length; c++) {
		packets[c] = (const Packet) { 0 };
	}
}