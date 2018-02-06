/**
 * Server - Handles file and packet management
 * on the server side. Check for exisitng files, divides
 * the file into packets, and sends them to the client.
 * @author Ryan Borgeson
 * @date 2/5/2018
 */
 #include "server.h"

int main(int argc, char **argv) {
	/* Socket file descriptor. */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Client's port. */
	int port;
	/* Server and client address information. */
	struct sockaddr_in serveraddr, clientaddr;
	/* Socket timeout options. */
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 250;

 	// Get the port to listen on.
  	printf("Listen on port: ");
  	scanf("%d", &port);

  	// Input validation
  	if (port < 10000 || port > 40000) {
  		printf("Specified port was invalid. (Ex. 10000 - 40000)\n");
  		exit(0);
	}

	// Setup socket options with time out.
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	// Bind to the specified port.
	bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	printf("Waiting...\n");

	// Wait for a message from the client.
	while(1) {
		int len = sizeof(clientaddr);
		char byteStream[sizeof(Packet)];
		int n = recvfrom(sockfd, byteStream, sizeof(Packet), 0, (struct sockaddr*)&clientaddr, &len);

		// Either timeout or print out the client's message.
		if (n != -1) {

			// Copy retrieved packet into packet structure.
			struct Packet rPacket;
			memcpy(&rPacket, byteStream, sizeof(Packet));

			if (rPacket.type == FILE_REQUEST) {
				on_requestFile(rPacket, sockfd, clientaddr);
			}	
		}
	}

	return 0;
}

void on_requestFile(Packet packet, int sockfd, struct sockaddr_in addr) {

	printf("Retrieve file: %s\n", packet.data);

	FILE *f = fopen(packet.data, "rb");
	Packet window[WINDOW_SIZE];
	int currentPacketID = 0;
	char tempData[PACKET_DATA_SIZE];

	// If file does not exist, send error message.
	if (f == NULL) {
		char errorMessage[] = "File does not exist.";
		sendPacket(createPacket(0, ERROR, 1, 0, errorMessage), sockfd, addr);
	} else {

		long size, totalPackets;

		// Get file size.
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		rewind(f);

		totalPackets = (size / PACKET_DATA_SIZE) + 1;

		// Break file into packets.
		int windowIndex = 0;

		while (currentPacketID < totalPackets) {

			// Check for an acknowledgement from the client.
			if (windowIndex == WINDOW_SIZE - 1 || currentPacketID == totalPackets - 1) {

				fseek(f, currentPacketID * PACKET_DATA_SIZE, SEEK_SET);
				fread(tempData, 1, PACKET_DATA_SIZE, f);
				rewind(f);
				window[windowIndex] = createPacket(currentPacketID, FILE_RESPONSE, totalPackets, size, tempData);
				
				// Send window and wait for acknowledgements.
				on_receiveAcknowledgement(sockfd, addr, windowIndex + 1, &window);
				clearWindow(&window, WINDOW_SIZE);
				windowIndex = 0;

			} else {

				fseek(f, currentPacketID * PACKET_DATA_SIZE, SEEK_SET);
				fread(tempData, 1, PACKET_DATA_SIZE, f);
				rewind(f);
				window[windowIndex] = createPacket(currentPacketID, FILE_RESPONSE, totalPackets, size, tempData);

				windowIndex++;

			}
			currentPacketID++;
		}
		fclose(f);
	}
}

int on_receiveAcknowledgement(int sockfd, struct sockaddr_in addr, int expected, Packet * window) {
	Packet receivedPackets[expected];

	int count = 0,
		p = 0,
		attempts = 0;

	// Set each packet to have ID 0
	clearWindow(&receivedPackets, expected);

	// Send each packet and check for an ack at the en.
	for (p = 0; p < expected; p++) {
		printf("Attempting to send: %d Checksum: %#x\n", window[p].id, window[p].checksum);
		sendPacket(window[p], sockfd, addr);
	}

	// Wait for a response from the user.
	while(count < expected) {
		int len = sizeof(addr);
		char byteStream[sizeof(Packet)];
		int n = recvfrom(sockfd, byteStream, sizeof(Packet), 0, (struct sockaddr*)&addr, &len);

		if (n == -1) {
			// Send each packet and check for an ack at the end.
			for (p = 0; p < expected; p++) {
				printf("Attempting to send: %d Checksum: %#x\n", window[p].id, window[p].checksum);
				sendPacket(window[p], sockfd, addr);
			}

			// Only allow so many attempts if the last packet
			// has already been sent.
			if (expected < WINDOW_SIZE) {
				attempts++;
			}

			if (attempts > MAX_ATTEMPTS)
				break;

		} else {
			// Copy retrieved packet into packet structure.
			struct Packet rPacket;
			memcpy(&rPacket, byteStream, sizeof(Packet));

			if (!packetExists(&receivedPackets, rPacket, expected)) {
				receivedPackets[count] = rPacket;
				printf("Packet ACK received %d\n", rPacket.id);
				count++;
			}
		}
	}
	return -1;
}
