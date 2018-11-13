#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_stats_p(hook_data *);

mapi_hfn_list_av1 stats_p_hfnlist[] = {
	{"doing_stats_p", (hookfn) show_stats_p},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(stats_p_spy, NULL, NULL, NULL, NULL, stats_p_hfnlist, "$Revision: 498 $");

void
show_stats_p(hook_data *data)
{
	sendto_realops_snomask(SNO_SPY, L_ALL,
			     "STATS p requested by %s (%s@%s) [%s]",
			     data->client->name, data->client->username,
			     data->client->host, data->client->servptr->name);
}
