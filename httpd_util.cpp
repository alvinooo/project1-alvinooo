#include "httpd_util.h"
#include "Stream.h"
#include "Request.h"
#include "Response.h"

using namespace std;

ThreadArgs::ThreadArgs(int _socket, string _doc_root)
	: socket(_socket), doc_root(_doc_root) { }

ThreadArgs::~ThreadArgs() { }

void * HandleTCPClient(void * args) {

	// pthread_detach(pthread_self());

	int clntSocket = ((ThreadArgs *) args)->socket;
	string doc_root = ((ThreadArgs *) args)->doc_root;
	// std::cerr << "client: " << clntSocket << std::endl;

	char buffer[BUFSIZE]; // Used to construct HTTP request

	// Receive the request
	ssize_t numBytesRcvd = 0;

	Stream stream;
	Request req;
	int keep_alive = 1;
	while (keep_alive) {

		memset(buffer, 0, BUFSIZE);

		// Receive message from client
		// Start timeout
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
		if (numBytesRcvd < 0)
			DieWithSystemMessage("recv() failed");
		else if (numBytesRcvd == 0)
			break;
		stream.append(buffer, numBytesRcvd);
		int moreRequests = stream.getRequest(req);
		while (moreRequests) {
			if (req.complete || !req.valid) {
				// Reset timeout
				Response res = Response(req, clntSocket, doc_root);
				if (res.line.find("400") != string::npos) {
					close(clntSocket);
					keep_alive = 0;
					break;
				}
			}
			if (req.header["Connection"].find("close") != string::npos) {
				close(clntSocket);
				keep_alive = 0;
				break;
			}
			moreRequests = stream.getRequest(req);
		}
	}
	return (NULL);
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
