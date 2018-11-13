#include "stdinc.h"
#include "channel.h"
#include "client.h" 
#include "ircd.h"
#include "numeric.h"
#include "logger.h"
#include "s_serv.h"
#include "s_conf.h"
#include "s_newconf.h" 
#include "send.h"
#include "whowas.h"
#include "match.h"
#include "hash.h" 
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int mo_ojoin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);


struct Message ojoin_msgtab = {
	"OJOIN", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, mg_ignore, {mo_ojoin, 2}}
};

mapi_clist_av1 ojoin_clist[] = { &ojoin_msgtab, NULL };

DECLARE_MODULE_AV1(ojoin, NULL, NULL, ojoin_clist, NULL, NULL, "$Revision: 3554 $");

/*
** mo_ojoin
**      parv[1] = channel
*/
static int
mo_ojoin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Channel *chptr;
	int move_me = 0;

	/* admins only */
	if(!IsOperAdmin(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "admin");
		return 0;
	}

	if(*parv[1] == '@' || *parv[1] == '%' || *parv[1] == '+')
	{
		parv[1]++;
		move_me = 1;
	}

	if((chptr = find_channel(parv[1])) == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHCHANNEL,
				   form_str(ERR_NOSUCHCHANNEL), parv[1]);
		return 0;
	}

	if(IsMember(source_p, chptr))
	{
		sendto_one_notice(source_p, ":Please part %s before using OJOIN", parv[1]);
		return 0;
	}

	if(move_me == 1)
		parv[1]--;

	sendto_wallops_flags(UMODE_WALLOP, &me,
			     "OJOIN called for %s by %s!%s@%s",
			     parv[1], source_p->name, source_p->username, source_p->host);
	ilog(L_MAIN, "OJOIN called for %s by %s",
	     parv[1], get_oper_name(source_p));
	/* only sends stuff for #channels remotely */
	sendto_server(NULL, chptr, NOCAPS, NOCAPS,
			":%s WALLOPS :OJOIN called for %s by %s!%s@%s",
			me.name, parv[1],
			source_p->name, source_p->username, source_p->host);

	if(*parv[1] == '@')
	{
		add_user_to_channel(chptr, source_p, CHFL_CHANOP);
		sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
			      ":%s SJOIN %ld %s + :@%s",
			      me.id, (long) chptr->channelts, chptr->chname, source_p->id);
		send_channel_join(chptr, source_p);
		sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s +o %s",
				     me.name, chptr->chname, source_p->name);

	}
	else if(*parv[1] == '+')
	{
		add_user_to_channel(chptr, source_p, CHFL_VOICE);
		sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
			      ":%s SJOIN %ld %s + :+%s",
			      me.id, (long) chptr->channelts, chptr->chname, source_p->id);
		send_channel_join(chptr, source_p);
		sendto_channel_local(ALL_MEMBERS, chptr, ":%s MODE %s +v %s",
				     me.name, chptr->chname, source_p->name);
	}
	else
	{
		add_user_to_channel(chptr, source_p, CHFL_PEON);
		sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
			      ":%s JOIN %ld %s +",
			      source_p->id, (long) chptr->channelts, chptr->chname);
		send_channel_join(chptr, source_p);
	}

	/* send the topic... */
	if(chptr->topic != NULL)
	{
		sendto_one(source_p, form_str(RPL_TOPIC), me.name,
			   source_p->name, chptr->chname, chptr->topic);
		sendto_one(source_p, form_str(RPL_TOPICWHOTIME), me.name,
			   source_p->name, chptr->chname, chptr->topic_info, chptr->topic_time);
	}

	source_p->localClient->last_join_time = rb_current_time();
	channel_member_names(chptr, source_p, 1);

	return 0;
}
