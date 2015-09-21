#ifndef __ARBITER_INTERNAL_H__
#define __ARBITER_INTERNAL_H__

#include "ics-node.h"
#include "oiu-client.h"
#include "riu-client.h"

typedef struct {
	oiu_t *o_head;
	riu_t *r_head;
    oiu_client_t *oclient;
    riu_client_t *rclient1;
    riu_client_t *rclient2;
    riu_client_t *rclient3;
    riu_client_t *rclient4;
} arbiter_data_t;

#endif
