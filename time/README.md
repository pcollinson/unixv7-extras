# Making time Y2K compliant

Applications on the system that deal with time use a library routine _ctime.c_. It contains _gmtime_ that computes meaningful human time from the kernel clock running in seconds from 1970. This is a 32 bit value, and the problems with it running out of bits are well known. There are other routines in _ctime.c_ that are wrappers for _gmtime_ delivering the decoded information in various formats.

Unlike V6, there isn't a lot to do for V7. It's compiler is dealing nicely (or nearly nicely) with 32 bit 'longs'.

The _ctime.c_ routines also contain the timezone information, the offset from GMT and the local timezone names. These are set to suit users on the east side of the USA. The names of the timezone and the offset are in _timezone.c_, and that file can contain your timezone names. However, the actual days in the year that the time flips in and out of daylight savings time are set in _ctime.c_.

The main command that needs changing is _date_ that needs a tiny change to understand the two digits of the year given in its argument when setting time. As I was using _tp_ a lot, I've made a small change in that command to understand current dates - see [tp](tp).  The code should be compiled after the library has been updated.

## Installation

The _makefile_ is used to make a version of _date_ using the code in this directory, the intention is to provide a way of testing date setting using the new routines without having to change the C library. Running ```make``` will compile _date.c_, _ctime.c_ and _timezone.c_.

The binaries for  _ctime.c_ and _timezone.c_  can be installed in _/usr/lib_ using ```make install```.

## Changes to timezone.c:

I've added the name for the UK's summer time. If you want to add other European timezones, remember that the offset from GMT should be negative.

## Changes to _ctime.c_

First, I've added the day in the year that the UK changes from GMT to BST and back. I've done this using defines retaining the old code.

Second, I considered installing the 'correct' code to determine whether a year is a leap year or not. However, a computational experiment shows that there is no need to worry about this until 2100. So the current algorithm breaks well after we run out of space in a 32bit word to store the time. So I left the routine alone.

## Changes to _date.c_

The year in the date is set using two digits. I've installed a heuristic rule: if the two digit year is greater than or equal to 70 than the century is 1900, otherwise it's 2000.

## How the kernel remembers the date

When the system is booted, the system clock is set from the date stored in the superblock of the root filesystem. This is updated in all mounted devices by the _sync_ system call.

I've used the DecTape reader to set the time in V7 at startup, see [../v7conf/3-settime](../v7conf/3-settime).

## Programs that use the
