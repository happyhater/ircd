#include "stdinc.h"
#include "client.h"
#include "common.h"
#include "numeric.h"
#include "ircd.h"
#include "s_conf.h"
#include "supported.h"
#include "chmode.h"

rb_dlink_list isupportlist;

struct isupportitem
{
	const char *name;
	const char *(*func)(const void *);
	const void *param;
	rb_dlink_node node;
};

void
add_isupport(const char *name, const char *(*func)(const void *), const void *param)
{
	struct isupportitem *item;

	item = rb_malloc(sizeof(struct isupportitem));
	item->name = name;
	item->func = func;
	item->param = param;
	rb_dlinkAddTail(item, &item->node, &isupportlist);
}

const void *
change_isupport(const char *name, const char *(*func)(const void *), const void *param)
{
	rb_dlink_node *ptr;
	struct isupportitem *item;
	const void *oldvalue = NULL;

	RB_DLINK_FOREACH(ptr, isupportlist.head)
	{
		item = ptr->data;

		if (!strcmp(item->name, name))
		{
			oldvalue = item->param;

			// item->name = name;
			item->func = func;
			item->param = param;

			break;
		}
	}

	return oldvalue;
}

void
delete_isupport(const char *name)
{
	rb_dlink_node *ptr, *next_ptr;
	struct isupportitem *item;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, isupportlist.head)
	{
		item = ptr->data;

		if (!strcmp(item->name, name))
		{
			rb_dlinkDelete(ptr, &isupportlist);
			rb_free(item);
		}
	}
}

/* XXX caching? */
void
show_isupport(struct Client *client_p)
{
	rb_dlink_node *ptr;
	struct isupportitem *item;
	const char *value;
	char buf[512];
	int extra_space;
	unsigned int nchars, nparams;
	int l;

	extra_space = strlen(client_p->name);
	/* UID */
	if (!MyClient(client_p) && extra_space < 9)
		extra_space = 9;
	/* :<me.name> 005 <nick> <params> :are supported by this server */
	/* form_str(RPL_ISUPPORT) is %s :are supported by this server */
	extra_space += strlen(me.name) + 1 + strlen(form_str(RPL_ISUPPORT));

	nchars = extra_space, nparams = 0, buf[0] = '\0';
	RB_DLINK_FOREACH(ptr, isupportlist.head)
	{
		item = ptr->data;
		value = (*item->func)(item->param);
		if (value == NULL)
			continue;
		l = strlen(item->name) + (EmptyString(value) ? 0 : 1 + strlen(value));
		if (nchars + l + (nparams > 0) >= sizeof buf || nparams + 1 > 12)
		{
			sendto_one_numeric(client_p, RPL_ISUPPORT, form_str(RPL_ISUPPORT), buf);
			nchars = extra_space, nparams = 0, buf[0] = '\0';
		}
		if (nparams > 0)
			rb_strlcat(buf, " ", sizeof buf), nchars++;
		rb_strlcat(buf, item->name, sizeof buf);
		if (!EmptyString(value))
		{
			rb_strlcat(buf, "=", sizeof buf);
			rb_strlcat(buf, value, sizeof buf);
		}
		nchars += l;
		nparams++;
	}
	if (nparams > 0)
		sendto_one_numeric(client_p, RPL_ISUPPORT, form_str(RPL_ISUPPORT), buf);
}

const char *
isupport_intptr(const void *ptr)
{
	static char buf[15];
	rb_snprintf(buf, sizeof buf, "%d", *(const int *)ptr);
	return buf;
}

const char *
isupport_boolean(const void *ptr)
{

	return *(const int *)ptr ? "" : NULL;
}

const char *
isupport_string(const void *ptr)
{

	return (const char *)ptr;
}

const char *
isupport_stringptr(const void *ptr)
{
	return *(char * const *)ptr;	
}

static const char *
isupport_chanmodes(const void *ptr)
{
	static char result[80];

	rb_snprintf(result, sizeof result, "%s%sbq,k,flj,%s",
			ConfigChannel.use_except ? "e" : "",
			ConfigChannel.use_invex ? "I" : "",
			cflagsbuf);
	return result;
}

static const char *
isupport_chantypes(const void *ptr)
{
	return ConfigChannel.disable_local_channels ? "#" : "&#";
}

static const char *
isupport_chanlimit(const void *ptr)
{
	static char result[30];

	rb_snprintf(result, sizeof result, "%s:%i",
		ConfigChannel.disable_local_channels ? "#" : "&#", ConfigChannel.max_chans_per_user);
	return result;
}

static const char *
isupport_maxlist(const void *ptr)
{
	static char result[30];

	rb_snprintf(result, sizeof result, "bq%s%s:%i",
			ConfigChannel.use_except ? "e" : "",
			ConfigChannel.use_invex ? "I" : "",
			ConfigChannel.max_bans);
	return result;
}

static const char *
isupport_targmax(const void *ptr)
{
	static char result[200];

	rb_snprintf(result, sizeof result, "NAMES:1,LIST:1,KICK:1,WHOIS:1,PRIVMSG:%d,NOTICE:%d,ACCEPT:,MONITOR:",
			ConfigFileEntry.max_targets,
			ConfigFileEntry.max_targets);
	return result;
}

static const char *
isupport_extban(const void *ptr)
{
	const char *p;
	static char result[200];

	p = get_extban_string();
	if (EmptyString(p))
		return NULL;
	rb_snprintf(result, sizeof result, "$,%s", p);
	return result;
}

void
init_isupport(void)
{
	static int maxmodes = MAXMODEPARAMS;
	static int nicklen = NICKLEN-1;
	static int channellen = LOC_CHANNELLEN;
	static int topiclen = TOPICLEN;

	/* only non-module features belong here */

	add_isupport("CHANTYPES", isupport_chantypes, NULL);
	add_isupport("EXCEPTS", isupport_boolean, &ConfigChannel.use_except);
	add_isupport("INVEX", isupport_boolean, &ConfigChannel.use_invex);
	add_isupport("CHANMODES", isupport_chanmodes, NULL);
	add_isupport("CHANLIMIT", isupport_chanlimit, NULL);
	add_isupport("PREFIX", isupport_string, "(ov)@+");
	add_isupport("MAXLIST", isupport_maxlist, NULL);
	add_isupport("MODES", isupport_intptr, &maxmodes);
	add_isupport("NETWORK", isupport_stringptr, &ServerInfo.network_name);
	add_isupport("STATUSMSG", isupport_string, "@+");
	add_isupport("CALLERID", isupport_string, "g");
	add_isupport("CASEMAPPING", isupport_string, "rfc1459");
	add_isupport("CHARSET", isupport_string, "ascii");
	add_isupport("NICKLEN", isupport_intptr, &nicklen);
	add_isupport("CHANNELLEN", isupport_intptr, &channellen);
	add_isupport("TOPICLEN", isupport_intptr, &topiclen);
	add_isupport("DEAF", isupport_string, "D");
	add_isupport("FNC", isupport_string, "");
	add_isupport("TARGMAX", isupport_targmax, NULL);
	add_isupport("EXTBAN", isupport_extban, NULL);
	add_isupport("CLIENTVER", isupport_string, "3.0");
}
