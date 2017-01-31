#include "Stream.h"

using namespace std;

Stream::Stream(): bufferPtr(0)
{
	memset(buffer, 0, REQ_MAX_SIZE + BUFSIZE);
}

Stream::~Stream() { }

void Stream::append(char * chunk, int size)
{
	// Append network bytes to end of buffer
	memcpy(buffer + bufferPtr, chunk, size);
	bufferPtr += size;

	cout << "Append" << endl;
	debugPrint(buffer, size);
}

int Stream::getRequest(Request &req)
{
	// Check for request end
	char * end = strstr(buffer, "\r\n\r\n");
	if (!end)
		return 0;

	// Treat non-terminated requests as invalid
	if (!end && bufferPtr >= REQ_MAX_SIZE) {
		memset(buffer, 0, REQ_MAX_SIZE + BUFSIZE);
		bufferPtr = 0;
		req.valid = 0;
		return 0;
	}

	// Copy buffer into HTTP Request
	ptrdiff_t endIndex = end - buffer;
	cout << "Retrieving up to " << endIndex << endl;
	debugPrint(buffer, endIndex);
	req.parse(buffer, endIndex);

	// Clear out the old frame
	memset(buffer, 0, endIndex + DUAL_CRLF);

	// Shift any remaining bytes to front
	memcpy(buffer, end + DUAL_CRLF, bufferPtr - endIndex);

	// Clear out remaining bytes
	memset(end + DUAL_CRLF + bufferPtr - endIndex,
		0, );

	// Reset buffer pointer to front
	bufferPtr -= endIndex + DUAL_CRLF;

	cout << "Reset Buffer" << endl;
	debugPrint(buffer, bufferPtr);

	return 1;
}
