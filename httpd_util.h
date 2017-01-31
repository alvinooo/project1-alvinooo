#ifndef HTTPD_UTIL_H
#define HTTPD_UTIL_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <string>
#include <cstddef>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define TIMEOUT 5

class ThreadArgs
{
public:
	ThreadArgs(int _socket, std::string _doc_root);
	~ThreadArgs();

	int socket;
	std::string doc_root;
};

void DieWithSystemMessage(const char *msg);
void HandleTCPClient(void * args);
void debugPrint(char *buffer, int size);

enum sizeConstants
{
	BUFSIZE = 512,
	REQ_MAX_SIZE = 8192,
	DUAL_CRLF = 4
};

#endif // HTTPD_UTIL_H
