# unixv7-extras - making V7 more usable

Having spent some time working on UNIX V6 using the SIMH machine emulator for the PDP11/40. I decided to do the same for UNIX V7.

The version I've used runs on a simulated PDP11/70 and is from Keith Bostic. It can be found on [The Unix Heritage Society archive](https://www.tuhs.org/Archive/Distributions/Research/Keith_Bostic_v7]). It is a tape that you can unpack to create a single RP06 image. You can avoid the tape unpacking and use the RP06 disk image in these files [see v7conf/distro](v7conf/distro).

I must acknowledge the work of the folks at [The Unix Heritage Society](https://www.tuhs.org) who have made it their business to collect old source and versions of the Unix system. Along with all the people that made tapes and so left us some history. The  [TUHS archive](https://www.tuhs.org/Archive/) is a goldmine for people looking back at what was done.

## What's here

All of the below are fully documented with installation instructions. Directories called _orig_ contain the original code, makefiles and shell scripts contain compilation and installation commands.

This list is pretty much in the order that I worked on things.

* __[bootstrap](bootstrap)__ - bootstrap SIMH running V7.

* __[v7conf](v7conf)__ - tools to recompile the kernel to add in devices, and copies of the distribution including an initial RP06 image.

* __[sh-test](sh-test)__ - I found that my distribution was missing _/bin/[_, needed to support _if_ statements in the shell.

* __[uxtp](https://github.com/pcollinson/unixv6-extras/tree/main/uxtp)__ - I needed something to get files in and out of V6 so I decided to implement a version of _tp_, V6's magnetic tape archive program. I continued to use my _uxtp_ program to get files in and out of V7. V7 does have the _tar_ program, but its format is binary and isn't accessible from modern versions of _tar_.

* __[halt](halt)__ -  Back in the 1970's, I realised that the file system mostly failed after Unix  was shut down. I created a command that would kill all the running processes, and added a system call to the kernel to halt the system safely. Doing this made the filesystem breakages mostly disappear. You tend to take a simulated system up and down considerably more frequently than a physical system, so re-implementing this feature was a must. I've changed the way it works a little for V7.

* __[ssp](ssp)__ - I installed a version of Bill Joy's _ssp_ program so I could suppress blank lines when using the _man_ command.

* __[fsck](fsck)__ - There was a release called the _v7 addenda_ which can be found on the [TUHS archive](https://www.tuhs.org/Archive/Distributions/Research/), download _v7addenda.tar.gz_. The distribution contains a version of the _fsck_ program, I've put some time to make some small changes to it can be used on V7.

* __[tty](tty)__ - The V7 terminal driver was designed to work with printing devices and really doesn't work well with screens. I hated it in the 70's and that hate hasn't gone away. This version of _tty_ originated on a distribution that came from Nijmegen University. It allows you to use the DELETE key  to delete characters and for the characters to be removed from the screen when it's hit. Most control characters can be changed using the _stty_ command, which I decided to re-write because there wasn't a version supporting the new _tty_ driver.

* __[time](time)__ - V7 supports 32-bit arithmetic and times post 2000 can be displayed and entered. Some small changes are needed to allow this. Some programs that use _localtime_ can need changes and recompiling. I've also found a way to set the time on system boot.

## Licenses

Several licenses apply to this repo - see [License](License.md).
