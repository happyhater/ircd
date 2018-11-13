#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_conf.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int m_users(struct Client *, struct Client *, int, const char **);

struct Message users_msgtab = {
	"USERS", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_users, 0}, {m_users, 0}, mg_ignore, mg_ignore, {m_users, 0}}
};

mapi_clist_av1 users_clist[] = { &users_msgtab, NULL };
DECLARE_MODULE_AV1(users, NULL, NULL, users_clist, NULL, NULL, "$Revision: 254 $");

/*
 * m_users
 *      parv[1] = servername
 */
static int
m_users(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(hunt_server(client_p, source_p, ":%s USERS :%s", 1, parc, parv) == HUNTED_ISME)
	{
		sendto_one_numeric(source_p, RPL_LOCALUSERS,
				   form_str(RPL_LOCALUSERS),
				   rb_dlink_list_length(&lclient_list), 
				   Count.max_loc,
				   rb_dlink_list_length(&lclient_list), 
				   Count.max_loc);

		sendto_one_numeric(source_p, RPL_GLOBALUSERS, 
				   form_str(RPL_GLOBALUSERS),
				   Count.total, Count.max_tot,
				   Count.total, Count.max_tot);
	}

	return 0;
}
