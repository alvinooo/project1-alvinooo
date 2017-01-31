#include "httpd_util.h"
#include "Stream.h"
#include "Request.h"
#include "Response.h"

using namespace std;

ThreadArgs::ThreadArgs(int _socket, string _doc_root)
	: socket(_socket), doc_root(_doc_root) { }

ThreadArgs::~ThreadArgs() { }

void HandleTCPClient(void * args) {

	// pthread_detach(pthread_self());

	int clntSocket = ((ThreadArgs *) args)->socket;
	string doc_root = ((ThreadArgs *) args)->doc_root;
	// std::cerr << "client: " << clntSocket << std::endl;

	char buffer[BUFSIZE]; // Used to construct HTTP request

	// Receive the request
	ssize_t numBytesRcvd = 0;

	Stream stream;
	Request req;
	for (;;) {

		memset(buffer, 0, BUFSIZE);
		alarm(TIMEOUT);

		// Receive message from client
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);

		if (numBytesRcvd < 0) {
			// Close connection on timeout
			if (errno == EINTR) {
				cerr << "Client timed out... closing connection" << endl;
				close(clntSocket);
				return;
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
					close(clntSocket);
					return;
				}
			}
			if (req.header["Connection"].find("close") != string::npos) {
				close(clntSocket);
				return;
			}
		}
	}
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
