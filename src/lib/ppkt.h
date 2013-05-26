#ifndef _PPKT_H_
#define _PPKT_H_

#include <stdint.h>
#include "profinet.h"

struct ppkt_t;

struct ppkt_t *ppkt_create(unsigned int size);
void ppkt_free(struct ppkt_t *p);

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p);
uint8_t* ppkt_payload(struct ppkt_t *p);
unsigned int ppkt_length(struct ppkt_t *p);

profinet_err_t ppkt_send(int fd, struct ppkt_t *p);

#endif
