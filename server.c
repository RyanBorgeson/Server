#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "com.h"


void on_requestFile(struct Packet packet, int sockfd, struct sockaddr_in addr);
void on_responseFile();

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
			switch(pk.type) {
				case ACK:
				break;
				case FILE_REQUEST:
					on_requestFile(pk, sockfd, clientaddr);
					break;
				case FILE_RESPONSE:
					on_responseFile();
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

		while (currentPacketID < totalPackets) {
			// Break file into packets.
			int windowIndex = currentPacketID % WINDOW_SIZE;

			// Check for an acknowledgement from the client.
			if (currentPacketID % WINDOW_SIZE == WINDOW_SIZE - 1 || currentPacketID == totalPackets - 1) {

				window[windowIndex] = (const struct Packet) { 0 };
				window[windowIndex].id = currentPacketID;
				window[windowIndex].totalPackets = totalPackets;
				window[windowIndex].totalBytes = size;
				window[windowIndex].type = FILE_RESPONSE;
				printf("Packet: %d - ack\n", currentPacketID);

				fseek(f, currentPacketID * WINDOW_DATA_SIZE, SEEK_SET);
				fread(window[windowIndex].data, 1, WINDOW_DATA_SIZE, f);

				rewind(f);

				// Send each packet and check for an ack at the end.
				for (int p = 0; p < WINDOW_SIZE; p++) {
					char hex[sizeof(struct Packet)];
					memcpy(hex, &window[p], sizeof(struct Packet));
					sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
				}

				// Wait for a response from the user.
				while(1) {
					int len = sizeof(addr);
					char line[sizeof(struct Packet)];
					int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&addr, &len);

					// Either timeout or print out the client's message.
					if (n == -1) {
						printf("Timed out while waiting to receive\n");
						break;
					} else {
						// Copy retrieved packet into packet structure.
						struct Packet pk;
						memcpy(&pk, line, sizeof(struct Packet));

						printf("packet received %d %d\n", pk.type, strlen(pk.data));
						// Check for any errors or missed frames.
						if (strlen(pk.data) > 0) {
							// There was an error so clear the window
							// an place back at the start of the missed frame.
							for (int x = 0; x < WINDOW_SIZE; x++)
								window[x] = (const struct Packet) { 0 };

							windowIndex = 0;
							currentPacketID = atoi(pk.data);
						} else {
							break;	
						}
					}

				}

			} else {

				//if (currentPacketID != 1) {
				printf("Packet: %d\n", currentPacketID);
				window[windowIndex] = (const struct Packet) { 0 };
				window[windowIndex].id = currentPacketID;
				window[windowIndex].totalPackets = totalPackets;
				window[windowIndex].totalBytes = size;
				window[windowIndex].type = FILE_RESPONSE;
				fseek(f, currentPacketID * WINDOW_DATA_SIZE, SEEK_SET);
				fread(window[windowIndex].data, 1, WINDOW_DATA_SIZE, f);
				//}

				rewind(f);
			}
			currentPacketID++;
		}
		fclose(f);
	}
}

void on_responseFile() {

}