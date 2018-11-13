#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "hook.h"
#include "modules.h"

static int m_admin(struct Client *, struct Client *, int, const char **);
static int mr_admin(struct Client *, struct Client *, int, const char **);
static int ms_admin(struct Client *, struct Client *, int, const char **);
static void do_admin(struct Client *source_p);

static void admin_spy(struct Client *);

struct Message admin_msgtab = {
	"ADMIN", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{mr_admin, 0}, {m_admin, 0}, {ms_admin, 0}, mg_ignore, mg_ignore, {ms_admin, 0}}
};

int doing_admin_hook;

mapi_clist_av1 admin_clist[] = { &admin_msgtab, NULL };
mapi_hlist_av1 admin_hlist[] = { 
	{ "doing_admin",	&doing_admin_hook },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(admin, NULL, NULL, admin_clist, admin_hlist, NULL, "$Revision: 3368 $");

/*
 * mr_admin - ADMIN command handler
 *      parv[1] = servername   
 */
static int
mr_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0L;

	if((last_used + ConfigFileEntry.pace_wait) > rb_current_time())
	{
		sendto_one(source_p, form_str(RPL_LOAD2HI), 
			   me.name, 
			   EmptyString(source_p->name) ? "*" : source_p->name, 
			   "ADMIN");
		return 0;
	}
	else
		last_used = rb_current_time();

	do_admin(source_p);

	return 0;
}

/*
 * m_admin - ADMIN command handler
 *      parv[1] = servername
 */
static int
m_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0L;

	if(parc > 1)
	{
		if((last_used + ConfigFileEntry.pace_wait) > rb_current_time())
		{
			sendto_one(source_p, form_str(RPL_LOAD2HI),
				   me.name, source_p->name, "ADMIN");
			return 0;
		}
		else
			last_used = rb_current_time();
		
		if(hunt_server(client_p, source_p, ":%s ADMIN :%s", 1, parc, parv) != HUNTED_ISME)
			return 0;
	}

	do_admin(source_p);

	return 0;
}


/*
 * ms_admin - ADMIN command handler, used for OPERS as well
 *      parv[1] = servername
 */
static int
ms_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(hunt_server(client_p, source_p, ":%s ADMIN :%s", 1, parc, parv) != HUNTED_ISME)
		return 0;

	do_admin(source_p);

	return 0;
}


/*
 * do_admin
 *
 * inputs	- pointer to client to report to
 * output	- none
 * side effects	- admin info is sent to client given
 */
static void
do_admin(struct Client *source_p)
{
	if(IsPerson(source_p))
		admin_spy(source_p);

	sendto_one_numeric(source_p, RPL_ADMINME, form_str(RPL_ADMINME), me.name);
	if(AdminInfo.name != NULL)
		sendto_one_numeric(source_p, RPL_ADMINLOC1, form_str(RPL_ADMINLOC1), AdminInfo.name);
	if(AdminInfo.description != NULL)
		sendto_one_numeric(source_p, RPL_ADMINLOC2, form_str(RPL_ADMINLOC2), AdminInfo.description);
	if(AdminInfo.email != NULL)
		sendto_one_numeric(source_p, RPL_ADMINEMAIL, form_str(RPL_ADMINEMAIL), AdminInfo.email);
}

/* admin_spy()
 *
 * input	- pointer to client
 * output	- none
 * side effects - event doing_admin is called
 */
static void
admin_spy(struct Client *source_p)
{
	hook_data hd;

	hd.client = source_p;
	hd.arg1 = hd.arg2 = NULL;

	call_hook(doing_admin_hook, &hd);
}
