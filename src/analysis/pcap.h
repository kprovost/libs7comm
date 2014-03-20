#ifndef _PCAP_H_
#define _PCAP_H_

#include "ppkt.h"
#include "proto.h"

struct proto_t pcap_proto;

void* pcap_connect(const char *filename, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos);
void pcap_disconnect(void *dev);

err_t pcap_poll(void *dev);

#endif
