#ifndef _ISO_H_
#define _ISO_H_

#include <stdint.h>
#include "ppkt.h"

err_t profinet_iso_send(struct profinet_dev *dev, struct ppkt_t **p);

#endif
