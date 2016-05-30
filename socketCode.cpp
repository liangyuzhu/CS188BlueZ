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

#include <string>
#include <vector>
using namespace std;

const int portNum = 4000;
const string localhost = "127.0.0.1";

const int PACKET_SIZES[] = {1, -1, 6, 2, -1, 2, -1};

int id;
double avgX, avgY, varX, varY, minX, minY, maxX, maxY;
string graphStr;
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
    addr.sin_addr.s_addr = inet_addr(localhost.c_str());
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


vector<char> readData (int sockfd) {
    char c;
    std::string response;
    while(read(sockfd, &c, 1) > 0) {
        response += c;
    }

    vector<char> toRet;
    for (int i = 0; i < response.size(); i++) {
        toRet.push_back(response[i]);
    }
    return toRet;
}

void processData(vector<char> &v, int type) {
    int checkType = *((int*)v[0]);
    
    if (checkType != type) {
        cout << "Packet is type " << checkType << ". Expected type " << type << endl;
        exit(1);
    }
    
    switch (type) {
        case 1:
            id = *((int*)v[1]);
            break;
        case 4:
            avgX = *((double*)v[2]);
            avgY = *((double*)v[4]);
            varX = *((double*)v[6]);
            varY = *((double*)v[8]);
            minX = *((double*)v[10]);
            minY = *((double*)v[12]);
            maxX = *((double*)v[14]);
            maxY = *((double*)v[16]);
            graphStr = "";
            for (int i = 18; i < v.size(); i++) {
                graphStr += v[i];
            }
            break;
        case 6:
            exit(1);
            break;
        default:
            exit(1);
    }
}

void writeData(int sockfd, int type, int id = -1, int x = -1, int y = -1) {
    if (type > 6 || PACKET_SIZES[type] == -1)
        exit(1);
    int* output = new int[PACKET_SIZES[type]*4];
    output[0] = type;
    output[1] = id;	//undefined for type 0;
    
    double* dx;
    double* dy;
    
    switch(type) {
        case 0:
            break;
        case 2:
            output[1] = id;
            dx = (double*)output[2];
            dy = (double*)output[3];
            *dx = x;
            *dy = y;
            break;
        case 3:
            output[1] = id;
            break;
        case 5:
            output[1] = id;
            break;
        default:
            exit(1);
    }
    
    write(sockfd, (char*)output, PACKET_SIZES[type]*4);
    delete[] output;
}

int main() {
    if (setupSocket()) {
        cout << "Socket setup failed, revert to normal output" << endl;
        return 1;
    }
    
    vector<char> v;
    
    writeData(sockfd, 0);
    v = readData(sockfd);
    processData(v, 1);
    
    int testDataX[] = {1,2,3,4,5,6,7};
    int testDataY[] = {3,8,5,2,3,4,5};
    
    for (int i = 0; i < 7; i++) {
        writeData(sockfd, 2, id, testDataX[i], testDataY[i]);
    }
    
    writeData(sockfd, 3, id);
    v = readData(sockfd);
    processData(v, 4);
    
    double avgX, avgY, varX, varY, minX, minY, maxX, maxY;
    cout << "Avg X: " << avgX << endl;
    cout << "Avg Y: " << avgY << endl;
    cout << "Var X: " << varX << endl;
    cout << "Var Y: " << varY << endl;
    cout << "Min X: " << minX << endl;
    cout << "Min Y: " << minY << endl;
    cout << "Max X: " << maxX << endl;
    cout << "Max Y: " << maxY << endl;
    cout << graphStr << endl;
}
