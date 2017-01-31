#ifndef STREAM_H
#define STREAM_H

#include <utility>
#include <vector>
#include <cstddef>
#include <string.h>
#include "httpd_util.h"
#include "Request.h"

using namespace std;

class Stream
{
public:
	Stream();
	~Stream();
	void append(char * chunk, int size);
	int getRequest(Request &req);

	int bufferPtr;
	char buffer[REQ_MAX_SIZE + BUFSIZE]; // 8 KB + overflow space
};

#endif // STREAM_H
