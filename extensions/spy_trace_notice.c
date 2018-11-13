#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_trace(hook_data_client *);

mapi_hfn_list_av1 trace_hfnlist[] = {
	{"doing_trace", (hookfn) show_trace},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(trace_spy, NULL, NULL, NULL, NULL, trace_hfnlist, "$Revision: 498 $");

void
show_trace(hook_data_client *data)
{
	if(data->target)
		sendto_realops_snomask(SNO_SPY, L_ALL,
				"trace requested by %s (%s@%s) [%s] on %s",
				data->client->name, data->client->username,
				data->client->host, data->client->servptr->name,
				data->target->name);
	else
		sendto_realops_snomask(SNO_SPY, L_ALL,
				"trace requested by %s (%s@%s) [%s]",
				data->client->name, data->client->username,
				data->client->host, data->client->servptr->name);
}
