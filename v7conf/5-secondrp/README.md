# 5-secondrp

If you need to repair the root file system, it's probably OK to use _icheck_, _dcheck_ or _fsck_ in single user mode. The kernel will be buffering the disk blocks for the filesystem, so use the block interface _/dev/rp0_ as the device to repair. Broken filesystems can often be read. However it's safer to repair offline filesystems.

If you need to repair the root file system on the main RP06 disk, it might be wise to create a second RP06 disk, and replace the new disk file with the current contents of _rp06-0.disk_. You can then check and fix the root file system that's offline, and then replace your working copy with the fixed image.

## v7boot.ini - SIMH Config

Adds a second RP06 disk.

## v6config - Kernel config

No change is needed to the kernel

## Installation

The _rp2.mk_ creates entries for the new disk in _/dev_.  The _hp_ driver for RP06 supports several overlapping partition settings supplying layouts for disks of different sizes. This V7 installation uses three - _rp0_ (minor device 0) for the first section of the disk, _swap_ (minor device 1) and _rp3_ (minor device 7) that uses the remainder of the RP06.

For the spare disk on drive 1, the minor device has 8 added to the device number. The value in the minor number above the bottom 3-bits selects the drive. I decided to ignore the swap partition and assign _rp4_ for the small partition and _rp5_ for the rest.

To create the devices, copy _rp2.mk_ to _dev_, go there and run

``` sh
make -f rp2.mk
```

again you can give the _makefile_ a _clean_ argument to remove the entries.

## Reformatting the second disk

If you want to create new filesystem on _rp4_ or _rp5_, you can use one or both of the _rp4.proto_ or _rp5.proto_. They are configured to match the sizes of the original distribution RP06.

``` sh
/etc/mkfs /dev/rrp4 rp4.proto
/etc/mkfs /dev/rrp5 rp5.proto
```

Use _icheck_, _dcheck_ or _fsck_ to check the disks.
