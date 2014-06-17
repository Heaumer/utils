#include <u.h>
#include <libc.h>
#include <thread.h>
#include <9pclient.h>
#include <acme.h>

enum {
	Dirnamel = EVENTSIZE*UTFmax+1,
};

Win		*win;

void
cd(char *s)
{
	char	cwd[Dirnamel];

	if (strlen(s)+1 >= sizeof cwd)
		sysfatal("directory too long");

	memset(cwd, 0, sizeof cwd);
	strcpy(cwd, s);

	winname(win, "%s", cwd);

	winctl(win, "get");
}

int
isdir(char *s)
{
	Dir *d;
	int ret;

	d = dirstat(s);

	if (d == nil)
		return 0;

	ret = d->mode & DMDIR;

	free(d);

	return ret;
}

void
interp(Event *e)
{
	if ((e->c2 == 'L' || e->c2 == 'l') && isdir(e->text))
		cd(e->text);
	else if (e->c2 == 'X' || e->c2 == 'x') {
		if (strcmp(e->text, "Del") == 0 || strcmp(e->text, "Delete") == 0) {
			windel(win, 1);
			winfree(win);
			threadexitsall(nil);
		}
		winwriteevent(win, e);
	}
	else
		winwriteevent(win, e);
}

void
threadmain(int argc, char *argv[])
{
	Event e1, e2;

	/* cd requires win to be initialized first */
	win = newwin();

	if (argc > 1)
		cd(argv[1]);
	else
		/*
		 * XXX $HOME set but not $home.
		 */
		cd(getenv("HOME"));

	for (;;) {
		winreadevent(win, &e1);

		if (e1.c1 == 'M') {
			interp(&e1);
			if (e1.flag & 2) {
				winreadevent(win, &e2);
				interp(&e2);
			}
			continue;
		}
		winwriteevent(win, &e1);
	}
}
