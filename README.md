# Description
Collection of tiny tools, snippets, etc. You may find
shell scripts in [cfg/bin](https://github.com/Heaumer/cfg/tree/master/bin).
Feel free to report anything about those!

# ucol.c, ucol.awk
Format (spaces) input to ouput columns.
`-n` specifies the number of spaces between columns (default: 1).
Output to stdout; input to stdin by default, single file may be specified.

The 'u' stands for unicode : string length calculated according
to utf8, by skipping continuation bytes.

The ucol.awk is an awk version, shorter but slower. Use `-v 'n=2'`
instead of `-n 2`. Unicode support depends on awk'length();
eg. works with gawk, but not BSD's awk nor plan9's awk.

# See.c
Browse directory with left-click in acme (tested with p9p only):
more intuitive and avoid opening that much windows.

The `-n` flag opens a windows, where files opened from See's
window will be displayed. This reduce windows pollution, and
is especially nice to use acme on a small screen.

# serve.c, http.sh, mime-types, client.c
Starts listening on a given port. For each new connection,
launch a given program and redirects its stdin/stdout to
the connection socket. Somehow, it mimics Plan9's `aux/listen`
mechanism, or some parts of inetd.

Echo server (tcp7 usually):

	% ./s 7007 cat

Rsh-like:

	% ./s 22022 sh

See http.sh for a sample read-only httpd: (using mime-types)

	% ./s 8080 http.sh

Finally client.c opens a connection to a remote server
and redirect stdin/stdout to the connection socket.

One may use nc(1), nc.go or client.c to tests serve.c (or
a browser for http.sh).

# nc.go
A small netcat-like tool.

# fakerootns.c (idea from nsz, #morpheus)
Fakeroot-like using linux namespaces.

	% ./fakerootns bash
	# id
	uid=0(root) gid=0(root) groups=0(root),65534
	# cd /tmp
	#  rm -rf test/
	# mkdir test
	# touch test/test
	# tar czf test.tgz test
	# tar tzvf test.tgz
	drwxr-xr-x root/root         0 2014-06-18 19:00 test/
	-rw-r--r-- root/root         0 2014-06-18 19:00 test/test
	# exit
	% id
	uid=1000(mb) gid=100(users) groups=100(users),...

## See also fake.c, id.c
For curiosity, sfake.c contains a syscall hijacking using
ptrace for linux x86\_84 (cf. id.c). It was a first attempt to
have a fakeroot supporting statically linked binary while being simple, alternatives
being:

* "standard" fakeroot, only uses LD\_PRELOAD, thus won't work with statically linked binaries;
* fakeroot-ng uses LD\_PRELOAD and ptrace, but is 5K C++;

Insecurity of namespaces are definitely not a problem here as we're
not using them to isolate potentially malicious users. Furthermore, it's much
more efficient than the ptrace solution, while being concise.

# npipe.c
A combination of serve.c and client.c: for each incoming connexion on a given
port, traffic is forwarded to a distant server:port.

Note that if upper protocol uses TCP/IP data (eg. 'Host:' of HTTP header),
it won't be changed.

# lockf.c
Similar to procmail's lockfile, except it doesn't wait for lock file to
be available but return with exit status 1 it file can't be created.

Use `-f` to use mkfifo(3) rather than open(2).

# Shell scripts (sh/)
## Acme/Text edition related
### acme
Acme wrapper, setting autoindent (-a), font and load dump file (-l).

### +/-
Indent/Unindent (sed, tabulation).

### cls
Clean current win window

### put/putall
Put current window; putall put'em'all.

### closerr
Close error window.

### b
Use like this:

  < b foo

To generate

  \begin{foo}
  \end{foo}

### mktex
Create pdf file from tex, compiling three time for TOC, references.

### atex/atroff
Automatic tex/troff from current window.

### c+/c-
Comment/Uncomment. Cryptic arguments to keep commands shorts.

## X11
### bepo/us
Switch to either bepo/us keymap (X11)

### dual
Dual screen via xrandr.

### dwm\_status
Set dwm status (battery percentage, date).

## mardown
Markdown perl script imported.

## Others
### htmlindex
Create an html index of a directory.

### unhmtml
Try to remove what seems to be HTML tags.

### xmlind
Indent xml files (xslt).

### imp
Import (imagemagick) an image, upload it an open a firefox
on the given URL.

