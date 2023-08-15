# Halt the running UNIX system safely

This code provides a small program and a system call to halt the UNIX V7 processor safely.

Back in 1976, I came to the conclusion that the UNIX filesystem tended to get corrupted when the system was closed down.

I implemented program called _killunix_, that would:

1.  Find and kill all the running processes.
2.  Then make a system call that would sync the disks and then call a piece of assembler in the kernel that halted the processor.

Closure was clean, and filesystem problems became rare. I now find on a simulated PDP11, I've been taking the system up and down very frequently sometimes causing disk problems, so implementing this system was an early step.

I've recoded the _killunix_ program as _halt_ for simpler use on SIMH. I've called it _halt_ so if it's typed on the host system by accident, it does nothing.

To install this system you need to:

1.  Run _installsys_, this:
    a. Installs _mch.s_ into _/usr/sys/conf/mch.s_. The version of the _mch.s_ has been edited to include the code in _ins_mch.s_. The original _mch.s_ is copied to a new the _config/orig_ directory.

    b. Replaces _/usr/sys/sys/sys4.c_ with _sys4.c_. A new function from _ins_sys4.c_ is appended at the end of the original file.

	c. Replaces /usr/sys/sys/sysent.c with sysent.c. This nominates an unused system call id, 62 to link to the stopunix() function in _sys4.c_. If you change the number, alter _stopunix.s_.

2.  Now recompile and install your kernel. The scripts I use to do this  can be found on _[../v7conf](../v7conf)_.

3.  Compile the halt command using _make_  will generate the command. Install  it in /bin and halt will now take your machine  down relatively safely.

## The _halt_ command

When the command is run, it lists the process ids of the processes that it kills. You may get the output from any shell you are running as it gets nuked.

The _halt_ command gets addresses from _/unix_ and reads data from the running kernel. If you've update the _/unix_ file, then processes will not be killed because the addresses it needs are not available but the processor will still halt.
