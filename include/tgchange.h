#ifndef INCLUDED_tgchange_h
#define INCLUDED_tgchange_h

#include "ircd_defs.h"

/* finds a channel where source_p has op or voice and target_p is a member */
struct Channel *find_allowing_channel(struct Client *source_p, struct Client *target_p);
/* checks if source_p is allowed to send to target_p */
int add_target(struct Client *source_p, struct Client *target_p);
/* checks if source_p is allowed to send to chptr */
int add_channel_target(struct Client *source_p, struct Channel *chptr);
/* allows source_p to send to target_p */
void add_reply_target(struct Client *source_p, struct Client *target_p);

#endif /* INCLUDED_tgchange_h */
