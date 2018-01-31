#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "com.h"

int processReceivedPackets(struct Packet * recv, int currentPacket, char * newFileName);
int comparePackets(const void * p1, const void * p2);
void on_missedPacket(int missedId, int sockfd, struct sockaddr_in addr);
void on_sendAck(int sockfd, struct sockaddr_in addr);

int main(int argc, char **argv) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	char ip[50] = "127.0.0.1";
  	int port = 12000;
  	char filename[100], newFileName[100];


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
	struct Packet recvPacket[WINDOW_SIZE];

	while(1) {
		int len = sizeof(serveraddr);
		char line[sizeof(struct Packet)];
		int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&serveraddr, &len);
	
		// Once a response is received, print out the response and close the socket.
		if (n != -1) {
			struct Packet pk;
			memcpy(&pk, line, sizeof pk);
			//printf("Packets: %d \t Current: %d \t Data: %s\n", pk.totalPackets, pk.id, pk.data);

			if (strlen(pk.data) > 0) {
				
				//fwrite(pk.data, WINDOW_DATA_SIZE, 1, f);

				recvPacket[packetCount % WINDOW_SIZE] = pk;


				if (packetCount % WINDOW_SIZE == WINDOW_SIZE - 1 || packetCount == pk.totalPackets - 1) {
					int missedPacket = -1;
					if ((missedPacket = processReceivedPackets(recvPacket, packetCount, newFileName)) != packetCount) {
						on_missedPacket(missedPacket, sockfd, serveraddr);

						// Clear window buffer
						for (int x = 0; x < WINDOW_SIZE; x++)
							recvPacket[x] = (const struct Packet) { 0 };

						packetCount = missedPacket;
					} else {
						on_sendAck(sockfd, serveraddr);
					}
				}



				if (packetCount >= pk.totalPackets - 1) {
					break;
					close(sockfd);
				}
				packetCount++;
			}


		}

	}

	return 0;
}

int processReceivedPackets(struct Packet * recv, int currentPacket, char * newFileName) {
	qsort(recv, WINDOW_SIZE, sizeof(struct Packet), comparePackets);
	static int lastId = 0;

	FILE *f = fopen(newFileName, "ab");

	for (int i = 0; i < WINDOW_SIZE; i++) {
		if (recv[i].id > currentPacket - WINDOW_SIZE && recv[i].type == FILE_RESPONSE) {

			//printf("%d %d\n", lastId, recv[i].id);

			if (recv[i].id - lastId > 1)
				return lastId + 1;

			// Last packet
			if (recv[i].id == recv[i].totalPackets - 1) {
				fwrite(recv[i].data, WINDOW_DATA_SIZE - ((recv[i].totalPackets * WINDOW_DATA_SIZE) - recv[i].totalBytes), 1, f);
			} else {
				fwrite(recv[i].data, WINDOW_DATA_SIZE, 1, f);
			}
			lastId = recv[i].id;
			recv[i] = (const struct Packet) { 0 };
		}
	}

	fclose(f);
	return currentPacket;
}

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

void on_missedPacket(int missedId, int sockfd, struct sockaddr_in addr) {
	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.type = ACK;
	sprintf(pk.data, "%d", missedId);
	
	char hex[sizeof(struct Packet)];
	memcpy(hex, &pk, sizeof pk);
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}

void on_sendAck(int sockfd, struct sockaddr_in addr) {
	// Create packet
	struct Packet pk = (const struct Packet) { 0 };
	pk.type = ACK;
	
	char hex[sizeof(struct Packet)];
	memcpy(hex, &pk, sizeof pk);
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}