# Description
Collection of tiny tools, snippets, etc. You may find
shell scripts in [cfg/bin](https://github.com/Heaumer/cfg/tree/master/bin).
Feel free to report anything about those!

# ucol.c
Format (spaces) input to ouput columns.
/-n/ specifies the number of spaces between columns (default: 1).
Output to stdout.
input to stdin by default, single file may be specified.

The 'u' stands for unicode : string length calculated according
to utf8, by skipping continuation bytes.

# See.c
Browse directory with left-click in acme (tested with p9p only):
more intuitive and avoid opening that much windows.

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

	% /fakerootns bash
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
