/**
 * Server - Handles file and packet management
 * on the server side. Check for exisitng files, divides
 * the file into packets, and sends them to the client.
 * @author Ryan Borgeson
 * @date 2/5/2018
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "packet.h"

/* Max number of attempts to send last packet. */
#define MAX_ATTEMPTS 50

/**
 * Main - Main execution point for the server.
 * @param argc Number of arguments.
 * @param argv List of arguments.
 * @return Returns integer.
 */
int main(int argc, char **argv);

/**
 * On Request File - When a file is requested from the client,
 * the server attempts to read the file and divide it into packets.
 * @param packet Packet containing file request information.
 * @param sockfd Socket file descriptor.
 * @param addr Server and client address information.
 */
void on_requestFile(Packet packet, int sockfd, struct sockaddr_in addr);

/**
 * On Receive acknowledgement - Attempts to send the window
 * containing packets to the user.
 * @param sockfd Socket file descriptor.
 * @param addr Client and server address information.
 * @param expected Number of packets expected to be sent.
 * @param window Window containing a list of packets.
 * @return Success or unsuccessful.
 */
int on_receiveAcknowledgement(int sockfd, struct sockaddr_in addr, int expected, Packet * window);