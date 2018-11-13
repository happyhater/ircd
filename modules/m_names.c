#include "stdinc.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "s_serv.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int m_names(struct Client *, struct Client *, int, const char **);

struct Message names_msgtab = {
	"NAMES", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_names, 0}, mg_ignore, mg_ignore, mg_ignore, {m_names, 0}}
};

mapi_clist_av1 names_clist[] = { &names_msgtab, NULL };
DECLARE_MODULE_AV1(names, NULL, NULL, names_clist, NULL, NULL, "$Revision: 254 $");

static void names_global(struct Client *source_p);

/*
 * m_names
 *      parv[1] = channel
 */
static int
m_names(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0;
	struct Channel *chptr = NULL;
	char *s;

	if(parc > 1 && !EmptyString(parv[1]))
	{
		char *p = LOCAL_COPY(parv[1]);
		if((s = strchr(p, ',')))
			*s = '\0';

		if(!check_channel_name(p))
		{
			sendto_one_numeric(source_p, ERR_BADCHANNAME,
					   form_str(ERR_BADCHANNAME),
					   (unsigned char *) p);
			return 0;
		}

		if((chptr = find_channel(p)) != NULL)
			channel_member_names(chptr, source_p, 1);
		else
			sendto_one(source_p, form_str(RPL_ENDOFNAMES), 
				   me.name, source_p->name, p);
	}
	else
	{
		if(!IsOper(source_p))
		{
			if((last_used + ConfigFileEntry.pace_wait) > rb_current_time())
			{
				sendto_one(source_p, form_str(RPL_LOAD2HI),
					   me.name, source_p->name, "NAMES");
				sendto_one(source_p, form_str(RPL_ENDOFNAMES),
					   me.name, source_p->name, "*");
				return 0;
			}
			else
				last_used = rb_current_time();
		}

		names_global(source_p);
		sendto_one(source_p, form_str(RPL_ENDOFNAMES), 
			   me.name, source_p->name, "*");
	}

	return 0;
}

/*
 * names_global
 *
 * inputs       - pointer to client struct requesting names
 * output       - none
 * side effects - lists all non public non secret channels
 */
static void
names_global(struct Client *source_p)
{
	int mlen;
	int tlen;
	int cur_len;
	int dont_show = NO;
	rb_dlink_node *lp, *ptr;
	struct Client *target_p;
	struct Channel *chptr = NULL;
	struct membership *msptr;
	char buf[BUFSIZE];
	char *t;

	/* first do all visible channels */
	RB_DLINK_FOREACH(ptr, global_channel_list.head)
	{
		chptr = ptr->data;
		channel_member_names(chptr, source_p, 0);
	}
	cur_len = mlen = rb_sprintf(buf, form_str(RPL_NAMREPLY), 
				    me.name, source_p->name, "*", "*");
	t = buf + mlen;

	/* Second, do all clients in one big sweep */
	RB_DLINK_FOREACH(ptr, global_client_list.head)
	{
		target_p = ptr->data;
		dont_show = NO;

		if(!IsPerson(target_p) || IsInvisible(target_p))
			continue;

		/* we want to show -i clients that are either:
		 *   a) not on any channels
		 *   b) only on +p channels
		 *
		 * both were missed out above.  if the target is on a
		 * common channel with source, its already been shown.
		 */
		RB_DLINK_FOREACH(lp, target_p->user->channel.head)
		{
			msptr = lp->data;
			chptr = msptr->chptr;

			if(PubChannel(chptr) || IsMember(source_p, chptr) ||
			   SecretChannel(chptr))
			{
				dont_show = YES;
				break;
			}
		}

		if(dont_show)
			continue;

		if((cur_len + NICKLEN + 2) > (BUFSIZE - 3))
		{
			sendto_one(source_p, "%s", buf);
			cur_len = mlen;
			t = buf + mlen;
		}

		tlen = rb_sprintf(t, "%s ", target_p->name);
		cur_len += tlen;
		t += tlen;
	}

	if(cur_len > mlen)
		sendto_one(source_p, "%s", buf);
}
