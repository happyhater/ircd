#ifndef INCLUDED_s_auth_h
#define INCLUDED_s_auth_h

struct Client;

extern void start_auth(struct Client *);
extern void init_auth(void);
extern void delete_auth_queries(struct Client *);

#endif /* INCLUDED_s_auth_h */
