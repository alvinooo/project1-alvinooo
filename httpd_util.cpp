#include "httpd_util.h"
#include "Stream.h"
#include "Request.h"
#include "Response.h"

using namespace std;

void * HandleTCPClient(void * args)
{

	pthread_detach(pthread_self());

	ThreadArgs * threadArgs = (ThreadArgs *) args;
	int clntSocket = threadArgs->socket;
	string doc_root = threadArgs->doc_root;
	unsigned long addr = threadArgs->addr;

	char buffer[BUFSIZE]; // Used to construct HTTP request

	// Receive the request
	ssize_t numBytesRcvd = 0;

	Stream stream;
	Request req;

	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	if (setsockopt(clntSocket, SOL_SOCKET, SO_RCVTIMEO,
		(struct timeval *) &tv, sizeof(struct timeval)) != 0) {
		cerr << errno << endl;
		delete ((ThreadArgs *) args);
		DieWithSystemMessage("setsockopt() failed");
	}

	for (;;) {

		memset(buffer, 0, BUFSIZE);

		// Receive message from client
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);

		if (numBytesRcvd < 0) {

			// Close connection on timeout
			if (errno == EWOULDBLOCK) {
				cerr << "Client timed out... closing connection" << endl;
				delete ((ThreadArgs *) args);
				return NULL;
			} else {
				delete ((ThreadArgs *) args);
				DieWithSystemMessage("recv() failed");
			}
		}
		else if (numBytesRcvd == 0) {
			delete ((ThreadArgs *) args);
			return NULL;
		}

		// Frame incoming bytes into requests
		stream.append(buffer, numBytesRcvd);

		// Extract full requests from byte buffer
		while (stream.getRequest(req)) {
			if (req.complete || !req.valid) {
				Response res = Response(req, clntSocket, addr, doc_root);
				if (res.line.find("400") != string::npos) {
					delete ((ThreadArgs *) args);
					return NULL;
				}
			}
			if (req.header["Connection"].find("close") != string::npos) {
				delete ((ThreadArgs *) args);
				return NULL;
			}
		}
	}
	return NULL;
}

void DieWithSystemMessage(const char *msg)
{
	cerr << msg << endl;
	exit(1);
}

void debugPrint(char * s, int size)
{
	for (int i = 0; i < size; i++) {
		switch (s[i]) {
			case '\r':
				cout << "<cr>";
				break;
			case '\n':
				cout << "<lf>";
				break;
			case '\0':
				cout << "<NULL>";
				break;
			default:
				cout << s[i];
		}
	}
	cout << endl;
}
