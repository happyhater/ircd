#ifndef INCLUDED_s_user_h
#define INCLUDED_s_user_h

#include "config.h"

struct Client;
struct User;
struct oper_conf;
extern time_t LastUsedWallops;

extern int valid_hostname(const char *hostname);
extern int valid_username(const char *username);

extern int user_mode(struct Client *, struct Client *, int, const char **);
extern void send_umode(struct Client *, struct Client *, int, int, char *);
extern void send_umode_out(struct Client *, struct Client *, int);
extern int show_lusers(struct Client *source_p);
extern int register_local_user(struct Client *, struct Client *, const char *);

extern int introduce_client(struct Client *client_p, struct Client *source_p, 
			    struct User *user, const char *nick, int use_euid);

extern void change_nick_user_host(struct Client *target_p, const char *nick, const char *user,
				  const char *host, int newts, const char *format, ...);

extern int user_modes[256];
extern unsigned int find_umode_slot(void);
extern void construct_umodebuf(void);

extern int oper_up(struct Client *, struct oper_conf *);

#endif
