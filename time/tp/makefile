CFLAGS=-s -O

all:	tp
	rm *.o

install: tp /bin/tp
	sh install

tp:	tp0.c tp1.o tp2.o tp3.o
	cc $(CFLAGS) -n tp0.c tp1.o tp2.o tp3.o -o tp

tp0.c tp1.c tp2.c tp3.c: tp.h

clean:
	rm *.o
