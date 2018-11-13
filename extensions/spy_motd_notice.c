#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_motd(hook_data *);

mapi_hfn_list_av1 motd_hfnlist[] = {
	{"doing_motd", (hookfn) show_motd},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(motd_spy, NULL, NULL, NULL, NULL, motd_hfnlist, "$Revision: 498 $");

void
show_motd(hook_data *data)
{
	sendto_realops_snomask(SNO_SPY, L_ALL,
			     "motd requested by %s (%s@%s) [%s]",
			     data->client->name, data->client->username,
			     data->client->host, data->client->servptr->name);
}
