# Testing fsck

This directory contains testing programs that will induce problems in a file system.

## How to compile

* ```cd lib```
* ```make``` - will create a library called _testlib_ in the current directory.
* ```cd  ..```
* ```make``` will generate three binaries.

## How to use

Running these tests on a live file system is a bad idea. I've added a new RK05 to my system to provide a test platform. See [../../v7conf/4-testrk](../../v7conf/4-testrk) for how to do this. Once the device is added, make a file system on the drive and copy some files in to give the file system some substance.

The tests here are intended to be done on a 'disk' whose file system is sacrificial.

## Tests

To avoid finger trouble, the file _conf.ini_ holds the name of the disk, the directory used to mount it, and the name of a safety file that must be present for the disk to be modified. Doing this avoids using the command line which is subject to finger trouble.

Use the commands below one at a time and fix the filesystem using _fsck_ after the damage is caused.

### confpr
Prints out the current settings in conf.ini.

### superb
Will print superblock contents and can cause a duplicate block to be added to the free block list in the superblock on the disk. Fix the filesystem when you break it.

``` C
Usage: superb [-z][-v] [device]
-z - corrupt superblock freelist - uses conf.ini for settings
-v - verbose, default is to just print error messages

device - print superblock info for the device, -v not needed
superb with no arguments, prints out the superblock on the current
testing filesystem, and puts the verbose flag on.

The -z flag is needed to corrupts the filesystem.
```

### badino
Introduces various errors into the filesystem. They are intended to be  used and then the filesystem repaired before using another action.

The general format:

``` C
badino [-l][-z][-v] ID

ID is an action number in the range 0-5
Other config information in conf.ini
-z is needed to create the bad file or directory
-v verbose, print all text not just error messages
-l list action numbers and their actions

The -z flag is needed to break the file system. The code will
create a directory called baddir, a file called zero, and a file
called fiveblks - depending on what it wants to do.

badino -l prints

0: Clear link in empty file
1: Clear link in file with contents
2: Clear link in empty directory
3: Clear link in a directory with content
4: Create incorrect link count in a directory
5: Corrupt directory entry, set entry in directory to an illegal value

```

## Library

I put a lot of useful code into the [library](lib).
