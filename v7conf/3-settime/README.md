# 3-settime - set system time at bootstrap

This cannot be done without making changes to the _date_ command, see [date](../../time).

## v7boot.ini - SIMH Bootstrap

Adds the DecTape driver to the simulation. Before the device is added it calls the _timecmd.py_ script to setup the 'tape'.x

The SIMH driver will create a file _tc0.dec_ for the tape if none exists. DecTape is a blocked device, and the tape format is just a set of 512-byte blocks. SIMH reads in and buffers the contents.

## v7conf - Kernel config

Adds the DecTape driver.

## Installation

Add a '#' at the start of the _timecmd.py_ line in _v7conf.ini_, for now. Reboot the system with the new _v7conf.ini_. This will create the _tc.dec_ file for the tape.

Compile the kernel using _v7conf_ and reboot.

Use _tc.mk_ to create _/dev/tap0_, copy the file to _/dev_ and

``` sh
make -f tc.mk
```

Adding _clean_ to the _make_ command will remove the entries.

You can use the _tp_ program to test that things are working, to write to the DecTape:

``` sh
tp xv SOMEFILE
```

The contents won't appear in _tc0.dec_ until the simulation is stopped, or the device is re-attached.  I use a SIMH _do_ script to do this - _reldt_.

The _uxtp_ program will read DecTapes in _tp_ format, it understands that files ending in _.dec_ are DecTapes.

Once the DecTape is working, the time setting script in Python: _timecmd.py_  can be uncommented in _v7boot.ini_. The script writes a _date_ command and a newline at the start of block zero of the tape before the tape is attached.

Finally, you can install _settime_ in _/etc/_ on the system, and call that in _/etc/rc_. This script is called when the system goes multiuser before the terminals are started.

Running this means that you lose a few seconds while you are getting the system to start, but it's better than running the _date_ command by hand.
