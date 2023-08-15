# make file for ttys
# copy to /dev
# make -f ttys.mk
TTYS=tty00 tty01 tty02 tty03

all: $(TTYS)
	chmod 622 $(TTYS)

tty00:
	/etc/mknod tty00 c 3 0

tty01:
	/etc/mknod tty00 c 3 1

tty02:
	/etc/mknod tty00 c 3 2

tty03:
	/etc/mknod tty00 c 3 2

clean:
	rm -f $(TTYS)
