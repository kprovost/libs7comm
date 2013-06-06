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

struct ppkt_t *ppkt_append_footer(struct ppkt_t *footer, struct ppkt_t *p)
{
    assert(footer);

    if (! p)
        return footer;

    struct ppkt_t *it = p;
    while (it->next) it = it->next;
    it->next = footer;

    return p;
}

struct ppkt_t *ppkt_coalesce(struct ppkt_t *p, size_t size)
{
    assert(p);
    assert(size > 0);

    assert(ppkt_chain_size(p) >= size);

    if (ppkt_size(p) >= size)
        return p;

    struct ppkt_t *new = ppkt_alloc(size);

    size_t offset = 0;
    while (offset < size)
    {
        size_t p_size = ppkt_size(p);
        size_t to_add = size < p_size ? size : p_size;

        memcpy(new->payload, ppkt_payload(p), size);
        offset += to_add;

        if (to_add == p_size)
        {
            struct ppkt_t *tmp = p;
            p = p->next;
            tmp->next = NULL;
            ppkt_free(tmp);
        }
        else
        {
            ppkt_pull(p, to_add);
        }
    }

    new->next = p;

    return new;
}

void ppkt_split(struct ppkt_t *front, struct ppkt_t **back, size_t cut)
{
    assert(front);
    assert(back);

    struct ppkt_t *it = front;
    size_t offset = 0;
    while (it)
    {
        size_t pkt_size = ppkt_size(it);

        if (offset + pkt_size >= cut)
        {
            *back = it->next;
            size_t diff = cut - (offset + pkt_size);
            if (*back)
                ppkt_pull(*back, diff);
            assert(it->size >= diff);
            it->size -= diff;
            return;
        }

        it = it->next;
        offset += ppkt_size(it);
    }
    assert(0);
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

void ppkt_cut(struct ppkt_t *p, size_t size)
{
    assert(p);
    assert(! p->next); // Doesn't support chains yet
    assert(size <= ppkt_size(p));

    p->size -= size;
}
