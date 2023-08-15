/*
 * DZ-11 driver
 *
 * The code here is quite different in spots from
 * the released and untested dz driver
 * If you want to use CARRIER on the device
 * define CARRIER
 * Peter Collinson
 * July 2023
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"

#define NDZ11	1
#define NDZLINE	NDZ11*8

/*
 * Hardware bits
 */
#define CSRDFLT	050140

#define PRTYERR	010000
#define FRAMERR	020000

#define _8BITS	   030
#define _7BITS	  0120
#define OPARITY	  0200
#define LPRDFLT	010040

#define	DTROFF	     0
#define	DTRON	     1

#define SSPEED	     7

#define	DZADDR	((struct device *)0160100)

struct tty dz11[NDZLINE];

char	dzstab[] = {
	0,	/* 0 baud */
	020,	/* 50 baud */
	021,	/* 75 baud */
	022,	/* 110 baud */
	023,	/* 134.5 baud */
	024,	/* 150 baud */
	0,	/* 200 baud - XXX */
	025,	/* 300 baud */
	026,	/* 600 baud */
	027,	/* 1200 baud */
	030,	/* 1800 baud */
	032,	/* 2400 baud */
	034,	/* 4800 baud */
	036,	/* 9600 baud */
	0,	/* EXTA */
	0	/* EXTB */
};

struct device {
	int	dzcsr;
	int	dzrbuf;
	char	dztcr;
	char	dzdtr;
	char	dztbuf;
	char	dzbrk;
};

#define	dzlpr	dzrbuf
#define	dzmsr	dzbrk

#define SCANRATE     2
#define SCANON	    01
#define SCANLOCK    02

char dzto[NDZLINE];
char dzstat;
int ndz11 = NDZLINE;

/*
 * open a DZ-11 line
 */
dzopen(dev, flag)
{
	register struct tty *tp;
	register line;
	extern dzstart();

	if((line = minor(dev)) >= NDZLINE){
		u.u_error = ENXIO;
		return;
	}
	tp = &dz11[line];
	if((tp->t_state&(ISOPEN|WOPEN)) == 0){
		tp->t_oproc = dzstart;
		tp->t_iproc = NULL;
		ttychars(tp);
		tp->t_ispeed = SSPEED;
		tp->t_ospeed = SSPEED;
		tp->t_flags = XTABS|CRMOD|ECHO|LCASE;
		dzparam(line);
	}
	else if((tp->t_state&XCLUDE) && u.u_uid != 0){
		u.u_error = EBUSY;
		return;
	}
	dzmodem(line, DTRON);
#ifdef CARRIER
	spl6();
	while((tp->t_state&CARR_ON) == 0){
		tp->t_state |= WOPEN;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	ttyopen(dev, tp);
	spl0();
#else
	tp->t_state |= CARR_ON;
	ttyopen(dev, tp);
#endif CARRIER
}

/*
 * close a DZ-11 line
 */
dzclose(dev)
{
	register struct tty *tp;
	register line;

	line = minor(dev);
	tp = &dz11[line];
	tp->t_pgrp = 0;
	wflushtty(tp);
	if(tp->t_state&HUPCLS){
		dzmodem(line, DTROFF);
	}
	tp->t_state &= CARR_ON;
	tp->t_xstate = 0;
}

/*
 * read from a DZ-11 line
 */
dzread(dev)
{
	ttread(&dz11[minor(dev)]);
}

/*
 * write on a DZ-11 line
 */
dzwrite(dev)
{
	ttwrite(&dz11[minor(dev)]);
}

/*
 * ioctl for DZ-11
 */
dzioctl(dev, cmd, addr, flag)
{
	register struct tty *tp;
	register line;

	line = minor(dev);
	tp = &dz11[line];
	if(ttioccomm(cmd, tp, (caddr_t)addr, line)){
		if(cmd == TIOCSETP || cmd == TIOCSETN || cmd == TIOCSETA)
			dzparam(line);
	}
	else
		u.u_error = ENOTTY;
}

/*
 * set DZ-11 hardware registers
 */
dzparam(dev)
{
	register struct tty *tp;
	register struct device *dzp;
	register lpr;

	tp = &dz11[dev];
	dzp = &DZADDR[dev>>3];
	dzp->dzcsr = CSRDFLT;
	if(dzstat == 0){
		dzscan();
		dzstat |= SCANON;
	}
	if(tp->t_ispeed == 0){
		dzmodem(dev, DTROFF);
		return;
	}
	lpr = (dzstab[tp->t_ispeed] << 8)|LPRDFLT|(dev&07);
	if((tp->t_flags&(ODDP|EVENP)) == 0 || (tp->t_flags&RAW))
		lpr |= _8BITS;
	else {
		lpr |= _7BITS;
		if((tp->t_flags&EVENP) == 0)
			lpr |= OPARITY;
	}
	dzp->dzlpr = lpr;
}

/*
 * DZ-11 receiver interrupt
 */
dzrint(dev)
{
	register struct tty *tp;
	register struct device *dzp;
	register c, i, n;

	dzstat |= SCANLOCK;
	spl5();
	n = minor(dev);
	for(i = 0; i < NDZ11; i++){
		dzp = &DZADDR[n];
		while((c = dzp->dzrbuf) < 0){
			tp = &dz11[((c >> 8)&07)|(n << 3)];
			if(tp >= &dz11[NDZLINE])
				continue;
			if((tp->t_state&ISOPEN) == 0){
				wakeup((caddr_t)&tp->t_rawq);
				continue;
			}
			if(c&FRAMERR){
				if(tp->t_flags&RAW)
					c = 0;
				else
					c = CINTR;
			}
			if(c&PRTYERR){
				if((tp->t_flags&(EVENP|ODDP)) == EVENP || (tp->t_flags&(EVENP|ODDP)) == ODDP)
					continue;
			}
			ttyinput(c, tp);
		}
		if(++n >= NDZ11)
			n = 0;
	}
	dzstat &= ~SCANLOCK;
}

/*
 * DZ-11 transmitter interrupt
 */
dzxint(dev)
{
	register struct tty *tp;
	register struct device *dzp;
	register i, n;

	n = minor(dev);
	for(i = 0; i < NDZ11; i++){
		dzp = &DZADDR[n];
		while(dzp->dzcsr < 0){
			tp = &dz11[((n << 3)|(dzp->dzcsr >> 8)&07)];
			dzp->dztbuf = tp->t_char;
			tp->t_state &= ~BUSY;
			dzstart(tp);
		}
		if(++n >= NDZ11)
			n = 0;
	}
}

/*
 * start routine for DZ-11
 */
dzstart(tp)
register struct tty *tp;
{
	register struct device *dzp;
	register unit, c;
	int s;

	unit = tp - dz11;
	dzp = &DZADDR[unit>>3];
	unit = (1 << (unit&07));
	s = spl5();
	if((tp->t_state&(TIMEOUT|BUSY)) || (tp->t_xstate&XPAGE1)){
		splx(s);
		return;
	}
	if(tp->t_state&TTSTOP){
		dzp->dztcr &= ~unit;
		splx(s);
		return;
	}
	if((c = getc(&tp->t_outq)) >= 0){
		if(c >= 0200 && (tp->t_flags&RAW) == 0){
			dzp->dztcr &= ~unit;
			if(c == 0200){
				tp->t_xstate |= XPAGE1;
			} else {
				tp->t_state |= TIMEOUT;
				dzto[tp - dz11] = (((c&0177)+6+(SCANRATE-1))/SCANRATE)+1;
			}
		} else {
			tp->t_char = c;
			tp->t_state |= BUSY;
			dzp->dztcr |= unit;
		}
		if((tp->t_outq.c_cc <= TTLOWAT) && (tp->t_state&ASLEEP)){
			tp->t_state &= ~ASLEEP;
#ifdef MX
			if(tp->t_chan)
				mcstart(tp->t_chan, (caddr_t)&tp->t_outq);
			else
#endif MX
				wakeup((caddr_t)&tp->t_outq);
		}
	}
	else
		dzp->dztcr &= ~unit;
	splx(s);
}

/*
 * set/reset DTR
 */
dzmodem(dev, flag)
{
	register struct device *dzp;
	register bit;

	dzp = &DZADDR[dev>>3];
	bit = (1 << (dev&07));
	if(flag == DTROFF)
		dzp->dzdtr &= ~bit;
	else
		dzp->dzdtr |= bit;
}

/*
 * scan lines for input, and process any timeouts
 */
dzscan()
{
	register n;
#ifdef CARRIER
	static cscan;

	if(cscan <= 0){
		dzcarrier();
		cscan = HZ*2;
	}
	else
		cscan -= SCANRATE;
#endif CARRIER
	for(n = 0; n < NDZLINE; n++){
		if((dzto[n] > 0) && (--dzto[n] <= 0)){
			dz11[n].t_state &= ~TIMEOUT;
			dzstart(&dz11[n]);
		}
	}
	if((dzstat&SCANLOCK) == 0)
		dzrint(0);
	timeout(dzscan, (caddr_t)0, SCANRATE);
}

#ifdef CARRIER
/*
 * scan DZ-11 lines for carrier transitions
 */
dzcarrier()
{
	register struct tty *tp;
	register struct device *dzp;
	register i;
	char bit;

	for(i = 0; i < NDZLINE; i++){
		dzp = &DZADDR[i>>3];
		tp = &dz11[i];
		bit = (1 << (i&07));
		if(dzp->dzmsr&bit){
			if((tp->t_state&CARR_ON) == 0){
				wakeup((caddr_t)&tp->t_rawq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if(tp->t_state&CARR_ON){
				if(tp->t_state&ISOPEN){
					signal(tp->t_pgrp, SIGHUP);
					dzp->dzdtr &= ~bit;
					flushtty(tp);
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
}
#endif CARRIER
