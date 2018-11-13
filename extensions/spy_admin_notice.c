#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

void show_admin(hook_data *);

mapi_hfn_list_av1 admin_hfnlist[] = {
	{"doing_admin", (hookfn) show_admin},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(admin_spy, NULL, NULL, NULL, NULL, admin_hfnlist, "$Revision: 498 $");

void
show_admin(hook_data *data)
{
	sendto_realops_snomask(SNO_SPY, L_ALL,
			     "admin requested by %s (%s@%s) [%s]",
			     data->client->name, data->client->username,
			     data->client->host, data->client->servptr->name);
}
