#include "stdinc.h"
#include "channel.h"
#include "client.h"
#include "common.h"

ExtbanFunc extban_table[256] = { NULL };

int
match_extban(const char *banstr, struct Client *client_p, struct Channel *chptr, long mode_type)
{
	const char *p;
	int invert = 0, result = EXTBAN_INVALID;
	ExtbanFunc f;

	if (*banstr != '$')
		return 0;
	p = banstr + 1;
	if (*p == '~')
	{
		invert = 1;
		p++;
	}
	f = extban_table[(unsigned char) ToLower(*p)];
	if (*p != '\0')
	{
		p++;
		if (*p == ':')
			p++;
		else
			p = NULL;
	}
	if (f != NULL)
		result = f(p, client_p, chptr, mode_type);
	else
		result = EXTBAN_INVALID;

	if (invert)
		return result == EXTBAN_NOMATCH;
	else
		return result == EXTBAN_MATCH;
}

int
valid_extban(const char *banstr, struct Client *client_p, struct Channel *chptr, long mode_type)
{
	const char *p;
	int result = EXTBAN_INVALID;
	ExtbanFunc f;

	if (*banstr != '$')
		return 0;
	p = banstr + 1;
	if (*p == '~')
		p++;
	f = extban_table[(unsigned char) ToLower(*p)];
	if (*p != '\0')
	{
		p++;
		if (*p == ':')
			p++;
		else
			p = NULL;
	}
	if (f != NULL)
		result = f(p, client_p, chptr, mode_type);
	else
		result = EXTBAN_INVALID;

	return result != EXTBAN_INVALID;
}

const char *
get_extban_string(void)
{
	static char e[256];
	int i, j;

	j = 0;
	for (i = 1; i < 256; i++)
		if (i == ToLower(i) && extban_table[i])
			e[j++] = i;
	e[j] = 0;
	return e;
}
