#ifndef INCLUDED_send_h
#define INCLUDED_send_h

#include "ratbox_lib.h"
#include "ircd_defs.h"
#include "config.h"		/* HAVE_STDARG_H */

struct Client;
struct Channel;
struct monitor;

/* The nasty global also used in s_serv.c for server bursts */
extern unsigned long current_serial;

extern struct Client *remote_rehash_oper_p;

extern void send_pop_queue(struct Client *);

extern void send_queued(struct Client *to);

extern void sendto_one(struct Client *target_p, const char *, ...) AFP(2, 3);
extern void sendto_one_notice(struct Client *target_p,const char *, ...) AFP(2, 3);
extern void sendto_one_prefix(struct Client *target_p, struct Client *source_p,
			      const char *command, const char *, ...) AFP(4, 5);
extern void sendto_one_numeric(struct Client *target_p,
			       int numeric, const char *, ...) AFP(3, 4);

extern void sendto_server(struct Client *one, struct Channel *chptr,
			  unsigned long caps, unsigned long nocaps,
			  const char *format, ...) AFP(5, 6);

extern void sendto_channel_flags(struct Client *one, int type, struct Client *source_p,
				 struct Channel *chptr, const char *, ...) AFP(5, 6);
extern void sendto_channel_message(struct Client *one, int type, struct Client *source_p,
				 struct Channel *chptr, const char *command, const char *target,
				 const char *, ...) AFP(7, 8);
extern void sendto_channel_opmod(struct Client *one, struct Client *source_p,
				 struct Channel *chptr, const char *command,
				 const char *text);

extern void sendto_channel_local(int type, struct Channel *, const char *, ...) AFP(3, 4);
extern void sendto_channel_local_butone(struct Client *, int type, struct Channel *, const char *, ...) AFP(4, 5);

extern void sendto_channel_local_with_capability(int type, int caps, int negcaps, struct Channel *, const char *, ...) AFP(5, 6);
extern void sendto_channel_local_with_capability_butone(struct Client *, int type, int caps, int negcaps, struct Channel *,
							const char *, ...) AFP(6, 7);

extern void sendto_common_channels_local(struct Client *, int cap, const char *, ...) AFP(3, 4);
extern void sendto_common_channels_local_butone(struct Client *, int cap, const char *, ...) AFP(3, 4);


extern void sendto_match_butone(struct Client *, struct Client *,
				const char *, int, const char *, ...) AFP(5, 6);
extern void sendto_match_servs(struct Client *source_p, const char *mask, 
				int capab, int, const char *, ...) AFP(5, 6);

extern void sendto_monitor(struct monitor *monptr, const char *, ...) AFP(2, 3);

extern void sendto_anywhere(struct Client *, struct Client *, const char *,
			    const char *, ...) AFP(4, 5);

extern void sendto_anywhere_message(struct Client *, struct Client *, const char *,
			    const char *, ...) AFP(4, 5);

extern void sendto_realops_snomask(int, int, const char *, ...) AFP(3, 4);
extern void sendto_realops_snomask_from(int, int, struct Client *, const char *, ...) AFP(4, 5);

extern void sendto_wallops_flags(int, struct Client *, const char *, ...) AFP(3, 4);

extern void kill_client(struct Client *client_p, struct Client *diedie,
			 const char *pattern, ...) AFP(3, 4);
extern void kill_client_serv_butone(struct Client *one, struct Client *source_p, 
				    const char *pattern, ...) AFP(3, 4);

#define L_ALL 	0
#define L_OPER 	1
#define L_ADMIN	2
#define L_NETWIDE 256 /* OR with L_ALL or L_OPER */

#define NOCAPS          0	/* no caps */

/* used when sending to #mask or $mask */
#define MATCH_SERVER  1
#define MATCH_HOST    2

#endif /* INCLUDED_send_h */
