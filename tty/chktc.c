#include <stdio.h>

#include "sys/sgtty.h"
#include "sys/deftty.h"

struct sgttyb sgttyb;		/* stty/gtty - TIOCSETP/TIOCGETP */
struct tchars tchars;		/* extra characters - TIOCSETA/TIOCGETA */

main()
{
	int fd;

	fd = 1;
	if (ioctl(fd, TIOCGETA, &sgttyb) < 0) {
		printf("Cannot get stty information");
	}
	if (ioctl(fd, TIOCGETC, &tchars) < 0) {
		printf("Cannot get ttchars information");
	}

	printf("%s %o\n", "t_intrc", tchars.t_intrc);
	printf("%s %o\n", "t_quitc", tchars.t_quitc);
	printf("%s %o\n", "t_startc", tchars.t_startc);
	printf("%s %o\n", "t_stopc", tchars.t_stopc);
	printf("%s %o\n", "t_eofc", tchars.t_eofc);
	printf("%s %o\n", "t_brkc", tchars.t_brkc);
}
