#include "iso.h"
#include "profinet.h"
#include "profinet_types.h"
#include <arpa/inet.h>
#include <assert.h>

profinet_err_t profinet_iso_send(struct profinet_dev *dev, struct ppkt_t **p)
{
    assert(p);
    assert(*p);

    struct ppkt_t *isohdr = ppkt_create(sizeof(struct profinet_iso_header));
    if (! isohdr)
        return PROFINET_ERR_NO_MEM;

    struct profinet_iso_header *hdr = (struct profinet_iso_header*)ppkt_payload(isohdr);

    hdr->prot = PROFINET_ISO_PROTOCOL;
    hdr->ch1 = 0;
    hdr->ch2 = 0;
    hdr->xxxx1 = 0;
    hdr->func = PROFINET_ISO_FUNCTION_PDU_TRANSPORT;
    hdr->xxxx2 = 0;

    *p = ppkt_prefix_header(isohdr, *p);
    assert(*p);

    hdr->len = ppkt_chain_size(*p);

    return ppkt_send(dev->fd, *p);
}
