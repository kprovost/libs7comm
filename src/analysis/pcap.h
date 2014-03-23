#ifndef _PCAP_H_
#define _PCAP_H_

#include "ppkt.h"
#include "proto.h"

struct proto_t pcap_proto;

void* pcap__open(const char *filename, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos);
err_t pcap__connect(void *dev);
void pcap__disconnect(void *dev);
void pcap__close(void *dev);

err_t pcap__poll(void *dev);

#endif
