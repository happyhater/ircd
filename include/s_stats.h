#ifndef INCLUDED_s_stats_h
#define INCLUDED_s_stats_h

#define _1MEG     (1024.0)
#define _1GIG     (1024.0*1024.0)
#define _1TER     (1024.0*1024.0*1024.0)
#define _GMKs(x)  ( (x > _1TER) ? "Terabytes" : ((x > _1GIG) ? "Gigabytes" : \
                  ((x > _1MEG) ? "Megabytes" : "Kilobytes")))
#define _GMKv(x)  ( (x > _1TER) ? (float)(x/_1TER) : ((x > _1GIG) ? \
                  (float)(x/_1GIG) : ((x > _1MEG) ? (float)(x/_1MEG) : (float)x)))

struct Client;

/*
 * statistics structures
 */
struct ServerStatistics
{
	unsigned int is_cl;	/* number of client connections */
	unsigned int is_sv;	/* number of server connections */
	unsigned int is_ni;	/* connection but no idea who it was */
	unsigned long long int is_cbs;	/* bytes sent to clients */
	unsigned long long int is_cbr;	/* bytes received to clients */
	unsigned long long int is_sbs;	/* bytes sent to servers */
	unsigned long long int is_sbr;	/* bytes received to servers */
	unsigned long long int is_cti;	/* time spent connected by clients */
	unsigned long long int is_sti;	/* time spent connected by servers */
	unsigned int is_ac;	/* connections accepted */
	unsigned int is_ref;	/* accepts refused */
	unsigned int is_unco;	/* unknown commands */
	unsigned int is_wrdi;	/* command going in wrong direction */
	unsigned int is_unpf;	/* unknown prefix */
	unsigned int is_empt;	/* empty message */
	unsigned int is_num;	/* numeric message */
	unsigned int is_kill;	/* number of kills generated on collisions */
	unsigned int is_save;	/* number of saves generated on collisions */
	unsigned int is_asuc;	/* successful auth requests */
	unsigned int is_abad;	/* bad auth requests */
	unsigned int is_rej;	/* rejected from cache */
	unsigned int is_thr;	/* number of throttled connections */
	unsigned int is_ssuc;	/* successful sasl authentications */
	unsigned int is_sbad;	/* failed sasl authentications */
	unsigned int is_tgch;	/* messages blocked due to target change */
};

extern struct ServerStatistics ServerStats;

#endif /* INCLUDED_s_stats_h */
