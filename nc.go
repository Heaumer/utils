package main

import (
	"io"
	"log"
	"net"
	"os"
	"strconv"
)

func help(argv0 string) {
	log.Println("Usage:\t", argv0, "-l port")
	log.Println("\t\t", argv0, "host port")

	os.Exit(1)
}

// read lines from r, write to w
func rw(r io.Reader, w io.Writer, m chan int) {
	buf := make([]byte, 4096)
	for {
		n, err := r.Read(buf)
		if err != nil || n == 0 {
			m <- 1
			return
		}
		n, err = w.Write(buf[0:n])
		if err != nil || n == 0 {
			m <- 1
			return
		}
	}
}

func handle(c net.Conn) {
	m := make(chan int)
	// read from network, write to stdout
	go rw(c, os.Stdout, m)
	// read from stdin, write to network
	go rw(os.Stdin, c, m)
	<- m; <- m
	c.Close()
}


func dial(host, port string) {
	c, err := net.Dial("tcp", host+":"+port)
	if err != nil {
		log.Fatal(err)
	}

	handle(c)
}

func listen(port string) {
	l, err := net.Listen("tcp", ":"+port)
	if err  != nil {
		log.Fatal(err)
	}

	for {
		c, err := l.Accept()
		if err != nil {
			continue
		}
		go handle(c)
	}
}

func main() {
	if len(os.Args) != 3 {
		help(os.Args[0])
	}

	port, err := strconv.Atoi(os.Args[2])
	if err != nil || port < 0 || port > 65535 {
		log.Fatal(os.Args[2], " is not a valid port number")
	}

	if os.Args[1] == "-l" {
		listen(os.Args[2])
	} else {
		dial(os.Args[1], os.Args[2])
	}
}
