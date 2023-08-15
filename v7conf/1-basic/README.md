# 1-basic

This is the basic setup for the V7 system that is created by the installation.

## v7boot.ini - SIMH Bootstrap

Sets up a PDP11/70 with 2M of memory.  It has a single RP06 disk and a tapedrive. Finally it will boot from block zero on the disk.

## v7conf - the kernel config

Defines the root and swap partitions on the RP06, and includes the tape drive interface.
