#include <stdio.h>

dysize(y)
int y;
{
	/* called internally */
	if (y < 500) y += 1900;

	if (((y%4) == 0 && (y%100) != 0) || (y%400) == 0)
		return(366);
	return(365);
}

odysize(y)
int y;
{
	if((y%4) == 0)
		return(366);
	return(365);
}

isleap(n)
int n;
{
	if (n == 366) return 'Y';
	return 'N';
}

main()
{
	int yr, havey;
	int lp, lfp, olp, olfp;

	for (yr = 70; yr <= 170; yr++) {
		havey = 0;
		lp = isleap(dysize(yr));
		if (lp == 'Y') havey++;
		lfp = isleap(dysize(yr + 1900));
		if (lfp == 'Y') havey++;
		olp = isleap(odysize(yr));
		if (olp == 'Y') havey++;
		olfp = isleap(odysize(yr + 1900));
		if (olfp == 'Y') havey++;
		if (havey)
			printf("%4d\t%d\t%c\t%c\t%c\t%c\n",
			       yr, yr+1900, lp, lfp, olp, olfp);
	}
}
