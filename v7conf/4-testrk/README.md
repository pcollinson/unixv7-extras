# 4-testrk - Add an RK05 drive

I wanted to add an RK05 drive to test bad file systems with [fsck](../../fsck). It builds on the settings in [3-settime](../3-settime).

## v7boot.ini - SIMH Config

Adds a single RK05 drive.

## v6config - Kernel config

Adds the _rk_ driver.

## Installation

Once the new kernel is built, copy _rk.mk_ to _dev_ and

``` sh
make -f rk.mk
```

Adding _clean_ to the _make_ command will remove the entries.

To make a filesystem on the new RK05, use the _mkfs_ program. _mfks_ needs a prototype file, and _rk.proto_ will do this. NB the contents of the proto file has changed some V6. The second line has two numbers, the first is the number of blocks to be used for the file system (4872), the second is the number of files that the system can contain - A.K.A the number of inodes - 2880. You need to be superuser:

``` sh
# /etc/mkfs /dev/rrk0 rk.proto
/usr/mdec/rkuboot: cannot  open init
m/n = 3 500
# icheck /dev/rrk0
/dev/rrk0:
files      2 (r=1,d=1,b=0,c=0)
used       1 (i=0,ii=0,iii=0,d=1)
free    4508
missing    0
```

It cannot find the boot file for the RK05, it complains but still makes the file system.  I've looked and cannot find a working _rkuboot.s_.

The file _rksys.proto_ sets up the RK05 with a system area and some blocks for swapping.
