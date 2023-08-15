/*
 * A clist structure is the head
 * of a linked list queue of characters.
 * The characters are stored in 4-word
 * blocks containing a link and several characters.
 * The routines getc and putc
 * manipulate these structures.
 */
struct clist {
	int	c_cc;		/* character count */
	char	*c_cf;		/* pointer to first char */
	char	*c_cl;		/* pointer to last char */
};

/*
 * Special character structure.
 * All special characters may be redefined, so
 * they are held within the tty structure.
 */
struct tc {
	char	t_intrc;	/* interrupt */
	char	t_quitc;	/* quit */
	char	t_startc;	/* start output */
	char	t_stopc;	/* stop output */
	char	t_eofc;		/* end-of-file */
	char	t_brkc;		/* input delimiter (like nl) */
};

/*
 * A tty structure is needed for
 * each UNIX character device that
 * is used for normal terminal IO.
 * The routines in tty.c handle the
 * common code associated with
 * these structures.
 * The definition and device dependent
 * code is in each driver. (kl.c dc.c dh.c)
 */
struct tty {
	struct clist t_rawq;	/* input list from device */
	struct clist t_outq;	/* output list to device */
	int	(* t_oproc)();	/* routine to start output */
	int	(* t_iproc)();	/* routine to start input */
	short	t_pgrp;		/* process group name */
	caddr_t	t_addr;		/* device address */
	dev_t	t_dev;		/* device number */
	char	t_ispeed;	/* input speed */
	char	t_ospeed;	/* output speed */
	char	t_erase;	/* erase character */
	char	t_kill;		/* kill character */
	short	t_flags;	/* mode, settable by ioctl call */
	char	t_nldly;	/* delay for newline */
	char	t_crdly;	/* delay for carriage return */
	char	t_htdly;	/* delay for horizontal tab */
	char	t_vtdly;	/* delay for vertical tab */
	char	t_width;	/* max. line length for folding */
	char	t_length;	/* max. screen length for paging */
	union {
		struct tc;
		struct clist t_ctlq;
	} t_un;
	short	t_state;	/* internal state, not visible externally */
	short	t_xstate;	/* as above, for extra stuff */
	char	t_lnum;		/* line number on screen */
	char	t_col;		/* printing column of device */
	char	t_delct;	/* number of delimiters in raw q */
	char	t_char;		/* character temporary */
	struct chan *t_chan;	/* destination channel */
	caddr_t	t_linep;	/* aux line discipline pointer */
	char	t_line;		/* line discipline */
};

#define	tun	tp->t_un

/*
 * structure of arg for ioctl
 */
struct	ttiocb {
	char	ioc_ispeed;
	char	ioc_ospeed;
	char	ioc_erase;
	char	ioc_kill;
	short	ioc_flags;
	char	ioc_nldly;
	char	ioc_crdly;
	char	ioc_htdly;
	char	ioc_vtdly;
	char	ioc_width;
	char	ioc_length;
};

#define	TTIPRI	28
#define	TTOPRI	29

#include "../h/deftty.h"

/*
 * limits
 */
#define	TTHIWAT	100
#define	TTLOWAT	50
#define	TTYHOG	256

/*
 * modes
 */
#define	TANDEM	01
#define	CBREAK	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define SCOPE	0400
#define INDCTL	01000
#define XTABS	02000

/*
 * Hardware bits
 */
#define	DONE	0200
#define	IENABLE	0100

/*
 * Internal state bits
 */
#define	TIMEOUT	01		/* Delay timeout in progress */
#define	WOPEN	02		/* Waiting for open to complete */
#define	ISOPEN	04		/* Device is open */
#define	FLUSH	010		/* outq has been flushed during DMA */
#define	CARR_ON	020		/* Software copy of carrier-present */
#define	BUSY	040		/* Output in progress */
#define	ASLEEP	0100		/* Wakeup when output done */
#define	XCLUDE	0200		/* exclusive-use flag against open */
#define	TTSTOP	0400		/* Output stopped by ctl-s */
#define	HUPCLS	01000		/* Hang up upon last close */
#define	TBLOCK	02000		/* tandem queue blocked */
#define	CNTLQ	0100000		/* interpret t_un as clist */
/*
 * More internal state bits (for t_xstate)
 */
#define XPAGE	01	/* output is being paged */
#define XPAGE1	02	/* set to indicate stopped at page boundary */
#define XPAGE2	04	/* set to stop ttwrite after a page is on the queue */
#define XLITRL	010	/* last character was a literal escape */
#define XERASE	020	/* erase string in progress */

/*
 * tty ioctl commands
 */
#define	TIOCGETD	(('t'<<8)|0)
#define	TIOCSETD	(('t'<<8)|1)
#define	TIOCHPCL	(('t'<<8)|2)
#define	TIOCMODG	(('t'<<8)|3)
#define	TIOCMODS	(('t'<<8)|4)
#define	TIOCGETP	(('t'<<8)|8)
#define	TIOCSETP	(('t'<<8)|9)
#define	TIOCSETN	(('t'<<8)|10)
#define TIOCSETA	(('t'<<8)|11)
#define TIOCGETA	(('t'<<8)|12)
#define	TIOCEXCL	(('t'<<8)|13)
#define	TIOCNXCL	(('t'<<8)|14)
#define TIOCNRD		(('t'<<8)|15)
#define	TIOCFLUSH	(('t'<<8)|16)
#define	TIOCSETC	(('t'<<8)|17)
#define	TIOCGETC	(('t'<<8)|18)
#define	DIOCLSTN	(('d'<<8)|1)
#define	DIOCNTRL	(('d'<<8)|2)
#define	DIOCMPX		(('d'<<8)|3)
#define	DIOCNMPX	(('d'<<8)|4)
#define	DIOCSCALL	(('d'<<8)|5)
#define	DIOCRCALL	(('d'<<8)|6)
#define	DIOCPGRP	(('d'<<8)|7)
#define	DIOCGETP	(('d'<<8)|8)
#define	DIOCSETP	(('d'<<8)|9)
#define	DIOCLOSE	(('d'<<8)|10)
#define	DIOCTIME	(('d'<<8)|11)
#define	DIOCRESET	(('d'<<8)|12)
#define	FIOCLEX		(('f'<<8)|1)
#define	FIONCLEX	(('f'<<8)|2)
#define	MXLSTN		(('x'<<8)|1)
#define	MXNBLK		(('x'<<8)|2)
