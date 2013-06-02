#ifndef _PDU_H_
#define _PDU_H_

#include <stdint.h>
#include "ppkt.h"
#include "profinet.h"

err_t profinet_pdu_send(struct profinet_dev *dev, struct ppkt_t **p);

#endif
