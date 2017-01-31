#ifndef HTTPD_H
#define HTTPD_H

#include "httpd_util.h"

void start_httpd(unsigned short port, std::string doc_root);

static const int MAXPENDING = 5; // Maximum outstanding connection requests

#endif // HTTPD_H
