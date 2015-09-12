#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "ansi-utils.h"
#include "utlist.h"

#include "arbiter-data.h"
#include "arbiter-func.h"
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

    arbiter_new_oiu(&oiu);

    switch(request->msg_id) {
        case ABT_UP:
            n = snprintf(oiu->id, sizeof(oiu->id) - 1, "%s", request->abt_up.username);
            oiu->id[n] = '\0';

            if (request->abt_up.code == 1)
                oiu->is_online  = 1;
            else {
                oiu->is_online = 0;
            }

            time(&timer);
            oiu->recv_time = timer;

            if (strcmp(request->abt_up.type, "OIU") == 0)
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
            //SAVE TO LIST DONE
            // NOW SEND THE LIST TO OIUC BY MULTICAST
            // (MOVED INTO ARBITER-FUNC.C)
/*
            DL_FOREACH_SAFE(udata->o_head, o_node, o_temp) {
                areq.msg_id = OIUC_GB;
                if (request->abt_up.code == 1) //Need more condition
                    areq.oiuc_gb.is_alive = 1;
                else
                    areq.oiuc_gb.is_alive = 0;
                strncpy(areq.oiuc_gb.id, request->abt_up.username, sizeof(areq.oiuc_gb.id));
                oiu_client_send(oclient, &areq);
            }       */
            break;
        default:
            printf("Unknow request. Exit now\n");
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
	double recv_time;
	time_t timer;


	int f_quit;

	if( argc < 3 ) {
		usage(argv[0]);
	}

    //RESPONE
    oiu_client_open(&oclient, argv[1]);
    arbiter_auto_send(&aserver);

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
					recv_time = difftime(timer, o_node->recv_time);
					printf("Node: type =%s, id=%s, is_online=%d ", DEVICE_TYPE[o_node->type], o_node->id, o_node->is_online);
                    if (o_node->is_online == 1)
                        printf("(On)\n");
                    else
                        printf("(Downtime = %.f second)\n", (o_node->is_online == 0 ? recv_time:0) );
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
