#ifndef _PDU_H_
#define _PDU_H_

#include <stdint.h>
#include "ppkt.h"

profinet_err_t profinet_pdu_send(struct profinet_dev *dev, struct ppkt_t **p);

#endif
