#include "stdinc.h"
#include "client.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "s_serv.h"
#include "packet.h"


static int m_away(struct Client *, struct Client *, int, const char **);

struct Message away_msgtab = {
	"AWAY", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_away, 0}, {m_away, 0}, mg_ignore, mg_ignore, {m_away, 0}}
};

mapi_clist_av1 away_clist[] = { &away_msgtab, NULL };
DECLARE_MODULE_AV1(away, NULL, NULL, away_clist, NULL, NULL, "$Revision: 3370 $");

/*
** m_away
**      parv[1] = away message
*/
static int
m_away(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(MyClient(source_p) && !IsFloodDone(source_p))
		flood_endgrace(source_p);

	if(!IsClient(source_p))
		return 0;

	if(parc < 2 || EmptyString(parv[1]))
	{
		/* Marking as not away */
		if(source_p->user->away != NULL)
		{
			/* we now send this only if they were away before --is */
			sendto_server(client_p, NULL, CAP_TS6, NOCAPS,
				      ":%s AWAY", use_id(source_p));
			free_away(source_p);

			sendto_common_channels_local_butone(source_p, CLICAP_AWAY_NOTIFY, ":%s!%s@%s AWAY",
							    source_p->name, source_p->username, source_p->host);
		}
		if(MyConnect(source_p))
			sendto_one_numeric(source_p, RPL_UNAWAY, form_str(RPL_UNAWAY));
		return 0;
	}

	if(source_p->user->away == NULL)
		allocate_away(source_p);
	if(strncmp(source_p->user->away, parv[1], AWAYLEN - 1))
	{
		rb_strlcpy(source_p->user->away, parv[1], AWAYLEN);
		sendto_server(client_p, NULL, CAP_TS6, NOCAPS, 
			      ":%s AWAY :%s", use_id(source_p), source_p->user->away);
	}
	
	if(MyConnect(source_p))
		sendto_one_numeric(source_p, RPL_NOWAWAY, form_str(RPL_NOWAWAY));

	sendto_common_channels_local_butone(source_p, CLICAP_AWAY_NOTIFY, ":%s!%s@%s AWAY :%s",
					    source_p->name, source_p->username, source_p->host,
					    source_p->user->away);

	return 0;
}
