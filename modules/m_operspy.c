#include "stdinc.h"
#include "send.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "config.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "hash.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static int ms_operspy(struct Client *client_p, struct Client *source_p,
		      int parc, const char *parv[]);

struct Message operspy_msgtab = {
	"OPERSPY", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_ignore, mg_ignore, mg_ignore, {ms_operspy, 2}, mg_ignore}
};

mapi_clist_av1 operspy_clist[] = { &operspy_msgtab, NULL };
DECLARE_MODULE_AV1(operspy, NULL, NULL, operspy_clist, NULL, NULL, "$Revision: 254 $");

/* ms_operspy()
 *
 * parv[1] - operspy command
 * parv[2] - optional params
 */
static int
ms_operspy(struct Client *client_p, struct Client *source_p,
	   int parc, const char *parv[])
{
	static char buffer[BUFSIZE];
	char *ptr;
	int cur_len = 0;
	int len, i;

	if(parc < 4)
	{
		report_operspy(source_p, parv[1],
			    parc < 3 ? NULL : parv[2]);
	}
	/* buffer all remaining into one param */
	else
	{
		ptr = buffer;
		cur_len = 0;

		for(i = 2; i < parc; i++)
		{
			len = strlen(parv[i]) + 1;

			if((size_t)(cur_len + len) >= sizeof(buffer))
				return 0;

			rb_snprintf(ptr, sizeof(buffer) - cur_len, "%s ",
				 parv[i]);
			ptr += len;
			cur_len += len;
		}

		report_operspy(source_p, parv[1], buffer);
	}

	return 0;
}

