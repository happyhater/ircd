#include "stdinc.h"
#include "logger.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "hostmask.h"
#include "client.h"
#include "match.h"
#include "hash.h"

/* conf_add_fields()
 * 
 * inputs       - pointer to config item, host/pass/user/operreason/date fields
 * output       - NONE
 * side effects - update respective fields with pointers
 */
static void
conf_add_fields(struct ConfItem *aconf,	const char *host_field,
		const char *pass_field,	const char *user_field,
		const char *operreason_field, const char *date_field,
		const char *setter_field)
{
	if(host_field != NULL)
		aconf->host = rb_strdup(host_field);
	if(pass_field != NULL)
	{
		if(!EmptyString(date_field))
		{
			aconf->passwd = rb_malloc(strlen(pass_field) + strlen(date_field) + 4);
			rb_sprintf(aconf->passwd, "%s (%s)", pass_field, date_field);
		}
		else
			aconf->passwd = rb_strdup(pass_field);
	}
	if(user_field != NULL)
		aconf->user = rb_strdup(user_field);
	if(operreason_field != NULL)
		aconf->spasswd = rb_strdup(operreason_field);
	if(setter_field != NULL)
		aconf->name = rb_strdup(setter_field);
}

/*
 * parse_k_file
 * Inputs       - pointer to line to parse
 * Output       - NONE
 * Side Effects - Parse one new style K line
 */

void
parse_k_file(FILE * file)
{
	struct ConfItem *aconf;
	char *user_field = NULL;
	char *reason_field = NULL;
	char *operreason_field = NULL;
	char *host_field = NULL;
	char *date_field = NULL;
	char *setter_field = NULL;
	char line[BUFSIZE];
	char *p;

	while (fgets(line, sizeof(line), file))
	{
		if((p = strchr(line, '\n')) != NULL)
			*p = '\0';

		if((*line == '\0') || (*line == '#'))
			continue;

		user_field = getfield(line);
		if(EmptyString(user_field))
			continue;

		host_field = getfield(NULL);
		if(EmptyString(host_field))
			continue;

		reason_field = getfield(NULL);
		if(EmptyString(reason_field))
			continue;

		operreason_field = getfield(NULL);
		date_field = getfield(NULL);
		setter_field = getfield(NULL);

		aconf = make_conf();
		aconf->status = CONF_KILL;
		conf_add_fields(aconf, host_field, reason_field,
				user_field, operreason_field, date_field, setter_field);

		if(aconf->host != NULL)
			add_conf_by_address(aconf->host, CONF_KILL, aconf->user, NULL, aconf);
	}
}

/*
 * parse_d_file
 * Inputs       - pointer to line to parse
 * Output       - NONE
 * Side Effects - Parse one new style D line
 */

void
parse_d_file(FILE * file)
{
	struct ConfItem *aconf;
	char *reason_field = NULL;
	char *host_field = NULL;
	char *operreason_field = NULL;
	char *date_field = NULL;
	char *setter_field = NULL;
	char line[BUFSIZE];
	char *p;

	while (fgets(line, sizeof(line), file))
	{
		if((p = strchr(line, '\n')))
			*p = '\0';

		if((*line == '\0') || (line[0] == '#'))
			continue;

		host_field = getfield(line);
		if(EmptyString(host_field))
			continue;

		reason_field = getfield(NULL);
		if(EmptyString(reason_field))
			continue;

		operreason_field = getfield(NULL);
		date_field = getfield(NULL);
		setter_field = getfield(NULL);

		aconf = make_conf();
		aconf->status = CONF_DLINE;
		conf_add_fields(aconf, host_field, reason_field, "", operreason_field, date_field, setter_field);
		conf_add_d_conf(aconf);
	}
}

char *
xline_encode_spaces(const char *mask)
{
	int i, j;
	int spaces = 0;
	int backslash = 0;
	char *encoded;

	for (i = 0; mask[i] != '\0'; i++)
		if (mask[i] == ' ')
			spaces++;
	encoded = rb_malloc(i + spaces + 1);
	for (i = 0, j = 0; mask[i] != '\0'; i++)
	{
		if (mask[i] == '\\')
			backslash = !backslash;
		else if (mask[i] == ' ')
		{
			if (!backslash)
				encoded[j++] = '\\';
			encoded[j++] = 's';
			backslash = 0;
			continue;
		}
		else
			backslash = 0;
		encoded[j++] = mask[i];
	}
	return encoded;
}

void
parse_x_file(FILE * file)
{
	struct ConfItem *aconf;
	char *gecos_field = NULL;
	char *reason_field = NULL;
	char line[BUFSIZE];
	char *p;
	char *encoded;

	while (fgets(line, sizeof(line), file))
	{
		if((p = strchr(line, '\n')))
			*p = '\0';

		if((*line == '\0') || (line[0] == '#'))
			continue;

		gecos_field = getfield(line);
		if(EmptyString(gecos_field))
			continue;

		/* field for xline types, which no longer exist */
		getfield(NULL);

		reason_field = getfield(NULL);
		if(EmptyString(reason_field))
			continue;

		/* sanity checking */
		if((find_xline_mask(gecos_field) != NULL) ||
		   (strchr(reason_field, ':') != NULL))
			continue;

		encoded = xline_encode_spaces(gecos_field);
		if (find_xline_mask(encoded) != NULL)
		{
			rb_free(encoded);
			continue;
		}

		aconf = make_conf();
		aconf->status = CONF_XLINE;

		aconf->name = encoded;
		aconf->passwd = rb_strdup(reason_field);

		rb_dlinkAddAlloc(aconf, &xline_conf_list);
	}
}

void
parse_resv_file(FILE * file)
{
	struct ConfItem *aconf;
	char *reason_field;
	char *host_field;
	char line[BUFSIZE];
	char *p;

	while (fgets(line, sizeof(line), file))
	{
		if((p = strchr(line, '\n')))
			*p = '\0';

		if((*line == '\0') || (line[0] == '#'))
			continue;

		host_field = getfield(line);
		if(EmptyString(host_field))
			continue;

		reason_field = getfield(NULL);
		if(EmptyString(reason_field))
			continue;

		if(IsChannelName(host_field))
		{
			if(hash_find_resv(host_field))
				continue;

			aconf = make_conf();
			aconf->status = CONF_RESV_CHANNEL;
			aconf->port = 0;

			aconf->name = rb_strdup(host_field);
			aconf->passwd = rb_strdup(reason_field);
			add_to_resv_hash(aconf->name, aconf);
		}
		else if(clean_resv_nick(host_field))
		{
			if(find_nick_resv_mask(host_field))
				continue;

			aconf = make_conf();
			aconf->status = CONF_RESV_NICK;
			aconf->port = 0;

			aconf->name = rb_strdup(host_field);
			aconf->passwd = rb_strdup(reason_field);
			rb_dlinkAddAlloc(aconf, &resv_conf_list);
		}
	}
}

/*
 * getfield
 *
 * inputs	- input buffer
 * output	- next field
 * side effects	- field breakup for ircd.conf file.
 */
char *
getfield(char *newline)
{
	static char *line = NULL;
	char *end, *field;

	if(newline != NULL)
		line = newline;

	if(line == NULL)
		return (NULL);

	field = line;

	/* XXX make this skip to first " if present */
	if(*field == '"')
		field++;
	else
		return (NULL);	/* mal-formed field */

	end = strchr(line, ',');

	while(1)
	{
		/* no trailing , - last field */
		if(end == NULL)
		{
			end = line + strlen(line);
			line = NULL;

			if(*end == '"')
			{
				*end = '\0';
				return field;
			}
			else
				return NULL;
		}
		else
		{
			/* look for a ", to mark the end of a field.. */
			if(*(end - 1) == '"')
			{
				line = end + 1;
				end--;
				*end = '\0';
				return field;
			}

			/* search for the next ',' */
			end++;
			end = strchr(end, ',');
		}
	}

	return NULL;
}
