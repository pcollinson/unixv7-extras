/*
 * Create bad inodes on the disk
 *
 * Strategy:
 *	Create a new file to 'corrupt'
 *      need to be given a block device that's
 *      not mounted. This will be mounted on /mnt
 *	the necessary files created, and the
 * 	then the inodes for the files will be
 * 	messed with. fsck can then be run to
 *	patch the file system.
 *
 * 	Usage: badino [-l][-z][-v] ID
 *	ID is an action number to execute
 *	Other config information in conf.ini
 *	-z is needed to create the bad file or directory
 *	-v verbose, print all text not just error messages
 *	-l list action numbers and their actions
 *
 * Peter Collinson
 * June 2023
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "lib/lib.h"


#define zerofile "zero"	 	/* zero length file */
#define fiveblk "fiveblk"  	/* file with five blocks */
#define baddir "baddir"		/* directory */

#define DOUMNT 1
#define NOUMNT 0

int zflag;			 /* create bad file */

/* actions */
int clearlink(), clearadd(), badct(), badent();

/* descriptions */
#define ad0 "Clear link in empty file"
#define ad1 "Clear link in file with contents"
#define ad2 "Clear link in empty directory"
#define ad3 "Clear link in a directory with content"
#define ad4 "Create incorrect link count in a directory"
#define ad5 "Corrupt directory entry, set entry in directory to an illegal value"

struct actn {
	char *fname;	 /* file name to make */
	int isdir;       /* if 1, make a directory */
	int blks;	 /* if isdir == 0, then use this file size in blocks */
	int (*funcn)();  /* function to call to make things happen */
	char *desc;	 /* description */


} acttab[] {
	zerofile, 0, 0, &clearlink, ad0, 	/* action 0 - clear link in empty file */
        fiveblk,  0, 5, &clearlink, ad1,  	/* action 1 - clear link in file with contents */
        baddir,   1, 0, &clearlink, ad2,  	/* action 2 - clear link in directory */
        baddir,   1, 0, &clearadd,  ad3,  	/* action 3 - clear link in a directory with content */
        baddir,   1, 0, &badct,     ad4,  	/* action 4 - Create incorrect link count in a directory */
        baddir,   1, 0, &badent,    ad5,  	/* action 5 - Corrupt directory entry, set entry in
						 * directory to an illegal value */
	0, 0, 0, 0
};

/* convert inodes to block numbers, and offset */
/* current is the inode decode in play */
struct ino2blk {
	daddr_t bno;
	int offset;
} current;

/* Expanded disk addresses from a disk inode
 * created by addrexp
 */
daddr_t iaddr[NADDR];

/* buffer for reading inode blocks */
char blkbuf[BLKSIZ];

main(argc, argv)
int argc;
char **argv;
{
	int actionid;
	int acnt;

	if (suser() == 0) {
		printf("Run as root\n");
		exit(1);
	}

	acnt = ctactions();

	actionid = -1;

	if (argc == 1) {
		usage("No arguments");
	}

	while (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'l':
			listactions();
			exit(0);
		case 'z':
			zflag = 1;
			continue;
		case 'v':
			verbose = 1;
			continue;
		case 'q':
			/* assist debugging */
			abort();
			continue;
		default:
			usage();
		}
	}

	if (argc == 0) {
		usage("Please supply an action id");
	} else {
		argv++;
		actionid = atoi(*argv);
		if (actionid < 0 || actionid >= acnt)
			usage("Action id out of range");
	}
	if (actionid < 0)
		usage("Please supply an action id");

	/*
	 * safety checks and get info from conf.ini
	 */
	config(zflag);

	(*acttab[actionid].funcn)(&acttab[actionid]);
}

/* count the number of actions we have */
ctactions()
{
	register struct actn *ap;
	register int ct;

	ct = 0;
	for (ap = acttab; ap->fname; ap++)
		ct++;
	return(ct);
}

/* List actions */
listactions()
{
	register struct actn *ap;
	register int ct;

	for (ap = acttab; ap->fname; ap++) {
		printf("%2d: %s\n", ct, ap->desc);
		ct++;
	}
}

usage(msg)
char *msg;
{
	int acnt;

	acnt = ctactions();
  	printf("*** %s\n", msg);
	printf("Usage: badinode [-l][-z][-v] ID\n");
	printf("ID is an action number in the range 0-%d\n", acnt-1);
	printf("Other config information in conf.ini\n");
	printf("-z is needed to create the bad file or directory\n");
	printf("-v verbose, print all text not just error messages\n");
	printf("-l list action numbers and their actions\n");
	exit(1);
}

/*
 * start of actions
 */

/*
 * create a file, or directory
 * then set link count to 0 on the disk
 */
clearlink(ap)
struct actn *ap;
{
	register struct dinode *ip;
	ino_t ino;

	ino = inoset(ap, DOUMNT);

	ip = geti(ino);
	if (verbose)
		inopr(ino, ip);
	/* are we zapping */
	if (zflag) {
		/* doing the deed */
		ip->di_nlink = 0;
		puti();
		printf("Inode %d - %s now corrupt, run fsck or icheck -s to repair\n", ino, ap->fname);
	}
}

/*
 * create a directory, install a single file
 * then set link count to 0 on the disk
 */
clearadd(ap)
struct actn *ap;
{
	register struct dinode *ip;
	ino_t ino;

	ino = inoset(ap, NOUMNT);

	/* Now add content */
	addfile(mountpt, ap->fname, "fiveblk", 5);
	umntdev();

	ip = geti(ino);
	if (verbose)
		inopr(ino, ip);
	/* are we zapping */
	if (zflag) {
		/* doing the deed */
		ip->di_nlink = 0;
		puti();
		printf("Inode %d - %s now corrupt, run fsck or icheck -s to repair\n", ino, ap->fname);
	}
}

/*
 * create a directory, install a single file
 * then increase the link count by 5
 */
badct(ap)
struct actn *ap;
{
	register struct dinode *ip;
	int ino;

	ino = inoset(ap, NOUMNT);
	/* Now add content */
	addfile(mountpt, ap->fname, "fiveblk", 5);
	umntdev();

	ip = geti(ino);
	if (verbose)
		inopr(ino, ip);
	/* are we zapping */
	if (zflag) {
		/* doing the deed */
		ip->di_nlink = ip->di_nlink + 3;
		puti();
		printf("Inode %d - %s now corrupt, run fsck or icheck -s to repair\n", ino, ap->fname);
	}
}

/*
 * create a directory, insert two files
 * set inode value in an entry to more than
 * max inodes on the disk
 */
badent(ap)
struct actn *ap;
{
	ino_t ino;
	int imax;
	daddr_t dblk;
	int fd;
	off_t fsize;
	struct direct *dp;
	struct dinode *ip;

	/* need to evalate max inodes on the system */
	imax = maxino();

	ino = inoset(ap, NOUMNT);

	/* Add files */
	addfile(mountpt, ap->fname, "fiveblk", 5);
	addfile(mountpt, ap->fname, "zero", 0);
	umntdev();

	ip = geti(ino);
	if (verbose)
		inopr(ino, ip);

	/* get directory block from extended list*/
	dblk = iaddr[0];
	fsize = ip->di_size;

	/* about to zap the inode info */
	if ((fd = open(device, 2)) < 0) {
		perror(efmt("Open R/W access", device));
		exit(1);
	}
        bread(fd, blkbuf, dblk);

	dp = findent(blkbuf, "fiveblk", fsize);
	if (dp == 0) {
		printf("Cannot find fiveblk in the directory\n");
		close(fd);
		return;
	}
	printf("Found Inode: %d, filename: fiveblk\n", dp->d_ino);
	if (zflag) {
		dp->d_ino = imax + 30;
		printf("Directory entry is now corrupt, run fsck or icheck -s to repair\n", ino, ap->fname);
		printf("NB - fsck should also reconnect the file in lost+found\n");
		bwrite(fd, blkbuf, dblk);
	}
	close(fd);
}

/*
 * actions have lots of repeated code
 * put them into functions here
 */

/*
 * inoset the initial file from action
 * return ino of created file
 * DOUMNT and NOUMNT defines are used
 * to make the code more readable
 */
inoset(ap, doumount)
struct actn *ap;
int doumount;
{
	int ino;

	if (dev_is_mounted == 0)
		mntdev();
	ino = crfile(ap);
	if (doumount == DOUMNT)
		umntdev();
	return(ino);
}

/* add a file under a directory */
addfile(base, dirname, fname, blks)
char *base;
char *dirname;
char *fname;
int blks;
{
	char contents[64];

	makepath(contents, base, dirname);
	addpath(contents, fname);
	fileblk(contents, blks);
}

/*
 * get max number of inodes
 */
maxino()
{
	struct filsys *sup;
	int imax;
	int fd;

	sync();
	if ((fd = open(device, 0)) < 0) {
		perror(efmt("Open", device));
		exit(1);
	}
        bread(fd, blkbuf, SUPERB);
	close(fd);
	sup = blkbuf;
	imax = sizeof(struct dinode) * sup->s_isize;
	return(imax);
}

/*
 * find a filename in a directory block
 * return pointer to directory entry
 */
findent(bbuf, lookfor, fsize)
char *bbuf;
char *lookfor;
daddr_t fsize;
{
	register struct direct *dp;
	register int i;
	char fpadded[16];

	dp = bbuf;
	if (fsize > 512)
		fsize = 512;
	/* ignore . and .. */
	dp = &dp[2];
	for (fsize = fsize - 2*sizeof(struct direct);
	     fsize > 0;
	     fsize = fsize - sizeof(struct direct)) {
		for (i = 0; i < DIRSIZ; i++) {
			fpadded[i] = dp->d_name[i];
		}
		fpadded[DIRSIZ] = '\0';
		if (dp->d_ino != 0 && streq(fpadded, lookfor)) {
			return(dp);
		}
		dp++;
	}
	return(0);
}

/*
 * create test files
 * return inode of created object
 */
crfile(ap)
struct actn *ap;
{
	struct stat stino;
	char destfile[64];
	int mkstat;

	makepath(destfile, mountpt, ap->fname);

 	if (stat(destfile, &stino) >= 0) {
		return(stino.st_ino);
	}

	if (ap->isdir) {
		if ((mkstat = mkdir(destfile)) != 0) {
			printf("Mkdir of %s failed, status %d\n", destfile, mkstat);
			exit(1);
		}
		if (verbose)
			printf("Created dir %s in %s\n", destfile, device);

	} else {
		fileblk(destfile, ap->blks);
		if (verbose)
			printf("Created file %s in %s\n", destfile, device);
	}
 	if (stat(destfile, &stino) < 0) {
		perror(efmt("Stat", destfile));
		exit(1);
	}
	printf("%s, inode %d\n", destfile, stino.st_ino);
	return(stino.st_ino);
}

/*
 * create a file with random contents
 * of the number of blocks in bcnt
 */
fileblk(fname, bcnt)
char *fname;
int bcnt;
{
	register int fd;
	register int i;
	struct stat stino;
	char buf[512];

	/* don't make files that exist */
	if (stat(fname, &stino) == 0)
		return;

	fd = creat(fname, 0644);
	if (fd < 0) {
		perror(efmt("Create", fname));
		exit(1);
	}
	for (i = 0; i < bcnt; i++) {
		randblk(&buf);
		if (write(fd, buf, 512) < 0) {
			perror(efmt("Write", fname));
			exit(1);
		}
	}
	close(fd);
}

/*
 * Generate 512 random characters
 * in 8 lines of 63 characters and a newline
 */
randblk(dp)
char *dp;
{
	register *cp;
	register int i;
	int l;

	seedrand();

	for (l = 0; l < 8; l++) {
		for (i = 0; i < 16; i++) {
			cp = randchars();
			*dp++ = *cp++;
			*dp++ = *cp++;
			*dp++ = *cp++;
			if (i == 15)
				*dp++ = '\n';
			else
				*dp++ = *cp++;
		}
	}
}

/* random list of 63 characters */
char srclist[] "Ob1j4ioaGmIueswzvT9JhgNp62Z0CHYd3BqWXk Ql7U8DyK5LrVARfPnEtMxcSF";

/*
 * use rand to generate random chars from the list
 * rand will give us a number max of 077777
 * generate 4 characters from this number
 * by masking with 077000 and shifting by 9
 */
randchars()
{
	static char quad[4];
	register char *qp;
	register int mask;
	register int ashift;
	int randv;

	randv = rand();
	mask = 077000;
	qp = quad;
	for (ashift = 9; ashift >= 0; ashift =- 3) {
		if (ashift) {
			*qp++ = srclist[((randv & mask) >> ashift)];
		} else {
			*qp++ = srclist[(randv & mask)];
 		}
		mask = mask >> 3;
	 }
	 return quad;
}

/*
 * seed rand
 */
seedrand()
{
	int tvec[2];

	time(tvec);
	srand(tvec[1]);
}

/*
 * Code dealing with inodes on the disk
 */

/* Convert an inode number to a block address */
i2b(ino)
ino_t ino;
{
	register ino_t ino15;

	ino15 = ino + 15;
	current.bno = (daddr_t)(ino15 >> 3);
	current.offset = ino15&07;
}

/* Given an inode return a pointer to its contents */
geti(ino)
ino_t ino;
{
	register struct dinode *ip;
	int fd;

	sync();
	/* open the device */
	if ((fd = open(device, 0)) < 0) {
		printf("Cannot open: %s\n", device);
		exit(1);
	}
	i2b(ino);
	bread(fd, &blkbuf, current.bno);
	close(fd);

	ip = blkbuf;
	ip += current.offset;
	/* expand addresses */
	addrexp(ip);
	return (ip);
}

addrexp(ip)
struct dinode *ip;
{
	register char *p1;
	register char *p2;
	int i;

	p1 = (char *)iaddr;
	p2 = (char *)ip->di_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
}

/* write the block back */
puti()
{
	int fd;

	/* open the device */
	if ((fd = open(device, 1)) < 0) {
		printf("Cannot open: %s\n", device);
		exit(1);
	}
	bwrite(fd, &blkbuf, current.bno);
	close(fd);
}


/* Print an inode */
inopr(ino, ip)
ino_t ino;
register struct dinode *ip;
{
	register int i;

	printf("Inode %d:\n", ino);
	printf("Mode  %o\n", ip->di_mode);
	printf("Nlink %d\n", ip->di_nlink);
	printf("Uid   %d\n", ip->di_uid);
	printf("Gid   %d\n", ip->di_uid);
	printf("Size  %D\n", ip->di_size);
	printf("Addr  ");
	printf("Addr  ");
	for (i = 0; i < NADDR; i++)
		printf("%D ", iaddr[i]);
	printf("\n");
	printf("Atime %s", ctime(&ip->di_atime));
	printf("Mtime %s", ctime(&ip->di_mtime));
	printf("Ctime %s", ctime(&ip->di_ctime));
}
