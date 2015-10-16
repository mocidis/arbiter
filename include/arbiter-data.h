#ifndef __ARBITER_INTERNAL_H__
#define __ARBITER_INTERNAL_H__

#include "ics-node.h"
#include "oiu-client.h"
#include "riu-client.h"

#define MAX_DEVICE 20

typedef struct rclient_data_s{
    riu_client_t *rclient;
    char username[10]; 
    int is_used;
}rclient_data_t;

typedef struct {
	oiu_t *o_head;
	riu_t *r_head;
    oiu_client_t *oclient;
    rclient_data_t *rclient_data[MAX_DEVICE];
} arbiter_data_t;

#endif
