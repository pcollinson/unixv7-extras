CONFIG=v7conf

unix:	l.o mch.o c.o ../sys/LIB1 ../dev/LIB2
	ld -o unix -X -i l.o mch.o c.o ../sys/LIB1 ../dev/LIB2

c.o:	c.c
	cc -c c.c

l.o:	l.s c.c
	as - -o l.o l.s

c.c:	mkconf $(CONFIG)
	mkconf < $(CONFIG)

mkconf:	mkconf.c
	cc -o mkconf -O mkconf.c

mch.o:	mch0.s mch.s
	as -o mch.o mch0.s mch.s

all:
	cd ../sys; cc -c -O *.c; mklib; rm *.o
	cd ../dev; cc -c -O *.c; mklib; rm *.o
