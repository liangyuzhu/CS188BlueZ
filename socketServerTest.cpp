#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <pthread.h>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

const int portNum = 4000;
const char localhost[] = "127.0.0.1";

int sockfd;

int main(int argc, char *argv[])
{
	// prevent SIGPIPE errors from crashing program
	signal(SIGPIPE, SIG_IGN);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);


	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	// bind address to socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portNum);     // short, network byte order
	addr.sin_addr.s_addr = inet_addr(localhost);
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	// set socket to listen status
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}


	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize;
	int clientSockfd;
	
	// accept a new connection
	clientSockfd = accept(sockfd, (struct sockaddr*)&(clientAddr), &(clientAddrSize));
	
	char c[1000];
	int output[1000];

	// read 0
	read(clientSockfd, &c, 4);
  	
  	// write 1
  	output[0] = 1;
  	output[1] = 2350;
  	write(clientSockfd, (char*)output, 8);
  	
  	// read 3
  	read(clientSockfd, &c, 8);
  	
  	// write 4
  	double avgX, avgY, varX, varY, minX, minY, maxX, maxY;
	avgX = 13;
	avgY = 14;
	varX = 34.5;
	varY = 0.04;
	minX = 543.2;
	minY = 345;
	maxX = 13.4;
	maxY = 234.4;
  		
  	output[0] = 4;
  	output[1] = 2350;
  	output[2] = *((int*)(&avgX));
  	output[3] = *(((int*)(&avgX))+1);
  	output[4] = *((int*)(&avgY));
  	output[5] = *(((int*)(&avgY))+1);
  	output[6] = *((int*)(&varX));
  	output[7] = *(((int*)(&varX))+1);
  	output[8] = *((int*)(&varY));
  	output[9] = *(((int*)(&varY))+1);
  	output[10] = *((int*)(&minX));
  	output[11] = *(((int*)(&minX))+1);
	output[12] = *((int*)(&minY));
  	output[13] = *(((int*)(&minY))+1);
	output[14] = *((int*)(&maxX));
  	output[15] = *(((int*)(&maxX))+1);
	output[16] = *((int*)(&maxY));
  	output[17] = *(((int*)(&maxY))+1);
  	
  	write(clientSockfd, (char*)output, 18*4);
  	char graph[] = "abcde";
  	write(clientSockfd, graph, 6);
  	
  	return 0;
}
