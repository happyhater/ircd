#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"		/* ConfigFileEntry */
#include "s_serv.h"		/* uplink/IsCapable */
#include "hash.h"
#include "s_newconf.h"
#include "s_user.h"

#include <string.h>

static int m_dehelper(struct Client *, struct Client *, int, const char **);
static int me_dehelper(struct Client *, struct Client *, int, const char **);

static int do_dehelper(struct Client *source_p, struct Client *target_p);

struct Message dehelper_msgtab = {
	"DEHELPER", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_dehelper, 2}, mg_not_oper, mg_ignore, {me_dehelper, 2}, {m_dehelper, 2}}
};

mapi_clist_av1 dehelper_clist[] = { &dehelper_msgtab, NULL };
DECLARE_MODULE_AV1(dehelper, NULL, NULL, dehelper_clist, NULL, NULL, "$Revision: 254 $");

static int m_dehelper(struct Client *client_p, struct Client *source_p, int parc, const char **parv)
{
	struct Client *target_p;

	if (!IsAnyOper(source_p))
	{
		sendto_one_numeric(source_p, ERR_NOPRIVILEGES, form_str(ERR_NOPRIVILEGES));
		return 0;
	}
	if (!IsOperDehelper(source_p))
	{
		sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "dehelper");
		return 0;
	}

	if(!(target_p = find_named_person(parv[1])))
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), me.name, parv[1]);
		return 0;
	}

	if(MyClient(target_p))
		do_dehelper(source_p, target_p);
	else
		sendto_one(target_p, ":%s ENCAP %s DEHELPER %s",
				use_id(source_p), target_p->servptr->name, use_id(target_p));

	return 0;
}

static int me_dehelper(struct Client *client_p, struct Client *source_p, int parc, const char **parv)
{
	struct Client *target_p = find_person(parv[1]);
	if(!target_p)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), me.name, parv[1]);
		return 0;
	}
	if(!MyClient(target_p))
		return 0;

	do_dehelper(source_p, target_p);
	return 0;
}

static int do_dehelper(struct Client *source_p, struct Client *target_p)
{
	const char *fakeparv[4];

	if(!IsHelpOp(target_p))
		return 0;

	sendto_realops_snomask(SNO_GENERAL, L_NETWIDE, "%s is using DEHELPER on %s",
			source_p->name, target_p->name);
	sendto_one_notice(target_p, ":*** %s is using DEHELPER on you", source_p->name);

	fakeparv[0] = fakeparv[1] = target_p->name;
	fakeparv[2] = "-h";
	fakeparv[3] = NULL;
	user_mode(target_p, target_p, 3, fakeparv);
	return 0;
}




