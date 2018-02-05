/**
 * Client - Client side that allows the user to connect
 * to the server and retrieve a file ensuring the each packet
 * is received and not corrupt.
 * @author Ryan Borgeson
 * @date 2/5/2018
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

/**
 * Main - Where the program initially executes
 * from.
 * @param argc Number of arguments.
 * @param argv List of arguments.
 * @return Returns and integer.
 */
int main(int argc, char **argv);

/**
 * Send Ack - Sends an acknowledgement to the server
 * by providing the socket file descriptor, server address
 * information, and the ID of the received packet that is
 * being acknowledged.
 * @param sockfd Socket file descriptor.
 * @param addr Server address information.
 * @param id ID of the received packet.
 */
void sendAck(int sockfd, struct sockaddr_in addr, int id);

/**
 * Save File - Saves the sorted list of packets using the
 * provided file name.
 * @param packets A list of packets to save.
 * @param newFileName The name of the new file.
 */
void saveFile(struct Packet * packets, char * newFileName);
