# 2-add-dc - add extra terminals to the basic system

## v7boot.ini - SIMH Bootstrap

Adding  4 DC11 lines to [1-basic](../1-basic). These lines are accessible from _telnet_ using _localhost_, port 5555. You can chose your own port value.

## v7conf - Kernel config

Adds 4dc lines

## Installation

Reboot using the new _v7boot.ini_, then compile and reinstall the kernel with the new _v7conf_.

Use _ttys.mk_ to create the terminal nodes in _/dev_. Copy the file to _/dev_ and

``` sh
make -f ttys.mk
```

Once that is done, you need to tell the _init_ program what terminals are to be started. The suggested values are in _ttys_ and this is be copied to _/etc_. The first character of each line is '1' or '0' to enable the terminal line. The second character is passed as an argument to _getty_ to select terminal settings. A copy of my _ttys_ file can be found here.

To make all this work, reboot the system.
