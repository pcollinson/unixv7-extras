/*
 * ssp - single space output
 *
 * Bill Joy UCB August 25, 1977
 *
 * Compress multiple empty lines to a single empty line.
 * Option - compresses to nothing.
 *
 * Peter Collinson June 2023
 * nroff will output escape sequences, 033 and another character
 * for up and down movements on the line.
 * This code now removes those sequences, which makes pages like
 * man printf
 * look OK.
 * Of course, there should be a special tool for this, but it's
 * convenient to insert it here.
 */

#include <stdio.h>

char	poof, hadsome;

extern	int fout;

main(argc, argv)
	int argc;
	char *argv[];
{
	register int c;
	FILE *f;

	argc--, argv++;
	do {
		while (argc > 0 && argv[0][0] == '-') {
			poof = 1;
			argc--, argv++;
		}
		f = stdin;
		if (argc > 0) {
			if ((f = fopen(argv[0], "r")) < 0) {
				flush();
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		for (;;) {
			c = getc(f);
			if (c == -1)
				break;
			if (c == '\033') {
				c = getc(f);
				if (c == -1)
					break;
				continue;
			}
			if (c != '\n') {
				hadsome = 1;
				putchar(c);
				continue;
			}

			/*
			 * Eat em up
			 */
			if (hadsome)
				putchar('\n');
			c = getc(f);
			if (c == -1)
				break;
			if (c != '\n') {
				putchar(c);
				hadsome = 1;
				continue;
			}
			do
				c = getc(f);
			while (c == '\n');
			if (!poof && hadsome)
				putchar('\n');
			if (c == -1)
				break;
			putchar(c);
			hadsome = 1;
		}
	} while (argc > 0);
}
