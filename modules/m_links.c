#include "stdinc.h"
#include "client.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "hook.h"
#include "scache.h"

static int m_links(struct Client *, struct Client *, int, const char **);
static int mo_links(struct Client *, struct Client *, int, const char **);
static char * clean_string(char *dest, const unsigned char *src, size_t len);

struct Message links_msgtab = {
	"LINKS", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_links, 0}, {mo_links, 0}, mg_ignore, mg_ignore, {mo_links, 0}}
};

int doing_links_hook;

mapi_clist_av1 links_clist[] = { &links_msgtab, NULL };
mapi_hlist_av1 links_hlist[] = {
	{ "doing_links",	&doing_links_hook },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(links, NULL, NULL, links_clist, links_hlist, NULL, "$Revision: 254 $");

/*
 * m_links - LINKS message handler
 *      parv[1] = servername mask
 * or
 *      parv[1] = server to query 
 *      parv[2] = servername mask
 */
static int
m_links(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(ConfigServerHide.flatten_links && !IsExemptShide(source_p))
		scache_send_flattened_links(source_p);
	else
		mo_links(client_p, source_p, parc, parv);

	return 0;
}

static int
mo_links(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	const char *mask = "";
	struct Client *target_p;
	char clean_mask[2 * HOSTLEN + 4];
	hook_data hd;

	rb_dlink_node *ptr;

	if(parc > 2)
	{
		if(strlen(parv[2]) > HOSTLEN)
			return 0;
		if(hunt_server(client_p, source_p, ":%s LINKS %s :%s", 1, parc, parv)
		   != HUNTED_ISME)
			return 0;

		mask = parv[2];
	}
	else if(parc == 2)
		mask = parv[1];

	if(*mask)		/* only necessary if there is a mask */
		mask = collapse(clean_string
				(clean_mask, (const unsigned char *) mask, 2 * HOSTLEN));

	hd.client = source_p;
	hd.arg1 = mask;
	hd.arg2 = NULL;

	call_hook(doing_links_hook, &hd);

	RB_DLINK_FOREACH(ptr, global_serv_list.head)
	{
		target_p = ptr->data;

		if(*mask && !match(mask, target_p->name))
			continue;

		/* We just send the reply, as if theyre here theres either no SHIDE,
		 * or theyre an oper..  
		 */
		sendto_one_numeric(source_p, RPL_LINKS, form_str(RPL_LINKS),
				   target_p->name, target_p->servptr->name,
				   target_p->hopcount,
				   target_p->info[0] ? target_p->info : "(Unknown Location)");
	}

	sendto_one_numeric(source_p, RPL_ENDOFLINKS, form_str(RPL_ENDOFLINKS),
			   EmptyString(mask) ? "*" : mask);

	return 0;
}

static char *
clean_string(char *dest, const unsigned char *src, size_t len)
{
	char *d = dest;
	s_assert(0 != dest);
	s_assert(0 != src);

	if(dest == NULL || src == NULL)
		return NULL;

	while (*src && (len > 1))
	{
		if(*src & 0x80)	/* if high bit is set */
		{
			*d++ = '.';
			--len;
			if(len <= 1)
				break;
		}
		else if(!IsPrint(*src))	/* if NOT printable */
		{
			*d++ = '^';
			--len;
			if(len <= 1)
				break;
			*d++ = 0x40 + *src;	/* turn it into a printable */
		}
		else
			*d++ = *src;
		++src;
		--len;
	}
	*d = '\0';
	return dest;
}
