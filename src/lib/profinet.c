#include "profinet.h"
#include "profinet_types.h"
#include <stddef.h>
#include <malloc.h>
#include <assert.h>

struct profinet_dev
{
    int fd;
};

struct profinet_dev* profinet_connect(const char *plc_addr)
{
    assert(plc_addr);
    return NULL;
}

void profinet_disconnect(struct profinet_dev *dev)
{
    assert(dev);
    free(dev);
}

uint16_t profinet_read_word(struct profinet_dev *dev, int db, int number)
{
    assert(dev);
    return 0;
}
