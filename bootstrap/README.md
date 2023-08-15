# Booting V7 on a SIMH PDP11/70

To generate a working V7 installation, I followed [Installing v7 on SIMH](https://gunkies.org/wiki/Installing_v7_on_SIMH). I include a copy of the RP06 disk image that is made by following the instructions on that page.  It can be used to give you a fast start to running V7.

## Basic installation

Start by making a directory to work in, say _v7_.

Get a working simulator, _git clone_ [https://github.com/simh/simh](https://github.com/simh/simh), change into the directory and run ```make pdp11```, you'll find the _pdp11_ command in the _BIN_ directory. You can install this binary in the _v7_ directory or on your own _bin_ directory if you have one.

Copy _rp06-0.disk.gz_ to the _v7_ directory, and unzip it using the _gunzip_ command.

Copy the _v7_ command script into your _v7_ directory.

This script expects to use a file called _v7boot.ini_ to configure the installation. It defines the devices that the _pdp11_ will support,  the _v7conf.ini_ file in this directory matches the current kernel installed on the disk. Copy _v7boot.ini_ into your _v7_ directory.

## Initial boot

Change into your _v7_ directory and type ``v7``:

``` sh
$ v7

PDP-11 simulator V4.0-0 Current        git commit id: 37bc857b

After Disabling XQ is displayed type boot
and at the : prompt type hp(0,0)unix

Disabling XQ
./v7boot.ini-10> att tm0 tm0.tap
%SIM-INFO: TM0: creating new file
%SIM-INFO: TM0: Tape Image 'tm0.tap' scanned as SIMH format
```
SIMH has created an empty file _tm0.tap_ that is the tape image, I'll come back to that later. There is a puzzling lack of a prompt, but it's waiting for you to type ``boot``  to load a small program to get things started. You can use ^H to delete characters. Back to the listing (including the last line we had above):

``` sh
%SIM-INFO: TM0: Tape Image 'tm0.tap' scanned as SIMH format
boot
:
```
Now we have a prompt _:_, and need to load the kernel code, type ``hp(0,0)unix``. This loads the code for the kernel from HP device zero, looking for the file in partition zero. Note this allows you to load other versions of the kernel, always keep a working version around.

``` sh
boot
: hp(0,0)unix
mem = 2020544
#
```
You are now booted into a single user system, using just the root partition. If you type stuff, you'll find that it comes out in upper-case, which is convenient if your console is an ASR33 teletype. To get into multiuser mode, type Control-D, which is not echoed:

``` sh
: hp(0,0)unix
mem = 2020544
# RESTRICTED RIGHTS: USE, DUPLICATION, OR DISCLOSURE
IS SUBJECT TO RESTRICTIONS STATED IN YOUR CONTRACT WITH
WESTERN ELECTRIC COMPANY, INC.
WED DEC 31 19:10:00 EST 1969

login:
```

You can login as ```root``` with a password of ```root```, or use ```dmr``` as a user with no password, and type a few commands:

``` sh
login: dmr
$ ls
$ pwd
/usr/dmr
$
```

To stop the simulation, you need to ensure that any disk writes are written to the disk before stopping, to do this type ```sync``` a few times. Then hit ^E to return to the simulator, and finally ```q``` to quit.

``` sh
$ pwd
/usr/dmr
$ sync
$ sync
$ sync
$ sync
$
Simulation stopped, PC: 002306 (MOV (SP)+,177776)
sim> q
Goodbye
```

# Getting files in and out

Although V7 has a _tar_, it uses a binary format, and not the text format that was eventually adopted by POSIX to become the standard. I wrote a version of the _tp_ program in Python for the V6 project - see [uxtp](https://github.com/pcollinson/unixv6-extras/tree/main/uxtp). Sadly, _tp_ on the V6 and V7 won't create directories, so I included the ability to add a shell script to the dump called _makedirs.sh_ that creates any directory that is needed. This has proved a very useful idea.

Here's a demo of getting files from this distribution into V7. Let's move the contents _sh-test_ which looks for  and installs _/bin/['_ which I found was missing on my distribution when I started writing shell scripts.  The _tp_ internal file format includes the path name to the files, and you are limited to 31 characters, so you need to start close to the source. The program will write into the _tm0.tap_ that SIMH made earlier. Your paths may vary below.

``` sh
$ cd unixv7-extras
$ uxtp -rvs ../v7/tm0.tap sh-test
Adding makedirs.sh
Adding sh-test/README.md
Adding sh-test/check
```

Now you can fire up the V7 system and login:

``` sh
$ tp xvm makedirs.sh
x makedirs.sh
End
$ sh makedirs.sh
$ tp xvm
x makedirs.sh
x sh-test/README.md
x sh-test/check
End
$ ls sh-test
README.md
check
$
```

This sequence first gets the _makedirs.sh_ file from the tape, and next it's run to create any necessary directories. Then _tp_ command extracts the contents into the directories that were made. Finally run _ls_ command to look at the files.

You'll find that if you write to the tape from V7 using ```tp rm files```, it will appear on _tm0.tap_ and can be read and used immediately. Incidentally, this is not true for DECTAPE.

If you want to get files into the system while V7 is live, then you need to detach and reattach the tape file. I have a little SIMH control file that resets the tape. First you need to type ^E to get back to the simulator control level. This pauses the running V7 and you can return using the _co_ command. SIMH has a ```do``` command that allows you to put commands in a file. I have included a file called _relm_ which detaches and re-attaches the tape drive.

``` sh
$
Simulation stopped, PC: 002360 (MOV (SP)+,177776)
sim> do relm
./relm-2> att tm0 tm0.tap
%SIM-INFO: TM0: Tape Image 'tm0.tap' scanned as SIMH format

```

The ^E I typed at the V7 prompt is not shown. The _relm_ file ends in ```co``` to continue back to the running system. To get the prompt again, type return. You can now read the new data from the tape.

## What next?

The next step is to configure the kernel adding new devices - see [v7conf](../v7conf).
