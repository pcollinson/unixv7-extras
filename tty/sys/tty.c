#
/*
 * general TTY subroutines
 *
 * Defines are
 * MX to include calls to the MX code, it's not clear that
 * this works.
 * DELTAB (defined) delete tabs on input line
 * SGTTY (defined) include gtty and stty hooks
 * Peter Collinson
 * July 2023
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/proc.h"
#ifdef MX
#include "../h/mx.h"
#endif MX
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/reg.h"
#include "../h/conf.h"


struct cblock {
	struct cblock *c_next;
	char	c_info[CBSIZE];
};

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by '\', the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 * The lower-case representation is also or-ed with the parity bit.
 */
#define P	0200

char maptab[] = {
    0,     P,     P,     0,     P,     0,     0,     P,
    P,     0,     0,     P,     0,     P,     P,     0,
    P,     0,     0,     P,     0,     P,     P,     0,
    0,     P,     P,     0,     P,     0,     0,     P,
    P,   '|',     0,     P,     0,     P,     P,   '`',
  '{', P+'}',     P,     0,     P,     0,     0,     P,
    0,     P,     P,     0,     P,     0,     0,     P,
    P,     0,     0,     P,     0,     P,     P,     0,
    P,     0,     0,     P,     0,     P,     P,     0,
    0,     P,     P,     0,     P,     0,     0,     P,
    0,     P,     P,     0,     P,     0,     0,     P,
    P,     0,     0,     P,  '\\',     P, P+'~',     0,
    0, P+'A', P+'B',   'C', P+'D',   'E',   'F', P+'G',
P+'H',   'I',   'J', P+'K',   'L', P+'M', P+'N',   'O',
P+'P',   'Q',   'R', P+'S',   'T', P+'U', P+'V',   'W',
  'X', P+'Y', P+'Z',     0,     P,     0,     0,     P
};

/*
 * cchars is used to  determine which characters
 * are non-printing (^G is printing).  Non-printing
 * characters can be echoed as a pair of printing characters.
 */
char cchars[] = {
	0177, 0300, 0377, 0377,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0, 0200
};

/*
 * wchars is used to determine whether a character is
 * alphanumeric, and therefore part of a word.
 */
char wchars[] = {
	   0,    0,    0,    0, 0200,    0, 0377,   03,
	0376, 0377, 0377,   07, 0376, 0377, 0377,   07
};

#define iscntrl(c)	(ttybit(cchars, c))
#define isspcl(c)	(ttyspcl(c, tp))
#define isupper(c)	('A' <= c && c <= 'Z')
#define isword(c)	(ttybit(wchars, c))
#define max(a, b)	((a) > (b) ? (a): (b))
#define ubyte(c)	((c)&0377)

#define SGTTY		/* include stty and gtty */
#define DELTAB		/* on-screen deletion of tabs if both SCOPE and XTABS */

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttyopen(dev, tp)
dev_t dev;
register struct tty *tp;
{
	register struct proc *pp;

	pp = u.u_procp;
	tp->t_dev = dev;
	if(pp->p_pgrp == 0){
		u.u_ttyp = tp;
		u.u_ttyd = dev;
		if(tp->t_pgrp == 0)
			tp->t_pgrp = pp->p_pid;
		pp->p_pgrp = tp->t_pgrp;
	}
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}

/*
 * set default control characters
 * and sizes
 */
ttychars(tp)
register struct tty *tp;
{
	tun.t_intrc = CINTR;
	tun.t_quitc = CQUIT;
	tun.t_startc = CSTART;
	tun.t_stopc = CSTOP;
	tun.t_eofc = CEOT;
	tun.t_brkc = CBRK;
	tp->t_erase = CERASE;
	tp->t_kill = CKILL;
	tp->t_width = 0;
	tp->t_length = 0;
}

/*
 * clean tp on last close
 */
ttyclose(tp)
register struct tty *tp;
{
	tp->t_pgrp = 0;
	wflushtty(tp);
	tp->t_state = 0;
	tp->t_xstate = 0;
}

#ifdef SGTTY
/*
 * stty/gtty writearound
 */
stty()
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCSETP;
	ioctl();
}

gtty()
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCGETP;
	ioctl();
}
#endif SGTTY

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
ioctl()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;
	register dev_t dev;
	register fmt;

	uap = (struct a *)u.u_ap;
	if((fp = getf(uap->fdes)) == NULL)
		return;
	if(uap->cmd == FIOCLEX){
		u.u_pofile[uap->fdes] |= EXCLOSE;
		return;
	}
	if(uap->cmd == FIONCLEX){
		u.u_pofile[uap->fdes] &= ~EXCLOSE;
		return;
	}
	ip = fp->f_inode;
	fmt = ip->i_mode&IFMT;
	if(fmt != IFCHR && fmt != IFMPC){
		u.u_error = ENOTTY;
		return;
	}
	dev = (dev_t)ip->i_un.i_rdev;
	(*cdevsw[major(dev)].d_ioctl)(dev, uap->cmd, uap->cmarg, fp->f_flag);
}

/*
 * Common code for several tty ioctl commands
 */
ttioccomm(com, tp, addr, dev)
register struct tty *tp;
caddr_t addr;
{
	unsigned t;
	register n;
	extern int nldisp;
	struct	sttiocb {
		char	ioc_ispeed;
		char	ioc_ospeed;
		char	ioc_erase;
		char	ioc_kill;
		short	ioc_flags;
	};

	switch(com){
	/*
	 * get discipline number
	 */
	case TIOCGETD:
		t = tp->t_line;
		if(copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;
	/*
	 * set line discipline
	 */
	case TIOCSETD:
		if(copyin(addr, (caddr_t)&t, sizeof(t))){
			u.u_error = EFAULT;
			break;
		}
		if(t >= nldisp){
			u.u_error = ENXIO;
			break;
		}
		if(tp->t_line)
			(*linesw[tp->t_line].l_close)(tp);
		if(t)
			(*linesw[t].l_open)(dev, tp, addr);
		if(u.u_error == 0)
			tp->t_line = t;
		break;
	/*
	 * prevent more opens on channel
	 */
	case TIOCEXCL:
		tp->t_state |= XCLUDE;
		break;
	case TIOCNXCL:
		tp->t_state &= ~XCLUDE;
		break;
	/*
	 * Set new parameters
	 */
	case TIOCSETP:
		wflushtty(tp);
	case TIOCSETN:
		if(copyin(addr, (caddr_t)&tp->t_ispeed, sizeof(struct sttiocb))){
			u.u_error = EFAULT;
			return(1);
		}
		break;
	/*
	 * send current parameters to user
	 */
	case TIOCGETP:
		if(copyout((caddr_t)&tp->t_ispeed, addr, sizeof(struct sttiocb)))
			u.u_error = EFAULT;
		break;
	/*
	 * Set all new parameters
	 */
	case TIOCSETA:
		wflushtty(tp);
		n = tp->t_length;
		if(copyin(addr, (caddr_t)&tp->t_ispeed, sizeof(struct ttiocb))){
			u.u_error = EFAULT;
			return(1);
		}
		if((tp->t_length &= 0177) != n){
			if(tp->t_length)
				tp->t_xstate |= XPAGE;
			else
				tp->t_xstate &= ~XPAGE;
			tp->t_lnum = 0;
		}
		break;
	/*
	 * Send all current parameters to user
	 */
	case TIOCGETA:
		if(copyout((caddr_t)&tp->t_ispeed, addr, sizeof(struct ttiocb)))
			u.u_error = EFAULT;
		break;
	/*
	 * Hang up line on last close
	 */
	case TIOCHPCL:
		tp->t_state |= HUPCLS;
		break;
	case TIOCNRD:
		t = 0;
		if(tp->t_flags&(RAW|CBREAK))
			t = tp->t_rawq.c_cc;
		else if(tp->t_delct) {
			register char *cp;

			cp = tp->t_rawq.c_cf;
			while(ubyte(*cp++) != 0377)
				t++;
		}
		if(copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;
	case TIOCFLUSH:
		flushtty(tp);
		break;
	/*
	 * ioctl entries to line discipline
	 */
	case DIOCSETP:
	case DIOCGETP:
		(*linesw[tp->t_line].l_ioctl)(com, tp, addr);
		break;
	/*
	 * set and fetch special characters
	 */
	case TIOCSETC:
		if(copyin(addr, (caddr_t)&tun, sizeof(struct tc)))
			u.u_error = EFAULT;
		break;
	case TIOCGETC:
		if(copyout((caddr_t)&tun, addr, sizeof(struct tc)))
			u.u_error = EFAULT;
		break;
	default:
		return(0);
	}
	return(1);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
wflushtty(tp)
register struct tty *tp;
{
	spl5();
	while (tp->t_outq.c_cc && tp->t_state&CARR_ON){
		(*tp->t_oproc)(tp);
		tp->t_state |= ASLEEP;
		sleep((caddr_t)&tp->t_outq, TTOPRI);
	}
	flushtty(tp);
	spl0();
}

/*
 * flush all TTY queues
 */
flushtty(tp)
register struct tty *tp;
{
	register s;

	wakeup((caddr_t)&tp->t_rawq);
	wakeup((caddr_t)&tp->t_outq);
	s = spl6();
	tp->t_state &= ~TTSTOP;
	tp->t_xstate &= ~(XPAGE1|XPAGE2|XLITRL|XERASE);
	(*cdevsw[major(tp->t_dev)].d_stop)(tp);
	while(getc(&tp->t_outq) >= 0);
	while(getc(&tp->t_rawq) >= 0);
	tp->t_delct = 0;
	tp->t_lnum = 0;
	splx(s);
}

/*
 * block transfer input handler.
ttyrend(tp, pb, pe)
register struct tty *tp;
register char *pb, *pe;
{
	int	tandem;

	tandem = tp->t_flags&TANDEM;
	if(tp->t_flags&RAW){
		b_to_q(pb, pe-pb, &tp->t_rawq);
#ifdef MX
		if(tp->t_chan)
			sdata(tp->t_chan);
		else
#endif MX
			wakeup((caddr_t)&tp->t_rawq);
	} else {
		tp->t_flags &= ~TANDEM;
		while(pb < pe)
			ttyinput(*pb++, tp);
		tp->t_flags |= tandem;
	}
	if(tandem)
		ttyblock(tp);
}
 */

/*
 * Place a character on TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also do erase, kill processing, and echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ttyinput(c, tp)
register c;
register struct tty *tp;
{
	register int t_flags;
#ifdef MX
	register struct chan *cp;
#endif MX

#ifdef INSTRM
	tk_nin += 1;
#endif INSTRM
	c &= 0377;
	if((tp->t_flags&RAW) == 0)
		c &= 0177;
	t_flags = tp->t_flags;
	if(t_flags&TANDEM)
		ttyblock(tp);
	if((t_flags&RAW) == 0 || (t_flags&(RAW|CBREAK)) == (RAW|CBREAK)){
		if(tp->t_state&TTSTOP){
			if(c == tun.t_startc){
				tp->t_state &= ~TTSTOP;
				ttstart(tp);
				return;
			}
			if(c == tun.t_stopc)
				return;
			tp->t_state &= ~TTSTOP;
			ttstart(tp);
		} else {
			if(c == tun.t_stopc){
				tp->t_state |= TTSTOP;
				(*cdevsw[major(tp->t_dev)].d_stop)(tp);
				return;
			}
			if(c == tun.t_startc)
				return;
		}
	}
	if((t_flags&RAW) == 0){
		if(tp->t_xstate&XLITRL){
			tp->t_xstate &= ~XLITRL;
			if((t_flags&LCASE) && (maptab[c]&0177))
				c = maptab[c]&0177;
			else if(!isspcl(c)){
				putc('\\', &tp->t_rawq);
				goto contin;
			}
			putc(c, &tp->t_rawq);
			goto out2;
		contin:;
		}
		if((c == CPAGE) && tp->t_length){
			tp->t_xstate ^= XPAGE;
			if((tp->t_xstate&XPAGE1) == 0)
				goto out2;
		}
		if(tp->t_xstate&XPAGE1){
			tp->t_xstate &= ~(XPAGE1|XPAGE2);
			if(c == '\r' || c == '\n')
				tp->t_lnum = tp->t_length-1;
			else
				tp->t_lnum = 0;
			if(c != tun.t_quitc && c != tun.t_intrc)
				goto out3;
		}
		if(c == tun.t_quitc || c == tun.t_intrc){
			flushtty(tp);
			c = (c == tun.t_intrc) ? SIGINT:SIGQUIT;
#ifdef MX
			if(tp->t_chan)
				scontrol(tp->t_chan, M_SIG, c);
			else {
#endif MX
				signal(tp->t_pgrp, c);
				ttyecho('\n', tp);
				ttstart(tp);
#ifdef MX
			}
#endif MX
			return;
		}
		if(t_flags&CBREAK)
			goto out0;
		if(c == tp->t_erase){
			if(iscntrl(c)){
				ttywipe(zapc(&tp->t_rawq), tp);
				goto out3;
			}
			else {
				zapc(&tp->t_rawq);
				goto out2;
			}
		}
		else if(c == tp->t_kill){
			if(((t_flags&SCOPE) == 0) || (!iscntrl(c))){
				while(zapc(&tp->t_rawq) >= 0);
				ttyecho(c, tp);
				ttyecho('\n', tp);
			}
			else {
				while((c = zapc(&tp->t_rawq)) >= 0){
					if(ubyte(tp->t_col) > 0)
						ttywipe(c, tp);
				}
			}
			goto out3;
		}
		else if(c == tun.t_eofc){
			ttyecho('\n', tp);
			goto out1;
		}
		else if(c == CLITERAL){
			tp->t_xstate |= XLITRL;
			ttyecho(c, tp);
			goto out3;
		}
		else if(c == CRETYPE){
			if(tp->t_outq.c_cc)
				goto out3;
#ifdef DELTAB
			ttyecho('\n', tp);
			ttyretype(tp, 1);
#else
			ttyretype(tp);
#endif DELTAB
			goto out3;
		}
		else if(c == CWORD){
			ttywipe((c = zapc(&tp->t_rawq)), tp);
			if(isword(c)){
				while(isword(c = zapc(&tp->t_rawq)))
					ttywipe(c, tp);
				if(c >= 0)
					putc(c, &tp->t_rawq);
			}
			goto out3;
		}
	}
out0:
	if(c == '\r' && (t_flags&CRMOD))
		c = '\n';
	if((t_flags&LCASE) && isupper(c))
		c += 'a'-'A';
	if(tp->t_rawq.c_cc > TTYHOG){
		flushtty(tp);
		return;
	}
	putc(c, &tp->t_rawq);
out1:
	if((t_flags&(RAW|CBREAK)) || (c == '\n' || c == tun.t_eofc || c == tun.t_brkc)){
		if((t_flags&(RAW|CBREAK)) == 0 && putc(0377, &tp->t_rawq) == 0)
			tp->t_delct++;
#ifdef MX
		if((cp = tp->t_chan) != NULL)
			sdata(cp);
		else
#endif MX
			wakeup((caddr_t)&tp->t_rawq);
	}
out2:
	if(ttyecho(c, tp) == 0)
		return;
out3:
	ttstart(tp);
}

#ifdef DELTAB
/*
 * ttyretype performs two functions:
 *	1) print the input buffer on CRETYPE (flag = 1);
 *	2) count the column position for tab deletion
 *	   (flag = 0).
 */
ttyretype(tp, flag)
register struct tty *tp;
{
	register char *cp;
	register int c, n;
	register int width, col;

	cp = tp->t_rawq.c_cf;
	n = tp->t_delct;
	width = tp->t_width ? ubyte(tp->t_width): 0377;
	col = 0;
	for(c = tp->t_rawq.c_cc; c--; ){
		if(n){
			if(ubyte(*cp++) == 0377)
				n--;
		}
		else {
			if((tp->t_flags&LCASE) && isupper(*cp) || isspcl(*cp)){
				if(flag)
					ttyecho('\\', tp);
				else
					col++;
			}
			if(flag)
				ttyecho(*cp++, tp);
			else {
				if(*cp++ == '\t')
					col += 8 - (col&07);
				else
					col++;
				if(col >= width)
					col -= width;
			}
		}
		if(((int)cp & CROUND) == 0)
			cp = (((struct cblock *)cp) - 1)->c_next->c_info;
	}
	return(col);
}
#else
ttyretype(tp)
register struct tty *tp;
{
	register char *cp;
	register int c, n;

	cp = tp->t_rawq.c_cf;
	n = tp->t_delct;
	ttyecho('\n', tp);
	for(c = tp->t_rawq.c_cc; c--; ){
		if(n){
			if((*cp++&0377) == 0377)
				n--;
		}
		else {
			if((tp->t_flags&LCASE) && isupper(*cp) || isspcl(*cp))
				ttyecho('\\', tp);
			ttyecho(*cp++, tp);
		}
		if(((int)cp & CROUND) == 0)
			cp = (((struct cblock *)cp) - 1)->c_next->c_info;
	}
}
#endif DELTAB

/*
 * Echo a character on the terminal.
 * All echoed characters are only 7 bits wide.
 */
ttyecho(c, tp)
register c;
register struct tty *tp;
{
	c &= 0177;
	if((tp->t_flags&ECHO) == 0 || (iscntrl(c) && (tp->t_flags&INDCTL) == 0))
		return(0);
	if(tp->t_xstate&XERASE){
		tp->t_xstate &= ~XERASE;
		ttyoutput(']', tp);
	}
	if((tp->t_flags&LCASE) && isupper(c))
		c += 'a'-'A';
	/*
	 * kludge for paging - yuck
	 */
	if((c == '\n') && (tp->t_xstate&XPAGE) && ((tp->t_flags&RAW) == 0)){
		if(tp->t_lnum > 0)
			tp->t_lnum--;
	}
	ttyoutput(c, tp);
	return(1);
}

/*
 * indicate the deletion of a character
 * on the terminal
 */
ttywipe(c, tp)
register c;
register struct tty *tp;
{
	if((tp->t_flags&ECHO) == 0)
		return;
	if(c < 0){
		ttyoutput(CBELL, tp);
		return;
	}
	if(tp->t_flags&SCOPE){
		register n = 1;

		if(iscntrl(c)){
			n = 0;
			if(tp->t_flags&INDCTL)
				n = 2;
			if(isspcl(c))
				n++;
		}
		else if(c == '\t'){
#ifdef DELTAB
			if(n = ttyretype(tp, 0))
				n = ubyte(tp->t_col) - max(n,ubyte(tp->t_htdly));
			else if((n = ubyte(tp->t_col) - ubyte(tp->t_htdly)) == 0)
				n = 8;
			if(ubyte(tp->t_htdly) >= (ubyte(tp->t_col) - n))
				tp->t_htdly = 0;
#else
			ttyretype(tp);
			return;
#endif DELTAB
		}
		else if(tp->t_flags&LCASE){
			if(isupper(c) || c == '`' || c >= '{')
				n++;
		}
		if((n < 0) || (n > ubyte(tp->t_col))) {
			/* if(tp->t_col) */
				ttyoutput('\n',tp);
			ttyretype(tp,1);
		}
		else
		while(n--){
			ttyoutput('\b', tp);
			ttyoutput(' ', tp);
			ttyoutput('\b', tp);
		}
		return;
	}
	if((tp->t_xstate&XERASE) == 0){
		tp->t_xstate |= XERASE;
		ttyoutput('[', tp);
	}
	ttyoutput(c, tp);
}

/*
 * remove a character from the end of
 * a queue, unless it is a delimeter
 */
zapc(q)
register struct clist *q;
{
	extern struct cblock *cfreelist;
	register struct cblock *bp;
	register char *cp;
	register int c, s;

	s = spl6();
	if(((cp = q->c_cl) == NULL) || ((c = ubyte(*--cp)) == 0377)){
		splx(s);
		return(-1);
	}
	q->c_cl = cp;
	cp -= sizeof(char *);
	if(--q->c_cc <= 0){
		bp = (struct cblock *) ((int) cp & ~CROUND);
		q->c_cf = q->c_cl = NULL;
		bp->c_next = cfreelist;
		cfreelist = bp;
	}
	else if(((int) cp & CROUND) == 0){
		bp = (struct cblock *) cp;
		bp->c_next = cfreelist;
		cfreelist = bp;
		bp = (struct cblock *) q->c_cf;
		bp = (struct cblock *) ((int) bp & ~CROUND);
		while(bp->c_next != (struct cblock *) cp)
			bp = bp->c_next;
		bp->c_next = NULL;
		q->c_cl = &bp->c_info[CBSIZE];
	}
	splx(s);
	return(c);
}

/*
 * ttybit returns non-zero if the bit in the area is set
 */
ttybit(area, bit)
char *area;
register int bit;
{
	return(bit < 0 ? 0: area[bit >> 3]&(1 << (bit&07)));
}

/*
 * Check if this is one of the special characters.
 */
ttyspcl(c, tp)
register c;
register struct tty *tp;
{
	register char *cp;
	register n;

	if(c < 0)
		return(0);
	for(cp = &tun.t_intrc, n = 0; n < sizeof(struct tc); n++){
		if(c == *cp++)
			return(1);
	}
	if((c == tp->t_erase) || (c == tp->t_kill))
		return(1);
	if((c == CRETYPE) || (c == CWORD) || ((c == CPAGE) && tp->t_length))
		return(1);
	return(0);
}

/*
 * Send stop character on input overflow.
 */
ttyblock(tp)
register struct tty *tp;
{
	register x;
	x = tp->t_rawq.c_cc;
	if(x > TTYHOG){
		flushtty(tp);
		tp->t_state &= ~TBLOCK;
	}
	if((x >= TTYHOG/2) && ((tp->t_state&TBLOCK) == 0)) {
		if(putc(tun.t_stopc, &tp->t_outq) == 0){
			tp->t_state |= TBLOCK;
			ttstart(tp);
		}
	}
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
ttyoutput(c, tp)
register c;
register struct tty *tp;
{
	register char *colp;

#ifdef INSTRM
	tk_nout += 1;
#endif INSTRM
	/*
	 * Turn <nl> to <cr><lf> if desired.
	 */
	if(c == '\n' && (tp->t_flags&CRMOD))
		ttyoutput('\r', tp);
	if((tp->t_flags&RAW) == 0){
		c &= 0177;
		/*
		 * Print control characters as ^<char+0140>.
		 */
		if((tp->t_flags&INDCTL) && iscntrl(c)){
			ttyoutput('^', tp);
			c |= 0100;
		}
		/*
		 * Whoa there Trigger! - it's a page boundary
		 */
		if(tp->t_length && (tp->t_xstate&XPAGE)){
			if((c == '\n' && (ubyte(++(tp->t_lnum)) >= ubyte(tp->t_length))) || c == '\014'){
				tp->t_lnum = 0;
				tp->t_xstate |= XPAGE2;
				putc(0200, &tp->t_outq);
			}
		}
	}
	/*
	 * For upper-case-only terminals,
	 * generate escapes.
	 * Also, upper-case characters are printed
	 * with a preceeding '\\'.
	 */
	if(tp->t_flags&LCASE){
		if('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if(isupper(c))
			ttyoutput('\\', tp);
		else {
			colp = "({)}!|^~'`";
			while(*colp++){
				if(c == *colp++){
					ttyoutput('\\', tp);
					c = colp[-2];
					break;
				}
			}
		}
	}
	/*
	 * If it is a printing character, count it,
	 * and fold the line if it is too long.
	 */
	colp = &tp->t_col;
	/*
	 * Turn tabs to spaces as required
	 */
	if(c == '\t' && (tp->t_flags&XTABS)){
#ifdef DELTAB
		if(tp->t_htdly == 0)
			tp->t_htdly = tp->t_col;
#endif DELTAB
		do
			ttyoutput(' ', tp);
		while((*colp)&07);
		return;
	}
	if(' ' <= c && c <= '~'){
		if(tp->t_width && (ubyte(*colp) >= ubyte(tp->t_width)))
			ttyoutput('\n', tp);
		(*colp)++;
		putc(c, &tp->t_outq);
		return;
	}
	putc(c, &tp->t_outq);
	/*
	 * Calculate delays.
	 * The delays are indicated by characters above 0200.
	 * In raw mode there are no delays and the
	 * transmission path is 8 bits wide. However, do the
	 * column calculation.
	 */
	switch(c&0177){
	case '\b':
		if(*colp)
			(*colp)--;
	default:
		return;
	case '\t':
		*colp |= 07;
		(*colp)++;
		c = tp->t_htdly;
		break;
	case '\n':
#ifdef DELTAB
		if(tp->t_flags&XTABS)
			tp->t_htdly = 0;
#endif DELTAB
		*colp = 0;
		c = tp->t_nldly;
		break;
	case 013:
	case 014:
		c = tp->t_vtdly;
		break;
	case '\r':
		*colp = 0;
		c = tp->t_crdly;
		break;
	}
	if(c && ((tp->t_flags&RAW) == 0))
		putc(c|0200, &tp->t_outq);
}

/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{
	tp->t_state &= ~TIMEOUT;
	ttstart(tp);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 */
ttstart(tp)
register struct tty *tp;
{
	register s;

	s = spl5();
	if((tp->t_state&(TIMEOUT|TTSTOP|BUSY)) == 0)
		(*tp->t_oproc)(tp);
	splx(s);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register int c;

	if((tp->t_state&CARR_ON) == 0)
		return(0);
	spl5();
	while(((tp->t_flags&(RAW|CBREAK)) == 0 && tp->t_delct == 0)
	      || ((tp->t_flags&(RAW|CBREAK)) != 0 && tp->t_rawq.c_cc == 0)){
#ifdef MX
		if((tp->t_state&CARR_ON) == 0 || tp->t_chan != NULL)
#else
		if((tp->t_state&CARR_ON) == 0)
#endif MX
			return(tp->t_rawq.c_cc);
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	spl0();
	if(tp->t_flags&(RAW|CBREAK)){
		while(tp->t_rawq.c_cc && passc(getc(&tp->t_rawq)) >= 0);
		goto out;
	}
	while((c = getc(&tp->t_rawq)) != 0377 && passc(c) >= 0);
	if((c != 0377) && ubyte(*(tp->t_rawq.c_cf)) == 0377)
		c = getc(&tp->t_rawq);
	if(c == 0377)
		tp->t_delct--;
	if(tp->t_length){
		tp->t_xstate |= XPAGE;
		tp->t_xstate &= ~(XPAGE1|XPAGE2);
		tp->t_lnum = 0;
	}
	else
		tp->t_xstate &= ~XPAGE;
out:
	if((tp->t_state&TBLOCK) && tp->t_rawq.c_cc < TTYHOG/5){
		if(putc(tun.t_startc, &tp->t_outq) == 0){
			tp->t_state &= ~TBLOCK;
			ttstart(tp);
		}
	}
	return(tp->t_rawq.c_cc);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
caddr_t
ttwrite(tp)
register struct tty *tp;
{
	register c;

	if((tp->t_state&CARR_ON) == 0)
		return(NULL);
	while(u.u_count){
		spl5();
		while(tp->t_outq.c_cc > TTHIWAT || (tp->t_xstate&XPAGE2)){
			ttstart(tp);
			tp->t_state |= ASLEEP;
#ifdef MX
			if(tp->t_chan)
				return((caddr_t)&tp->t_outq);
#endif MX
			sleep((caddr_t)&tp->t_outq, TTOPRI);
		}
		spl0();
		if((c = cpass()) < 0)
			break;
		ttyoutput(c, tp);
	}
	ttstart(tp);
	return(NULL);
}
