#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_info(hook_data *);

mapi_hfn_list_av1 info_hfnlist[] = {
	{"doing_info", (hookfn) show_info},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(info_spy, NULL, NULL, NULL, NULL, info_hfnlist, "$Revision: 498 $");

void
show_info(hook_data *data)
{
	sendto_realops_snomask(SNO_SPY, L_ALL,
			     "info requested by %s (%s@%s) [%s]",
			     data->client->name, data->client->username,
			     data->client->host, data->client->servptr->name);
}
