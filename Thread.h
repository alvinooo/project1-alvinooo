#ifndef THREAD
#define THREAD

#include "httpd_util.h"

class ThreadPool
{
public:
	ThreadPool(void * (routine)(void *), void * args, unsigned int poolSize);
	~ThreadPool();

	std::vector<pthread_t *> pool;
	unsigned int available;
	pthread_mutex_t threadPoolMutex;
};

class ThreadArgs
{
public:
	ThreadArgs(pthread_t * thread, int _socket, std::string _doc_root,
		unsigned long addr=0, ThreadPool * pool=NULL);
	~ThreadArgs();

	pthread_t * thread;
	int socket;
	std::string doc_root;
	unsigned long addr;
	ThreadPool * pool;
};

#endif