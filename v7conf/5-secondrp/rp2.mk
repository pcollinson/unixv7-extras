# Makefile for second rp disk
# Use rp4 and rp5 for partitions
# provide access to copy of rp0
RP4=rp4
RP5=rp5

all: $(RP4) $(RP5)
	chmod go-w $(RP4) r$(RP4) $(RP5) r$(RP5)

$(RP4):
	/etc/mknod $(RP4) b 6 8
	/etc/mknod r$(RP4) c 14 8

$(RP5):
	/etc/mknod $(RP5) b 6 15
	/etc/mknod r$(RP5) c 14 15

clean:
	rm -f $(RP4) r$(RP4) $(RP5) r$(RP5)
