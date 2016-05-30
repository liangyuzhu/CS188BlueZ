#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

const int portNum = 4000;
const char localhost[] = "127.0.0.1";

const int WRITE_PACKET_SIZES[] = {1, -1, 6, 2, -1, 2, -1};
const int READ_PACKET_SIZES[] = {-1, 2, -1, -1, 18, -1, 1};

int id;
double avgX, avgY, varX, varY, minX, minY, maxX, maxY;
char graphStr[10000];
int sockfd;

int setupSocket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNum);
    addr.sin_addr.s_addr = inet_addr(localhost);
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return 2;
    }
    
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
        perror("getsockname");
        return 3;
    }
    return 0;
}


char* readData (int sockfd, int type) {
	if (type > 6 || READ_PACKET_SIZES[type] == -1)
        exit(1);

    char* response = (char*)malloc(10000);
    int size = READ_PACKET_SIZES[type]*4;
    read(sockfd, response, size);
    
    if (type == 4) {	//read graph string
    	char ch;
    	while (true) {
    		read(sockfd, &ch, 1);
    		if (ch == 0)
    			break;
    		response[size++] = ch;
    	}
    }

    return response;
}

void processData(char* v, int type) {

    int checkType = *((int*)v);

    if (checkType != type) {
        printf("Packet is type %i. Expect type %i\n", checkType, type);
        exit(1);
    }

	int i = 72;
    switch (type) {
        case 1:
            id = *((int*)(v+4));
            break;
        case 4:
            avgX = *((double*)(v+8));
            avgY = *((double*)(v+16));
            varX = *((double*)(v+24));
            varY = *((double*)(v+32));
            minX = *((double*)(v+40));
            minY = *((double*)(v+48));
            maxX = *((double*)(v+56));
            maxY = *((double*)(v+64));
            while (v[i] != '\0') {
            	graphStr[i-72] = v[i];
            	i++;
            }
            graphStr[i] = '\0';
            break;
        case 6:
            exit(1);
            break;
        default:
            exit(1);
    }
    free(v);
}

void writeData(int sockfd, int type, int id = -1, int x = -1, int y = -1) {
    if (type > 6 || WRITE_PACKET_SIZES[type] == -1)
        exit(1);
    int* output = (int*)malloc(1000*4);
    output[0] = type;
    output[1] = id;	//undefined for type 0;
    
    double* dx;
    double* dy;
    
    switch(type) {
        case 0:
            break;
        case 2:
            dx = (double*)(output+2);
            dy = (double*)(output+4);
            *dx = x;
            *dy = y;
            break;
        case 3:
            break;
        case 5:
            break;
        default:
            exit(1);
    }
    
    write(sockfd, (char*)output, WRITE_PACKET_SIZES[type]*4);
    free(output);
}

int main() {
    if (setupSocket()) {
        printf("Socket setup failed, revert to normal output\n");
        return 1;
    }
    
    char* c;
    
    writeData(sockfd, 0);
    c = readData(sockfd, 1);
    processData(c, 1);
    
//     int testDataX[] = {1,2,3,4,5,6,7};
//     int testDataY[] = {3,8,5,2,3,4,5};
//     
//     for (int i = 0; i < 7; i++) {
//         writeData(sockfd, 2, id, testDataX[i], testDataY[i]);
//     }
    
    writeData(sockfd, 3, id);
    c = readData(sockfd, 4);
    processData(c, 4);
    
    printf("Avg X: %f\n", avgX);
    printf("Avg Y: %f\n", avgY);
    printf("Var X: %f\n", varX);
    printf("Var Y: %f\n", varY);
    printf("Min X: %f\n", minX);
    printf("Min Y: %f\n", minY);
    printf("Max X: %f\n", maxX);
    printf("Max Y: %f\n", maxY);
	printf("Graph Str:\n%s\n", graphStr);

}
