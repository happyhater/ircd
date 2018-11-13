#ifndef __GETOPT_H_INCLUDED__
#define __GETOPT_H_INCLUDED__

struct lgetopt
{
	const char *opt;	/* name of the argument */
	void *argloc;		/* where we store the argument to it (-option argument) */
	enum
	{ INTEGER, YESNO, STRING, USAGE, ENDEBUG }
	argtype;
	const char *desc;	/* description of the argument, usage for printing help */
};

extern struct lgetopt myopts[];

void usage(char *);
void parseargs(int *, char ***, struct lgetopt *);

#endif /* __GETOPT_H_INCLUDED__ */
