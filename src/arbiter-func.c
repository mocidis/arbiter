#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utlist.h"
#include "arbiter-func.h"
#include "arbiter-data.h"
#include "oiu-client.h"

#include "time.h"

void *oiu_server_proc(void *param) {
    arbiter_server_t *aserver = (arbiter_server_t *)param;
    arbiter_data_t *udata = (arbiter_data_t *)aserver->user_data;
    oiu_t *o_node, *o_temp;
    riu_t *r_node, *r_temp;

    time_t timer;
    struct tm * timeinfo;

    oiu_client_t *oclient = udata->oclient;
    oiu_request_t areq;

    // NOW SEND THE LIST TO OIUC BY MULTICAST
    while(1) {
        time(&timer);
        if (udata->o_head != NULL){
            DL_FOREACH_SAFE(udata->o_head, o_node, o_temp) {
                areq.msg_id = OIUC_GB;

                strncpy(areq.oiuc_gb.type, "OIUC", sizeof(areq.oiuc_gb.type));

                if (o_node->is_online == 1 && (timer - o_node->recv_time ) < 15)
                    areq.oiuc_gb.is_online = 1;
                else
                    areq.oiuc_gb.is_online = 0;
                strncpy(areq.oiuc_gb.id, o_node->id, sizeof(areq.oiuc_gb.id));

                timeinfo = localtime (&o_node->recv_time);
                strftime (areq.oiuc_gb.timestamp, 20,"%H:%M:%S", timeinfo);                           

                oiu_client_send(oclient, &areq);
            }
        }
        if (udata->r_head != NULL){
            DL_FOREACH_SAFE(udata->r_head, r_node, r_temp) {
                areq.msg_id = OIUC_GB;

                strncpy(areq.oiuc_gb.type, "RIUC", sizeof(areq.oiuc_gb.type));

                if (r_node->is_online == 1 && (timer - r_node->recv_time ) < 15)
                    areq.oiuc_gb.is_online = 1;
                else
                    areq.oiuc_gb.is_online = 0;
                strncpy(areq.oiuc_gb.id, r_node->id, sizeof(areq.oiuc_gb.id));

                areq.oiuc_gb.n_ports = r_node->n_ports;

                timeinfo = localtime (&r_node->recv_time);
                strftime (areq.oiuc_gb.timestamp, 20,"%H:%M:%S", timeinfo);                           

                oiu_client_send(oclient, &areq);
            }
        }
        sleep(5);
    }
}

void arbiter_auto_send(arbiter_server_t *aserver) {
    pthread_create(&aserver->master_thread, NULL, oiu_server_proc, aserver);
}
