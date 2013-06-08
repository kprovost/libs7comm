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
    assert(p != hdr);

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

    assert(it != footer);
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
        size_t to_add = size - offset;
        if (to_add > p_size)
            to_add = p_size;

        memcpy(new->payload + offset, ppkt_payload(p), size);
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

    assert(new != p);
    new->next = p;

    return new;
}

static void ppkt_split_ppkt(struct ppkt_t *front, struct ppkt_t **back, size_t cut)
{
    assert(front);
    assert(back);
    assert(cut < ppkt_size(front));

    struct ppkt_t *new = ppkt_alloc(ppkt_size(front) - cut);
    memcpy(ppkt_payload(new), ppkt_payload(front) + cut, ppkt_size(front) - cut);

    ppkt_cut(front, ppkt_size(front) - cut);

    assert(new != front->next);
    new->next = front->next;
    *back = new;
    front->next = NULL;
}

void ppkt_split(struct ppkt_t *front, struct ppkt_t **back, size_t cut)
{
    assert(front);
    assert(back);

    struct ppkt_t *it = front;
    size_t offset = 0;

    while ((offset + ppkt_size(it)) < cut)
    {
        offset += ppkt_size(it);
        it = it->next;
        assert(it); // I.e. our chain is at least 'cut' long
    }

    if ((offset + ppkt_size(it)) == cut)
    {
        // Simple case, we can cut the chain between the packets
        *back = it->next;
        it->next = NULL;
        return;
    }
    assert(offset < cut);

    ppkt_split_ppkt(it, back, cut - offset);
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
    assert(size <= ppkt_size(p)); // Doesn't support chains yet

    p->size -= size;
}
