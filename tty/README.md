# Unix V7 replacement terminal driver

The original V7 terminal driver was designed to be used with printing devices and doesn't play too well with the 'glass terminals' that we all use today. Personally I find that I don't get on with the use of '#' to delete a character, and 'delete' to send the interrupt signal. This was as true in the 1980's as it is now.

The _tty_ driver included here came from a distribution from Nijmegen University in Holland. It's similar  to the driver I've adopted for UNIX V6.  It came from a tape image [Torsten_Hippe_v7](https://www.tuhs.org/Archive/Distributions/Research/Torsten_Hippe_v7/)  I found on the [www.tuhs.org archive](https://www.tuhs.org/Archive). There are a couple of versions of this code available on the Archive, one is from [Nijmegen](https://www.tuhs.org/Archive/Distributions/Research/Nijmegen_v7/), but I think it's later than the version I am using here. This version is somewhat simpler and does the job.

## Installation steps

* Install and build the new kernel code replacing the old _tty.c_, its header and drivers. This uses files in the _sys_ directory. For more on that see [sys/README](sys).

* Run _make_ to compile the commands in this directory.  When installing the system, define _LOCAL_ in the _makefile_ to make compilations uses header files from the _sys_ directory. This allows you to create the binaries that will work with the newly compiled kernel, but without having to replace the files in _/bin_ and _/etc_.

*  Once you've installed the kernel code, you'll be surprised that the new erase/kill characters you have installed in the kernel are not set up when you login. This will continue until you replace _/etc/getty_ and _/bin/login_. The _sett_ program can be compiled and installed to set the values to what are now the defaults. It's a temporary measure but makes your life easier. On the plus side, your Interrupt character (by default ^C) will work now.

* Install _getty_ in _/etc/getty_,  reboot and check that it works. You do need to reboot, because _getty_ will be open and running in a live system. I've made quite a few changes to _getty_ to make it work better with the system API. The program is forked by _init_ and sits on terminal lines waiting for people to login. It uses _/etc/ttys_ to choose a setting for a specific terminal connection. Set the first character to 1 to enable a terminal The second character in each line of the file selects a terminal setup value from a table compiled into _getty_. Use '4' for the console, and '3' if you have additional terminals using the DC interface. However, when you install _getty_, you'll still find that _sett_ or _stty_ will still be needed, you need to replace _login_ before the full control character settings will appear on login.

* Install _login_ in  _/bin/login_,  reboot and check that it works. Now the default character controls will all be available to you on login.

* Install _stty_ in _/bin/stty.c_. I've written this version of _stty_,  because I couldn't find a version that fully supported the terminal driver. I've also created a version of the _man_ page for _stty_.

* Once things are working, and the appropriate files are installed in _/usr/sys/h_ and _/usr/include_, you can recompile the binaries and re-install removing the _LOCAL_ declaration from  the _makefile_.

## Directory

* [sys](sys) - Files for installing the new terminal interface and associated drivers into your kernel.

* orig - the original files from the V7 distribution.
