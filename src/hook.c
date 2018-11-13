#include "stdinc.h"
#include "hook.h"
#include "match.h"

hook *hooks;

#define HOOK_INCREMENT 1000

int num_hooks = 0;
int last_hook = 0;
int max_hooks = HOOK_INCREMENT;

#ifdef USE_IODEBUG_HOOKS
int h_iosend_id;
int h_iorecv_id;
int h_iorecvctrl_id;
#endif
int h_burst_client;
int h_burst_channel;
int h_burst_finished;
int h_server_introduced;
int h_server_eob;
int h_client_exit;
int h_umode_changed;
int h_new_local_user;
int h_new_remote_user;
int h_introduce_client;
int h_can_kick;
int h_privmsg_user;
int h_privmsg_channel;

void
init_hook(void)
{
	hooks = rb_malloc(sizeof(hook) * HOOK_INCREMENT);

#ifdef USE_IODEBUG_HOOKS
	h_iosend_id = register_hook("iosend");
	h_iorecv_id = register_hook("iorecv");
	h_iorecvctrl_id = register_hook("iorecvctrl");
#endif

	h_burst_client = register_hook("burst_client");
	h_burst_channel = register_hook("burst_channel");
	h_burst_finished = register_hook("burst_finished");
	h_server_introduced = register_hook("server_introduced");
	h_server_eob = register_hook("server_eob");
	h_client_exit = register_hook("client_exit");
	h_umode_changed = register_hook("umode_changed");
	h_new_local_user = register_hook("new_local_user");
	h_new_remote_user = register_hook("new_remote_user");
	h_introduce_client = register_hook("introduce_client");
	h_can_kick = register_hook("can_kick");
	h_privmsg_user = register_hook("privmsg_user");
	h_privmsg_channel = register_hook("privmsg_channel");
}

/* grow_hooktable()
 *   Enlarges the hook table by HOOK_INCREMENT
 */
static void
grow_hooktable(void)
{
	hook *newhooks;

	newhooks = rb_malloc(sizeof(hook) * (max_hooks + HOOK_INCREMENT));
	memcpy(newhooks, hooks, sizeof(hook) * num_hooks);

	rb_free(hooks);
	hooks = newhooks;
	max_hooks += HOOK_INCREMENT;
}

/* find_freehookslot()
 *   Finds the next free slot in the hook table, given by an entry with
 *   h->name being NULL.
 */
static int
find_freehookslot(void)
{
	int i;

	if((num_hooks + 1) > max_hooks)
		grow_hooktable();

	for(i = 0; i < max_hooks; i++)
	{
		if(!hooks[i].name)
			return i;
	}

	/* shouldnt ever get here */
	return(max_hooks - 1);
}

/* find_hook()
 *   Finds an event in the hook table.
 */
static int
find_hook(const char *name)
{
	int i;

	for(i = 0; i < max_hooks; i++)
	{
		if(!hooks[i].name)
			continue;

		if(!irccmp(hooks[i].name, name))
			return i;
	}

	return -1;
}

/* register_hook()
 *   Finds an events position in the hook table, creating it if it doesnt
 *   exist.
 */
int
register_hook(const char *name)
{
	int i;

	if((i = find_hook(name)) < 0)
	{
		i = find_freehookslot();
		hooks[i].name = rb_strdup(name);
		num_hooks++;
	}

	return i;
}

/* add_hook()
 *   Adds a hook to an event in the hook table, creating event first if
 *   needed.
 */
void
add_hook(const char *name, hookfn fn)
{
	int i;

	i = register_hook(name);

	rb_dlinkAddAlloc(fn, &hooks[i].hooks);
}

/* remove_hook()
 *   Removes a hook from an event in the hook table.
 */
void
remove_hook(const char *name, hookfn fn)
{
	int i;

	if((i = find_hook(name)) < 0)
		return;

	rb_dlinkFindDestroy(fn, &hooks[i].hooks);
}

/* call_hook()
 *   Calls functions from a given event in the hook table.
 */
void
call_hook(int id, void *arg)
{
	hookfn fn;
	rb_dlink_node *ptr;

	/* The ID we were passed is the position in the hook table of this
	 * hook
	 */
	RB_DLINK_FOREACH(ptr, hooks[id].hooks.head)
	{
		fn = ptr->data;
		fn(arg);
	}
}

