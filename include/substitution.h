#include "stdinc.h"

#ifndef SUBSTITUTION_H
#define SUBSTITUTION_H

extern void substitution_append_var(rb_dlink_list *varlist, const char *name, const char *value);
extern void substitution_free(rb_dlink_list *varlist);
extern char *substitution_parse(const char *fmt, rb_dlink_list *varlist);

#endif
