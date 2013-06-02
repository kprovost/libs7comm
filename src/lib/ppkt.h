#ifndef _PPKT_H_
#define _PPKT_H_

#include <stdint.h>
#include <stddef.h>
#include "err.h"

struct ppkt_t;

typedef err_t (*ppkt_receive_function_t)(struct ppkt_t *p);

struct ppkt_t *ppkt_alloc(size_t size);
struct ppkt_t *ppkt_create(uint8_t *data, size_t size);
void ppkt_free(struct ppkt_t *p);

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p);

uint8_t* ppkt_payload(struct ppkt_t *p);

/* Size of this packet, without chained packets */
size_t ppkt_size(struct ppkt_t *p);

/* Total size of the chain */
size_t ppkt_chain_size(struct ppkt_t *p);

/* Number of ppkt_t in the chain */
size_t ppkt_chain_count(struct ppkt_t *p);

struct ppkt_t* ppkt_next(struct ppkt_t *p);

#endif
