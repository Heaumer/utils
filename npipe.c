#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
	Maxclient = 1024
};

void
help(char *argv0)
{
	fprintf(stderr, "%s [-h] port host port\n", argv0);
	exit(0);
}

void
npipe(int c, struct hostent *dserv, int dport)
{
	char in[BUFSIZ], out[BUFSIZ];
	struct sockaddr_in sin;
	int nin, nout;
	int s;

	/* socket to remote server */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket");
		close(c);
		exit(1);
	}

	/* connection information */
	memset(&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy((char *)&sin.sin_addr.s_addr, (char *)dserv->h_addr, dserv->h_length);
	sin.sin_port = htons(dport);

	/* connect to remote server */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect");
		close(c);
		exit(1);
	}

	/* one process to read from c and write to s */
	if (fork() == 0) {
		for (;;) {
			memset(in, '\0', sizeof(in));
			nin = read(c, in, sizeof(in));
			if (nin <= 0)
				break;
			write(s, in, nin);
			in[nin] = 0;
		}
		exit(0);
	}

	/* one process to read from s and write to s */
	for (;;) {
		memset(out, '\0', sizeof(out));
		nout = read(s, out, sizeof(out));
		if (nout <= 0)
			break;
		write(c, out, nout);
		out[nout] = 0;
	}
	close(c);
	exit(0);
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin, cin;
	int lport, dport, s, c;
	struct hostent *dserv;
	socklen_t clen;

	if (argc < 4)
		help(argv[0]);

	/* listening port */
	errno = 0;
	lport = strtol(argv[1], NULL, 10);
	if (errno != 0) {
		fprintf(stderr, "%s is not a valid port number\n", argv[1]);
		return 1;
	}

	/* remote server info */
	dserv = gethostbyname(argv[2]);
	if (dserv == NULL) {
		perror(argv[2]);
		return 1;
	}

	/* remote server port */
	dport = strtol(argv[3], NULL, 10);
	if (errno != 0) {
		fprintf(stderr, "%s is not a valid port number\n", argv[1]);
		return 1;
	}

	/* listening socket */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket");
		return 1;
	}

	memset(&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(lport);

	/* bind the listening socket */
	if (bind(s, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return 1;
	}

	/* and listen */
	if (listen(s, Maxclient) == -1) {
		perror("listen");
		return 1;
	}

	clen = sizeof(cin);

	/* for each incoming connection */
	for (;;) {
		c = accept(s, (struct sockaddr *)&cin, &clen);
		if (c == -1) {
			perror("accept");
			return 1;
		}

		/* launch npipe and wait again */
		switch (fork()) {
		case -1:
			perror("fork");
			return 1;
		case 0:
			npipe(c, dserv, dport);
			exit(1);
		default:
			break;
		}
	}

	return 0;
}
