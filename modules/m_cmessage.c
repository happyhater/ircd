#include "stdinc.h"
#include "client.h"
#include "channel.h"
#include "numeric.h"
#include "msg.h"
#include "modules.h"
#include "hash.h"
#include "send.h"
#include "s_conf.h"
#include "packet.h"
#include "supported.h"

static int _modinit(void);
static void _moddeinit(void);

static int m_cmessage(int, enum message_type, struct Client *, struct Client *, int, const char **);
static int m_cprivmsg(struct Client *, struct Client *, int, const char **);
static int m_cnotice(struct Client *, struct Client *, int, const char **);

struct Message cprivmsg_msgtab = {
	"CPRIVMSG", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, {m_cprivmsg, 4}, mg_ignore, mg_ignore, mg_ignore, {m_cprivmsg, 4}}
};
struct Message cnotice_msgtab = {
	"CNOTICE", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, {m_cnotice, 4}, mg_ignore, mg_ignore, mg_ignore, {m_cnotice, 4}}
};

mapi_clist_av1 cmessage_clist[] = { &cprivmsg_msgtab, &cnotice_msgtab, NULL };
DECLARE_MODULE_AV1(cmessage, _modinit, _moddeinit, cmessage_clist, NULL, NULL, "$Revision: 1543 $");

static const char *cmdname[MESSAGE_TYPE_COUNT] = {
	[MESSAGE_TYPE_PRIVMSG] = "PRIVMSG",
	[MESSAGE_TYPE_NOTICE] = "NOTICE",
};

static int _modinit(void)
{
	add_isupport("CPRIVMSG", isupport_string, "");
	add_isupport("CNOTICE", isupport_string, "");
	return 0;
}

static void _moddeinit(void)
{
	delete_isupport("CPRIVMSG");
	delete_isupport("CNOTICE");
}

#define PRIVMSG 0
#define NOTICE 1

static int
m_cprivmsg(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	return m_cmessage(PRIVMSG, MESSAGE_TYPE_PRIVMSG, client_p, source_p, parc, parv);
}

static int
m_cnotice(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	return m_cmessage(NOTICE, MESSAGE_TYPE_NOTICE, client_p, source_p, parc, parv);
}

static int
m_cmessage(int p_or_n, enum message_type msgtype,
		struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	struct Channel *chptr;
	struct membership *msptr;

	hook_data_privmsg_user hdata;

	if(!IsFloodDone(source_p))
		flood_endgrace(source_p);

	if((target_p = find_named_person(parv[1])) == NULL)
	{
		if(p_or_n != NOTICE)
			sendto_one_numeric(source_p, ERR_NOSUCHNICK,
					form_str(ERR_NOSUCHNICK), parv[1]);
		return 0;
	}

	if((chptr = find_channel(parv[2])) == NULL)
	{
		if(p_or_n != NOTICE)
			sendto_one_numeric(source_p, ERR_NOSUCHCHANNEL,
					form_str(ERR_NOSUCHCHANNEL), parv[2]);
		return 0;
	}

	if((msptr = find_channel_membership(chptr, source_p)) == NULL)
	{
		if(p_or_n != NOTICE)
			sendto_one_numeric(source_p, ERR_NOTONCHANNEL,
					form_str(ERR_NOTONCHANNEL),
					chptr->chname);
		return 0;
	}

	if(!is_chanop_voiced(msptr))
	{
		if(p_or_n != NOTICE)
			sendto_one(source_p, form_str(ERR_VOICENEEDED),
				me.name, source_p->name, chptr->chname);
		return 0;
	}

	if(!IsMember(target_p, chptr))
	{
		if(p_or_n != NOTICE)
			sendto_one_numeric(source_p, ERR_USERNOTINCHANNEL,
					form_str(ERR_USERNOTINCHANNEL),
					target_p->name, chptr->chname);
		return 0;
	}

	if(MyClient(target_p) && (IsSetCallerId(target_p) || (IsSetRegOnlyMsg(target_p) && !source_p->user->suser[0])) &&
	   !accept_message(source_p, target_p) && !IsOper(source_p))
	{
		if (IsSetRegOnlyMsg(target_p) && !source_p->user->suser[0])
		{
			if (p_or_n != NOTICE)
				sendto_one_numeric(source_p, ERR_NONONREG,
						form_str(ERR_NONONREG),
						target_p->name);
			return 0;
		}
		if(p_or_n != NOTICE)
			sendto_one_numeric(source_p, ERR_TARGUMODEG,
					form_str(ERR_TARGUMODEG), target_p->name);

		if((target_p->localClient->last_caller_id_time +
		    ConfigFileEntry.caller_id_wait) < rb_current_time())
		{
			if(p_or_n != NOTICE)
				sendto_one_numeric(source_p, RPL_TARGNOTIFY,
						form_str(RPL_TARGNOTIFY),
						target_p->name);

			sendto_one(target_p, form_str(RPL_UMODEGMSG),
				me.name, target_p->name, source_p->name,
				source_p->username, source_p->host);

			target_p->localClient->last_caller_id_time = rb_current_time();
		}

		return 0;
	}

	if(p_or_n != NOTICE)
		source_p->localClient->last = rb_current_time();

	hdata.msgtype = msgtype;
	hdata.source_p = source_p;
	hdata.target_p = target_p;
	hdata.text = parv[3];
	hdata.approved = 0;

	call_hook(h_privmsg_user, &hdata);

	if (hdata.approved != 0)
		return;

	/* the hook may have killed them */
	if (IsAnyDead(source_p))
		return;

	sendto_anywhere_message(target_p, source_p, cmdname[msgtype], "%s", hdata.text);

	return 0;
}
