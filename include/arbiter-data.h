#ifndef __ARBITER_INTERNAL_H__
#define __ARBITER_INTERNAL_H__

#include "ics-node.h"
#include "oiu-client.h"

typedef struct {
	oiu_t *o_head;
	riu_t *r_head;
    oiu_client_t *oclient;
} arbiter_data_t;

#endif
