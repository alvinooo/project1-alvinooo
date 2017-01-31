#ifndef RESPONSE_H
#define RESPONSE_H

#include "Request.h"
#include "httpd_util.h"

class Response
{
public:
	Response(Request &req, int clntSocket, string doc_root);
	~Response();

	Request req;
	string path;
	int socket;
	string line;
	vector<string> header;

private:
	void buildResponse(string doc_root);
	void sendResponse();
};

#endif // RESPONSE
