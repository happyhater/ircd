#ifndef _BLACKLIST_H_
#define _BLACKLIST_H_

/* A configured DNSBL */
struct Blacklist {
	unsigned int status;	/* If CONF_ILLEGAL, delete when no clients */
	int refcount;
	char host[IRCD_RES_HOSTLEN + 1];
	char reject_reason[IRCD_BUFSIZE];
	unsigned int hits;
	time_t lastwarning;
};

/* A lookup in progress for a particular DNSBL for a particular client */
struct BlacklistClient {
	struct Blacklist *blacklist;
	struct Client *client_p;
	struct DNSQuery dns_query;
	rb_dlink_node node;
};

/* public interfaces */
struct Blacklist *new_blacklist(char *host, char *reject_entry);
void lookup_blacklists(struct Client *client_p);
void abort_blacklist_queries(struct Client *client_p);
void unref_blacklist(struct Blacklist *blptr);
void destroy_blacklists(void);

extern rb_dlink_list blacklist_list;

#endif
