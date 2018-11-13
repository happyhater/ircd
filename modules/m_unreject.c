#include "stdinc.h"
#include "client.h"
#include "s_conf.h"
#include "hostmask.h"
#include "reject.h"
#include "msg.h"
#include "modules.h"
#include "send.h"

static int mo_unreject(struct Client *, struct Client *, int, const char **);

struct Message unreject_msgtab = {
	"UNREJECT", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, mg_ignore, {mo_unreject, 2}}
};

mapi_clist_av1 unreject_clist[] = { &unreject_msgtab, NULL };
DECLARE_MODULE_AV1(unreject, NULL, NULL, unreject_clist, NULL, NULL, "$Revision: 3161 $");

/*
 * mo_unreject
 *
 */
static int
mo_unreject(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(ConfigFileEntry.reject_after_count == 0 || ConfigFileEntry.reject_ban_time == 0 ||
	   ConfigFileEntry.reject_duration == 0)
	{
		sendto_one_notice(source_p, ":Reject cache is disabled");
		return 0;			
	}

	if(!parse_netmask(parv[1], NULL, NULL))
	{
		sendto_one_notice(source_p, ":Unable to parse netmask %s", parv[1]);
		return 0;
	}	
	
	if(remove_reject_ip(parv[1]))
		sendto_one_notice(source_p, ":Removed reject for %s", parv[1]);
	else
		sendto_one_notice(source_p, ":Unable to remove reject for %s", parv[1]);

	return 0;
}
