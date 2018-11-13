#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_links(hook_data *);

mapi_hfn_list_av1 links_hfnlist[] = {
	{"doing_links", (hookfn) show_links},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(links_spy, NULL, NULL, NULL, NULL, links_hfnlist, "$Revision: 498 $");

void
show_links(hook_data *data)
{
	const char *mask = data->arg1;

	sendto_realops_snomask(SNO_SPY, L_ALL,
			     "LINKS '%s' requested by %s (%s@%s) [%s]",
			     mask, data->client->name, data->client->username,
			     data->client->host, data->client->servptr->name);
}
