#define _GNU_SOURCE /* CLONE_NEWUSER */
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH 256
	
static char mapg[32], mapu[32];

int
wr(char *mapfile, char *val)
{
	char path[MAX_PATH];
	int fd;

	memset(path, '\0', sizeof(path));
	snprintf(path, sizeof(path)-1, "/proc/self/%s", mapfile);

	fd = open(path, O_RDWR);
	if (fd == -1) {
		perror(path);
		return -1;
	}
	if (write(fd, val, strlen(val)+1) == -1) {
		perror(path);
		return -1;
	}

	return close(fd);
}

int
ex(void *args)
{
	char **argv;

	argv = (char **)args;

	if (wr("gid_map", mapg) == -1)
		return -1;
	if (wr("uid_map", mapu) == -1)
		return -1;

	return execvp(argv[0], argv);
}

#define STACKSZ (1024*1024)
static char stack[STACKSZ];

int
fakeroot(char *argv[])
{
	pid_t pid;

	pid = clone(ex, stack+STACKSZ, CLONE_NEWUSER|SIGCHLD, argv);

	if (pid == -1) {
		perror("clone");
		return -1;
	}

	waitpid(pid, NULL, 0);
	return 0;
}

int
main(int argc, char *argv[])
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0) {
		fprintf(stderr, "%s <cmd>\n", argv[0]);
		return 0;
	}

	memset(mapg, '\0', sizeof(mapg));
	memset(mapu, '\0', sizeof(mapu));

	snprintf(mapg, sizeof(mapg)-1, "0 %d 1", getgid());
	snprintf(mapu, sizeof(mapu)-1, "0 %d 1", getuid());

	return fakeroot(argv+1);
}
