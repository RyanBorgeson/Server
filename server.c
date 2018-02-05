#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"


void on_requestFile(struct Packet packet, int sockfd, struct sockaddr_in addr);

int main(int argc, char **argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int port = 12000;

 	// Get the port to listen on.
  	printf("Listen on port: ");
  	scanf("%d", &port);

  	// Input validation
  	if (port < 10000 || port > 40000) {
  		printf("Specified port was invalid. (Ex. 10000 - 40000)\n");
  		exit(0);
	}

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	struct sockaddr_in serveraddr,clientaddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	// Wait for a message from the client.
	while(1) {
		int len = sizeof(clientaddr);
		char line[sizeof(struct Packet)];
		int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&clientaddr, &len);

		// Either timeout or print out the client's message.
		if (n == -1) {
			printf("Timed out while waiting to receive\n");
		} else {
			// Copy retrieved packet into packet structure.
			struct Packet pk;
			memcpy(&pk, line, sizeof pk);

			// Determine the type of the packet.
			switch((int)pk.type) {
				case ACK:
				break;
				case FILE_REQUEST:
					on_requestFile(pk, sockfd, clientaddr);
					break;
			}

	
		}

	}

	return 0;
	
}

void on_requestFile(struct Packet packet, int sockfd, struct sockaddr_in addr) {

	printf("Retrieve file: %s\n", packet.data);

	FILE *f = fopen(packet.data, "rb");
	struct Packet window[WINDOW_SIZE];
	int currentPacketID = 0;

	// If file does not exist, send error message.
	if (f == NULL) {
		char hex[sizeof(struct Packet)];
		struct Packet errorPacket = (const struct Packet) { 0 };
		errorPacket.type = ERROR;
		strcpy(errorPacket.data, "Files does not exist.");
		memcpy(hex, &errorPacket, sizeof(struct Packet));
		sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
	} else {

		long size, totalPackets;


		// Get file size.
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		rewind(f);

		totalPackets = (size / WINDOW_DATA_SIZE) + 1;

		// Break file into packets.
		int windowIndex = 0;

		while (currentPacketID < totalPackets) {

			// Check for an acknowledgement from the client.
			if (windowIndex == WINDOW_SIZE - 1 || currentPacketID == totalPackets - 1) {

				window[windowIndex] = (const struct Packet) { 0 };
				window[windowIndex].id = currentPacketID;
				window[windowIndex].totalPackets = totalPackets;
				window[windowIndex].totalBytes = size;
				window[windowIndex].type = FILE_RESPONSE;

				fseek(f, currentPacketID * WINDOW_DATA_SIZE, SEEK_SET);
				fread(window[windowIndex].data, 1, WINDOW_DATA_SIZE, f);
				window[windowIndex].checksum = (unsigned char)checksum(window[windowIndex]);

				rewind(f);

				

				if (!on_receiveAcknowledgement(sockfd, addr, windowIndex + 1, &window))
				{
				}
				int c = 0;
				for (c = 0; c < WINDOW_SIZE; c++) {
					window[c] = (const struct Packet) { 0 };
				}
				windowIndex = 0;

			} else {

				printf("Send Packet: %d\n", currentPacketID);
				window[windowIndex] = (const struct Packet) { 0 };
				window[windowIndex].id = currentPacketID;
				window[windowIndex].totalPackets = totalPackets;
				window[windowIndex].totalBytes = size;
				window[windowIndex].type = FILE_RESPONSE;

				fseek(f, currentPacketID * WINDOW_DATA_SIZE, SEEK_SET);
				fread(window[windowIndex].data, 1, WINDOW_DATA_SIZE, f);
				window[windowIndex].checksum = (unsigned char)checksum(window[windowIndex]);

				windowIndex++;

				rewind(f);
			}
			currentPacketID++;
		}
		fclose(f);
	}
}

int on_receiveAcknowledgement(int sockfd, struct sockaddr_in addr, int expected, struct Packet * window) {
	struct Packet receivedPackets[expected];

	int count = 0;

	// Set each packet to have ID 0
	int p = 0;
	for (p = 0; p < expected; p++) {
		receivedPackets[p] = (const struct Packet) { 0 };
	}

	// Send each packet and check for an ack at the en.
	for (p = 0; p < WINDOW_SIZE; p++) {
		printf("Attempting to send: %d Checksum: %#x\n", window[p].id, window[p].checksum);
		char hex[sizeof(struct Packet)];
		memcpy(hex, &window[p], sizeof(struct Packet));
		sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
	}

	// Wait for a response from the user.
	while(count < expected) {
		int len = sizeof(addr);
		char line[sizeof(struct Packet)];
		int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&addr, &len);

		if (n == -1) {
			// Send each packet and check for an ack at the end.
			int p;
			for (p = 0; p < WINDOW_SIZE; p++) {
				printf("Attempting to send: %d Checksum: %#x\n", window[p].id, window[p].checksum);
				char hex[sizeof(struct Packet)];
				memcpy(hex, &window[p], sizeof(struct Packet));
				sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
			}

		} else {
			// Copy retrieved packet into packet structure.
			struct Packet pk;
			memcpy(&pk, line, sizeof(struct Packet));

			if (!packetExists(&receivedPackets, pk, expected)) {
				receivedPackets[count] = pk;
				printf("Packet ACK received %d\n", pk.id);
				count++;
			}
		}
	}
	return -1;
}
