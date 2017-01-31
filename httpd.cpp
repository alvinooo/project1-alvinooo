#include "httpd.h"
#include "httpd_util.h"

void start_httpd(unsigned short port, std::string doc_root)
{
	std::cerr << "Starting server (port: " << port <<
		", doc_root: " << doc_root << ")" << std::endl;
	
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

		// pthread_t thread;
		void * args = new ThreadArgs(clntSock, doc_root);
		// std::cerr << "client should be: " << clntSock << std::endl;
		// pthread_create(&thread, NULL, HandleTCPClient, args);
		HandleTCPClient(args);
		delete ((ThreadArgs *) args);
	}
}
