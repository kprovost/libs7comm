#ifndef _PROFINET_H_
#define _PROFINET_H_

#include <stdint.h>

struct profinet_dev;

typedef enum
{
    PROFINET_ERR_NONE,
    PROFINET_ERR_UNKNOWN
} profinet_err_t;
#define PROFINET_OK(x) (x == PROFINET_ERR_NONE)

struct profinet_dev* profinet_connect(const char *plc_addr);
void profinet_disconnect(struct profinet_dev *dev);

profinet_err_t profinet_read_word(struct profinet_dev *dev, int db, int number, uint16_t *value);

#endif
