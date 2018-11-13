#ifndef _CHARYBDIS_IRCD_LINKER_H
#define _CHARYBDIS_IRCD_LINKER_H

#include <dlfcn.h>

struct ircd_symbol
{
	char *sym;	/* name of symbol to be bound to ptr */
	void *ptr;	/* ptr to symbol in library */
};

extern void build_symtable(void *, struct ircd_symbol *);
extern void report_symtable(struct ircd_symbol *);

#endif
