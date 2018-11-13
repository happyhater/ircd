#include "stdinc.h"
#include "client.h"
#include "common.h"		/* FALSE */
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "msg.h"
#include "modules.h"
#include "logger.h"
#include "s_conf.h"

static int m_error(struct Client *, struct Client *, int, const char **);
static int ms_error(struct Client *, struct Client *, int, const char **);

struct Message error_msgtab = {
	"ERROR", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{m_error, 0}, mg_ignore, mg_ignore, {ms_error, 0}, mg_ignore, mg_ignore}
};

mapi_clist_av1 error_clist[] = {
	&error_msgtab, NULL
};

DECLARE_MODULE_AV1(error, NULL, NULL, error_clist, NULL, NULL, "$Revision: 494 $");

/* Determine whether an ERROR message is safe to show (no IP address in it) */
static int
is_safe_error(const char *message)
{
	char prefix2[100];
	const char *p;

	if (!strncmp(message, "Closing Link: 127.0.0.1 (", 25))
		return 1;
	rb_snprintf(prefix2, sizeof prefix2,
			"Closing Link: 127.0.0.1 %s (", me.name);
	if (!strncmp(message, prefix2, strlen(prefix2)))
		return 1;
	if (!strncmp(message, "Restart by ", 11))
		return 1;
	if (!strncmp(message, "Terminated by ", 14))
		return 1;

	if (!ircncmp(message, "Closing Link", 12))
		return 0;
	if (strchr(message, '['))
		return 0;
	p = strchr(message, '.');
	if (p != NULL && p[1] != '\0')
		return 0;
	if (strchr(message, ':'))
		return 0;

	return 1;
}

/*
 * Note: At least at protocol level ERROR has only one parameter,
 * although this is called internally from other functions
 * --msa
 *
 *      parv[*] = parameters
 */
int
m_error(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	const char *para;
	int hideit = ConfigFileEntry.hide_error_messages;

	para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";

	if (IsAnyServer(client_p))
	{
		ilog(L_SERVER, "Received ERROR message from %s: %s",
		     log_client_name(source_p, SHOW_IP), para);
	}

	if(is_safe_error(para))
		hideit = 0;
	if(IsAnyServer(client_p))
	{
		if (hideit < 2)
			sendto_realops_snomask(SNO_GENERAL, (hideit ? L_ADMIN : L_ALL) | L_NETWIDE,
					"ERROR :from %s -- %s",
					client_p->name, para);
		if (hideit > 0)
			sendto_realops_snomask(SNO_GENERAL, (hideit == 1 ? L_OPER : L_ALL) | L_NETWIDE,
					"ERROR :from %s -- <hidden>",
					client_p->name);
	}

	exit_client(client_p, source_p, source_p, "ERROR");

	return 0;
}

static int
ms_error(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	const char *para;
	int hideit = ConfigFileEntry.hide_error_messages;

	para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";

	ilog(L_SERVER, "Received ERROR message from %s: %s",
	     log_client_name(source_p, SHOW_IP), para);

	if(is_safe_error(para))
		hideit = 0;
	if(hideit == 2)
		return 0;

	if(client_p == source_p)
	{
		sendto_realops_snomask(SNO_GENERAL, hideit ? L_ADMIN : L_ALL, "ERROR :from %s -- %s",
				     client_p->name, para);
	}
	else
	{
		sendto_realops_snomask(SNO_GENERAL, hideit ? L_ADMIN : L_ALL, "ERROR :from %s via %s -- %s",
				     source_p->name, client_p->name, para);
	}

	return 0;
}
