#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "ansi-utils.h"
#include "utlist.h"

#include "arbiter-data.h"
#include "object-pool.h"

#include "arbiter-server.h"
#include "oiu-client.h"

void usage(char *app) {
    printf("Usage: %s <answer-conn-string> <listen-conn-string>\n", app);
    exit(-1);
}

void arbiter_new_oiu(oiu_t **new) {
    *new = malloc(sizeof(oiu_t));
    EXIT_IF_TRUE(*new == NULL, "Cannot alloc memory\n");
    return;
}
void arbiter_delete_oiu(oiu_t *del) {
    if( del != NULL) free(del);
}

static int cmp_id(oiu_t *a, oiu_t *b) {
    return strcmp(a->id, b->id);
}

static void on_request(arbiter_server_t *aserver, arbiter_request_t *request) {
    arbiter_data_t *udata = (arbiter_data_t *)aserver->user_data;
    oiu_t *oiu, *o_node;
    int n;
    time_t timer;
    
    oiu_client_t *oclient = udata->oclient;
    oiu_request_t areq;

    arbiter_new_oiu(&oiu);

    switch(request->msg_id) {
        case REG_STATE_MSG:
            n = snprintf(oiu->id, sizeof(oiu->id) - 1, "%s", request->reg_state_msg.username);
            oiu->id[n] = '\0';

            if (request->reg_state_msg.code == 1)
                oiu->is_online  = 1;
            else {
                time(&timer);
                oiu->downtime = timer;
                oiu->is_online = 0;
            }

            if (strcmp(request->reg_state_msg.type, "OIU") == 0)
                oiu->type = OIU;
            else
                oiu->type = RIU;

            LL_SEARCH(udata->o_head, o_node, oiu, cmp_id);

            if (o_node == NULL){
                oius_append( &udata->o_head, oiu);
            }
            else {
                DL_REPLACE_ELEM(udata->o_head, o_node, oiu);
            }

            areq.msg_id = REG_STATE_RESP;
            areq.reg_state_resp.code = 200;
            strncpy(areq.reg_state_resp.msg, "OK", sizeof(areq.reg_state_resp.msg));
            oiu_client_send(oclient, &areq);
            break;
        default:
            exit(-1);
    }
}

static void on_init_done(arbiter_server_t *aserver) {
    arbiter_data_t *adata = malloc(sizeof(arbiter_data_t));

    adata->o_head = malloc(sizeof(oiu_t));
    adata->r_head = malloc(sizeof(riu_t));

    oius_init(&adata->o_head);
    rius_init(&adata->r_head);

    aserver->user_data =  adata;
}

int main(int argc, char *argv[]) {
	arbiter_server_t aserver;
    oiu_client_t oclient;
	oiu_t *o_node, *o_temp;

	char option[10];
	double downtime;
	time_t timer;


	int f_quit;

	if( argc < 3 ) {
		usage(argv[0]);
	}

    //RESPONE
    oiu_client_open(&oclient, argv[1]);   

    //LISTEN
	aserver.on_request_f = &on_request;
	aserver.on_init_done_f = &on_init_done;

	arbiter_server_init(&aserver, argv[2]);
	
	arbiter_data_t *u_data = (arbiter_data_t *)aserver.user_data;
    u_data->oclient = &oclient;

	arbiter_server_start(&aserver);
    
    //===============================================================//
	f_quit = 0;
	while(!f_quit) {
		if (fgets(option, sizeof(option), stdin) == NULL ) {
			puts("NULL command\n");
		}
		switch(option[0]) {
			case 'm':
				DL_FOREACH(u_data->o_head, o_node) {
					time(&timer);
					downtime = difftime(timer, o_node->downtime);
					printf("Node: type =%s, id=%s, is_online=%d ", DEVICE_TYPE[o_node->type], o_node->id, o_node->is_online);
                    if (o_node->is_online == 1)
                        printf("(On)\n");
                    else
                        printf("(Downtime = %.f second)\n", (o_node->is_online == 0 ? downtime:0) );
                }
                    break;
			case 'q':
				f_quit = 1;
				break;
			default:
				break;
		}
	}

	DL_FOREACH_SAFE(u_data->o_head, o_node, o_temp) {
		DL_DELETE(u_data->o_head, o_node);
		arbiter_delete_oiu(o_node);
	}
	arbiter_server_end(&aserver);

	return 0;
}
