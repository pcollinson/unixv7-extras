# make file for rk05 disk
# copy to /dev
# make -f rk.mk
RK0=rk0

all: RK0
	chmod 666 $(RK0) r$(RK0)

RK0:
	/etc/mknod $(RK0) b 0 0
	/etc/mknod r$(RK0) c 9 0

clean:
	rm -f $(RK0) r$(RK0)
