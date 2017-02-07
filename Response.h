#ifndef RESPONSE_H
#define RESPONSE_H

#include <unordered_map>
#include "Request.h"
#include "httpd_util.h"

class Access
{
public:
	vector<pair<unsigned long, int> > allowIP;
	vector<pair<unsigned long, int> > denyIP;
};

class Response
{
public:
	Response(Request &req, int clntSocket, unsigned long addr, string doc_root);
	~Response();

	Request req;
	string path;
	int socket;
	string line;
	vector<string> header;

	// Extension 2 .htaccess methods/variables
	static void parseAccess(string path);
	static unsigned long ipAddr(string host);
	static bool allow(unsigned long addr, string path);
	static bool search(unsigned long addr,
		vector<pair<unsigned long, int> > rules);

	static unordered_map<string, Access> directoryAccess;

private:
	void buildResponse(string doc_root);
	void sendResponse();
};

#endif // RESPONSE
