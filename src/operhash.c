#include <ratbox_lib.h>
#include "stdinc.h"
#include "match.h"
#include "hash.h"
#include "operhash.h"

#define OPERHASH_MAX_BITS 7
#define OPERHASH_MAX (1<<OPERHASH_MAX_BITS)

#define hash_opername(x) fnv_hash_upper((const unsigned char *)(x), OPERHASH_MAX_BITS)

struct operhash_entry
{
	char *name;
	int refcount;
};

static rb_dlink_list operhash_table[OPERHASH_MAX];

const char *
operhash_add(const char *name)
{
	struct operhash_entry *ohash;
	unsigned int hashv;
	rb_dlink_node *ptr;

	if(EmptyString(name))
		return NULL;

	hashv = hash_opername(name);

	RB_DLINK_FOREACH(ptr, operhash_table[hashv].head)
	{
		ohash = ptr->data;

		if(!irccmp(ohash->name, name))
		{
			ohash->refcount++;
			return ohash->name;
		}
	}

	ohash = rb_malloc(sizeof(struct operhash_entry));
	ohash->refcount = 1;
	ohash->name = rb_strdup(name);

	rb_dlinkAddAlloc(ohash, &operhash_table[hashv]);

	return ohash->name;
}

const char *
operhash_find(const char *name)
{
	struct operhash_entry *ohash;
	unsigned int hashv;
	rb_dlink_node *ptr;

	if(EmptyString(name))
		return NULL;

	hashv = hash_opername(name);

	RB_DLINK_FOREACH(ptr, operhash_table[hashv].head)
	{
		ohash = ptr->data;

		if(!irccmp(ohash->name, name))
			return ohash->name;
	}

	return NULL;
}

void
operhash_delete(const char *name)
{
	struct operhash_entry *ohash;
	unsigned int hashv;
	rb_dlink_node *ptr;

	if(EmptyString(name))
		return;

	hashv = hash_opername(name);

	RB_DLINK_FOREACH(ptr, operhash_table[hashv].head)
	{
		ohash = ptr->data;

		if(irccmp(ohash->name, name))
			continue;

		ohash->refcount--;

		if(ohash->refcount == 0)
		{
			rb_free(ohash->name);
			rb_free(ohash);
			rb_dlinkDestroy(ptr, &operhash_table[hashv]);
			return;
		}
	}
}
