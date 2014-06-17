#!/bin/sh

root=./httproot/
path=

# mimetype
mtype="text/html"

# http status
status="200 OK"

htmlhead() {
	cat <<EOF
<!DOCTYPE HTML>
<html lang="en">
	<head>
		<meta charset=utf-8>
		<title>http.sh</title>
	</head>
	<body>
EOF
}

htmlfoot() {
	cat <<EOF
	</body>
</html>
EOF
}

# html list of files
listfiles() {
	htmlhead
	echo '<ul>'
	for f in `echo $1/*`; do
		u=`echo $f | tr -s './' | sed 's,'$root',,g'`
		echo '<li><a href="./'$u'">'$u'</a></li>'
	done
	echo '</ul>'
	htmlfoot
}

filext() {
	echo $1 | sed 's/.*\.//g'
}

# get a file and set mimetype
getfile() {
	ext=`filext $1`
	mtype=`awk '$1 == "'$ext'" { print $2 }' mime-types`
	cat $1
}

notfound() {
	htmlhead
	status="404 Not found"
	echo '<p>'$1' not found </p>'
	htmlfoot
}

# Get HTTP request (extract path)
while read line; do
	echo $line | grep '^GET' &>/dev/null &&
		path=`echo $line | awk '{ print $2 }' | tr -s './'`
	if [ "$line" == "" ] || [ "$line" == "" ]; then
		break
	fi
done

# build response
(
	if [ -d "$root$path" ]; then
		if [ -f "$root$path/index.html" ]; then
			getfile "$root$path/index.html"
		else
			listfiles "$root$path"
		fi
	elif [ -f "$root$path" ]; then
		getfile "$root$path"
	else
		notfound "$root$path"
	fi
) > /tmp/response.$$

# build header and send response
(
cat <<EOF
HTTP/1.1 $status
Server: http.sh
Date: `date -R`
Location: $path
Content-type: $mtype
Content-length: `wc -c /tmp/response.$$ | awk '{ print $1 }'`

EOF
) | sed 's/$/\r/g'

cat /tmp/response.$$

rm /tmp/response.$$
