# Install new tty driver & device interfaces

This version of tty.c came from a distribution from the University of Nijmegen.

## Summary of changes

The original tty driver for V7 used three character queues: one for output, and two for input. Characters from the devices were placed on an input queue and processed to the 'canonical' queue when the user pressed return. When return was entered, the canonical queue was passed into user space processing  all the delete characters and other editing characters. The new code uses a single input queue, and the characters used to modify the input data operate directly on the queue. There is a new routine _zapc_ that removes characters from the end of the queue to assist with the editing.

The change meant that the _tty_ structure in _tty.h_ was changed significantly. I've also moved all the special character definitions into _deftty.h_ to allow other programs to get at the values. The change also removes support for the Packet Driver and multiplexer which are controlled by defines in _tty.h_.

As well as _tty.c_ essentially being replaced, the drivers that use it need slight alterations.

## Files

* _dc.c_:  DC11 driver code. Small change to support the full range of _ioctl_ commands.
* _deftty.h_:   new file with control characters and modes abstracted from _tty.h_.
* _dh.c_:  DH11 driver, changes for initial terminal setup, and support for paging.
* _kl.c_:  KL11/DL11 driver.
* _dz.c_:  A reworked driver from the Nijmegen distribution.
* _pk.p_: The old code uses one of the terminal queues and clears the other. Now only one queue exists, and the code clears it and then uses it.
* _sgtty.h_: This header file contains public versions of the various tty setting values, it needs revision for the new _tty.c_.
* _tty.c_: New terminal driver.
* _tty.h_: New header file.

Original V7 files can be found in the _orig_ directory.

## Installation

These scripts all need an argument which is the destination in the filesystem you are changing. This is to stop running a script without intent and then wondering how to get back to where you were. You'll get the same error message for each script that is called with no argument, or the target directory cannot be opened.

Before you start this, I do recommend that you stop the simulator and take a copy of the _rp06-0.disk_.

You will need to run these scripts as root. They must to be run from this directory changing files in _/usr/sys_ or anywhere else.  They are all as defensive as possible and can be run more than once.

* __step1.sh__
provides backup files in various _orig_ directories. It creates directories called _orig_ in the various directories that contain files that will be overwritten or removed.  Files in any _orig_ directory are not overwritten. The script needs the _/bin/[_ command, and tests for it (see [sh-test](../../sh-test)).

* __step2.sh__
installs new files in the various system directories, files are not overwritten unless a backup exists in an _orig_ directory.

* __step3.sh__
is a verification step comparing the files in this directory with the versions in the destination tree.

You can now rebuild your new kernel. Scripts to assist with this can be found on [_../../v7conf_](../../v7conf). Make sure you keep a copy of your running kernel so you can revert in case of problems.
