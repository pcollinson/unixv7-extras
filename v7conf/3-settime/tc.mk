# make file for tc dec tape
# copy to /dev
# make -f tc.mk
TC0=tap0

all: TC0
	chmod 666 $(TC0)

TC0:
	/etc/mknod tap0 b 4 0

clean:
	rm -f $(TC0)
