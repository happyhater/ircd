#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "match.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"

static char buf[BUFSIZE];

static int m_userhost(struct Client *, struct Client *, int, const char **);

struct Message userhost_msgtab = {
	"USERHOST", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_userhost, 2}, mg_ignore, mg_ignore, mg_ignore, {m_userhost, 2}}
};

mapi_clist_av1 userhost_clist[] = { &userhost_msgtab, NULL };
DECLARE_MODULE_AV1(userhost, NULL, NULL, userhost_clist, NULL, NULL, "$Revision: 254 $");

/*
 * m_userhost added by Darren Reed 13/8/91 to aid clients and reduce
 * the need for complicated requests like WHOIS. It returns user/host
 * information only (no spurious AWAY labels or channels).
 */
static int
m_userhost(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Client *target_p;
	char response[NICKLEN * 2 + USERLEN + HOSTLEN + 30];
	char *t;
	int i;			/* loop counter */
	int cur_len;
	int rl;

	cur_len = rb_sprintf(buf, form_str(RPL_USERHOST), me.name, source_p->name, "");
	t = buf + cur_len;

	for (i = 1; i <= 5; i++)
	{
		if(parc < i + 1)
			break;

		if((target_p = find_person(parv[i])) != NULL)
		{
			/*
			 * Show real IP for USERHOST on yourself.
			 * This is needed for things like mIRC, which do a server-based
			 * lookup (USERHOST) to figure out what the clients' local IP
			 * is.  Useful for things like NAT, and dynamic dial-up users.
			 */
			if(MyClient(target_p) && (target_p == source_p))
			{
				rl = rb_sprintf(response, "%s%s=%c%s@%s ",
						target_p->name,
						SeesOper(source_p, target_p) ? "*" : "",
						(target_p->user->away) ? '-' : '+',
						target_p->username,
						target_p->sockhost);
			}
			else
			{
				rl = rb_sprintf(response, "%s%s=%c%s@%s ",
						target_p->name,
						SeesOper(source_p, target_p) ? "*" : "",
						(target_p->user->away) ? '-' : '+',
						target_p->username, target_p->host);
			}

			if((rl + cur_len) < (BUFSIZE - 10))
			{
				rb_sprintf(t, "%s", response);
				t += rl;
				cur_len += rl;
			}
			else
				break;
		}
	}

	sendto_one(source_p, "%s", buf);

	return 0;
}
