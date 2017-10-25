/*
 * TODO add option to center/right align columns
 * (eg. for code which have comments at the end)
 * cc ucol.c -o ucol # -W -Wall -Wextra -g
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SEP " \t\n"

enum {
    Maxcol = 128    /* supplementary columns are discarded */
};

int colsize[Maxcol];
int ncol;

/* Number of spaces between each column */
int nspace =  1;

/* By default, use maximum number of columns available */
int maxcol = Maxcol;

/* Should we keep indentation? */
int keepindent = 1;

/* Should we avoid considering spaces between quotes */
/* TODO */
int skipquotes = 0;

/* strlen() skipping utf8 continuation prefix */
int
wordlen(char *w)
{
    int n;

    for (n = 0; *w != '\0'; w++) {
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

    if (keepindent)
        while(isspace(*line))
            line++;

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

    if (keepindent)
        while(isspace(*line))
            putchar(*line++);

    for (p = strtok(line, SEP); p != NULL; p = strtok(NULL, SEP)) {
        fputs(p, stdout);
        /* having reached maxcol, only print a single space */
        if (n >= maxcol)
            putchar(' ');
        /* don't output unecessary spaces for last column */
        else if (n+1 < ncol)
            pspace(colsize[n++]-wordlen(p)+nspace);
    }

    putchar('\n');
}

int
help(char *argv0, int c)
{
    fprintf(stderr, "%s [-n nspace] [-m maxcol] [-k] [file]\n", argv0);
    return c;
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
                return help(argv[0], 1);
            i++;
            errno = 0;
            nspace = (int)strtol(argv[i], NULL, 10);
            if (errno != 0) {
                perror("nspace");
                return help(argv[0], 1);
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            if (i+1 >= argc)
                return help(argv[0], 1);
            i++;
            errno = 0;
            maxcol = (int)strtol(argv[i], NULL, 10);
            if (errno != 0) {
                perror("maxcol");
                return help(argv[0], 1);
            }
        } else if (strcmp(argv[i], "-k") == 0)
            keepindent = 0;
        else if (strcmp(argv[i], "-h") == 0)
            return help(argv[0], 0);
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
