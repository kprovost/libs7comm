#include "pdu.h"
#include "profinet_types.h"
#include "iso.h"
#include <arpa/inet.h>
#include <assert.h>

profinet_err_t profinet_pdu_send(struct profinet_dev *dev, struct ppkt_t **p)
{
    assert(p);
    assert(*p);

    struct ppkt_t *pduhdr = ppkt_create(sizeof(struct profinet_pdu_header));
    if (! pduhdr)
        return PROFINET_ERR_NO_MEM;

    struct profinet_pdu_header *hdr = (struct profinet_pdu_header*)ppkt_payload(pduhdr);

    *p = ppkt_prefix_header(pduhdr, *p);
    assert(*p);

    hdr->unknown = 0x32; // TODO constantify
    hdr->version = 3;
    hdr->unknown2 = 0;
    hdr->unknown3 = 0;
    hdr->plen = htons(ppkt_chain_size(*p));
    hdr->dlen = 0;

    return profinet_iso_send(dev, p);
}
