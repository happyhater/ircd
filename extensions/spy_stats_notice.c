#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_stats(hook_data_int *);

mapi_hfn_list_av1 stats_hfnlist[] = {
	{"doing_stats", (hookfn) show_stats},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(stats_spy, NULL, NULL, NULL, NULL, stats_hfnlist, "$Revision: 498 $");

void
show_stats(hook_data_int *data)
{
	char statchar = (char) data->arg2;

	if(statchar == 'L' || statchar == 'l')
	{
		const char *name = data->arg1;

		if(!EmptyString(name))
			sendto_realops_snomask(SNO_SPY, L_ALL,
					"STATS %c requested by %s (%s@%s) [%s] on %s",
					statchar, data->client->name,
					data->client->username,
					data->client->host,
					data->client->servptr->name, name);
		else
			sendto_realops_snomask(SNO_SPY, L_ALL,
					"STATS %c requested by %s (%s@%s) [%s]",
					statchar, data->client->name,
					data->client->username,
					data->client->host, data->client->servptr->name);
	}
	else
	{
		sendto_realops_snomask(SNO_SPY, L_ALL,
				"STATS %c requested by %s (%s@%s) [%s]",
				statchar, data->client->name, data->client->username,
				data->client->host, data->client->servptr->name);
	}
}
