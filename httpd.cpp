#include "httpd.h"
#include "httpd_util.h"
#include "Response.h"

unsigned int poolSize;

void start_httpd(unsigned short port, std::string doc_root)
{
	std::cerr << "Starting server (port: " << port <<
		", doc_root: " << doc_root << ", thread pool size " <<
		poolSize << ")" << std::endl;

	// Create listening socket
	int servSock;
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithSystemMessage("socket() failed");

	// DEBUG
	int opt = 1;
	if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) != 0)
		DieWithSystemMessage("setsockopt() failed");
	// DEBUG

	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(port);

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		DieWithSystemMessage("bind() failed");

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0)
		DieWithSystemMessage("listen() failed");

	if (poolSize > 0) {
		void * servArgs = new ThreadArgs(NULL, servSock, doc_root);
		ThreadPool pool = ThreadPool(acceptLoopWrapper, servArgs, poolSize);

		for (;;) {
			for (unsigned int i = 0; i < poolSize; i++) {
				pthread_join(*(pool.pool[i]), NULL);
			}
		}
	} else {
		acceptLoop(servSock, doc_root, NULL);
	}
}

void * acceptLoopWrapper(void * args)
{
	ThreadArgs * servArgs = (ThreadArgs *) args;
	acceptLoop(servArgs->socket, servArgs->doc_root, servArgs->pool);
	return NULL;
}

void acceptLoop(int servSock, std::string doc_root, ThreadPool * pool)
{
	for (;;) {
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);

		// Wait for a client to connect
		int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
		if (clntSock < 0)
			DieWithSystemMessage("accept() failed");

		char clntName[INET_ADDRSTRLEN]; // String to contain client address
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
			sizeof(clntName)) != NULL)
			std::cout << "Handling client " << clntName << "/" <<
				ntohs(clntAddr.sin_port) << std::endl;
		else
			std::cerr << "Unable to get client address" << std::endl;
		if (pool) {
			if (pool->available > 0) {
				pthread_mutex_lock(&pool->threadPoolMutex);
				pool->available--;
				pthread_mutex_unlock(&pool->threadPoolMutex);

				void * args = new ThreadArgs(NULL, clntSock, doc_root,
					clntAddr.sin_addr.s_addr, pool);
				HandleTCPClient(args);

				pthread_mutex_lock(&pool->threadPoolMutex);
				pool->available++;
				pthread_mutex_unlock(&pool->threadPoolMutex);
			}
		}
		else
			spawnThread(clntSock, clntAddr.sin_addr.s_addr, doc_root);
	}
}

void spawnThread(int clntSock, unsigned long clntAddr, std::string doc_root)
{
	pthread_t * thread = new pthread_t();
	void * args = new ThreadArgs(thread, clntSock, doc_root, clntAddr);
	pthread_create(thread, NULL, HandleTCPClient, args);
}
