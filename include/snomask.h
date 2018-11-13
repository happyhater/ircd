#ifndef SNOMASK_H
#define SNOMASK_H

#include "client.h"

#define SNO_ADD			1
#define SNO_DEL			2

#define SNO_CCONNEXT		0x00000001
#define SNO_BOTS		0x00000002
#define SNO_CCONN		0x00000004
#define SNO_DEBUG		0x00000008
#define SNO_FULL		0x00000010
#define SNO_SKILL		0x00000020
#define SNO_NCHANGE		0x00000040
#define SNO_REJ			0x00000080
#define SNO_GENERAL		0x00000100
#define SNO_UNAUTH		0x00000200
#define SNO_EXTERNAL		0x00000400
#define SNO_SPY			0x00000800
#define SNO_OPERSPY		0x00001000

char *construct_snobuf(unsigned int val);
unsigned int parse_snobuf_to_mask(unsigned int val, const char *sno);
unsigned int find_snomask_slot(void);

extern int snomask_modes[];

#endif
