#ifndef __CHARYBDIS_PRIVILEGE_H
#define __CHARYBDIS_PRIVILEGE_H

#include "stdinc.h"

enum {
	PRIV_NEEDOPER = 1
};
typedef unsigned int PrivilegeFlags;

struct PrivilegeSet {
	unsigned int status;	/* If CONF_ILLEGAL, delete when no refs */
	int refs;
	char *name;
	char *privs;
	PrivilegeFlags flags;
	rb_dlink_node node;
};

int privilegeset_in_set(struct PrivilegeSet *set, const char *priv);
struct PrivilegeSet *privilegeset_set_new(const char *name, const char *privs, PrivilegeFlags flags);
struct PrivilegeSet *privilegeset_extend(struct PrivilegeSet *parent, const char *name, const char *privs, PrivilegeFlags flags);
struct PrivilegeSet *privilegeset_get(const char *name);
struct PrivilegeSet *privilegeset_ref(struct PrivilegeSet *set);
void privilegeset_unref(struct PrivilegeSet *set);
void privilegeset_mark_all_illegal(void);
void privilegeset_delete_all_illegal(void);
void privilegeset_report(struct Client *source_p);

#endif
