/*
 * threads used to wait for the fork (thus avoiding double-fork)
 * cc serve.c -lpthread -o s && ./s cat
 */
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
	Port      = 8002,
	Maxclient = 42,
};

char **cmd;

typedef struct tsock tsock;

struct tsock {
	pthread_t thread;
	int       sock;
};

tsock threads[Maxclient];

/*
 * //error management;
 * specify cmd through argv;
 * add retries for accept;
 */
void *
servecmd(void *v)
{
	pid_t pid;
	tsock *t;

	t = (tsock *)v;

	pid = fork();

	switch(pid) {
	case -1:
		perror("fork");
		break;
	case 0:
		dup2(t->sock, 0);
		dup2(t->sock, 1);
		dup2(t->sock, 2);
		execvp(cmd[0], cmd);
		perror("execvp"); /* to socket.. */
		break;
	default:
		waitpid(pid, NULL, 0);
		break;
	}

	close(t->sock);
	t->sock = 0;

	return NULL;
}

int
help(char *argv0)
{
	fprintf(stderr, "%s [-p port] cmd [args]\n", argv0);
	return 1;
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin, cin;
	socklen_t clen;
	int port;	
	int s, c;
	int i;

	port = Port;

	cmd = NULL;

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
		else {
			cmd = argv+i;
		}
	}

	if (cmd == NULL || cmd[0] == NULL)
		return help(argv[0]);

	/* let fork()s end */
	signal(SIGCHLD, SIG_IGN);

	/*
	 * all sockets are available
	 */
	for (i = 0; i < Maxclient; i++)
		threads[i].sock = 0;

/*	if (argc > 1) {
		errno = 0;
		port = strtol(argv[1], NULL, 10);
		if (errno != 0) {
			fprintf(stderr, "invalid port number '%s' : %s\n", argv[1], strerror(errno));
			return -1;
		}
	}*/

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket()");
		return -1;
	}

	memset(&sin, '\0', sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}

	if (listen(s, Maxclient) == -1) {
		perror("listen");
		return -1;
	}

	clen = sizeof(cin);

	for (;;) {
		c = accept(s, (struct sockaddr *)&cin, &clen);
		if (c == -1) {
			perror("accept");
			return -1;
		}

		for (i = 0; i < Maxclient; i++) {
			if (threads[i].sock == 0) {
				threads[i].sock = c;
				pthread_create(&threads[i].thread, NULL, servecmd, threads+i);
				break;
			}
		}
		/* no room */
		if (i == Maxclient)
			close(c);
	}

	close(s);
	return 0;
}
