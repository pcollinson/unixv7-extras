/*
 * Put default terminal control characters into
 * one file, so they are more accessible from
 * other code that needs them
 */
/*
 * Default special characters.
 */
#define	CERASE	0177	/* DEL */
#define	CEOT	04	/* ^D */
#define	CKILL	025	/* ^U */
#define	CQUIT	034	/* ^\ */
#define	CINTR	03	/* ^C */
#define	CSTOP	023	/* ^S */
#define	CSTART	021	/* ^Q */
#define	CBRK	0377	/* -1 */

/*
 * Other control characters. These
 * are not redefinable by the user.
 */
#define CPAGE	 020	/* ^P - flip paging off/on for this process */
#define CRETYPE	 022	/* ^R - retype current line */
#define CWORD	 027	/* ^W - delete word */
#define	CLITERAL 026	/* ^V, the literal next char */

/*
 * Miscellaneous junk
 */
#define CBELL	07	/* ^G - (ding-dong) */
