#ifndef INCLUDED_packet_h
#define INCLUDED_packet_h

/*
 * this hides in here rather than a config.h because it really shouldn't
 * be tweaked unless you *REALLY REALLY* know what you're doing!
 * Remember, messages are only anti-flooded on incoming from the client, not on
 * incoming from a server for a given client, so if you tweak this you risk
 * allowing a client to flood differently depending upon where they are on
 * the network..
 *   -- adrian
 */
/* MAX_FLOOD is the amount of lines in a 'burst' we allow from a client, 
 * anything beyond MAX_FLOOD is limited to about one line per second.
 *
 * MAX_FLOOD_CONN is the amount of lines we allow from a client who has
 * just connected.  this allows clients to rejoin multiple channels
 * without being so heavily penalised they excess flood.
 */
/*
 * spb: Made these configurable
 */
#define MAX_FLOOD ConfigFileEntry.client_flood_burst_max
#define MAX_FLOOD_BURST MAX_FLOOD * 8


extern PF read_packet;
extern EVH flood_recalc;
extern void flood_endgrace(struct Client *);

#endif /* INCLUDED_packet_h */
