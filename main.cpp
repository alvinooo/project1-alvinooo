#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "httpd.h"

using namespace std;

void usage(char * argv0)
{
	cerr << "Usage: " << argv0 << " listen_port docroot_dir [pool-option] [N]" << endl;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	long int port = strtol(argv[1], NULL, 10);

	if (errno == EINVAL || errno == ERANGE) {
		usage(argv[0]);
		return 2;
	}

	if (port <= 0 || port > USHRT_MAX) {
		cerr << "Invalid port: " << port << endl;
		return 3;
	}

	string doc_root = argv[2];

	poolSize = 0;
	if (argc > 3 && strcmp(argv[3], "pool") == 0)
		poolSize = atoi(argv[4]);

	start_httpd(port, doc_root);

	return 0;
}
