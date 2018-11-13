#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "match.h"
#include "numeric.h"
#include "logger.h"
#include "s_conf.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_newconf.h"
#include "hash.h"

static int mo_die(struct Client *, struct Client *, int, const char **);
static int me_die(struct Client *, struct Client *, int, const char **);
static int do_die(struct Client *, const char *);

static struct Message die_msgtab = {
	"DIE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, {me_die, 1}, {mo_die, 0}}
};

mapi_clist_av1 die_clist[] = { &die_msgtab, NULL };

DECLARE_MODULE_AV1(die, NULL, NULL, die_clist, NULL, NULL, "$Revision: 3295 $");

/*
 * mo_die - DIE command handler
 */
static int
mo_die(struct Client *client_p __unused, struct Client *source_p, int parc, const char *parv[])
{
	if(!IsOperDie(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "die");
		return 0;
	}

	if(parc < 2 || EmptyString(parv[1]))
	{
		sendto_one_notice(source_p, ":Need server name /die %s", me.name);
		return 0;
	}

        if(parc > 2)
        {
                /* Remote die. Pass it along. */
                struct Client *server_p = find_server(NULL, parv[2]);
                if (!server_p)
                {
                        sendto_one_numeric(source_p, ERR_NOSUCHSERVER, form_str(ERR_NOSUCHSERVER), parv[2]);
                        return 0;
                }
                if (!IsMe(server_p))
                {
                        sendto_one(server_p, ":%s ENCAP %s DIE %s", source_p->name, parv[2], parv[1]);
                        return 0;
                }
        }

        return do_die(source_p, parv[1]);
}

static int
me_die(struct Client *client_p __unused, struct Client *source_p, int parc, const char *parv[])
{
        if(!find_shared_conf(source_p->username, source_p->host, source_p->servptr->name, SHARED_DIE))
        {
                sendto_one_notice(source_p, ":*** You do not have an appropriate shared block to "
                                "remotely shut down this server.");
                return 0;
        }

        return do_die(source_p, parv[1]);
}

static int
do_die(struct Client *source_p, const char *servername)
{
	if(irccmp(servername, me.name))
	{
		sendto_one_notice(source_p, ":Mismatch on /die %s", me.name);
		return 0;
	}

	ircd_shutdown(get_client_name(source_p, HIDE_IP));

	return 0;
}
