#include "ppkt.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

#define PPKT_FLAG_ALLOCATED 1

struct ppkt_t
{
    uint8_t       *payload;
    size_t         size;
    size_t         offset;
    struct ppkt_t *next;
    uint8_t        flags;
};

struct ppkt_t *ppkt_alloc(size_t size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = malloc(size);
    p->size = size;
    p->flags = PPKT_FLAG_ALLOCATED;

    return p;
}

struct ppkt_t *ppkt_create(uint8_t *data, size_t size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = data;
    p->size = size;
    p->flags = 0;

    return p;
}

void ppkt_free(struct ppkt_t *p)
{
    if (! p)
        return;

    assert(p->payload);

    if (p->next)
        ppkt_free(p->next);

    if (p->flags & PPKT_FLAG_ALLOCATED)
            free(p->payload);
    free(p);
}

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p)
{
    assert(p);
    assert(hdr);

    hdr->next = p;
    return hdr;
}

uint8_t* ppkt_payload(struct ppkt_t *p)
{
    assert(p);
    return p->payload + p->offset;
}

size_t ppkt_size(struct ppkt_t *p)
{
    assert(p);
    return p->size - p->offset;
}

size_t ppkt_chain_size(struct ppkt_t *p)
{
    assert(p);
    size_t length = 0;

    while (p)
    {
        length += ppkt_size(p);
        p = p->next;
    }

    return length;
}

size_t ppkt_chain_count(struct ppkt_t *p)
{
    assert(p);
    size_t length = 0;

    while (p)
    {
        length++;
        p = p->next;
    }

    return length;
}

struct ppkt_t* ppkt_next(struct ppkt_t *p)
{
    assert(p);
    return p->next;
}

void ppkt_pull(struct ppkt_t *p, size_t size)
{
    assert(p);
    assert(size <= ppkt_size(p));

    p->offset += size;
}
