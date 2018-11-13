#include "stdinc.h"
#include "client.h"
#include "monitor.h"
#include "hash.h"
#include "numeric.h"

static rb_dlink_list monitorTable[MONITOR_HASH_SIZE];
static rb_bh *monitor_heap;

void
init_monitor(void)
{
	monitor_heap = rb_bh_create(sizeof(struct monitor), MONITOR_HEAP_SIZE, "monitor_heap");
}

static inline unsigned int
hash_monitor_nick(const char *name)
{
	return fnv_hash_upper((const unsigned char *)name, MONITOR_HASH_BITS);
}

struct monitor *
find_monitor(const char *name, int add)
{
	struct monitor *monptr;
	rb_dlink_node *ptr;

	unsigned int hashv = hash_monitor_nick(name);

	RB_DLINK_FOREACH(ptr, monitorTable[hashv].head)
	{
		monptr = ptr->data;
		if(!irccmp(monptr->name, name))
			return monptr;
	
	}

	if(add)
	{
		monptr = rb_bh_alloc(monitor_heap);
		rb_strlcpy(monptr->name, name, sizeof(monptr->name));
		monptr->hashv = hashv;

		rb_dlinkAdd(monptr, &monptr->node, &monitorTable[hashv]);
		return monptr;
	}

	return NULL;
}

void
free_monitor(struct monitor *monptr)
{
	if (rb_dlink_list_length(&monptr->users) > 0)
		return;

	rb_dlinkDelete(&monptr->node, &monitorTable[monptr->hashv]);
	rb_bh_free(monitor_heap, monptr);
}

/* monitor_signon()
 *
 * inputs	- client who has just connected
 * outputs	-
 * side effects	- notifies any clients monitoring this nickname that it has
 * 		  connected to the network
 */
void
monitor_signon(struct Client *client_p)
{
	char buf[USERHOST_REPLYLEN];
	struct monitor *monptr = find_monitor(client_p->name, 0);

	/* noones watching this nick */
	if(monptr == NULL)
		return;

	rb_snprintf(buf, sizeof(buf), "%s!%s@%s", client_p->name, client_p->username, client_p->host);

	sendto_monitor(monptr, form_str(RPL_MONONLINE), me.name, "*", buf);
}

/* monitor_signoff()
 *
 * inputs	- client who is exiting
 * outputs	-
 * side effects	- notifies any clients monitoring this nickname that it has
 * 		  left the network
 */
void
monitor_signoff(struct Client *client_p)
{
	struct monitor *monptr = find_monitor(client_p->name, 0);

	/* noones watching this nick */
	if(monptr == NULL)
		return;

	sendto_monitor(monptr, form_str(RPL_MONOFFLINE), me.name, "*",
			client_p->name);
}

void
clear_monitor(struct Client *client_p)
{
	struct monitor *monptr;
	rb_dlink_node *ptr, *next_ptr;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, client_p->localClient->monitor_list.head)
	{
		monptr = ptr->data;

		rb_dlinkFindDestroy(client_p, &monptr->users);
		rb_free_rb_dlink_node(ptr);

		free_monitor(ptr->data);
	}

	client_p->localClient->monitor_list.head = client_p->localClient->monitor_list.tail = NULL;
	client_p->localClient->monitor_list.length = 0;
}
