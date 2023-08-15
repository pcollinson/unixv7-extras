fsck:   fsck.c
	cc -s -n -O -o fsck fsck.c

install: fsck
	cp fsck /bin
	cp fsck.1m /usr/man/man1

clean:
	rm fsck
