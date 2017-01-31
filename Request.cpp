#include "Request.h"

Request::Request(): complete(0), valid(1),
	method(""), path("") { }

Request::~Request() { }

void Request::parse(char * frame, int size)
{
	method.clear();
	path.clear();
	header.clear();
	complete = 0;
	valid = 1;

	parseLine(frame, size);
	if (!valid)
		return;
	parseHeader(frame, size);
}

void Request::parseLine(char * frame, int size)
{
	string line;
	char * lineEnd = strstr(frame, "\r\n");
	ptrdiff_t lineEndIndex = lineEnd - frame;
	line.append(frame, lineEndIndex);

	// Count 3 components of line
	int spaces = 0;
	for (unsigned int i = 0; i < line.size(); i++) {
		if (line[i] == ' ') {
			spaces++;
			while (line[i] == ' ')
				i++;
		}
	}

	// Validate
	if (spaces != 2 ||
		line.find("GET") != 0 ||
		line.find("HTTP/1.1") == string::npos) {
		valid = 0;
		return;
	}

	// Parse method
	method = "GET";

	// Parse path
	unsigned int pathStart = line.find(" ");
	while (line[pathStart] == ' ')
		pathStart++;
	unsigned int pathEnd = pathStart;
	while (line[pathEnd] != ' ')
		pathEnd++;
	path = line.substr(pathStart, pathEnd - pathStart);

	// If no header
	if (lineEndIndex + DUAL_CRLF == size)
		valid = 0;
}

void Request::parseHeader(char * frame, int size)
{
	// Start from second line
	char * keyValStart, * keyValEnd;
	char * lineEnd = strstr(frame, "\r\n");

	// Iterate through key-values
	while (lineEnd - frame < size - DUAL_CRLF) {
		keyValStart = lineEnd + 2;
		keyValEnd = strstr(keyValStart, "\r\n");
		lineEnd = strstr(keyValEnd, "\r\n");

		// Validate each key-value
		ptrdiff_t keyValSize = keyValEnd - keyValStart;
		char * colon = (char *) memchr(keyValStart, ':', keyValSize);
		if (!colon) {
			valid = 0;
			return;
		}

		// Store key-value pairs
		string key, value;
		key.append(keyValStart, colon - keyValStart);
		value.append(colon + 1, keyValEnd - colon);
		header[key] = value;
	}
	
	if (header.find("Host") == header.end()) {
		valid = 0;
		return;
	}

	complete = 1;
}
