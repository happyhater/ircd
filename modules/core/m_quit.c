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
#include "inline/stringops.h"

static int m_quit(struct Client *, struct Client *, int, const char **);
static int ms_quit(struct Client *, struct Client *, int, const char **);

struct Message quit_msgtab = {
	"QUIT", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{m_quit, 0}, {m_quit, 0}, {ms_quit, 0}, mg_ignore, mg_ignore, {m_quit, 0}}
};

mapi_clist_av1 quit_clist[] = { &quit_msgtab, NULL };

DECLARE_MODULE_AV1(quit, NULL, NULL, quit_clist, NULL, NULL, "$Revision: 1333 $");

/*
** m_quit
**      parv[1] = comment
*/
static int
m_quit(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	char *comment = LOCAL_COPY((parc > 1 && parv[1]) ? parv[1] : client_p->name);
	char reason[REASONLEN + 1];

	source_p->flags |= FLAGS_NORMALEX;

	if(strlen(comment) > (size_t) REASONLEN)
		comment[REASONLEN] = '\0';

	strip_colour(comment);

	if(ConfigFileEntry.client_exit && comment[0])
	{
		rb_snprintf(reason, sizeof(reason), "Quit: %s", comment);
		comment = reason;
	}

	if(!IsOper(source_p) &&
	   (source_p->localClient->firsttime + ConfigFileEntry.anti_spam_exit_message_time) >
	   rb_current_time())
	{
		exit_client(client_p, source_p, source_p, "Client Quit");
		return 0;
	}

	exit_client(client_p, source_p, source_p, comment);

	return 0;
}

/*
** ms_quit
**      parv[1] = comment
*/
static int
ms_quit(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	char *comment = LOCAL_COPY((parc > 1 && parv[1]) ? parv[1] : client_p->name);

	source_p->flags |= FLAGS_NORMALEX;
	if(strlen(comment) > (size_t) REASONLEN)
		comment[REASONLEN] = '\0';

	exit_client(client_p, source_p, source_p, comment);

	return 0;
}
