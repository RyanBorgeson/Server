#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "com.h"

int processReceivedPackets(struct Packet * recv, int currentPacket, char * newFileName);

void on_missedPacket(int missedId, int sockfd, struct sockaddr_in addr);
void sendAck(int sockfd, struct sockaddr_in addr, int id);



int main(int argc, char **argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	char ip[50] = "10.0.0.2";
  	int port = 12000;
  	char filename[100] = "home.png",
  		 newFileName[100] = "home2.png";


  	// Get port and IP of server.
  	/*printf("Enter Server IP: ");
  	scanf("%s", &ip);
 	printf("Enter Port: ");
 	scanf("%d", &port);
 	printf("Retrieve File: ");
 	scanf("%s", &filename);
 	printf("New File Name: ");
 	scanf("%s", &newFileName);
  	getchar();*/

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
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.id = 1;
	pk.type = FILE_REQUEST;
	strcpy(pk.data, filename);
	strcpy(pk.checksum, "");
	
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
			printf("Timed out while receiving.");
		} else {
			struct Packet pk;
			memcpy(&pk, line, sizeof pk);

			// Check to see if packets list is empty.
			if (packetsList == NULL)
			{
				packetsList = malloc(pk.totalPackets * sizeof(struct Packet));
			}

			if (pk.type == FILE_RESPONSE) {
				struct Packet newPacket;
				newPacket.id = packetCount;
				newPacket.type = pk.type;
				newPacket.totalPackets = pk.totalPackets;
				strcpy(newPacket.data, pk.data);
				strcpy(newPacket.checksum, pk.checksum);

				if (!packetExists(packetsList, pk, packetCount)) {
					packetsList[packetCount] = pk;
					sendAck(sockfd, serveraddr, pk.id);
					packetCount++;
					printf("Adding Packet: %d\n", pk.id);
				} else {
					printf("Not adding: %d\n", pk.id);
				}
			}

			if (packetCount >= pk.totalPackets) {
			int x = 0;

				for (x = 0; x < pk.totalPackets; x++)
				{

					printf("ID: %d  - Type: %d\n", packetsList[x].id, packetsList[x].type);
				}

				break;
				close(sockfd);
			}


		}

	}

	free(packetsList);

	return 0;
}


int packetExists(struct Packet * list, struct Packet packet, int count)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (list[i].id == packet.id)
			return 1;
	}
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
