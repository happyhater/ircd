#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "match.h"
#include "numeric.h"
#include "send.h"
#include "s_user.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_serv.h"

static int mo_adminwall(struct Client *, struct Client *, int, const char **);
static int me_adminwall(struct Client *, struct Client *, int, const char **);

struct Message adminwall_msgtab = {
	"ADMINWALL", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, {me_adminwall, 2}, {mo_adminwall, 2}}
};
                

mapi_clist_av1 adminwall_clist[] = { &adminwall_msgtab, NULL };
DECLARE_MODULE_AV1(adminwall, NULL, NULL, adminwall_clist, NULL, NULL, "$Revision: 20702 $");


/*
 * mo_adminwall (write to *all* admins currently online)
 *      parv[1] = message text
 */

static int
mo_adminwall(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])  
{ 
        if(!IsAdmin(source_p))
        {
                sendto_one(source_p, form_str(ERR_NOPRIVS),
                           me.name, source_p->name, "adminwall");
                return 0;
        }
        sendto_wallops_flags(UMODE_ADMIN, source_p, "ADMINWALL - %s", parv[1]);
        sendto_match_servs(source_p, "*", CAP_ENCAP, NOCAPS, "ENCAP * ADMINWALL :%s", parv[1]);
        return 0;
} 

static int
me_adminwall(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])  
{ 
        sendto_wallops_flags(UMODE_ADMIN, source_p, "ADMINWALL - %s", parv[1]);
        return 0;
}
