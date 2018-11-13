#include "stdinc.h"
#include "ircd_defs.h"
#include "common.h"
#include "s_conf.h"
#include "client.h"
#include "hash.h"
#include "cache.h"
#include "irc_dictionary.h"
#include "numeric.h"

struct cachefile *user_motd = NULL;
struct cachefile *oper_motd = NULL;
struct cacheline *emptyline = NULL;
rb_dlink_list links_cache_list;
char user_motd_changed[MAX_DATE_STRING];

struct Dictionary *help_dict_oper = NULL;
struct Dictionary *help_dict_user = NULL;

/* init_cache()
 *
 * inputs	-
 * outputs	-
 * side effects - inits the file/line cache blockheaps, loads motds
 */
void
init_cache(void)
{
	/* allocate the emptyline */
	emptyline = rb_malloc(sizeof(struct cacheline));
	emptyline->data[0] = ' ';
	emptyline->data[1] = '\0';
	user_motd_changed[0] = '\0';

	user_motd = cache_file(MPATH, "ircd.motd", 0);
	oper_motd = cache_file(OPATH, "opers.motd", 0);
	memset(&links_cache_list, 0, sizeof(links_cache_list));

	help_dict_oper = irc_dictionary_create(strcasecmp);
	help_dict_user = irc_dictionary_create(strcasecmp);
}

/* 
 * removes tabs from src, replaces with 8 spaces, and returns the length
 * of the new string.  if the new string would be greater than destlen,
 * it is truncated to destlen - 1
 */
static size_t
untabify(char *dest, const char *src, size_t destlen)
{
	size_t x = 0, i;
	const char *s = src;
	char *d = dest;

	while(*s != '\0' && x < destlen - 1)
	{
		if(*s == '\t')
		{
			for(i = 0; i < 8 && x < destlen - 1; i++, x++, d++)
				*d = ' ';
			s++;
		} else 
		{
			*d++ = *s++;
			x++;
		}
	}
	*d = '\0';
	return x;
}

/* cache_file()
 *
 * inputs	- file to cache, files "shortname", flags to set
 * outputs	- pointer to file cached, else NULL
 * side effects -
 */
struct cachefile *
cache_file(const char *filename, const char *shortname, int flags)
{
	FILE *in;
	struct cachefile *cacheptr;
	struct cacheline *lineptr;
	char line[BUFSIZE];
	char *p;

	if((in = fopen(filename, "r")) == NULL)
		return NULL;


	cacheptr = rb_malloc(sizeof(struct cachefile));

	rb_strlcpy(cacheptr->name, shortname, sizeof(cacheptr->name));
	cacheptr->flags = flags;

	/* cache the file... */
	while(fgets(line, sizeof(line), in) != NULL)
	{
		if((p = strpbrk(line, "\r\n")) != NULL)
			*p = '\0';

		if(!EmptyString(line))
		{
			lineptr = rb_malloc(sizeof(struct cacheline));
			untabify(lineptr->data, line, sizeof(lineptr->data));
			rb_dlinkAddTail(lineptr, &lineptr->linenode, &cacheptr->contents);
		}
		else
			rb_dlinkAddTailAlloc(emptyline, &cacheptr->contents);
	}

	if (0 == rb_dlink_list_length(&cacheptr->contents))
	{
		/* No contents. Don't cache it after all. */
		rb_free(cacheptr);
		cacheptr = NULL;
	}

	fclose(in);
	return cacheptr;
}

void
cache_links(void *unused)
{
	struct Client *target_p;
	rb_dlink_node *ptr;
	rb_dlink_node *next_ptr;
	char *links_line;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, links_cache_list.head)
	{
		rb_free(ptr->data);
		rb_free_rb_dlink_node(ptr);
	}

	links_cache_list.head = links_cache_list.tail = NULL;
	links_cache_list.length = 0;

	RB_DLINK_FOREACH(ptr, global_serv_list.head)
	{
		target_p = ptr->data;

		/* skip ourselves (done in /links) and hidden servers */
		if(IsMe(target_p) ||
		   (IsHidden(target_p) && !ConfigServerHide.disable_hidden))
			continue;

		/* if the below is ever modified, change LINKSLINELEN */
		links_line = rb_malloc(LINKSLINELEN);
		rb_snprintf(links_line, LINKSLINELEN, "%s %s :1 %s",
			   target_p->name, me.name, 
			   target_p->info[0] ? target_p->info : 
			    "(Unknown Location)");

		rb_dlinkAddTailAlloc(links_line, &links_cache_list);
	}
}

/* free_cachefile()
 *
 * inputs	- cachefile to free
 * outputs	-
 * side effects - cachefile and its data is free'd
 */
void
free_cachefile(struct cachefile *cacheptr)
{
	rb_dlink_node *ptr;
	rb_dlink_node *next_ptr;

	if(cacheptr == NULL)
		return;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, cacheptr->contents.head)
	{
		if(ptr->data != emptyline)
			rb_free(ptr->data);
	}

	rb_free(cacheptr);
}

/* send_user_motd()
 *
 * inputs	- client to send motd to
 * outputs	- client is sent motd if exists, else ERR_NOMOTD
 * side effects -
 */
void
send_user_motd(struct Client *source_p)
{
	struct cacheline *lineptr;
	rb_dlink_node *ptr;
	const char *myname = get_id(&me, source_p);
	const char *nick = get_id(source_p, source_p);
	if(user_motd == NULL || rb_dlink_list_length(&user_motd->contents) == 0)
	{
		sendto_one(source_p, form_str(ERR_NOMOTD), myname, nick);
		return;
	}

	sendto_one(source_p, form_str(RPL_MOTDSTART), myname, nick, me.name);

	RB_DLINK_FOREACH(ptr, user_motd->contents.head)
	{
		lineptr = ptr->data;
		sendto_one(source_p, form_str(RPL_MOTD), myname, nick, lineptr->data);
	}

	sendto_one(source_p, form_str(RPL_ENDOFMOTD), myname, nick);
}

void
cache_user_motd(void)
{
	struct stat sb;
	struct tm *local_tm;
	
	if(stat(MPATH, &sb) == 0) 
	{
		local_tm = localtime(&sb.st_mtime);

		if(local_tm != NULL) 
		{
			rb_snprintf(user_motd_changed, sizeof(user_motd_changed),
				 "%d/%d/%d %d:%d",
				 local_tm->tm_mday, local_tm->tm_mon + 1,
				 1900 + local_tm->tm_year, local_tm->tm_hour,
				 local_tm->tm_min);
		}
	} 
	free_cachefile(user_motd);
	user_motd = cache_file(MPATH, "ircd.motd", 0);
}


/* send_oper_motd()
 *
 * inputs	- client to send motd to
 * outputs	- client is sent oper motd if exists
 * side effects -
 */
void
send_oper_motd(struct Client *source_p)
{
	struct cacheline *lineptr;
	rb_dlink_node *ptr;

	if(oper_motd == NULL || rb_dlink_list_length(&oper_motd->contents) == 0)
		return;

	sendto_one(source_p, form_str(RPL_OMOTDSTART), 
		   me.name, source_p->name);

	RB_DLINK_FOREACH(ptr, oper_motd->contents.head)
	{
		lineptr = ptr->data;
		sendto_one(source_p, form_str(RPL_OMOTD),
			   me.name, source_p->name, lineptr->data);
	}

	sendto_one(source_p, form_str(RPL_ENDOFOMOTD), 
		   me.name, source_p->name);
}
