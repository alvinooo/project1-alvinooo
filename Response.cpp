#include "Response.h"

#define IPV4_BITS 32

unordered_map<string, Access> Response::directoryAccess = unordered_map<string, Access>();

Response::Response(Request &_req, int _socket, unsigned long addr, string doc_root)
	: req(_req), socket(_socket), line("HTTP/1.1 ")
{
	path = doc_root + req.path;
	
	// Read .htaccess
	string accessPath = path.substr(0, path.find_last_of('/')) + "/.htaccess";
	Response::parseAccess(accessPath);
	if (!Response::allow(addr, accessPath)) {
		line.append("403 Forbidden");
		sendResponse();
		return;
	}

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
	memset(resolvedRoot, 0, PATH_MAX);
	memset(resolvedPath, 0, PATH_MAX);

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
	errno = 0;
	stat(resolvedPath, &st);

	// HTTP/1.1 403 Forbidden
	if (!(st.st_mode & S_IRUSR)) {
		line.append("403 Forbidden");
		sendResponse();
		return;
	}

	// HTTP/1.0 404 Not Found
	if (errno == ENOENT) {
		line.append("404 Not Found");
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

// STATIC methods for .htaccess
void Response::parseAccess(string path)
{
	// Lookup file if necessary
	if (directoryAccess.find(path) != directoryAccess.end())
		return;
	ifstream file(path);
	if (!file)
		return;

	// Parse file
	string rule;
	Access access;
	while (getline(file, rule)) {
		bool allow = rule.find("allow") != string::npos;
		string host = rule.substr(rule.find("from") + 5);

		// Parse the IP address or make a DNS request
		if (host.find_first_of("0123456789") == 0) {
			string ipStr = host.substr(0, host.find("/"));
			unsigned long addr;
			inet_pton(AF_INET, ipStr.c_str(), &addr);
			addr = ntohl(addr);
			int bits = atoi(rule.substr(rule.find("/") + 1).c_str());
			if (allow)
				access.allowIP.push_back(make_pair(addr, bits));
			else
				access.denyIP.push_back(make_pair(addr, bits));
		} else {
			if (allow)
				access.allowIP.push_back(make_pair(ipAddr(host), IPV4_BITS));
			else
				access.denyIP.push_back(make_pair(ipAddr(host), IPV4_BITS));
		}
	}
	directoryAccess[path] = access;
	file.close();
}

unsigned long Response::ipAddr(string host)
{
	unsigned long addr;
	struct addrinfo addrCriteria;                   // Criteria for address match
  	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  	addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  	addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  	addrCriteria.ai_protocol = IPPROTO_TCP; 

  	struct addrinfo *clientAddr;
	const char * hostStr = host.c_str();
	string service = "http";
	const char * serviceStr = service.c_str();
	int rtnVal = getaddrinfo(hostStr, serviceStr, &addrCriteria, &clientAddr);
	if (rtnVal != 0) {
		string error;
		error.append("getaddrinfo() failed");
		error.append(gai_strerror(rtnVal));
		DieWithSystemMessage(error.c_str());
	}
	while (clientAddr->ai_next != NULL)
		clientAddr = clientAddr->ai_next;

	struct sockaddr_in * client = (struct sockaddr_in *) clientAddr->ai_addr;
	addr = ntohl(client->sin_addr.s_addr);
	freeaddrinfo(clientAddr);
	return addr;
}

bool Response::allow(unsigned long addr, string path)
{
	addr = ntohl(addr);
	if (search(addr, directoryAccess[path].allowIP))
		return true;
	if (search(addr, directoryAccess[path].denyIP))
		return false;
	return true;
}

bool Response::search(unsigned long addr,
	vector<pair<unsigned long, int> > rules)
{
	for (unsigned int i = 0; i < rules.size(); i++) {
		unsigned long ruleAddr = rules[i].first;
		int mask = rules[i].second;
		unsigned int clear = 0xFFFFFFFF << (IPV4_BITS - mask);
		if (mask == 0)
			clear = 0;
		if ((addr & clear) == (ruleAddr & clear))
			return true;
	}
	return false;
}
