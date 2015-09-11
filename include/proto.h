#ifndef __PROTO_H__
#define __PROTO_H__
struct uproto_request_s {
	int acc_id;
	int ret_code;
	char username[50];
};

struct uproto_response_s {
	int code;
	char msg[20];
};
#endif
