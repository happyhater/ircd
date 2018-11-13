#include "stdinc.h"
#include "modules.h"
#include "client.h"
#include "hook.h"
#include "ircd.h"
#include "send.h"

static void h_nn_server_eob(struct Client *);
static void h_nn_client_exit(hook_data_client_exit *);

mapi_hfn_list_av1 nn_hfnlist[] = {
	{ "server_eob", (hookfn) h_nn_server_eob },
	{ "client_exit", (hookfn) h_nn_client_exit },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(networknotice, NULL, NULL, NULL, NULL, nn_hfnlist, "$Revision: 1172 $");

/*
 * count_mark_downlinks
 *
 * inputs	- pointer to server to count
 *		- pointers to server and user count
 * output	- NONE
 * side effects - servers are marked
 * 		- server and user counts are added to given values
 */
static void
count_mark_downlinks(struct Client *server_p, int *pservcount, int *pusercount)
{
	rb_dlink_node *ptr;

	SetFloodDone(server_p);
	(*pservcount)++;
	*pusercount += rb_dlink_list_length(&server_p->serv->users);
	RB_DLINK_FOREACH(ptr, server_p->serv->servers.head)
	{
		count_mark_downlinks(ptr->data, pservcount, pusercount);
	}
}

static void
h_nn_server_eob(struct Client *source_p)
{
	int s = 0, u = 0;

	if (IsFloodDone(source_p))
		return;
	count_mark_downlinks(source_p, &s, &u);
	sendto_realops_snomask(SNO_GENERAL, L_ALL, "Netjoin %s <-> %s (%dS %dC)",
			source_p->servptr ? source_p->servptr->name : "?",
			source_p->name, s, u);
}

static void
h_nn_client_exit(hook_data_client_exit *hdata)
{
	struct Client *source_p;
	int s = 0, u = 0;
	char *fromnick;

	source_p = hdata->target;
	fromnick = IsClient(hdata->from) ? hdata->from->name : NULL;

	if (!IsServer(source_p))
		return;
	if (HasSentEob(source_p))
	{
		count_mark_downlinks(source_p, &s, &u);
		sendto_realops_snomask(SNO_GENERAL, L_ALL, "Netsplit %s <-> %s (%dS %dC) (%s%s%s%s)",
				source_p->servptr ? source_p->servptr->name : "?",
				source_p->name, s, u,
				fromnick ? "by " : "",
				fromnick ? fromnick : "",
				fromnick ? ": " : "",
				hdata->comment);
	}
	else
		sendto_realops_snomask(SNO_GENERAL, L_ALL, "Netsplit %s <-> %s (during burst) (%s%s%s%s)",
				source_p->servptr ? source_p->servptr->name : "?",
				source_p->name,
				fromnick ? "by " : "",
				fromnick ? fromnick : "",
				fromnick ? ": " : "",
				hdata->comment);
}
