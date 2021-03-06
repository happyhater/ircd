#ifndef INCLUDED_m_info_h
#define INCLUDED_m_info_h

#include "config.h"

typedef struct Information
{
	const char *name;	/* name of item */
	const char *strvalue;	/* value of item if it's a boolean */
	int intvalue;		/* value of item if it's an integer */
	const char *desc;	/* short description of item */
}
Info;

Info MyInformation[] = {

#ifdef CPATH
	{"CPATH", CPATH, 0, "Path to Main Configuration File"},
#else
	{"CPATH", "NONE", 0, "Path to Main Configuration File"},
#endif /* CPATH */

#ifdef DPATH
	{"DPATH", DPATH, 0, "Directory Containing Configuration Files"},
#else
	{"DPATH", "NONE", 0, "Directory Containing Configuration Files"},
#endif /* DPATH */

#ifdef HPATH
	{"HPATH", HPATH, 0, "Path to Operator Help Files"},
#else
	{"HPATH", "NONE", 0, "Path to Operator Help Files"},
#endif /* HPATH */

#ifdef UHPATH
	{"UHPATH", UHPATH, 0, "Path to User Help Files"},
#else
	{"UHPATH", "NONE", 0, "Path to User Help Files"},
#endif /* UH PATH */

#ifdef SOMAXCONN
	{"RATBOX_SOMAXCONN", "", SOMAXCONN,
	 "Maximum Queue Length of Pending Connections"},
#else
	{"RATBOX_SOMAXCONN", "", RATBOX_SOMAXCONN,
	 "Maximum Queue Length of Pending Connections"},
#endif /* SOMAXCONN */

#ifdef RB_IPV6
	{"IPV6", "ON", 0, "IPv6 Support"},
#else
	{"IPV6", "OFF", 0, "IPv6 Support"},
#endif

	{"JOIN_LEAVE_COUNT_EXPIRE_TIME", "", JOIN_LEAVE_COUNT_EXPIRE_TIME,
	 "Anti SpamBot Parameter"},

	{"KILLCHASETIMELIMIT", "", KILLCHASETIMELIMIT,
	 "Nick Change Tracker for KILL"},

#ifdef LPATH
	{"LPATH", LPATH, 0, "Path to Log File"},
#else
	{"LPATH", "NONE", 0, "Path to Log File"},
#endif /* LPATH */

	{"MAX_BUFFER", "", MAX_BUFFER, "Maximum Buffer Connections Allowed"},

	{"MAX_JOIN_LEAVE_COUNT", "", MAX_JOIN_LEAVE_COUNT,
	 "Anti SpamBot Parameter"},

	{"MIN_JOIN_LEAVE_TIME", "", MIN_JOIN_LEAVE_TIME,
	 "Anti SpamBot Parameter"},

#ifdef MPATH
	{"MPATH", MPATH, 0, "Path to MOTD File"},
#else
	{"MPATH", "NONE", 0, "Path to MOTD File"},
#endif /* MPATH */

	{"NICKNAMEHISTORYLENGTH", "", NICKNAMEHISTORYLENGTH,
	 "Size of WHOWAS Array"},

#ifdef OPATH
	{"OPATH", OPATH, 0, "Path to Operator MOTD File"},
#else
	{"OPATH", "NONE", 0, "Path to Operator MOTD File"},
#endif /* OPATH */

	{"OPER_SPAM_COUNTDOWN", "", OPER_SPAM_COUNTDOWN,
	 "Anti SpamBot Parameter"},

#ifdef HAVE_LIBCRYPTO
	{"HAVE_LIBCRYPTO", "ON", 0, "Enable OpenSSL CHALLENGE Support"},
#else
	{"HAVE_LIBCRYPTO", "OFF", 0, "Enable OpenSSL CHALLENGE Support"},
#endif /* HAVE_LIBCRYPTO */

#ifdef HAVE_LIBZ
	{"HAVE_LIBZ", "YES", 0, "zlib (ziplinks) support"},
#else
	{"HAVE_LIBZ", "NO", 0, "zlib (ziplinks)  support"},
#endif /* HAVE_LIBZ */

#ifdef PPATH
	{"PPATH", PPATH, 0, "Path to Pid File"},
#else
	{"PPATH", "NONE", 0, "Path to Pid File"},
#endif /* PPATH */

	{"SELECT_TYPE", SELECT_TYPE, 0, "Method of Multiplexed I/O"},

#ifdef SPATH
	{"SPATH", SPATH, 0, "Path to Server Executable"},
#else
	{"SPATH", "NONE", 0, "Path to Server Executable"},
#endif /* SPATH */

	{"TS_MAX_DELTA_DEFAULT", "", TS_MAX_DELTA_DEFAULT,
	 "Maximum Allowed TS Delta from another Server"},
	{"TS_WARN_DELTA_DEFAULT", "", TS_WARN_DELTA_DEFAULT,
	 "Maximum TS Delta before Sending Warning"},
#ifdef USE_IODEBUG_HOOKS
	{"USE_IODEBUG_HOOKS", "YES", 0, "IO Debugging support"},
#else
	{"USE_IODEBUG_HOOKS", "NO", 0, "IO Debugging support"},
#endif

	/*
	 * since we don't want to include the world here, NULL probably
	 * isn't defined by the time we read this, just use plain 0 instead
	 * 0 is guaranteed by the language to be assignable to ALL built
	 * in types with the correct results.
	 */
	{0, 0, 0, 0}
};


#endif /* INCLUDED_m_info_h */
