#include "stdinc.h"
#include "client.h"
#include "modules.h"
#include "send.h"
#include "numeric.h"

static int me_extendchans(struct Client *, struct Client *, int, const char **);

struct Message extendchans_msgtab = {
	"EXTENDCHANS", 0, 0, 0, MFLG_SLOW,
	{ mg_unreg, mg_ignore, mg_ignore, mg_ignore, {me_extendchans, 1}, mg_ignore}
};

mapi_clist_av1 extendchans_clist[] = { &extendchans_msgtab, NULL };

DECLARE_MODULE_AV1(extendchans, NULL, NULL, extendchans_clist, NULL, NULL, "$Revision: $");

static int
me_extendchans(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;

	target_p = find_person(parv[1]);
	if(target_p == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), parv[1]);
		return 0;
	}

	if(!MyClient(target_p))
		return 0;

	sendto_one_notice(target_p, ":*** %s is extending your channel limit", source_p->name);
	SetExtendChans(target_p);

	return 0;
}
