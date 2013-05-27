#include "ppkt.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct ppkt_t
{
    uint8_t       *payload;
    unsigned int   size;
    unsigned int   offset;
    struct ppkt_t *next;
};

struct ppkt_t *ppkt_create(unsigned int size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = malloc(size);
    p->size = size;

    return p;
}

void ppkt_free(struct ppkt_t *p)
{
    if (! p)
        return;

    assert(p->payload);

    if (p->next)
        ppkt_free(p->next);

    free(p->payload);
    free(p);
}

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p)
{
    p->next = hdr;
    return p;
}

uint8_t* ppkt_payload(struct ppkt_t *p)
{
    return p->payload + p->offset;
}

unsigned int ppkt_length(struct ppkt_t *p)
{
    assert(p);
    unsigned int length = 0;

    while (p)
    {
        length += p->size;
        p = p->next;
    }

    return length;
}

profinet_err_t ppkt_send(int fd, struct ppkt_t *p)
{
    assert(fd != -1);
    assert(p);

    unsigned int offset = 0;

    while (offset < p->size)
    {
        int ret = write(fd, p->payload + offset, p->size - offset);
        if (ret == -1)
            return PROFINET_ERR_SEND_FAILED;

        offset += ret;
    }

    if (p->next)
        return ppkt_send(fd, p->next);

    return PROFINET_ERR_NONE;
}
