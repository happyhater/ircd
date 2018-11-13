#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"

static int m_time(struct Client *, struct Client *, int, const char **);
static char *date(void);

struct Message time_msgtab = {
	"TIME", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_time, 0}, {m_time, 2}, mg_ignore, mg_ignore, {m_time, 0}}
};

mapi_clist_av1 time_clist[] = { &time_msgtab, NULL };
DECLARE_MODULE_AV1(time, NULL, NULL, time_clist, NULL, NULL, "$Revision: 254 $");

static const char *months[] = {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

static const char *weekdays[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

/*
 * m_time
 *      parv[1] = servername
 */
static int
m_time(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	/* this is not rate limited, so end the grace period */
	if(MyClient(source_p) && !IsFloodDone(source_p))
		flood_endgrace(source_p);

	if(hunt_server(client_p, source_p, ":%s TIME :%s", 1, parc, parv) == HUNTED_ISME)
		sendto_one_numeric(source_p, RPL_TIME, form_str(RPL_TIME),
				   me.name, date());

	return 0;
}

/* date()
 *
 * returns date in human readable form
 */
static char *
date(void)
{
	static char buf[80];
	char plus;
	struct tm *lt;
	struct tm *gm;
	struct tm gmbuf;
	time_t lclock;
	int minswest;

	lclock = rb_current_time();
	gm = gmtime(&lclock);
	memcpy((void *) &gmbuf, (void *) gm, sizeof(gmbuf));
	gm = &gmbuf;
	lt = localtime(&lclock);

	if(lt->tm_yday == gm->tm_yday)
		minswest = (gm->tm_hour - lt->tm_hour) * 60 + (gm->tm_min - lt->tm_min);
	else if(lt->tm_yday > gm->tm_yday && lt->tm_year == gm->tm_year)
		minswest = (gm->tm_hour - (lt->tm_hour + 24)) * 60;
	else
		minswest = ((gm->tm_hour + 24) - lt->tm_hour) * 60;

	plus = (minswest > 0) ? '-' : '+';

	if(minswest < 0)
		minswest = -minswest;

	rb_sprintf(buf, "%s %s %d %d -- %02u:%02u:%02u %c%02u:%02u",
		   weekdays[lt->tm_wday], months[lt->tm_mon], lt->tm_mday,
		   lt->tm_year + 1900, lt->tm_hour, lt->tm_min, lt->tm_sec,
		   plus, minswest / 60, minswest % 60);

	return buf;
}