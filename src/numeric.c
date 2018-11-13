#include "stdinc.h"
#include "setup.h"
#include "config.h"
#include "s_conf.h"
#include "numeric.h"
#include "match.h"
#include "common.h"		/* NULL cripes */

#include "messages.tab"

/*
 * form_str
 *
 * inputs	- numeric
 * output	- corresponding string
 * side effects	- NONE
 */
const char *
form_str(int numeric)
{
	const char *num_ptr;

	s_assert(-1 < numeric);
	s_assert(numeric < ERR_LAST_ERR_MSG);
	s_assert(0 != replies[numeric]);

	if(numeric > ERR_LAST_ERR_MSG)
		numeric = ERR_LAST_ERR_MSG;
	if(numeric < 0)
		numeric = ERR_LAST_ERR_MSG;

	num_ptr = replies[numeric];
	if(num_ptr == NULL)
		num_ptr = replies[ERR_LAST_ERR_MSG];

	return (num_ptr);
}
