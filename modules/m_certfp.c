#include "stdinc.h"
#include "client.h"
#include "common.h"
#include "match.h"
#include "hash.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "modules.h"

static int me_certfp(struct Client *, struct Client *, int, const char **);

struct Message certfp_msgtab = {
	"CERTFP", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_ignore, mg_ignore, mg_ignore, {me_certfp, 0}, mg_ignore}
};

mapi_clist_av1 certfp_clist[] = { &certfp_msgtab, NULL };

DECLARE_MODULE_AV1(certfp, NULL, NULL, certfp_clist, NULL, NULL, "$Revision$");

/*
** me_certfp
**      parv[1] = certfp string
*/
static int
me_certfp(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if (!IsPerson(source_p))
		return 0;
	if (parc != 2)
		return 0;

	rb_free(source_p->certfp);
	source_p->certfp = NULL;
	if (!EmptyString(parv[1]))
		source_p->certfp = rb_strdup(parv[1]);
	return 0;
}
