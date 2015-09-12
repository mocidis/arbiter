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
    oiu_t *oiu, *o_node, *o_temp;
    int n;
    time_t timer;

    oiu_client_t *oclient = udata->oclient;
    oiu_request_t areq;

    // NOW SEND THE LIST TO OIUC BY MULTICAST
    while(1) {
        time(&timer);
        if (udata->o_head != NULL){
            DL_FOREACH_SAFE(udata->o_head, o_node, o_temp) {
                areq.msg_id = OIUC_GB;
                if (o_node->is_online == 1 && (timer - o_node->recv_time ) < 15) //Need more condition
                    areq.oiuc_gb.is_alive = 1;
                else
                    areq.oiuc_gb.is_alive = 0;
                strncpy(areq.oiuc_gb.id, o_node->id, sizeof(areq.oiuc_gb.id));
                oiu_client_send(oclient, &areq);
            }
        }
        sleep(5);
    }
}

void arbiter_auto_send(arbiter_server_t *aserver) {
    pthread_create(&aserver->master_thread, NULL, oiu_server_proc, aserver);
}
