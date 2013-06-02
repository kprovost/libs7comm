#ifndef _PCAP_H_
#define _PCAP_H_

#include "ppkt.h"

struct pcap_dev_t;

struct pcap_dev_t* pcap_connect(const char *filename, ppkt_receive_function_t receive, void *user);
void pcap_disconnect(struct pcap_dev_t *dev);

err_t pcap_poll(struct pcap_dev_t *dev);

#endif
