/*
 * Code to set the terminal to operational VDU
 * workings before installation of working system
 * with new tty driver
 */
#include "sys/sgtty.h"
#include "sys/deftty.h"

struct sgttyb sgttyb;		/* stty/gtty - TIOCSETA/TIOCGETA */
struct tchars tchars;		/* extra characters - TIOCSETC/TIOCGETC */

main()
{
	int fd = 1;

	if (ioctl(fd, TIOCGETA, &sgttyb) < 0) {
		perror("Cannot get stty information");
	}
	if (ioctl(fd, TIOCGETC, &tchars) < 0) {
		perror("Cannot get ttchars information");
	}
	sgttyb.sg_erase = CERASE;
	sgttyb.sg_kill = CKILL;
	sgttyb.sg_width = 0;
	sgttyb.sg_length = 0;
	sgttyb.sg_nldly = 0;
	sgttyb.sg_crdly = 0;
	sgttyb.sg_htdly = 0;
	sgttyb.sg_vtdly = 0;
	sgttyb.sg_flags = ECHO|CRMOD|ODDP|EVENP|ANYP|SCOPE|INDCTL;
	tchars.t_intrc = CINTR;
	tchars.t_quitc = CQUIT;
	tchars.t_startc = CSTART;
	tchars.t_stopc = CSTOP;
	tchars.t_eofc = CEOT;
	tchars.t_brkc = CBRK;

	if (ioctl(fd, TIOCSETA, &sgttyb) < 0) {
		perror("Cannot set stty information");
	}
	if (ioctl(fd, TIOCSETC, &tchars) < 0) {
		perror("Cannot get ttchars information");
	}
}
