/**
 * Client - Client side that allows the user to connect
 * to the server and retrieve a file ensuring the each packet
 * is received and not corrupt.
 * @author Ryan Borgeson
 * @date 2/5/2018
 */
#include "client.h"

int main(int argc, char **argv) {

	/* Socket file descriptor. */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Packet counter. */
	int packetCount = 0;
	/* Server information. */
	struct sockaddr_in serveraddr;
	/* Socket timeout options. */
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 250;
	/* List of received packet structures. */
	Packet * packetsList;
	/* Server's IP. */
  	char ip[50];
  	/* Requested file name. */
  	char filename[100];
  	/* New file name. */
  	char newFileName[100];
  	/* Server port. */
  	int port;


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

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ip);

	// Setup socket options with time out.
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	// Create packet
	Packet fileRequestPacket = createPacket(1, FILE_REQUEST, 1, 0, filename);
	
	// Copy and send packet for file request.
	sendPacket(fileRequestPacket, sockfd, serveraddr);

	while(1) {
		int len = sizeof(serveraddr);
		char byteStream[sizeof(Packet)];
		int n = recvfrom(sockfd, byteStream, sizeof(Packet), 0, (struct sockaddr*)&serveraddr, &len);
	
		// Determine if there was a time out.
		if (n == -1) {
			// In case the inital packet didn't send, send it again.
			if (packetCount == 0) {
				sendPacket(fileRequestPacket, sockfd, serveraddr);
			}
		} else {
			Packet rPacket;
			memcpy(&rPacket, byteStream, sizeof(Packet));

			// If an error ocurred, let the user know.
			if (rPacket.type == ERROR) {
				printf("%s\n", rPacket.data);
				break;
			}

			// Check to see if packets list is empty.
			if (packetsList == NULL) {
				packetsList = malloc(rPacket.totalPackets * sizeof(Packet));
			}

			// If the packets list exists.
			if (packetsList != NULL) {
				if (rPacket.type == FILE_RESPONSE) {
					if (!packetExists(packetsList, rPacket, packetCount)) {
						if (isValidPacket(rPacket)) {
							packetsList[packetCount] = rPacket;
							sendAck(sockfd, serveraddr, rPacket.id);
							printf("Received Valid Packet: %d\n", rPacket.id);
							packetCount++;
						}
					} else {
						sendAck(sockfd, serveraddr, rPacket.id);
					}
				}
				// If all packets have been received, save the file.
				if (packetCount >= rPacket.totalPackets && rPacket.type == FILE_RESPONSE) {
					saveFile(packetsList, newFileName);
					close(sockfd);
					break;
				}
			}
		}
	}

	free(packetsList);
	return 0;
}

void sendAck(int sockfd, struct sockaddr_in addr, int id) {
	// Create and send acknowledgement.
	char data[] = "ack";
	sendPacket(createPacket(id, ACK, 1, 0, data), sockfd, addr);
}

void saveFile(Packet * packets, char * newFileName) {
	printf("Saving to file...\n");
	int totalPackets = packets[0].totalPackets, i;
	qsort(packets, totalPackets, sizeof(Packet), comparePackets);
	
	FILE *f = fopen(newFileName, "ab");

	for (i = 0; i < totalPackets; i++) {
		// The last packet may contain less than 1024 bytes worth of data.
		if (packets[i].id == totalPackets - 1) {
			fwrite(packets[i].data, PACKET_DATA_SIZE - ((totalPackets * PACKET_DATA_SIZE) - packets[i].totalBytes), 1, f);
		} else {
			fwrite(packets[i].data, PACKET_DATA_SIZE, 1, f);
		}
	}
	fclose(f);
}