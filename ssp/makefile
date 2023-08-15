CFLAGS=-s -n -O

ssp:	ssp.c
	cc $(CFLAGS) -o ssp ssp.c

install: ssp
	cp ssp /usr/bin
	cp ssp.1 /usr/man/man1
