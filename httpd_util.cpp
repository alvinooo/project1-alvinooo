#include "httpd_util.h"
#include "Stream.h"
#include "Request.h"
#include "Response.h"

using namespace std;

ThreadArgs::ThreadArgs(int _threadIndex, int _socket, string _doc_root)
	: threadIndex(_threadIndex), socket(_socket), doc_root(_doc_root) { }

ThreadArgs::~ThreadArgs() { }

std::vector<pthread_t *> ThreadArgs::threadOcean = std::vector<pthread_t *>();
pthread_mutex_t ThreadArgs::threadOceanMutex = PTHREAD_MUTEX_INITIALIZER;

void * HandleTCPClient(void * args) {

	pthread_detach(pthread_self());

	int clntSocket = ((ThreadArgs *) args)->socket;
	string doc_root = ((ThreadArgs *) args)->doc_root;

	char buffer[BUFSIZE]; // Used to construct HTTP request

	// Receive the request
	ssize_t numBytesRcvd = 0;

	Stream stream;
	Request req;
	for (;;) {

		memset(buffer, 0, BUFSIZE);
		// alarm(TIMEOUT);

		// Receive message from client
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);

		if (numBytesRcvd < 0) {

			// Close connection on timeout
			if (errno == EINTR) {
				cerr << "Client timed out... closing connection" << endl;
				cleanupThread(args);
				return NULL;
			} else {
				DieWithSystemMessage("recv() failed");
			}
		}
		else if (numBytesRcvd == 0)
			break;
		stream.append(buffer, numBytesRcvd);
		while (stream.getRequest(req)) {
			if (req.complete || !req.valid) {
				// Reset timeout
				alarm(0);
				Response res = Response(req, clntSocket, doc_root);
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
	alarm(0);

	int clntSocket = ((ThreadArgs *) args)->socket;
	int threadIndex = ((ThreadArgs *) args)->threadIndex;

	pthread_mutex_lock(&ThreadArgs::threadOceanMutex);
	delete ThreadArgs::threadOcean[threadIndex];
	ThreadArgs::threadOcean[threadIndex] = NULL;
	pthread_mutex_unlock(&ThreadArgs::threadOceanMutex);

	// for (unsigned int i = 0; i < ((ThreadArgs *) args)->threadOcean.size(); i++)
	// 	cerr << "Connection thread " << i << " " << ((ThreadArgs *) args)->threadOcean[i] << endl;

	close(clntSocket);
	delete ((ThreadArgs *) args);
}

void DieWithSystemMessage(const char *msg) {
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
