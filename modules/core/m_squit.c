#include "stdinc.h"
#include "client.h"
#include "common.h"		/* FALSE bleah */
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "logger.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hash.h"
#include "s_newconf.h"

static int ms_squit(struct Client *, struct Client *, int, const char **);
static int mo_squit(struct Client *, struct Client *, int, const char **);

struct Message squit_msgtab = {
	"SQUIT", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, {ms_squit, 0}, {ms_squit, 0}, mg_ignore, {mo_squit, 2}}
};

mapi_clist_av1 squit_clist[] = { &squit_msgtab, NULL };

DECLARE_MODULE_AV1(squit, NULL, NULL, squit_clist, NULL, NULL, "$Revision: 3161 $");

struct squit_parms
{
	const char *server_name;
	struct Client *target_p;
};

static struct squit_parms *find_squit(struct Client *client_p,
				      struct Client *source_p, const char *server);


/*
 * mo_squit - SQUIT message handler
 *      parv[1] = server name
 *      parv[2] = comment
 */
static int
mo_squit(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct squit_parms *found_squit;
	const char *comment = (parc > 2 && parv[2]) ? parv[2] : client_p->name;

        if (!IsOperRouting(source_p))
        {
                sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "routing");
                return 0;
        }

	if((found_squit = find_squit(client_p, source_p, parv[1])))
	{
		if(MyConnect(found_squit->target_p))
		{
			sendto_realops_snomask(SNO_GENERAL, L_ALL,
					     "Received SQUIT %s from %s (%s)",
					     found_squit->target_p->name,
					     get_client_name(source_p, HIDE_IP), comment);
			ilog(L_SERVER, "Received SQUIT %s from %s (%s)",
			     found_squit->target_p->name, log_client_name(source_p, HIDE_IP),
			     comment);
		}

		exit_client(client_p, found_squit->target_p, source_p, comment);
		return 0;
	}
	else
	{
		sendto_one_numeric(source_p, ERR_NOSUCHSERVER, form_str(ERR_NOSUCHSERVER), parv[1]);
	}

	return 0;
}

/*
 * ms_squit - SQUIT message handler
 *      parv[1] = server name
 *      parv[2] = comment
 */
static int
ms_squit(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	const char *comment = (parc > 2 && parv[2]) ? parv[2] : client_p->name;

	if(parc < 2)
		target_p = client_p;
	else
	{
		if((target_p = find_server(NULL, parv[1])) == NULL)
			return 0;

		if(IsMe(target_p))
			target_p = client_p;
		if(!IsServer(target_p))
			return 0;
	}

	/* Server is closing its link */
	if (target_p == client_p)
	{
		sendto_realops_snomask(SNO_GENERAL, L_ALL, "Server %s closing link (%s)",
				target_p->name, comment);
	}
	/*
	 **  Notify all opers, if my local link is remotely squitted
	 */
	else if(MyConnect(target_p))
	{
		sendto_realops_snomask(SNO_GENERAL, L_NETWIDE, "Remote SQUIT %s from %s (%s)",
				     target_p->name, source_p->name, comment);

		ilog(L_SERVER, "SQUIT From %s : %s (%s)", source_p->name, target_p->name, comment);
	}
	exit_client(client_p, target_p, source_p, comment);
	return 0;
}


/*
 * find_squit
 * inputs	- local server connection
 *		-
 *		-
 * output	- pointer to struct containing found squit or none if not found
 * side effects	-
 */
static struct squit_parms *
find_squit(struct Client *client_p, struct Client *source_p, const char *server)
{
	static struct squit_parms found_squit;
	struct Client *target_p = NULL;
	struct Client *p;
	rb_dlink_node *ptr;

	/* must ALWAYS be reset */
	found_squit.target_p = NULL;
	found_squit.server_name = NULL;


	/*
	 ** The following allows wild cards in SQUIT. Only useful
	 ** when the command is issued by an oper.
	 */

	RB_DLINK_FOREACH(ptr, global_serv_list.head)
	{
		p = ptr->data;
		if(IsServer(p) || IsMe(p))
		{
			if(match(server, p->name))
			{
				target_p = p;
				break;
			}
		}
	}

	if(target_p == NULL)
		return NULL;

	found_squit.target_p = target_p;
	found_squit.server_name = server;

	if(IsMe(target_p))
	{
		if(IsClient(client_p))
		{
			if(MyClient(client_p))
				sendto_one_notice(source_p, ":You are trying to squit me.");

			return NULL;
		}
		else
		{
			found_squit.target_p = client_p;
			found_squit.server_name = client_p->name;
		}
	}

	if(found_squit.target_p != NULL)
		return &found_squit;
	else
		return (NULL);
}
