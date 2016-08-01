/*
 * TODO add option for treat at most m < Maxcol column
 * TODO add option to center/right align columns
 * (eg. for code which have comments at the end)
 * cc ucol.c -o ucol # -W -Wall -Wextra -g
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SEP " \t\n"

/* keep indentation; nice for code, but can |ucol|+ so.. */
#define KEEPINDENT 0

enum {
    Maxcol = 128    /* supplementary columns are discarded */
};

static int colsize[Maxcol];
static int ncol;

static int nspace = 1;

/* strlen() skipping utf8 continuation prefix */
int
wordlen(char *w)
{
    int n;

    for (n = 0;*w != '\0'; w++) {
        /*
         * 0xC0 : 1100 0000
         * 0x80 : 1000 0000 (10xx xxxx is continuation prefix)
         */
        if (((*w) & 0xC0) != 0x80)
            n++;
    }

    return n;
}

int
countcols(char *line)
{
    char *p;
    int n;

    n = 0;

#if KEEPINDENT
    while(isspace(*line))
        line++;
#endif

    for (p = strtok(line, SEP); p != NULL && n < Maxcol; p = strtok(NULL, SEP)) {
        if(wordlen(p) > colsize[n])
            colsize[n] = wordlen(p);
        n++;
    }

    return n;
}

void
pspace(int n)
{
    while (n --> 0)
        putchar(' ');
}

void
fmtcols(char *line)
{
    char *p;
    int n;

    n = 0;

#if KEEPINDENT
    while(isspace(*line))
        putchar(*line++);
#endif

    for (p = strtok(line, SEP); p != NULL; p = strtok(NULL, SEP)) {
        fputs(p, stdout);
        /* don't output unecessary spaces for last column */
        if (n+1 < ncol)
            pspace(colsize[n++]-wordlen(p)+nspace);
    }

    putchar('\n');
}

int
help(char *argv0)
{
    fprintf(stderr, "%s [-n nspace] [file]\n", argv0);
    return 1;
}

int
main(int argc, char *argv[])
{
    char buf[BUFSIZ];
    FILE *in, *tmp;
    int ptmp[2];
    int i, n;

    in = stdin;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i+1 >= argc)
                return help(argv[0]);
            i++;
            errno = 0;
            nspace = (int)strtol(argv[i], NULL, 10);
            if (errno != 0)
                return help(argv[0]);
        } else if (strcmp(argv[i], "-h") == 0)
            return help(argv[0]);
        else {
            in = fopen(argv[i], "r");
            if (in == NULL) {
                perror(argv[i]);
                return 1;
            }
        }
    }

    if (pipe(ptmp) != 0) {
        perror("pipe");
        return 1;
    }

    /* for fgets */
    tmp = fdopen(ptmp[0], "r");
    if (tmp == NULL) {
        perror("fdopen");
        return 1;
    }

    ncol = 0;

    /*
     * read a first time; tee to pipe for second read.
     * get column size.
     */
    for (;;) {
        memset(buf, '\0', sizeof(buf));
        if (fgets(buf, sizeof(buf), in) == NULL)
            break;
        write(ptmp[1], buf, strlen(buf));

        n = countcols(buf);
        if (n > ncol)
            ncol = n;
    }

    close(ptmp[1]);

    /*
     * read a second time from pipe.
     * outputs
     */
    for (;;) {
        memset(buf, '\0', sizeof(buf));
        if (fgets(buf, sizeof(buf), tmp) == NULL)
            break;

        fmtcols(buf);
    }

    fclose(tmp); /* will close(ptmp[0]); */

    if (in != stdin)
        fclose(in);

    return 0;
}
