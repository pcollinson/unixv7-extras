#
/*
 * getty -- adapt to terminal speed on dialup, and call login
 *
 * Delays removed from tables
 * SCOPE and INDCTL added
 * in /etc/ttys
 * use 14console
 * and 13tty00 for remaining terminals
 */

#ifdef LOCAL
#include "sys/sgtty.h"
#include "sys/deftty.h"
#else
#include <sgtty.h>
#include <sys/deftty.h>
#endif


#include <signal.h>
struct sgttyb tmode;
struct tchars tchars = { CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK };


struct	tab {
	char	tname;		/* this table name */
	char	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	char	*message;	/* login message */
} itab[] = {

/* table '0'-1-2-3 300,1200,150,110 */

	'0', 1,
	ANYP+RAW, ANYP+ECHO,
	B300, B300,
	"\n\r\033;\007login: ",

	1, 2,
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B1200, B1200,
	"\n\r\033;login: ",

	2, 3,
	ANYP+RAW, EVENP+ECHO,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

	3, '0',
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B110, B110,
	"\n\rlogin: ",

/* table '-' -- Console TTY 110 */
	'-', '-',
	ANYP+RAW, ANYP+ECHO+CRMOD+LCASE,
	B110, B110,
	"\n\rlogin: ",

/* table '1' -- 150 */
	'1', '1',
	ANYP+RAW, EVENP+ECHO,
	B150, B150,
	"\n\r\033:\006\006\017login: ",

 /* table '2' -- 9600 */
	'2', '2',
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B9600, B9600,
	"\n\r\033;login: ",

/* table '3'-'5' -- 1200,300 */
	'3', '5',
	ANYP+RAW, ANYP+ECHO+CRMOD+SCOPE+INDCTL+XTABS,
	B1200, B1200,
	"\n\r\033;login: ",

/* table '5'-'3' -- 300,1200 */
	'5', '3',
	ANYP+RAW, ANYP+ECHO+CRMOD,
	B300, B300,
	"\n\r\033;\007login: ",

/* table '4' -- Console Decwriter */
	'4', '4',
	ANYP+RAW, ANYP+ECHO+CRMOD+SCOPE+INDCTL+XTABS,
	B300, B300,
	"\n\rlogin: ",

/* table 'i' -- Interdata Console */
	'i', 'i',
	RAW+CRMOD, CRMOD+ECHO+LCASE,
	0, 0,
	"\n\rlogin: ",

/* table 'l' -- LSI Chess Terminal */
	'l', 'l',
	ANYP+RAW/*+HUPCL*/, ANYP+ECHO/*+HUPCL*/,
	B300, B300,
	"*",
/* table '6' -- 2400 11/23 line */
	'6', '6',
	ANYP+RAW, ANYP+ECHO,
	B2400, B2400,
	"\n\rlogin: ",

};

#define	NITAB	sizeof itab/sizeof itab[0]
#define	EOT	04		/* EOT char */

char	name[16];
int	crmod;
int	upper;
int	lower;

char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,
	0202,0004,0003,0205,0005,0206,0201,0001,
	0201,0001,0001,0201,0001,0201,0201,0001,
	0001,0201,0201,0001,0201,0001,0001,0201,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0201
};

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	int tname;

	tname = '0';
	if (argc > 1)
		tname = argv[1][0];
	switch (tname) {

	case '3':		/* adapt to connect speed (212) */
		ioctl(0, TIOCGETA, &tmode);
		if (tmode.sg_ispeed==B300)
			tname = '0';
		else
			tname = '3';
		break;
	}
	for (;;) {
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == tname)
				break;
		if(tabp >= &itab[NITAB])
			tabp = itab;
		tmode.sg_flags = tabp->iflags;
		tmode.sg_ispeed = tabp->ispeed;
		tmode.sg_ospeed = tabp->ospeed;
		ioctl(0, TIOCSETA, &tmode);
		ioctl(0, TIOCSETC, &tchars);
		puts(tabp->message);
		if(getname()) {
			tmode.sg_erase = CERASE;
			tmode.sg_kill = CKILL;
			tmode.sg_flags = tabp->fflags;
			if(crmod)
				tmode.sg_flags |= CRMOD;
			if(upper)
				tmode.sg_flags |= LCASE;
			if(lower)
				tmode.sg_flags &= ~LCASE;
			ioctl(0, TIOCSETA, &tmode);
			putchr('\n');
			execl("/bin/login", "login", name, 0);
			exit(1);
		}
		tname = tabp->nname;
	}
}

getname()
{
	register char *np;
	register c;
	char cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	for (;;) {
		if (read(0, &cs, 1) <= 0)
			exit(0);
		if ((c = cs&0177) == 0)
			return(0);
		if (c==CEOT)
			exit(1);
		if (c=='\r' || c=='\n' || np >= &name[16])
			break;
		putchr(cs);
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z') {
			upper++;
			c += 'a'-'A';
		} else if (c==CERASE) {
			if (np > name)
				np--;
			continue;
		} else if (c==CKILL) {
			putchr('\r');
			putchr('\n');
			np = name;
			continue;
		} else if(c == ' ')
			c = '_';
		*np++ = c;
	}
	*np = 0;
	if (c == '\r')
		crmod++;
	return(1);
}


puts(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		putchr(*s++);
}

putchr(cc)
{
	char c;
	c = cc;
	c |= partab[c&0177] & 0200;
	write(1, &c, 1);
}
