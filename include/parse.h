#ifndef INCLUDED_parse_h_h
#define INCLUDED_parse_h_h

#include "irc_dictionary.h"

struct Message;
struct Client;

extern void parse(struct Client *, char *, char *);
extern void handle_encap(struct Client *, struct Client *, 
		         const char *, int, const char *parv[]);
extern void clear_hash_parse(void);
extern void mod_add_cmd(struct Message *msg);
extern void mod_del_cmd(struct Message *msg);
extern void report_messages(struct Client *);

extern struct Dictionary *alias_dict;

#endif /* INCLUDED_parse_h_h */
