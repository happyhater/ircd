#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int mo_close(struct Client *, struct Client *, int, const char **);

struct Message close_msgtab = {
	"CLOSE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, mg_ignore, {mo_close, 0}}
};

mapi_clist_av1 close_clist[] = { &close_msgtab, NULL };
DECLARE_MODULE_AV1(close, NULL, NULL, close_clist, NULL, NULL, "$Revision: 254 $");

/*
 * mo_close - CLOSE message handler
 *  - added by Darren Reed Jul 13 1992.
 */
static int
mo_close(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	rb_dlink_node *ptr;
	rb_dlink_node *ptr_next;
	int closed = 0;

	RB_DLINK_FOREACH_SAFE(ptr, ptr_next, unknown_list.head)
	{
		target_p = ptr->data;

		sendto_one(source_p, form_str(RPL_CLOSING), me.name, source_p->name,
			   get_client_name(target_p, SHOW_IP), target_p->status);

		(void) exit_client(target_p, target_p, target_p, "Oper Closing");
		closed++;
	}

	sendto_one(source_p, form_str(RPL_CLOSEEND), me.name, source_p->name, closed);
	return 0;
}
