/*
 * halt
 *
 * Kill all running processes
 * Kill all processes then call stopunix system call
 * which stops the processor
 *
 * Peter Collinson
 * July 2023
 */

#include <a.out.h>
#include <sys/param.h>
#include <sys/proc.h>

char *namelist "/unix";
char *corefile "/dev/kmem";

struct nlist nl[2];

struct proc proc[NPROC];
int proclist[NPROC];

unsigned getproc();
int *getps();

/* debug - set to non-zero for no killing or stopping */
#define DEBUG 0

main()
{
	register int *pr;
	int i, mypid, stopret;
	unsigned procaddr;

	if (!suser()) {
		printf("Use su to run this\n");
		exit(0);
	}

	/* get my pid */
	mypid = getpid();

	/*
	 * get address of _proc
	 * will be zero on error
	 * getps tests for this
	 */
	procaddr = getproc();

	/*
	 * early sync - sync returns if
	 * a sync is happening so
	 * delays make sense
	 */
	sync();
	sleep(1);

	/* get the details from proc */
	pr = getps(procaddr, mypid);

	/* pr is 0 on error, or no processes */
	if (pr) {
		printf("Kill running processes:\n");
		while (*pr) {
			printf("%d ", *pr);
			if (DEBUG == 0)
				kill(*pr, SIGKIL);
			pr++;
		}
		printf("\n");
		/*
		 * Allow processes to die
		 * because init is killed
		 * they will be in Zombie state
		 */
		sleep(3);
	}
	if (DEBUG == 0) {
		/* now shutdown the machine */
		sync();
		/* allow sync to settle */
		sleep(2);
		printf("Halting system\n");
		/* will return if update is busy */
		i = 0;
		while (stopunix() < 0) {
			printf("Disks busy\n");
			sleep(2);
			if (++i > 15) {
				printf("Halt abandoned - please stop by hand\n");
				break;
			}
		}
	}
	exit(0);
}

/*
 * Are we superuser or effective gid is superuser
 * return 1 if yes, 0 if no
 */
suser()
{

	return (getuid() == 0
		|| geteuid() == 0);
}

/*
 * Access the kernel's proc table
 * get active pids, ignore pid == 0  and mypid
 * return a zero terminated list or
 * zero if some failure or no processes to kill
 */
int *
getps(procaddr, mypid)
unsigned procaddr;
int mypid;
{
	register struct proc *pp;
	register int *lp;

	if (procaddr == 0
	    || readtable(procaddr) == 0)
		return(0);

	/*
	 * clear proclist so we can reuse this code
	 * if wanted
	 */
	for (lp = proclist; lp < &proclist[NPROC]; *lp++ = 0);

	/*
	 * We have data - but it might not be valid
	 * because /unix may not be the running kernel
	 * check all the p_stat values should be between 0 and SSTOP
	 */
	for (pp = &proc; pp < &proc[NPROC]; pp++) {
		if (pp->p_stat < 0
		    || pp->p_stat > SSTOP)
			return(0);
	}

	/*
	 * find active processes, ignore zombies
	 */
	lp = proclist;
	for (pp = &proc; pp < &proc[NPROC]; pp++) {
		if (pp->p_stat
		    && pp->p_stat != SZOMB
		    && pp->p_pid != 0
		    && pp->p_pid != mypid)
			*lp++ = pp->p_pid;
	}
	if (lp == proclist) {
		return(0);
	}
	return(proclist);
}

/* Get address of _proc in the current running kernel */
unsigned
getproc()
{
	copy7("_proc", nl[0].n_name);
	nlist(namelist, nl);
	if (nl[0].n_type == -1) {
		printf("Cannot get names from /unix\n");
		return(0);
	}
	return(nl[0].n_value);
}

/* read the table into memory */
readtable(procaddr)
unsigned procaddr;
{
	register int corefd;
	long pa;

	if ((corefd = open(corefile, 0)) < 0) {
		printf("Cannot open %s\n", corefile);
		return(0);
	}
	pa = procaddr;
	lseek(corefd, pa, 0);
	if (read(corefd, &proc, sizeof(proc)) != sizeof(proc)) {
		printf("Cannot read area for procs\n");
		close(corefd);
		return(0);
	}
	close(corefd);
	return(1);
}

/* copy names - limit space to 7 characters */
copy7(s, d)
register char *s, *d;
{	register i;

	for(i = 0; i < 7; i++)
		if(!(*d++ = *s++)) return;
}
