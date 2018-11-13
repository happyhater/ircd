#include "stdinc.h"
#include "client.h"
#include "common.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "restart.h"
#include "logger.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hash.h"

static int mo_restart(struct Client *, struct Client *, int, const char **);
static int me_restart(struct Client *, struct Client *, int, const char **);
static int do_restart(struct Client *source_p, const char *servername);

struct Message restart_msgtab = {
	"RESTART", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, {me_restart, 1}, {mo_restart, 0}}
};

mapi_clist_av1 restart_clist[] = { &restart_msgtab, NULL };
DECLARE_MODULE_AV1(restart, NULL, NULL, restart_clist, NULL, NULL, "$Revision: 3161 $");

/*
 * mo_restart
 *
 */
static int
mo_restart(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(!IsOperDie(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS),
			   me.name, source_p->name, "die");
		return 0;
	}

	if(parc < 2 || EmptyString(parv[1]))
	{
		sendto_one_notice(source_p, ":Need server name /restart %s", me.name);
		return 0;
	}

        if(parc > 2)
        {
                /* Remote restart. Pass it along. */
                struct Client *server_p = find_server(NULL, parv[2]);
                if (!server_p)
                {
                        sendto_one_numeric(source_p, ERR_NOSUCHSERVER, form_str(ERR_NOSUCHSERVER), parv[2]);
                        return 0;
                }
                if (!IsMe(server_p))
                {
                        sendto_one(server_p, ":%s ENCAP %s RESTART %s", source_p->name, parv[2], parv[1]);
                        return 0;
                }
        }

        return do_restart(source_p, parv[1]);
}

static int
me_restart(struct Client *client_p __unused, struct Client *source_p, int parc, const char *parv[])
{
        if(!find_shared_conf(source_p->username, source_p->host, source_p->servptr->name, SHARED_DIE))
        {
                sendto_one_notice(source_p, ":*** You do not have an appropriate shared block to "
                                "remotely restart this server.");
                return 0;
        }

        return do_restart(source_p, parv[1]);
}

static int
do_restart(struct Client *source_p, const char *servername)
{
	char buf[BUFSIZE];
	rb_dlink_node *ptr;
	struct Client *target_p;

	if(irccmp(servername, me.name))
	{
		sendto_one_notice(source_p, ":Mismatch on /restart %s", me.name);
		return 0;
        }

	RB_DLINK_FOREACH(ptr, lclient_list.head)
	{
		target_p = ptr->data;

		sendto_one_notice(target_p, ":Server Restarting. %s", get_client_name(source_p, HIDE_IP));
	}

	RB_DLINK_FOREACH(ptr, serv_list.head)
	{
		target_p = ptr->data;

		sendto_one(target_p, ":%s ERROR :Restart by %s",
			   me.name, get_client_name(source_p, HIDE_IP));
	}

	rb_sprintf(buf, "Server RESTART by %s", get_client_name(source_p, HIDE_IP));
	restart(buf);

	return 0;
}
