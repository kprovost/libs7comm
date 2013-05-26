#include "profinet.h"
#include "profinet_types.h"
#include <stddef.h>
#include <malloc.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

struct profinet_dev
{
    int fd;
};

struct profinet_dev* profinet_connect(const char *plc_addr)
{
    assert(plc_addr);
    int ret;
    struct profinet_dev *dev = NULL;
    struct hostent *hostent = NULL;

    hostent = gethostbyname(plc_addr);
    if (! hostent)
        return NULL;

    if (! hostent->h_addr)
        return NULL;

    dev = malloc(sizeof(struct profinet_dev));
    if (! dev)
        return NULL;

    dev->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dev->fd == -1)
        goto error;

    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_port = htons(PROFINET_PORT);
    memcpy(&in.sin_addr, hostent->h_addr, sizeof(struct in_addr));

    ret = connect(dev->fd, (struct sockaddr*)&in,
            sizeof(struct sockaddr_in));
    if (ret == -1)
        goto error;

    return dev;

error:
    if (dev && dev->fd != -1)
        close(dev->fd);

    free(dev);

    return NULL;
}

void profinet_disconnect(struct profinet_dev *dev)
{
    assert(dev);
    close(dev->fd);
    free(dev);
}

profinet_err_t profinet_read_word(struct profinet_dev *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    return PROFINET_ERR_UNKNOWN;
}
