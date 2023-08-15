/*
 * Replacement for stty.c that deals with the
 * new values supported by the new terminal driver
 *
 * Peter Collinson
 * July 2023
 */
#include <stdio.h>

#ifdef LOCAL
#include "sys/sgtty.h"
#include "sys/deftty.h"
#else
#include <sgtty.h>
#include <sys/deftty.h>
#endif

/* Speed definitions */
struct speeds
{
	char	*string;
	int	speed;
} speeds[] = {
	"0",	B0,    "50",	B50,   "75",	B75,   "110",	B110,
	"134",	B134,  "134.5",	B134,  "150",	B150,  "200",	B200,
	"300",	B300,  "600",	B600,  "1200",	B1200, "1800",	B1800,
	"2400",	B2400, "4800",	B4800, "9600",	B9600,
	"exta",	EXTA,  "extb",	EXTB,
	0,
};

/*
 * ioctl targets
 */
struct sgttyb sgttyb;		/* stty/gtty - TIOCSETA/TIOCGETA */
struct tchars tchars;		/* extra characters - TIOCSETC/TIOCGETC */

/* External string code */

/*
 * source data groups
 */
#define GSPEED		1		/* Speeds */
#define GCHAR		2		/* Special characters */
#define GFLAGS  	3		/* Flags */
#define GDELAY		4		/* Delays */
#define GSCREEN		5		/* Screen sizes */
#define GCOMBO  	6		/* Value sets other values */
#define GALIAS		7		/* Alias for another value */
#define GFN		8		/* call a function */

/* Data modes */
#define AINVT           01		/* reverse sense of initial - */

/*
 * Service functions
 */
int dosane();
int dohup();

/*
 * Argument decode and print
 */
struct alist {
	char *key;		/* lookup value */
	short group;		/* data groups */
	short mode;		/* Modes */
	union {
		char *target;	/* where to update */
		short mask;	/* mask to use for flags */
		int (*serv)();	/* function to call */
	} u;
} alist[] = {
	"erase",  GCHAR,   0,     &sgttyb.sg_erase,
	"kill",   GCHAR,   0,     &sgttyb.sg_kill,
	"intr",   GCHAR,   0,     &tchars.t_intrc,
	"quit",   GCHAR,   0,     &tchars.t_quitc,
	"stop",   GCHAR,   0,     &tchars.t_stopc,
	"start",  GCHAR,   0,     &tchars.t_startc,
	"eof",    GCHAR,   0,     &tchars.t_eofc,
	"brk",    GCHAR,   0,     &tchars.t_brkc,

	/* speeds - need to look for numbers separately */
	"ispeed", GSPEED,  0,     &sgttyb.sg_ispeed,
	"ospeed", GSPEED,  0,     &sgttyb.sg_ospeed,
	"exta",   GCOMBO,  0,     "ispeed exta;ospeed exta",
	"extb",   GCOMBO,  0,     "ispeed extb;ospeed extb",

	/* screen sizes */
	"rows",	  GSCREEN, 0,     &sgttyb.sg_width,
	"cols",	  GSCREEN, 0,     &sgttyb.sg_length,
	"col",	  GALIAS,  0,	  "cols",

	/* Delays */
	"nl",	  GDELAY,  0,     &sgttyb.sg_nldly,
	"cr",	  GDELAY,  0,     &sgttyb.sg_crdly,
	"tab",	  GDELAY,  0,     &sgttyb.sg_htdly,
	"ff",	  GDELAY,  0,     &sgttyb.sg_vtdly,

	/* Flags */
	"tandem", GFLAGS,  0,     TANDEM,
	"cbreak", GFLAGS,  0,     CBREAK,
	"lcase",  GFLAGS,  0,     LCASE,
	"echo",   GFLAGS,  0,     ECHO,
	"nl",     GFLAGS,  AINVT, CRMOD,
	"raw",    GFLAGS,  0,     RAW,
	"cooked", GCOMBO,  0,     "-raw",
	"oddp",   GFLAGS,  0,     ODDP,
	"evenp",  GFLAGS,  0,     EVENP,
	"anyp",   GFLAGS,  0,  	  ANYP,
	"vdu",    GFLAGS,  0,     SCOPE,
	"scope",  GALIAS,  0,     "vdu",
	"ctrl",   GFLAGS,  0,     INDCTL,
	"tabs",   GFLAGS,  0,     XTABS,
	"sane",   GFN,     0,     &dosane,
	"hup",	  GFN,	   0,	  &dohup,
	0
};

main(argc, argv)
int argc;
char	*argv[];
{
	int fd = 1;

	/* First look for -f /dev/tty?? to use a different tty */
	argv++;
	if (--argc >= 2) {
		if (strcmp(argv[0], "-f") == 0) {
			if ((fd = open(argv[1], "r")) < 0)
				fatal("Cannot open: %s\n", argv[1]);
			argc -= 2;
			argv += 2;
		}
	}
	/* get the values */
	if (ioctl(fd, TIOCGETA, &sgttyb) < 0) {
		fatal("Cannot get stty information");
	}
	if (ioctl(fd, TIOCGETC, &tchars) < 0) {
		fatal("Cannot get ttchars information");
	}
	if (argc == 0) {
		prmodes();
		exit(0);
	}
	while (*argv) {
		argv = argparse(argv);
	}
	if (ioctl(fd, TIOCSETA, &sgttyb) < 0) {
		fatal("Cannot set stty information");
	}
 	if (ioctl(fd, TIOCSETC, &tchars) < 0) {
		fatal("Cannot set ttchars information");
	}
	prmodes();
	exit(0);
}

fatal(fmt, arg)
char *fmt;
char *arg;
{
	fprintf(stderr, fmt, arg);
	exit(1);
}

argparse(argv)
char **argv;
{
	register char *arg;
	register struct alist *ap;
	int haveminus;
	short suffix;
	char buf[64];

	arg = *argv;
	/* if the argument starts with a number, then it's a speed */
	if (*arg >= '0' && *arg <= '9') {
		if (findspeed(arg) < 0) {
			fatal("Cannot recognise speed: %s\n", arg);
		}
		sprintf(buf, "ispeed %s;ospeed %s", arg, arg);
		docombo(buf);
		return ++argv;
	}
	/* check for initial turn off */
	haveminus = 0;
	if (*arg == '-') {
		haveminus = 1;
		arg++;
	}
	strtolower(arg);
	/*
	 * now see if this is some text, followed immediately by a number
	 * side effect if value is not -1, arg is now a string
	 */
	strncpy(buf, arg, sizeof(buf));
	suffix = argnumber(buf);
	/* search for a match */
	ap = findoption(buf, haveminus, suffix);
	if (ap == NULL)
		fatal("Cannot find a match for %s\n", *argv);
	/*
	 * This is looking hopeful
	 * now apply the values
	 */
	argv++;
	argv = applyvals(argv, ap, haveminus, suffix);
	return argv;
}

applyvals(argv, ap, haveminus, suffix)
char **argv;
register struct alist *ap;
int haveminus;
short suffix;
{
	short mask;
	short value;

	switch (ap->group) {
	case GSPEED:
		value = findspeed(*argv);
		if (value < 0)
			fatal("Unknown speed %s\n", *argv);
		argv++;
		*ap->u.target = value;
		break;
	case GCHAR:
		value = parsechar(*argv);
		argv++;
		*ap->u.target = value;
		break;
	case GFLAGS:
		mask = ap->u.mask;
		if (haveminus || (ap->mode&AINVT)) {
			mask = ~mask;
			sgttyb.sg_flags &= mask;
		} else if (!haveminus || ((ap->mode&AINVT)) == 0)
			sgttyb.sg_flags |= mask;
		break;
	case GDELAY:
		if (suffix < 0)
			fatal("%s needs a number between 0 and 127\n", ap->key);
		*ap->u.target = suffix;
		break;
	case GSCREEN:
		value = parseint(*argv);
		if (value == -1)
			fatal("Unknown argument for %s\n", ap->key);
		if (value == -2)
			fatal("Argument for %s not in range\n", ap->key);
		argv++;
		*ap->u.target = value;
		break;
	case GCOMBO:
		docombo(ap->u.target);
		break;
	case GFN:
		(*ap->u.serv)();
		break;
	}
	return argv;
}

char
parseint(arg)
register char *arg;
{
	register int v;

	v = 0;
	while (*arg >= '0' && *arg <= '9')
		v = v*10 + *arg++ - '0';
	if (*arg != '\0')
		return -1;
	if (v > 0177) {
		return -2;
	}
	return v;
}

parsechar(arg)
register char *arg;
{
	if (strcmp(arg, "del") == 0)
		return 0177;
	if (strcmp(arg, "esc") == 0)
		return 033;
	if (strcmp(arg, "brk") == 0)
		return -1;
	if (arg[0] == '^' && arg[1] != '\0') {
		return arg[1]&037;
	}
	return arg[0];
}

dosane()
{
	tchars.t_intrc = CINTR;
	tchars.t_quitc = CQUIT;
	tchars.t_startc = CSTART;
	tchars.t_stopc = CSTOP;
	tchars.t_eofc = CEOT;
	tchars.t_brkc = CBRK;
	sgttyb.sg_erase = CERASE;
	sgttyb.sg_kill = CKILL;
	sgttyb.sg_width = 0;
	sgttyb.sg_length = 0;
	sgttyb.sg_flags = ECHO+CRMOD+ODDP+EVENP+SCOPE+INDCTL;
	sgttyb.sg_nldly = 0;
	sgttyb.sg_crdly = 0;
	sgttyb.sg_htdly = 0;
	sgttyb.sg_vtdly = 0;
}

dohup()
{
	ioctl(1, TIOCHPCL, NULL);
}

findoption(arg, haveminus, suffix)
register char *arg;
int haveminus;
short suffix;
{
	register struct alist *ap;

	for (ap = alist; ap->key; ap++) {
		if (haveminus && (ap->group != GFLAGS && ap->group != GALIAS)) {
			continue;
		}
		if (suffix >= 0 && (ap->group != GDELAY)) {
			continue;
		}
		if (strcmp(arg, ap->key) == 0) {
			if (ap->group == GALIAS) {
				return findoption(ap->u.target, haveminus, suffix);
			}
			return ap;
		}
	}
	return NULL;
}

strtolower(*p)
register char *p;
{
	for (;*p; p++)
		if (*p >= 'A' && *p <= 'Z')
			*p += 040;
}

/*
 * parse an argument getting an embedded number
 * and make any preceding text into a string
 */
argnumber(*p)
register char *p;
{
	int retval = -1;

	/* first char cannot be a number */
	if (*p >= '0' && *p <= '9')
		return retval;
	/* look for number start */
	for (;*p; p++)
		if (*p >= '0' && *p <= '9')
			break;
	/* found - make the number into binary */
	if (*p) {
		retval = parseint(p);
		*p = '\0';
	}
	return retval;
}

/*
 * Generate separate arguments from a string
 * separators are space and ;
 * also ignore any leading space
 */
comboscan(src, avec, stbuf)
register char *src;
char *avec[];
register char *stbuf;
{
	register int i;

	i = 0;
	while (*src) {
		avec[i++] = stbuf;
		avec[i] = NULL;
		while (*src == ' ') src++;
		while (*src && *src != ' ' && *src != ';') {
			*stbuf++ = *src++;
		}
		if (*src)
			src++;
		*stbuf++ = '\0';
	}
	return avec;
}

docombo(arg)
char *arg;
{
	register char **ap;
	char *avec[20];
	char stbuf[128];

	/* Parse the arguments */
	ap = comboscan(arg, avec, stbuf);
	/* Action them */
	while (*ap) {
		ap = argparse(ap);
	}
}

findspeed(arg)
char *arg;
{
	register i;

	for (i = 0; speeds[i].string; i++) {
		if (strcmp(arg, speeds[i].string) == 0) {
			return speeds[i].speed;
		}
	}
	return -1;
}

/* buffering for output */
#define LINELEN 80

/* Buffer to amass the line */
char line[LINELEN];
int linelen;
/* buffer for value creation */
char  valbuf[LINELEN];

prmodes()
{
	register struct alist *ap;

	linelen = 0;
	/* Speeds */
 	if (sgttyb.sg_ispeed != sgttyb.sg_ospeed) {
		addto(prspeed("input speed", sgttyb.sg_ispeed), NULL);
		addto(prspeed("output speed", sgttyb.sg_ospeed), "; ");
	} else
		addto(prspeed("speed", sgttyb.sg_ispeed), NULL);

	/* screen size */
	for (ap = alist; ap->key; ap++) {
		if (ap->group == GSCREEN) {
			sprintf(valbuf, "%s %d", ap->key, *ap->u.target);
			addto(valbuf, "; ");
		}
	}
	fprintf(stderr, "%s\n", line);
	linelen = 0;

	/* characters */
	for (ap = alist; ap->key; ap++)
		if (ap->group == GCHAR)
			addto(prchar(ap->key, *ap->u.target), "; ");
	fprintf(stderr, "%s\n", line);
	linelen = 0;

	/* flags */
	for (ap = alist; ap->key; ap++)
		if (ap->group == GFLAGS)
			addto(visflag(ap->key, ap->u.mask, ap->mode&AINVT), " ");
	fprintf(stderr, "%s\n", line);
	linelen = 0;

	/* delays */
	for (ap = alist; ap->key; ap++)
		if (ap->group == GDELAY) {
			sprintf(valbuf, "%s%d", ap->key, *ap->u.target);
			addto(valbuf, "; ");
		}
	fprintf(stderr, "%s\n", line);
	linelen = 0;
}

/*
 * Add a string to the linebuffer
 * if there no space, print the line
 * and reset the buffer counts
 * prefix the text with a separator, unless there's
 * just been a newline
 */
addto(str, sep)
register char *str;
char *sep;
{
	int slen;
	int seplen;

	slen = strlen(str);
	seplen = 0;
	if (sep)
		seplen = strlen(sep);
	if (linelen > 0 && linelen + slen + seplen >= LINELEN) {
		fprintf(stderr, "%s\n", line);
		linelen = 0;
	}
	if (linelen != 0 && sep) {
		strcpy(&line[linelen], sep);
		linelen += seplen;
	}
	strcpy(&line[linelen], str);
	linelen += slen;
	return linelen;
}

/*
 * print speeds
 */
prspeed(c, s)
char *c;
{
	register struct speeds *sp;
	register char *thissp = "??";

	for (sp = speeds; sp->string; sp++) {
		if (sp->speed == s) {
			thissp = sp->string;
			break;
		}
	}

	sprintf(valbuf, "%s %s baud", c, thissp);
	return valbuf;
}

/*
 * print name = 'ch'
 */
prchar(key, ch)
char *key;
char ch;
{
	char chbuf[4];

	sprintf(valbuf, "%s = '%s'", key, vischar(chbuf, ch));
	return valbuf;
}

/*
 * return characters as a printable string
 */
vischar(chbuf, c)
register char *chbuf;
char c;
{

	if (c > 0 && c < 040) {
		if (c == 033)
			return "esc";
		chbuf[0] = '^';
		chbuf[1] = c | 0100;
		chbuf[2] = '\0';
		return chbuf;
	}
	if (c == 0177)
		return "del";
	if (c == -1)
		return "brk";
	chbuf[0] = c;
	chbuf[1] = '\0';
	return chbuf;
}

/*
 * Return flags as name or -name
 */
visflag(name, mask, invert)
char *name;
short mask;
int invert;
{
	register char *p;

	strcpy(valbuf, "-");
	p = valbuf;
	if (sgttyb.sg_flags&mask) {
		if (invert) p++;
	} else {
		if (!invert) p++;
	}
	strcpy(p, name);
	return valbuf;
}
