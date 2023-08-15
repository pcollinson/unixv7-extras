# Configure SIMH and V7

This directory contains various setups for the V7 kernel and the instructions that are needed for SIMH's corresponding startup file. If you want to set things up from scratch visit the [bootstrap](../bootstrap) directory that holds a preloaded disk image. There are step-by-step instructions on how to get going, and what to expect.

## Recompiling the kernel

The kernel source is found on _/usr/sys_, there are several directories:

* _conf_ - the place to configure and rebuild the kernel
* _dev_ - sources for device drivers
* _h_ - header files, the files are also duplicated on _/usr/include/sys_
* _sys_ - source files for the remainder of the kernel

The initial files in _conf_ are really there to rebuild the distribution. The supplied _makefile_ doesn't do a good job of rebuilding the system when some parts of the source changes. You'll find a script called _conftidy_ here that moves all the files that are not needed into a newly built _orig_ directory. The _makefile_ here generates _unix_ binaries and has dependencies that will recompile the kernel safely.

The process of building the kernel:

* The _makefile_ assumes that the two libraries _sys/LIB1_ and _dev/LIB2_ are current. If you've changed any of their contents, you need to run ```make all``` to recreate them.
* System configuration depends on a file called _v7conf_, which needs to be copied in from [1-basic](1-basic). This is automatically run through the _mkconf_ command to generate _c.c_  containing the device switch tables. It also makes _l.s_ which adds code to link the interrupt system of the PDP11 into the C code.
* Once _v7conf_ is in place, type ```make``` and a working kernel will appear in the _unix_ file.
* I tend to copy this to _/nunix_ until I am sure that it will boot. There's always mileage of hanging onto a _/ounix_, just in case.

## Kernel Configurations

When you change the devices in the kernel, you need to change the SIMH setup file to attach the new devices. In addition, you may need to make new device nodes in _/dev_. The various setups below each contain:

* _v7conf_ - the kernel configuration file
* _v7boot.ini_ - the SIMH control file
* _*.mk_ - a shell script to make the devices that are needed in the system to access the new devices
* Other config files

The setups are:

* [_1-basic_](1-basic) - the initial setup to run the RP06 system
* [_2-add-dc_](2-add-dc) - add extra terminals that can be accessed by _telnet_ or _gtelnet_ to give you another four live terminals for the system.

   I used this setup to install the other new parts of _unixv7-extras_:
   * [_halt_](../halt) - add a halt command to cleanly shutdown the system
   * [_tty_](../tty) - new terminal driver
   * [_time_](../time) - ensure that the time command can use current times

* [_3-settime_](3-settime) - use the DECTAPE drive to set the system time at boot time. A small python program run from SIMH setup writes the current time onto the DECTAPE.

* [_4-testrk_](4-testrk) - I needed a disk that I could use to 'break' and  test the [fsck](../fsck) command

* [_5-secondrp_](5-secondrp) - add a new RP06 disk that can be used to clone the system disk for fixing (if necessary) using offline access to the root partition.
