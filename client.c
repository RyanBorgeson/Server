#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"


int processReceivedPackets(struct Packet * recv, int currentPacket, char * newFileName);

void on_missedPacket(int missedId, int sockfd, struct sockaddr_in addr);
void sendAck(int sockfd, struct sockaddr_in addr, int id);

void saveFile(struct Packet * packets, char * newFileName);

int main(int argc, char **argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	char ip[50];
  	int port = 12000;
  	char filename[100],
  		 newFileName[100];


  	// Get port and IP of server.
  	printf("Enter Server IP: ");
  	scanf("%s", &ip);
 	printf("Enter Port: ");
 	scanf("%d", &port);
 	printf("Retrieve File: ");
 	scanf("%s", &filename);
 	printf("New File Name: ");
 	scanf("%s", &newFileName);
  	getchar();

  	// Input validation
  	if (port < 10000 || port > 40000) {
  		printf("Specified port was invalid. (Ex. 10000 - 40000)\n");
  		exit(0);
	}

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ip);

	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.id = 1;
	pk.type = FILE_REQUEST;
	strcpy(pk.data, filename);
	
	// Copy packet structure into a byte array
	// for transmission.
	char hex[sizeof(struct Packet)];
	memcpy(hex, &pk, sizeof pk);
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	// Wait for a response from the server.
	int packetCount = 0;
	struct Packet * packetsList;

	while(1) {
		int len = sizeof(serveraddr);
		char line[sizeof(struct Packet)];
		int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&serveraddr, &len);
	
		// Once a response is received, print out the response and close the socket.
		if (n == -1) {
			printf("Timed out while receiving.\n");

			// Check to see if the request actually sent.
			if (packetCount == 0) {
				sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
			}
		} else {
			struct Packet pk;
			memcpy(&pk, line, sizeof pk);

			// Check to see if packets list is empty.
			if (packetsList == NULL)
			{
				packetsList = malloc(pk.totalPackets * sizeof(struct Packet));
			}
			if (packetsList != NULL) {

				if (pk.type == FILE_RESPONSE) {
					struct Packet newPacket;
					newPacket.id = packetCount;
					newPacket.type = pk.type;
					newPacket.totalPackets = pk.totalPackets;
					strcpy(newPacket.data, pk.data);

					if (!packetExists(packetsList, pk, packetCount)) {
						if (isValidPacket(pk)) {
							packetsList[packetCount] = pk;
							sendAck(sockfd, serveraddr, pk.id);
							packetCount++;
							printf("Received Valid Packet: %d\n", pk.id);
						}
					} else {
						sendAck(sockfd, serveraddr, pk.id);
					}
						
				}

				if (packetCount >= pk.totalPackets && pk.type == FILE_RESPONSE) {
					saveFile(packetsList, newFileName);

					break;
					close(sockfd);
				}
			}

		}

	}

	free(packetsList);

	return 0;
}





void on_missedPacket(int missedId, int sockfd, struct sockaddr_in addr) {
	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.type = ACK;
	sprintf(pk.data, "%d", missedId);
	
	char hex[sizeof(struct Packet)];
	memcpy(hex, &pk, sizeof pk);
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}

void sendAck(int sockfd, struct sockaddr_in addr, int id) {
	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.type = ACK;
	pk.id = id;
	
	char hex[sizeof(struct Packet)];
	memcpy(hex, &pk, sizeof pk);
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}


void saveFile(struct Packet * packets, char * newFileName) {
	int totalPackets = packets[0].totalPackets, i;
	qsort(packets, totalPackets, sizeof(struct Packet), comparePackets);
	
	FILE *f = fopen(newFileName, "ab");

	for (i = 0; i < totalPackets; i++) {
		// Last packet
		if (packets[i].id == totalPackets - 1) {
			fwrite(packets[i].data, WINDOW_DATA_SIZE - ((totalPackets * WINDOW_DATA_SIZE) - packets[i].totalBytes), 1, f);
		} else {
			fwrite(packets[i].data, WINDOW_DATA_SIZE, 1, f);
		}
	}

	fclose(f);
}