#include "Response.h"

Response::Response(Request &_req, int _socket, string doc_root)
	: req(_req), socket(_socket), line("HTTP/1.1 ")
{
	path = doc_root + req.path;
	buildResponse(doc_root);
}

Response::~Response() { }

void Response::buildResponse(string doc_root)
{
	header.push_back("Server: 1337");

	// HTTP/1.0 400 Bad Request
	if (!req.valid) {
		line.append("400 Client Error");
		header.push_back("Connection: close");
		sendResponse();
		close(socket);
		return;
	}

	// Resolve the paths
	char resolvedRoot[PATH_MAX];
	char resolvedPath[PATH_MAX];

	realpath(doc_root.c_str(), resolvedRoot);
	realpath(path.c_str(), resolvedPath);

	// Convert doc_root to index.html
	if (strcmp(resolvedRoot, resolvedPath) == 0)
		strcpy(resolvedPath + strlen(resolvedRoot), "/index.html");

	// Check for escaping root directory
	if (strstr(resolvedPath, resolvedRoot) != resolvedPath) {
		line.append("404 Not Found");
		sendResponse();
		return;
	}

	// File info
	struct stat st;
	stat(resolvedPath, &st);

	// HTTP/1.0 404 Not Found
	if (errno == ENOENT) {
		line.append("404 Not Found");
		sendResponse();
		return;
	}

	// HTTP/1.1 403 Forbidden
	if (!(st.st_mode & S_IRUSR)) {
		line.append("403 Forbidden");
		sendResponse();
		return;
	}

	// HTTP/1.0 200 OK

	// Last-Modified
	char time[BUFSIZE];
	struct tm *gmt;
	gmt = gmtime(&st.st_mtime);
	strftime(time, BUFSIZE, "%x %X %Z", gmt);

	// Content-Type	
	char * extension = strrchr(resolvedPath, '.') + 1;
	string type;
	if (strstr(extension, "html"))
		type = "text/html";
	else if (strstr(extension, "jpg"))
		type = "image/jpeg";
	else if (strstr(extension, "png"))
		type = "image/png";

	// Content-Length
	stringstream ss;
	ss << st.st_size;

	line.append("200 OK");
	header.push_back("Last-Modified: ");
	header[1].append(time);
	header.push_back("Content-Type: ");
	header[2].append(type);
	header.push_back("Content-Length: ");
	header[3].append(ss.str());
	sendResponse();

	// Send actual file
	int fd = open(resolvedPath, O_RDONLY);
	sendfile(socket, fd, NULL, st.st_size);
	close(fd);
}

void Response::sendResponse()
{
	// Send line and header
	string response;
	response.append(line + "\r\n");
	for (unsigned int i = 0; i < header.size(); i++)
		response.append(header[i] + "\r\n");
	response.append("\r\n");
	ssize_t sentBytes = send(socket, response.c_str(), response.size(), 0);
	if (sentBytes < 0)
		DieWithSystemMessage("send() failed");
	else if (sentBytes != (unsigned int) response.size())
		DieWithSystemMessage("sent unexpected number of bytes");
}
