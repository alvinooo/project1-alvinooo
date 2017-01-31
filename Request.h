#ifndef REQUEST_H
#define REQUEST_H

#include <unordered_map>
#include "httpd_util.h"

using namespace std;

class Request
{
public:
	Request();
	~Request();
	void parse(char * frame, int size);

	int complete;
	int valid;
	string method;
	string path;
	unordered_map<string, string> header;

private:
	void parseLine(char * frame, int size);
	void parseHeader(char * frame, int size);
};

#endif // REQUEST
