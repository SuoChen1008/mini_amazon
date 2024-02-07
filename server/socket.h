#ifndef _SOCKET_H
#define _SOCKET_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

const int MAX_LENGTH = 65536;

int createServerSocket(const string & portNum);
int serverAcceptConnection(int serverFd, string & clientIp);
int clientRequestConnection(const string & hostName, const string & portNum);
int getPortNum(int socketFd);
void sendMsg(int socket_fd, const void * buf, int len);
string recvMsg(int socket_fd);

#endif