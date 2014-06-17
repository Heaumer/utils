#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int port = 8080;

int
main(int argc, char *argv[])
{
	char in[BUFSIZ], out[BUFSIZ];
	struct sockaddr_in sin;
	struct hostent *serv;
	int s;

	if (argc < 2) {
		fprintf(stderr, "%s <server> [port] (default is %d)\n", argv[0], Port);
		return -1;
	}

	port = Port;
	if (argc == 3) {
		errno = 0;
		port = strtol(argv[2], NULL, 10);
		if (errno) {
			fprintf(stderr, "invalid port number '%s' : %s\n", argv[2], strerror(errno));
			return -1;
		}
	}

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket()");
		return -1;
	}

	serv = gethostbyname(argv[1]);
	if (serv == NULL) {
		fprintf(stderr, "gethostbyname(%s): %s\n", argv[1], strerror(errno));
		return -1;
	}

	memset(&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy((char *)&sin.sin_addr.s_addr, (char *)serv->h_addr, serv->h_length);
	sin.sin_port = htons(port);

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect()");
		return -1;
	}

	if (fork() == 0) {
		for (;;) {
			memset(out, '\0', sizeof(out));
			if (read(s, out, sizeof(out)) < 0)
				break;
			printf("%s", out);
		}
		return 0;
	}

	for (;;) {
		memset(in, '\0', sizeof(in));
		if (fgets(in, sizeof(in), stdin) == NULL)
			break;
		write(s, in, strlen(in));
	}

	return 0;
}
