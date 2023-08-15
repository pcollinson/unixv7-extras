# fsck - file system check program

This version of _fsck_ was distributed in the _v7addenda_ tape, but needed a little bit of work to make it work nicely on V7. I've added some _#defines_ and associated _#ifdef_ sections to preserve the original code.

This version will put orphaned files into a _lost+found_ directory that needs to be installed at the top of any file system you intend to run the code on. I've included a shell script that will make a suitable directory with space in its blocks for some files.

If you use _fsck_ on a large partition, the program asks for a temporary file, this should not be on the disk being checked.

## Compilation

Use ```make``` to compile the code, ```make install``` to install the binary and manual page, and ```make clean``` to remove the _fsck_ binary.

## Tests

The [tests](tests) directory contains several testing programs that can induce a file system problem, that _fsck_ can fix.
