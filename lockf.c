#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int fd;

	if (argc < 2 || strcmp(argv[1], "-h") == 0) {
		fprintf(stderr, "%s [-f] <file>\n", argv[0]);
		return 2;
	}

	if (strcmp(argv[1], "-f") == 0)
		fd = mkfifo(argv[2], 0600);
	else
		fd = open(argv[1], O_CREAT|O_EXCL, 0600);

	if (fd == -1 /* && errno == EEXIST */)
		return 1;

	close(fd);
	return 0;
}
