#
/*
 * Superblock reader
 * Using conf.ini to establish test device
 * avoiding 'finger trouble'
 * and ensure that it's configured to safelt break a file system
 * See conf.ini
 *
 * Can be used with a device argument to print
 * superblock contents.
 *
 * -z - corrupt superblock freelist - uses conf.ini for settings
 * -v - verbose, default is to just print error messages
 * device - print superblock info for the device, -v not needed

 * Peter Collinson
 * June 2023
 */

#include "lib/lib.h"
#include <stdio.h>
#include <sys/param.h>
#include <sys/filsys.h>

char dbuf[BLKSIZ];

/* zap flag set */
int zflag;
/* use conf.ini to get info */
int useconf;

main(argc, argv)
int argc;
char **argv;
{
	register char *p;

	if (suser() == 0) {
		printf("Run as root\n");
		exit(1);
	}

	if (argc == 1)
		verbose = 1;
	while (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'z':
			zflag = 1;
			useconf = 1;
			continue;
		case 'v':
			verbose = 1;
			continue;
		default:
			usage();
		}
	}
	if (argc == 0)
		useconf = 1;
	else {
		argv++;
		if (zflag)
			usage();
		verbose = 1;
		strcpy(device, *argv);
	}
	if (useconf)
		config(zflag);
	if (zflag && dev_is_mounted)
		umntdev();
	process();
	exit(0);
}

/*
 * do the work
 */
process()
{
	int fd;

	if ((fd = open(device, 0)) < 0) {
		printf("Cannot open: %s\n", device);
		exit(1);
	}
	sync();
        bread(fd, &dbuf, SUPERB);
	close(fd);
	printsuper();
	if (zflag) {
		if ((fd = open(device, 2)) >= 0) {
			makebad(fd);
			close(fd);
			sync();
		} else {
			printf("Cannot open %s for read/write\n", device);
		}
	}
}

usage()
{
	printf("Usage: superb [-z][-v] [device]\n");
	printf("-z - corrupt superblock freelist - uses conf.ini for settings\n");
	printf("-v - verbose, default is to just print error messages\n");
	printf("device - print superblock info for the device, -v not needed\n");
	exit(1);
}

printsuper()
{
	register struct filsys *sup;

	if (verbose == 0) return;
	sup = dbuf;
	printf("%s:\n", device);
	printf("%6d size in blocks of I list\n", sup->s_isize);
	printf("%6D size in blocks of entire volume\n", sup->s_fsize);
	printf("%6d number free blocks (0-%d)\n", sup->s_nfree, NICFREE);
	printf("%6D block address of next %d free blocks\n", sup->s_free[0], NICFREE);
	if (sup->s_nfree != 0)
		blkprint(sup->s_free, sup->s_nfree);
	printf("%6d number of free I nodes (0-100)\n", sup->s_ninode);
	if (sup->s_ninode != 0)
		iprint(sup->s_inode, sup->s_ninode);
	printf("Last change: %s", ctime(&sup->s_time));
}

makebad(fd)
int fd;
{
	register struct filsys *sup;

	printf("Changing free list information in super block\n");
	sup = dbuf;
	if (sup->s_free[0] <= 1) {
		printf("Requires more than one free block\n");
		return;
	}
	printf("Duplicating first entry in block free list in second entry\n");
	/* duplicate block */
	sup->s_free[2] = sup->s_free[1];
	bwrite(fd, &dbuf, SUPERB);
	printf("Superblock is now corrupt, run fsck or icheck -s to repair\n");
}

blkprint(sp, free)
daddr_t *sp;
int free;
{
	register int i;
	register daddr_t *src;
	int linect;

	src = sp;
	linect = 0;
	for (i = 1; i < free; i++) {
		/* ignore the first entry */
		if (*(++src)) {
			printf("%7D", *src);
			if (++linect == 10) {
				printf("\n");
				linect = 0;
			}
		}
	}
	if (linect != 10) {
		printf("\n");
	}
}

iprint(sp, free)
ino_t *sp;
int free;
{
	register int i;
	register ino_t *src;
	int linect;

	src = sp;
	linect = 0;
	for (i = 1; i < free; i++) {
		/* ignore the first entry */
		if (*(++src)) {
			printf("%6d", *src);
			if (++linect == 10) {
				printf("\n");
				linect = 0;
			}
		}
	}
	if (linect != 10) {
		printf("\n");
	}
}
