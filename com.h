#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"

#define WINDOW_SIZE 5
#define WINDOW_DATA_SIZE 1024

void sendPacket(int sockfd, struct sockaddr_in addr, struct Packet packet);

//int receivePacket(int sockfd, struct sockaddr_in addr, struct Packet * packet);