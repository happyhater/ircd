#include "stdinc.h"
#include "client.h"
#include "modules.h"
#include "send.h"
#include "numeric.h"

static int me_identified(struct Client *, struct Client *, int, const char **);

static void h_im_nick_change(void *vdata);
static void h_im_burst_client(void *vdata);

struct Message identified_msgtab = {
	"IDENTIFIED", 0, 0, 0, MFLG_SLOW,
	{ mg_unreg, mg_ignore, mg_ignore, mg_ignore, {me_identified, 3}, mg_ignore}
};

mapi_hfn_list_av1 im_hfnlist[] = {
	{ "local_nick_change", (hookfn) h_im_nick_change },
	{ "remote_nick_change", (hookfn) h_im_nick_change },
	{ "burst_client", (hookfn) h_im_burst_client },
	{ NULL, NULL }
};

mapi_clist_av1 identified_clist[] = { &identified_msgtab, NULL };

DECLARE_MODULE_AV1(identify_msg, NULL, NULL, identified_clist, NULL, im_hfnlist, "$Revision: $");

static int
me_identified(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p = find_person(parv[1]);
	const char *nick = parv[2];

	if(target_p == NULL)
	{
		return 0;
	}

	if (irccmp(target_p->name, nick))
	{
		sendto_realops_snomask(SNO_DEBUG, L_ALL,
			"Dropping IDENTIFIED for %s due to nickname mismatch (%s)",
			target_p->name, nick);
		return 0;
	}

	if (parc > 3 && !irccmp(parv[3], "OFF"))
		ClearIdentifiedMsg(target_p);
	else
		SetIdentifiedMsg(target_p);

	return 0;
}

static void
h_im_nick_change(void *vdata)
{
	hook_data *data = vdata;
	if(data->client && 0 != irccmp(data->arg1, data->arg2))
		ClearIdentifiedMsg(data->client);
}

static void
h_im_burst_client(void *vdata)
{
	hook_data_client *data = vdata;
	struct Client *server = data->client;
	struct Client *target_p = data->target;

	if(IsIdentifiedMsg(target_p))
	    sendto_one(server, ":%s ENCAP * IDENTIFIED %s %s", me.id, use_id(target_p), target_p->name);
}

