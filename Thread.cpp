#include "Thread.h"

ThreadArgs::ThreadArgs(pthread_t * _thread, int _socket, std::string _doc_root,
	unsigned long _addr, ThreadPool * _pool)
	: thread(_thread), socket(_socket), doc_root(_doc_root), addr(_addr), pool(_pool) { }

ThreadArgs::~ThreadArgs()
{
	if (!pool)
		delete thread;
	close(socket);
}

ThreadPool::ThreadPool(void * (routine)(void *), void * args, unsigned int poolSize)
	:available(poolSize), threadPoolMutex(PTHREAD_MUTEX_INITIALIZER)
{
	pool = std::vector<pthread_t *>(poolSize, new pthread_t());
	((ThreadArgs *) args)->pool = this;
	for (unsigned int i = 0; i < poolSize; i++) {
		int returnValue = pthread_create(pool[i], NULL, routine, args);
		if (returnValue != 0) {
			std::string error;
			error.append("pthread_create() failed");
			error.append(strerror(returnValue));
			DieWithSystemMessage(error.c_str());
		}
	}
}

ThreadPool::~ThreadPool()
{
	for (unsigned int i = 0; i < pool.size(); i++) {
		if (pool[i]) {
			delete pool[i];
			pool[i] = NULL;
		}
	}
}
