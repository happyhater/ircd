#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "numeric.h"
#include "hook.h"
#include "msg.h"
#include "s_serv.h"		/* hunt_server */
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "cache.h"

static int m_motd(struct Client *, struct Client *, int, const char **);
static int mo_motd(struct Client *, struct Client *, int, const char **);

struct Message motd_msgtab = {
	"MOTD", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_motd, 0}, {mo_motd, 0}, mg_ignore, mg_ignore, {mo_motd, 0}}
};

int doing_motd_hook;

mapi_clist_av1 motd_clist[] = { &motd_msgtab, NULL };
mapi_hlist_av1 motd_hlist[] = {
	{ "doing_motd",	&doing_motd_hook },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(motd, NULL, NULL, motd_clist, motd_hlist, NULL, "$Revision: 254 $");

static void motd_spy(struct Client *);

/*
** m_motd
**      parv[1] = servername
*/
static int
m_motd(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0;

	if (parc < 2) {
		/* do nothing */
	} else if ((last_used + ConfigFileEntry.pace_wait) > rb_current_time()) {
		/* safe enough to give this on a local connect only */
		sendto_one(source_p, form_str(RPL_LOAD2HI),
			   me.name, source_p->name, "MOTD");
		sendto_one(source_p, form_str(RPL_ENDOFMOTD),
			   me.name, source_p->name);
		return 0;
	} else {
		last_used = rb_current_time();
	}

	if(hunt_server(client_p, source_p, ":%s MOTD :%s", 1, parc, parv) != HUNTED_ISME)
		return 0;

	motd_spy(source_p);
	send_user_motd(source_p);

	return 0;
}

/*
** mo_motd
**      parv[1] = servername
*/
static int
mo_motd(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(hunt_server(client_p, source_p, ":%s MOTD :%s", 1, parc, parv) != HUNTED_ISME)
		return 0;

	motd_spy(source_p);
	send_user_motd(source_p);

	return 0;
}

/* motd_spy()
 *
 * input        - pointer to client
 * output       - none
 * side effects - hook doing_motd is called
 */
static void
motd_spy(struct Client *source_p)
{
	hook_data data;

	data.client = source_p;
	data.arg1 = data.arg2 = NULL;

	call_hook(doing_motd_hook, &data);
}
