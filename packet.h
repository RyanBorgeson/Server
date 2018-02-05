/**
 * Packet.c
 * Methods for handling packets such as creating packets,
 * calculating checksums, and checking for valid packets.
 * @author Ryan Borgeson
 * @date 2/5/18
 */

#include <netinet/in.h>

#define NONE					0
#define FILE_REQUEST 			1
#define FILE_RESPONSE 			2
#define ERROR					3
#define ACK						4

#define WINDOW_SIZE 			5
#define PACKET_DATA_SIZE		1024

/* Packet structure */
typedef struct Packet {
	uint16_t id;
	uint16_t type;
	uint16_t totalPackets;
	int totalBytes;
	char data[1024];
	unsigned char checksum;
} Packet;

/**
 * Compare Packets - Compares two packets against each other
 * based on their ID. Used to sort a list of packets.
 * @param p1 First packet.
 * @param p2 Second packet.
 * @return 
 */
int comparePackets(const void * p1, const void * p2);

/**
 * Checksum - Computes the checksum of the provided packet
 * making sure to ignore the original checksum value if one
 * already exists.
 * @param packet Packet to calculate checksum on.
 * @return Calculated checksum value.
 */
unsigned char checksum(struct Packet packet);

/**
 * Is valid packet - Determines if the specified packet
 * is valid by re-calculating the checksum of the packet
 * and comparing it to the origin checksum value.
 * @param p Packet to check.
 */
int isValidPacket(struct Packet p);

/**
 * Packet exists - Provided a list of packets, the current packet, and
 * the number of packets in the list. Determines if the packet already
 * exists within the list.
 * @param list A list of packets.
 * @param packet Packet to check for existance.
 * @param count The length of the list.
 * @return Returns 1 if packet exists and 0 if it does not.
 */
int packetExists(struct Packet * list, struct Packet packet, int count);

Packet createPacket(uint16_t id, uint16_t type, uint16_t totalPackets, int totalBytes, char * data, unsigned char checksum);