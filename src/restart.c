#include "stdinc.h"
#include "restart.h"
#include "ircd.h"
#include "send.h"
#include "logger.h"
#include "s_conf.h"
#include "client.h"		
#include "ircd_signal.h"

/* external var */
extern char **myargv;

void
restart(const char *mesg)
{
	static int was_here = NO;	/* redundant due to restarting flag below */

	if(was_here)
		abort();
	was_here = YES;

	ilog(L_MAIN, "Restarting Server because: %s", mesg);

	server_reboot();
}

void
server_reboot(void)
{
	int i;
	char path[PATH_MAX+1];

	sendto_realops_snomask(SNO_GENERAL, L_NETWIDE, "Restarting server...");

	ilog(L_MAIN, "Restarting server...");
	
	/*
	 * XXX we used to call flush_connections() here. But since this routine
	 * doesn't exist anymore, we won't be flushing. This is ok, since 
	 * when close handlers come into existance, rb_close() will be called
	 * below, and the data flushing will be implicit.
	 *    -- adrian
	 *
	 * bah, for now, the program ain't coming back to here, so forcibly
	 * close everything the "wrong" way for now, and just LEAVE...
	 */
	for (i = 0; i < maxconnections; ++i)
		close(i);

	unlink(pidFileName);
	execv(SPATH, (void *)myargv);

	/* use this if execv of SPATH fails */
	rb_snprintf(path, sizeof(path), "%s/bin/ircd", ConfigFileEntry.dpath);

	execv(path, (void *)myargv);
	exit(-1);
}
