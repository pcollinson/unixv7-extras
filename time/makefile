all: date timezone.o ctime.o

date: date.c timezone.o ctime.o
	cc -s -n -O -o date date.c ctime.o timezone.o

timezone.o: timezone.c
	cc -c -O timezone.c

ctime.o: ctime.c
	cc -c -O ctime.c

libinstall: ctime.o timezone.o
	ar r /lib/libc.a timezone.o ctime.o

install: /bin/date date
	sh install

clean:
	rm date *.o
