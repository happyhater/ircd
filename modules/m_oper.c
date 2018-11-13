#include "stdinc.h"
#include "client.h"
#include "common.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "logger.h"
#include "s_user.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"
#include "cache.h"

static int m_oper(struct Client *, struct Client *, int, const char **);
static int me_oper(struct Client *, struct Client *, int, const char **);

struct Message oper_msgtab = {
	"OPER", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_oper, 3}, mg_ignore, mg_ignore, {me_oper, 1}, {m_oper, 3}}
};

mapi_clist_av1 oper_clist[] = { &oper_msgtab, NULL };
DECLARE_MODULE_AV1(oper, NULL, NULL, oper_clist, NULL, NULL, "$Revision: 1483 $");

static int match_oper_password(const char *password, struct oper_conf *oper_p);

/*
 * m_oper
 *      parv[1] = oper name
 *      parv[2] = oper password
 */
static int
m_oper(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct oper_conf *oper_p;
	const char *name;
	const char *password;

	name = parv[1];
	password = parv[2];

	if(IsAnyOper(source_p))
	{
		sendto_one(source_p, form_str(RPL_YOUREOPER), me.name, source_p->name);
		send_oper_motd(source_p);
		return 0;
	}

	/* end the grace period */
	if(!IsFloodDone(source_p))
		flood_endgrace(source_p);

	oper_p = find_oper_conf(source_p->username, source_p->orighost, 
				source_p->sockhost, name);

	if(oper_p == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOOPERHOST, form_str(ERR_NOOPERHOST));
		ilog(L_FOPER, "FAILED OPER (%s) by (%s!%s@%s) (%s)",
		     name, source_p->name,
		     source_p->username, source_p->host, source_p->sockhost);

		if(ConfigFileEntry.failed_oper_notice)
		{
			sendto_realops_snomask(SNO_GENERAL, L_NETWIDE,
					     "Failed OPER attempt - host mismatch by %s (%s@%s)",
					     source_p->name, source_p->username, source_p->host);
		}

		return 0;
	}

	if(IsOperConfNeedSSL(oper_p) && !IsSSLClient(source_p))
	{
		sendto_one_numeric(source_p, ERR_NOOPERHOST, form_str(ERR_NOOPERHOST));
		ilog(L_FOPER, "FAILED OPER (%s) by (%s!%s@%s) (%s) -- requires SSL/TLS",
		     name, source_p->name,
		     source_p->username, source_p->host, source_p->sockhost);

		if(ConfigFileEntry.failed_oper_notice)
		{
			sendto_realops_snomask(SNO_GENERAL, L_ALL,
					     "Failed OPER attempt - missing SSL/TLS by %s (%s@%s)",
					     source_p->name, source_p->username, source_p->host);
		}
		return 0;
	}

	if (oper_p->certfp != NULL)
	{
		if (source_p->certfp == NULL || strcasecmp(source_p->certfp, oper_p->certfp))
		{
			sendto_one_numeric(source_p, ERR_NOOPERHOST, form_str(ERR_NOOPERHOST));
			ilog(L_FOPER, "FAILED OPER (%s) by (%s!%s@%s) (%s) -- client certificate fingerprint mismatch",
			     name, source_p->name,
			     source_p->username, source_p->host, source_p->sockhost);

			if(ConfigFileEntry.failed_oper_notice)
			{
				sendto_realops_snomask(SNO_GENERAL, L_ALL,
						     "Failed OPER attempt - client certificate fingerprint mismatch by %s (%s@%s)",
						     source_p->name, source_p->username, source_p->host);
			}
			return 0;
		}
	}

	if(match_oper_password(password, oper_p))
	{
		oper_up(source_p, oper_p);

		ilog(L_OPERED, "OPER %s by %s!%s@%s (%s)",
		     name, source_p->name, source_p->username, source_p->host,
		     source_p->sockhost);
		return 0;
	}
	else
	{
		sendto_one(source_p, form_str(ERR_PASSWDMISMATCH),
			   me.name, source_p->name);

		ilog(L_FOPER, "FAILED OPER (%s) by (%s!%s@%s) (%s)",
		     name, source_p->name, source_p->username, source_p->host,
		     source_p->sockhost);

		if(ConfigFileEntry.failed_oper_notice)
		{
			sendto_realops_snomask(SNO_GENERAL, L_NETWIDE,
					     "Failed OPER attempt by %s (%s@%s)",
					     source_p->name, source_p->username, source_p->host);
		}
	}

	return 0;
}

static int
me_oper(struct Client *client_p, struct Client *source_p, int parc, const char **parv)
{
	if(!IsOper(source_p))
		return 0;
	if(IsClient(source_p))
		source_p->user->opername = rb_strdup(parv[1]);

	return 0;
}

/*
 * match_oper_password
 *
 * inputs       - pointer to given password
 *              - pointer to Conf 
 * output       - YES or NO if match
 * side effects - none
 */
static int
match_oper_password(const char *password, struct oper_conf *oper_p)
{
	const char *encr;

	/* passwd may be NULL pointer. Head it off at the pass... */
	if(EmptyString(oper_p->passwd))
		return NO;

	if(IsOperConfEncrypted(oper_p))
	{
		/* use first two chars of the password they send in as salt */
		/* If the password in the conf is MD5, and ircd is linked   
		 * to scrypt on FreeBSD, or the standard crypt library on
		 * glibc Linux, then this code will work fine on generating
		 * the proper encrypted hash for comparison.
		 */
		if(!EmptyString(password))
			encr = rb_crypt(password, oper_p->passwd);
		else
			encr = "";
	}
	else
		encr = password;

	if(strcmp(encr, oper_p->passwd) == 0)
		return YES;
	else
		return NO;
}
