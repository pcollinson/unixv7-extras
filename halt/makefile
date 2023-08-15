# Make halt
halt: halt.c stopunix.o
	cc -O -o halt halt.c stopunix.o

stopunix.o: stopunix.s
	as -o stopunix.o stopunix.s

install: halt
	mv halt /bin/halt
	rm *.o
