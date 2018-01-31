#include "com.h"


void sendPacket(int sockfd, struct sockaddr_in addr, struct Packet packet) {
	char hex[sizeof(struct Packet)];
	memcpy(hex, &packet, sizeof(struct Packet));
	sendto(sockfd, hex, sizeof(struct Packet), 0, (struct sockaddr*)&addr, sizeof(addr));
}

/*int receivePacket(int sockfd, struct sockaddr_in addr, struct Packet * packet) {
	int len = sizeof(addr);
	char line[sizeof(struct Packet)];
	int n = recvfrom(sockfd, line, sizeof(struct Packet), 0, (struct sockaddr*)&addr, &len);
	
	if (n != -1) {
		memcpy(packet, line, sizeof(struct Packet));
		return 0;
	}
	return n;
}*/