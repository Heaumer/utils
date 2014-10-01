#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define UPATH	"/tmp/broadcast-unix"

enum {
	Port      = 9000,
	Maxclients = 128
};

int verbose = 0;

int clients[Maxclients];
int nclients;
pthread_mutex_t mclients;

void *
acceptloop(void *a)
{
	struct sockaddr_in cin;
	socklen_t clen;
	int c, s, i;

	s = *(int *)a;

	for (;;) {
		c = accept(s, (struct sockaddr *)&cin, &clen);
		if (c == -1) {
			perror("accept");
			return a;
		}
		pthread_mutex_lock(&mclients);
		if (nclients >= Maxclients)
			write(c, "no room.\n", 9), close(c);
		else
			for (i = 0; i < Maxclients; i++)
				if (clients[i] == 0) {
					clients[i] = c, nclients++;
					break;
				}
		pthread_mutex_unlock(&mclients);
	}

	return a;
}

/*
 * read from named FIFO.
 * FIFO is re-opened when closed.
 */
int
readunix(char *path, char buf[], int n)
{
	static int fd = -1;
	int r;

	if (fd == -1) {
		fd = open(path, O_RDONLY, EWOULDBLOCK);
		if (fd == -1) {
			perror(path);
			return -1;
		}
	}

	r = read(fd, buf, n);
	if (r == -1)
		perror("read");
	if (r == 0)
		close(fd), fd = -1;

	return r;
}

int
help(char *argv0)
{
	fprintf(stderr, "%s [-v] [-p port] [socketpath=%s]\n",
		argv0, UPATH);
	return 1;
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	pthread_t athread;
	char buf[BUFSIZ];
	char *upath;
	int n, i, s;
	int port;
	int on;

	/*
	 * don't SIGTERM on broken pipes.
	 * used to detect client disconnection, EPIPE on write.
	 */
	signal(SIGPIPE, SIG_IGN);

	port = Port;
	upath = UPATH;

	nclients = 0;
	memset(clients, '\0', sizeof(clients));
	pthread_mutex_init(&mclients, NULL);

	memset(buf, '\0', sizeof(buf));

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0) {
			if (i+1 >= argc)
				return help(argv[0]);
			i++;
			errno = 0;
			port = (int)strtol(argv[i], NULL, 10);
			if (errno != 0)
				return help(argv[0]);
		} else if (strcmp(argv[i], "-h") == 0)
			return help(argv[0]);
		else if (strcmp(argv[i], "-v") == 0)
			verbose = 1;
		else
			upath = argv[i];
	}

	if (unlink(upath) == -1)
	if (errno != ENOENT) {
		perror(upath);
		return -1;
	}

	if (mkfifo(upath, 0600) == -1) {
		perror(upath);
		return -1;
	}

	if (verbose)
		printf("Reading from '%s'.\n", upath);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket()");
		return -1;
	}

	on = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
		perror("bind");
		return -1;
	}

	if (listen(s, Maxclients) == -1) {
		perror("listen");
		return -1;
	}

	if (pthread_create(&athread, NULL, acceptloop, &s) == -1) {
		perror("pthread_create");
		return -1;
	}

	if (verbose)
		printf("Listening on :%d.\n", port);

	while ((n = readunix(upath, buf, sizeof(buf))) >= 0) {
		pthread_mutex_lock(&mclients);
		for (i = 0; n > 0 && i < Maxclients; i++)
			if (clients[i] != 0)
			if (write(clients[i], buf, n) == -1)
			if (errno == EPIPE)
				close(clients[i]), clients[i] = 0, nclients--;

		pthread_mutex_unlock(&mclients);
		memset(buf, '\0', sizeof(buf));
	}

	pthread_cancel(athread);
	pthread_mutex_destroy(&mclients);

	/* bye */
	for (i = 0; i < Maxclients; i++)
		if (clients[i] != 0)
			close(clients[i]);

	return 0;
}
