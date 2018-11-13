#include "stdinc.h"
#include "class.h"
#include "hook.h"
#include "client.h"
#include "hash.h"
#include "common.h"
#include "hash.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "s_user.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int me_snote(struct Client *, struct Client *, int, const char **);

struct Message snote_msgtab = {
	"SNOTE", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_not_oper, mg_ignore, mg_ignore, {me_snote, 3}, mg_ignore}
};

mapi_clist_av1 snote_clist[] = { &snote_msgtab, NULL };
DECLARE_MODULE_AV1(snote, NULL, NULL, snote_clist, NULL, NULL, "$Revision: 623 $");

/*
 * me_snote
 *      parv[1] = snomask letter
 *	parv[2] = message
 */
static int
me_snote(struct Client *client_p, struct Client *source_p, int parc, 
	const char *parv[])
{
	/* if there's more than just two params, this is a protocol
	 * violation, but it seems stupid to drop servers over it,
	 * shit happens afterall -nenolod
	 */
	if (parc > 3)
		return 0;
	if (!IsServer(source_p))
		return 0;

	sendto_realops_snomask_from(snomask_modes[(unsigned char) *parv[1]],
		L_ALL, source_p, "%s", parv[2]);

	return 0;
}
