#include "httpd_util.h"
#include "Stream.h"
#include "Request.h"
#include "Response.h"

using namespace std;

ThreadArgs::ThreadArgs(int _threadIndex, int _socket, unsigned long _addr, string _doc_root)
	: threadIndex(_threadIndex), socket(_socket), addr(_addr), doc_root(_doc_root) { }

ThreadArgs::~ThreadArgs() { }

std::vector<pthread_t *> ThreadArgs::threadOcean = std::vector<pthread_t *>();
pthread_mutex_t ThreadArgs::threadOceanMutex = PTHREAD_MUTEX_INITIALIZER;

void * HandleTCPClient(void * args) {

	pthread_detach(pthread_self());

	int clntSocket = ((ThreadArgs *) args)->socket;
	unsigned long addr = ((ThreadArgs *) args)->addr;
	string doc_root = ((ThreadArgs *) args)->doc_root;

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
		cleanupThread(args);
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
				cleanupThread(args);
				return NULL;
			} else {
				cleanupThread(args);
				DieWithSystemMessage("recv() failed");
			}
		}
		else if (numBytesRcvd == 0)
			break;

		stream.append(buffer, numBytesRcvd);

		while (stream.getRequest(req)) {
			if (req.complete || !req.valid) {
				Response res = Response(req, clntSocket, addr, doc_root);
				if (res.line.find("400") != string::npos) {
					cleanupThread(args);
					return NULL;
				}
			}
			if (req.header["Connection"].find("close") != string::npos) {
				cleanupThread(args);
				return NULL;
			}
		}
	}
	cleanupThread(args);
	return NULL;
}

void cleanupThread(void * args)
{
	int clntSocket = ((ThreadArgs *) args)->socket;
	int threadIndex = ((ThreadArgs *) args)->threadIndex;
	delete ((ThreadArgs *) args);

	pthread_mutex_lock(&ThreadArgs::threadOceanMutex);
	delete ThreadArgs::threadOcean[threadIndex];
	ThreadArgs::threadOcean[threadIndex] = NULL;
	pthread_mutex_unlock(&ThreadArgs::threadOceanMutex);

	close(clntSocket);
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
