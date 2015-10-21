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
#include "riu-client.h"
#include "ics-proto.h"

static void usage(char *app) {
    printf("Usage: %s <answer-conn-string> <listen-conn-string>\n", app);
    exit(-1);
}

static void arbiter_new_oiu(oiu_t **new) {
    *new = malloc(sizeof(oiu_t));
    EXIT_IF_TRUE(*new == NULL, "Cannot alloc memory\n");
    return;
}
static void arbiter_delete_oiu(oiu_t *del) {
    if( del != NULL) free(del);
}

static int cmp_id_oiu(oiu_t *a, oiu_t *b) {
    return strcmp(a->id, b->id);
}

static void arbiter_new_riu(riu_t **new) {
    *new = malloc(sizeof(riu_t));
    EXIT_IF_TRUE(*new == NULL, "Cannot alloc memory\n");
    return;
}
static void arbiter_delete_riu(riu_t *del) {
    if( del != NULL) free(del);
}

static int cmp_id_riu(riu_t *a, riu_t *b) {
    return (strcmp(a->id, b->id));
}

static void on_request(arbiter_server_t *aserver, arbiter_request_t *request) {
    arbiter_data_t *udata = (arbiter_data_t *)aserver->user_data;
    oiu_t *oiu, *o_node;
    riu_t *riu, *r_node;

    int i, n;
    time_t timer;
    char ip_addr[50];
    strncpy(ip_addr,"udp:", sizeof(ip_addr));

    arbiter_new_oiu(&oiu);
    arbiter_new_riu(&riu);

    switch(request->msg_id) {
        case ABT_UP:
            if (request->abt_up.type == DT_OIUC) {
                oiu->type = OIU;
                
                time(&timer);
                oiu->recv_time = timer;

                n = snprintf(oiu->id, sizeof(oiu->id) - 1, "%s", request->abt_up.username);
                oiu->id[n] = '\0';
                strncpy(oiu->desc, request->abt_up.desc, sizeof(request->abt_up.desc));

                oiu->is_online  = request->abt_up.is_online;


                LL_SEARCH(udata->o_head, o_node, oiu, cmp_id_oiu);

                if (o_node == NULL){
                    oius_append( &udata->o_head, oiu);
                }
                else {
                    DL_REPLACE_ELEM(udata->o_head, o_node, oiu);
                }
            }
            else if(request->abt_up.type == DT_RIUC) {
                riu->type = RIU;

                time(&timer);
                riu->recv_time = timer;

                strcpy(riu->id, request->abt_up.username);
                strncpy(riu->location, request->abt_up.location, sizeof(request->abt_up.location));
                strncpy(riu->desc, request->abt_up.desc, sizeof(request->abt_up.desc));
                strncpy(riu->ip_addr, request->abt_up.ip_addr, sizeof(request->abt_up.ip_addr));

                riu->is_online = request->abt_up.is_online;
                riu->is_tx = request->abt_up.is_tx;
                riu->is_sq = request->abt_up.is_sq;
                riu->frequence = request->abt_up.frequence;
                riu->port = request->abt_up.port;
                riu->volume = request->abt_up.volume;
                
                strncpy(riu->multicast_ip, request->abt_up.multicast_ip, sizeof(request->abt_up.multicast_ip));
                riu->stream_port = request->abt_up.stream_port;

                LL_SEARCH(udata->r_head, r_node, riu, cmp_id_riu);

                if (r_node == NULL){
                    rius_append( &udata->r_head, riu);
                }
                else {
                    DL_REPLACE_ELEM(udata->r_head, r_node, riu);
                }
                /////////////
                for (i = 0; i < MAX_DEVICE; i++ ){
                    if (udata->rclient_data[i]->is_used == 0) {
                        strncpy(udata->rclient_data[i]->username, request->abt_up.username, sizeof(udata->rclient_data[i]->username));
                        strcat(ip_addr,request->abt_up.ip_addr);
                        riu_client_open(udata->rclient_data[i]->rclient, ip_addr);
                        udata->rclient_data[i]->is_used = 1;
                        break;
                    }
                    if (0 == strcmp(udata->rclient_data[i]->username, request->abt_up.username))
                        break;
                }
            }
            //SAVE TO LIST DONE
            // NOW SEND THE LIST TO OIUC ON MULTICAST
            // (MOVED INTO ARBITER-FUNC.C)
            break;
        case ABT_PTT:
        /*
            if (0 == strcmp(request->abt_ptt.type, "OIU")) {
                req.msg_id = RIUC_PTT;

                strncpy(req.riuc_ptt.cmd_ptt, request->abt_ptt.cmd_ptt, sizeof(req.riuc_ptt.cmd_ptt));
                char *list = strdup(request->abt_ptt.list);
                char *device, *port;
                while (list != NULL) {
                    device = strsep(&list, ":");
                    port = strchr(device, '_');
                    *port='\0';
                    port++;
                    // puts(device);
                    // puts(port);;
                    strncpy(req.riuc_ptt.port_ptt, port, sizeof(req.riuc_ptt.port_ptt));
                    for (i = 0; i < MAX_DEVICE; i++) {
                        //printf("username = %s, id = %d\n", udata->rclient_data[i]->username, i);
                        if (0 == strcmp(udata->rclient_data[i]->username, device)) {
                            riu_client_send(udata->rclient_data[i]->rclient, &req);
                        }
                    }
                }
            }
            else if (0 == strcmp(request->abt_ptt.type, "RIU")){
                req.msg_id = OIUC_PTT;

                req.oiuc_ptt.ptt_on = request->abt_up.ptt_on;
                strncpy(req.oiuc_ptt.ptt_name, request->abt_ptt.ptt_name, sizeof(request->abt_ptt.ptt_name));
                strncpy(req.oiuc_ptt.sdp_ip, request->abt_up.sdp_ip, sizeof(request->abt_up.sdp_ip));
                req.oiuc_ptt.sdp_port = request->abt_up.sdp_port;

                oiu_client_send(udata->oclient, &req);
            }
            break;
        */
        default:
            printf("Unknow request. Exit now\n");
            exit(-1);
    }
}

static void on_init_done(arbiter_server_t *aserver) {
    int i;
    arbiter_data_t *adata = malloc(sizeof(arbiter_data_t));

    adata->o_head = malloc(sizeof(oiu_t));
    adata->r_head = malloc(sizeof(riu_t));

    oius_init(&adata->o_head);
    rius_init(&adata->r_head);

    aserver->user_data =  adata;

    for (i = 0; i < MAX_DEVICE; i++) {
        adata->rclient_data[i] = malloc(sizeof(rclient_data_t));
        adata->rclient_data[i]->rclient = malloc(sizeof(riu_client_t));
    }
}

int main(int argc, char *argv[]) {
	arbiter_server_t aserver;
    oiu_client_t oclient;

    oiu_t *o_node, *o_temp;
    riu_t *r_node, *r_temp;

    char option[10];
	double recv_time;
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
	aserver.on_open_socket_f = NULL;
	arbiter_server_init(&aserver, argv[2]);
	
    arbiter_auto_send(&aserver);//Check list then send list on multicast

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
                        printf("(Online)\n");
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

	DL_FOREACH_SAFE(u_data->r_head, r_node, r_temp) {
		DL_DELETE(u_data->r_head, r_node);
		arbiter_delete_riu(r_node);
	}

	arbiter_server_end(&aserver);

	return 0;
}
