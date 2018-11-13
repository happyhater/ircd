#ifndef INCLUDED_listener_h
#define INCLUDED_listener_h

#include "ircd_defs.h"

struct Client;

struct Listener
{
	struct Listener *next;	/* list node pointer */
	const char *name;	/* listener name */
	rb_fde_t *F;		/* file descriptor */
	int ref_count;		/* number of connection references */
	int active;		/* current state of listener */
	int ssl;		/* ssl listener */
	struct rb_sockaddr_storage addr;
	struct DNSQuery *dns_query;
	char vhost[HOSTLEN + 1];	/* virtual name of listener */
};

extern void add_listener(int port, const char *vaddr_ip, int family, int ssl);
extern void close_listener(struct Listener *listener);
extern void close_listeners(void);
extern const char *get_listener_name(const struct Listener *listener);
extern void show_ports(struct Client *client);
extern void free_listener(struct Listener *);

#endif /* INCLUDED_listener_h */
