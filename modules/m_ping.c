#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "match.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hash.h"
#include "s_conf.h"
#include "s_serv.h"

static int m_ping(struct Client *, struct Client *, int, const char **);
static int ms_ping(struct Client *, struct Client *, int, const char **);

struct Message ping_msgtab = {
	"PING", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_ping, 2}, {ms_ping, 2}, {ms_ping, 2}, mg_ignore, {m_ping, 2}}
};

mapi_clist_av1 ping_clist[] = { &ping_msgtab, NULL };
DECLARE_MODULE_AV1(ping, NULL, NULL, ping_clist, NULL, NULL, "$Revision: 254 $");

/*
** m_ping
**      parv[1] = origin
**      parv[2] = destination
*/
static int
m_ping(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	const char *destination;

	destination = parv[2];	/* Will get NULL or pointer (parc >= 2!!) */

	if(!EmptyString(destination) && !match(destination, me.name))
	{
		if((target_p = find_server(source_p, destination)))
		{
			sendto_one(target_p, ":%s PING %s :%s",
				   get_id(source_p, target_p),
				   source_p->name, get_id(target_p, target_p));
		}
		else
		{
			sendto_one_numeric(source_p, ERR_NOSUCHSERVER,
					   form_str(ERR_NOSUCHSERVER),
					   destination);
			return 0;
		}
	}
	else
		sendto_one(source_p, ":%s PONG %s :%s", me.name,
			   (destination) ? destination : me.name, parv[1]);

	return 0;
}

static int
ms_ping(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	const char *destination;

	destination = parv[2];	/* Will get NULL or pointer (parc >= 2!!) */

	if(!EmptyString(destination) && irccmp(destination, me.name) &&
	   irccmp(destination, me.id))
	{
		if((target_p = find_client(destination)) && IsServer(target_p))
			sendto_one(target_p, ":%s PING %s :%s", 
				   get_id(source_p, target_p), source_p->name,
				   get_id(target_p, target_p));
		/* not directed at an id.. */
		else if(!IsDigit(*destination))
			sendto_one_numeric(source_p, ERR_NOSUCHSERVER,
					   form_str(ERR_NOSUCHSERVER),
					   destination);
	}
	else
		sendto_one(source_p, ":%s PONG %s :%s", 
			   get_id(&me, source_p), me.name, 
			   get_id(source_p, source_p));

	return 0;
}
