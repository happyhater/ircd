#include "stdinc.h"
#include "channel.h"
#include "client.h"
#include "hash.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "s_serv.h"
#include "supported.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static rb_dlink_list safelisting_clients = { NULL, NULL, 0 };

static int _modinit(void);
static void _moddeinit(void);

static int m_list(struct Client *, struct Client *, int, const char **);
static int mo_list(struct Client *, struct Client *, int, const char **);

static void safelist_check_cliexit(hook_data_client_exit * hdata);
static void safelist_client_instantiate(struct Client *, struct ListClient *);
static void safelist_client_release(struct Client *);
static void safelist_one_channel(struct Client *source_p, struct Channel *chptr);
static void safelist_iterate_client(struct Client *source_p);
static void safelist_iterate_clients(void *unused);
static void safelist_channel_named(struct Client *source_p, const char *name);

struct Message list_msgtab = {
	"LIST", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_list, 0}, mg_ignore, mg_ignore, mg_ignore, {mo_list, 0}}
};

mapi_clist_av1 list_clist[] = { &list_msgtab, NULL };

mapi_hfn_list_av1 list_hfnlist[] = {
	{"client_exit", (hookfn) safelist_check_cliexit},
	{NULL, NULL}
};

DECLARE_MODULE_AV1(list, _modinit, _moddeinit, list_clist, NULL, list_hfnlist, "$Revision: 3372 $");

static struct ev_entry *iterate_clients_ev = NULL;

static int _modinit(void)
{
	iterate_clients_ev = rb_event_add("safelist_iterate_clients", safelist_iterate_clients, NULL, 3);

	/* ELIST=[tokens]:
	 *
	 * M = mask search
	 * N = !mask search
	 * U = user count search (< >)
	 * C = creation time search (C> C<)
	 * T = topic search (T> T<)
	 */
	add_isupport("SAFELIST", isupport_string, "");
	add_isupport("ELIST", isupport_string, "CTU");

	return 0;
}

static void _moddeinit(void)
{
	rb_event_delete(iterate_clients_ev);

	delete_isupport("SAFELIST");
	delete_isupport("ELIST");
}

static void safelist_check_cliexit(hook_data_client_exit * hdata)
{
	/* Cancel the safelist request if we are disconnecting
	 * from the server. That way it doesn't core. :P --nenolod
	 */
	if (MyClient(hdata->target) && hdata->target->localClient->safelist_data != NULL)
	{
		safelist_client_release(hdata->target);
	}
}

/* m_list()
 *      parv[1] = channel
 *
 * XXX - With SAFELIST, do we really need to continue pacing?
 *       In theory, the server cannot be lagged by this. --nenolod
 */
static int m_list(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0L;

	if (source_p->localClient->safelist_data != NULL)
	{
		sendto_one_notice(source_p, ":/LIST aborted");
		safelist_client_release(source_p);
		return 0;
	}

	if (parc < 2 || !IsChannelName(parv[1]))
	{
		/* pace this due to the sheer traffic involved */
		if (((last_used + ConfigFileEntry.pace_wait) > rb_current_time()))
		{
			sendto_one(source_p, form_str(RPL_LOAD2HI), me.name, source_p->name, "LIST");
			sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
			return 0;
		}
		else
			last_used = rb_current_time();
	}

	return mo_list(client_p, source_p, parc, parv);
}

/* mo_list()
 *      parv[1] = channel
 */
static int mo_list(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct ListClient *params;
	char *p, *args;
	int i, parse_failed = 0;

	if (source_p->localClient->safelist_data != NULL)
	{
		sendto_one_notice(source_p, ":/LIST aborted");
		safelist_client_release(source_p);
		return 0;
	}

	/* Single channel. */
	if (parc > 1 && IsChannelName(parv[1]))
	{
		safelist_channel_named(source_p, parv[1]);
		return 0;
	}

	/* Multiple channels, possibly with parameters. */
	params = rb_malloc(sizeof(struct ListClient));

	/* XXX rather arbitrary -- jilles */
	params->users_min = 3;
	params->users_max = INT_MAX;
	params->operspy = 0;
	params->created_min = params->topic_min = 
		params->created_max = params->topic_max = 0;

	if (parc > 1 && !EmptyString(parv[1]))
	{
		args = LOCAL_COPY(parv[1]);

		/* Cancel out default minimum. */
		params->users_min = 0;

		for (i = 0; i < 7; i++)
		{
			if ((p = strchr(args, ',')) != NULL)
				*p++ = '\0';

			if (*args == '<')
			{
				args++;
				if (IsDigit(*args))
				{
					params->users_max = atoi(args);
					if (params->users_max == 0)
						params->users_max = INT_MAX;
					else
						params->users_max--;
				}
				else
				{
					parse_failed = 1;
					break;
				}
			}
			else if (*args == '>')
			{
				args++;
				if (IsDigit(*args))
				{
					params->users_min = atoi(args) + 1;
				}
				else
				{
					parse_failed = 1;
					break;
				}
			}
			else if (*args == 'C' || *args == 'c')
			{
				args++;
				if (*args == '>')
				{
					/* Creation time earlier than last x minutes. */
					args++;
					if (IsDigit(*args))
					{
						params->created_max = rb_current_time() - (60 * atoi(args));
					}
					else
					{
						parse_failed = 1;
						break;
					}
				}
				else if (*args == '<')
				{
					/* Creation time within last x minutes. */
					args++;
					if (IsDigit(*args))
					{
						params->created_min = rb_current_time() - (60 * atoi(args));
					}
					else
					{
						parse_failed = 1;
						break;
					}
				}
				else
				{
					parse_failed = 1;
					break;
				}
			}
			else if (*args == 'T' || *args == 't')
			{
				args++;
				if (*args == '>')
				{
					/* Topic change time earlier than last x minutes. */
					args++;
					if (IsDigit(*args))
					{
						params->topic_max = rb_current_time() - (60 * atoi(args));
					}
					else
					{
						parse_failed = 1;
						break;
					}
				}
				else if (*args == '<')
				{
					/* Topic change time within last x minutes. */
					args++;
					if (IsDigit(*args))
					{
						params->topic_min = rb_current_time() - (60 * atoi(args));
					}
					else
					{
						parse_failed = 1;
						break;
					}
				}
				else
				{
					parse_failed = 1;
					break;
				}
			}
			/* Only accept operspy as the first option. */
			else if (*args == '!' && IsOperAuspex(source_p) && i == 0)
			{
				params->operspy = 1;
				report_operspy(source_p, "LIST", p);
			}
			else
			{
				parse_failed = 1;
				break;
			}

			if (EmptyString(p))
				break;
			else
				args = p;
		}
	}

	if (parse_failed)
	{
		sendto_one(source_p, form_str(RPL_LISTSTART), me.name, source_p->name);
		sendto_one_notice(source_p, ":Invalid parameters for /LIST");
		sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
		rb_free(params);
	}
	else
	{
		safelist_client_instantiate(source_p, params);
	}

	return 0;
}

/*
 * safelist_sendq_exceeded()
 *
 * inputs       - pointer to client that needs checking
 * outputs      - 1 if a client has exceeded the reserved
 *                sendq limit, 0 if not
 * side effects - none
 *
 * When safelisting, we only use half of the SendQ at any
 * given time.
 */
static int safelist_sendq_exceeded(struct Client *client_p)
{
	if (rb_linebuf_len(&client_p->localClient->buf_sendq) > (get_sendq(client_p) / 2))
		return YES;
	else
		return NO;
}

/*
 * safelist_client_instantiate()
 *
 * inputs       - pointer to Client to be listed,
 *                pointer to ListClient for params
 * outputs      - none
 * side effects - the safelist process begins for a
 *                client.
 *
 * Please do not ever call this on a non-local client.
 * If you do, you will get SIGSEGV.
 */
static void safelist_client_instantiate(struct Client *client_p, struct ListClient *params)
{
	s_assert(MyClient(client_p));
	s_assert(params != NULL);

	client_p->localClient->safelist_data = params;

	sendto_one(client_p, form_str(RPL_LISTSTART), me.name, client_p->name);

	/* pop the client onto the queue for processing */
	rb_dlinkAddAlloc(client_p, &safelisting_clients);

	/* give the user some initial data to work with */
	safelist_iterate_client(client_p);
}

/*
 * safelist_client_release()
 *
 * inputs       - pointer to Client being listed on
 * outputs      - none
 * side effects - the client is no longer being
 *                listed
 *
 * Please do not ever call this on a non-local client.
 * If you do, you will get SIGSEGV.
 */
static void safelist_client_release(struct Client *client_p)
{
	s_assert(MyClient(client_p));

	rb_dlinkFindDestroy(client_p, &safelisting_clients);

	rb_free(client_p->localClient->safelist_data);

	client_p->localClient->safelist_data = NULL;

	sendto_one(client_p, form_str(RPL_LISTEND), me.name, client_p->name);
}

/*
 * safelist_channel_named()
 *
 * inputs       - client pointer, channel name
 * outputs      - none
 * side effects - a named channel is listed
 */
static void safelist_channel_named(struct Client *source_p, const char *name)
{
	struct Channel *chptr;
	char *p;
	char *n = LOCAL_COPY(name);

	sendto_one(source_p, form_str(RPL_LISTSTART), me.name, source_p->name);

	if ((p = strchr(n, ',')))
		*p = '\0';

	if (*n == '\0')
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), name);
		sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
		return;
	}

	chptr = find_channel(n);

	if (chptr == NULL)
	{
		sendto_one_numeric(source_p, ERR_NOSUCHNICK, form_str(ERR_NOSUCHNICK), n);
		sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
		return;
	}

	if (!SecretChannel(chptr) || IsMember(source_p, chptr))
		sendto_one(source_p, form_str(RPL_LIST), me.name, source_p->name, "",
			   chptr->chname, rb_dlink_list_length(&chptr->members),
			   chptr->topic == NULL ? "" : chptr->topic);

	sendto_one(source_p, form_str(RPL_LISTEND), me.name, source_p->name);
	return;
}

/*
 * safelist_one_channel()
 *
 * inputs       - client pointer and channel pointer
 * outputs      - none
 * side effects - a channel is listed if it meets the
 *                requirements
 */
static void safelist_one_channel(struct Client *source_p, struct Channel *chptr)
{
	struct ListClient *safelist_data = source_p->localClient->safelist_data;

	if (SecretChannel(chptr) && !IsMember(source_p, chptr) && !safelist_data->operspy)
		return;

	if ((unsigned int)chptr->members.length < safelist_data->users_min
	    || (unsigned int)chptr->members.length > safelist_data->users_max)
		return;

	if (safelist_data->topic_min && chptr->topic_time < safelist_data->topic_min)
		return;

	/* If a topic TS is provided, don't show channels without a topic set. */
	if (safelist_data->topic_max && (chptr->topic_time > safelist_data->topic_max
		|| chptr->topic_time == 0))
		return;

	if (safelist_data->created_min && chptr->channelts < safelist_data->created_min)
		return;

	if (safelist_data->created_max && chptr->channelts > safelist_data->created_max)
		return;

	sendto_one(source_p, form_str(RPL_LIST), me.name, source_p->name,
		   (safelist_data->operspy && SecretChannel(chptr)) ? "!" : "",
		   chptr->chname, rb_dlink_list_length(&chptr->members),
		   chptr->topic == NULL ? "" : chptr->topic);
}

/*
 * safelist_iterate_client()
 *
 * inputs       - client pointer
 * outputs      - none
 * side effects - the client's sendq is filled up again
 */
static void safelist_iterate_client(struct Client *source_p)
{
	rb_dlink_node *ptr;
	int iter;

	for (iter = source_p->localClient->safelist_data->hash_indice; iter < CH_MAX; iter++)
	{
		if (safelist_sendq_exceeded(source_p->from) == YES)
		{
			source_p->localClient->safelist_data->hash_indice = iter;
			return;
		}

		RB_DLINK_FOREACH(ptr, channelTable[iter].head) 
			safelist_one_channel(source_p, (struct Channel *) ptr->data);
	}

	safelist_client_release(source_p);
}

static void safelist_iterate_clients(void *unused)
{
	rb_dlink_node *n, *n2;

	RB_DLINK_FOREACH_SAFE(n, n2, safelisting_clients.head) 
		safelist_iterate_client((struct Client *)n->data);
}
