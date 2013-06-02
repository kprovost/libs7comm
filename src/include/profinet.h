#ifndef _PROFINET_H_
#define _PROFINET_H_

#include <stdint.h>
#include "err.h"

struct profinet_dev;

struct profinet_dev* profinet_connect(const char *plc_addr);
void profinet_disconnect(struct profinet_dev *dev);

err_t profinet_read_word(struct profinet_dev *dev, int db, int number, uint16_t *value);

#endif
