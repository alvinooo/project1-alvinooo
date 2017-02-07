#ifndef HTTPD_H
#define HTTPD_H

#include "httpd_util.h"

extern unsigned int poolSize;

void start_httpd(unsigned short port, std::string doc_root);
void * acceptLoopWrapper(void * args);
void acceptLoop(int servSock, std::string doc_root, ThreadPool * pool);
void spawnThread(int clntSock, unsigned long addr, std::string doc_root);

static const int MAXPENDING = 100; // Maximum outstanding connection requests

#endif // HTTPD_H
