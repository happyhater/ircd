#ifndef INCLUDED_s_log_h
#define INCLUDED_s_log_h

#include "ircd_defs.h"

typedef enum ilogfile
{
	L_MAIN,
	L_USER,
	L_FUSER,
	L_OPERED,
	L_FOPER,
	L_SERVER,
	L_KILL,
	L_KLINE,
	L_OPERSPY,
	L_IOERROR,
	LAST_LOGFILE
} ilogfile;

struct Client;

extern void init_main_logfile(void);
extern void open_logfiles(void);
extern void close_logfiles(void);
extern void ilog(ilogfile dest, const char *fmt, ...) AFP(2, 3);
extern void inotice(const char *fmt, ...) AFP(1, 2);
extern void iwarn(const char *fmt, ...) AFP(1, 2);
extern void ierror(const char *fmt, ...) AFP(1, 2);
extern void report_operspy(struct Client *, const char *, const char *);
extern const char *smalldate(time_t);
extern void ilog_error(const char *);

#endif
