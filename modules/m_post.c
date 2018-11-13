#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"

static int mr_dumb_proxy(struct Client *, struct Client *, int, const char **);

struct Message post_msgtab = {
	"POST", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{mr_dumb_proxy, 0}, mg_ignore, mg_ignore, mg_ignore, mg_ignore, mg_ignore}
};
struct Message get_msgtab = {
	"GET", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{mr_dumb_proxy, 0}, mg_ignore, mg_ignore, mg_ignore, mg_ignore, mg_ignore}
};
struct Message put_msgtab = {
	"PUT", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{mr_dumb_proxy, 0}, mg_ignore, mg_ignore, mg_ignore, mg_ignore, mg_ignore}
};


mapi_clist_av1 post_clist[] = {
	&post_msgtab, &get_msgtab, &put_msgtab, NULL
};
DECLARE_MODULE_AV1(post, NULL, NULL, post_clist, NULL, NULL, "$Revision: 498 $");


/*
** mr_dumb_proxy
**      parv[1] = comment
*/
static int
mr_dumb_proxy(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	sendto_realops_snomask(SNO_REJ, L_NETWIDE,
			     "HTTP Proxy disconnected: [%s@%s]",
			     client_p->username, client_p->host);
	exit_client(client_p, source_p, source_p, "Client Exit");

	return 0;
}
