#include "tcp.h"

#include <stddef.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/tcp.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

struct tcp_dev_t
{
    int fd;
    ppkt_receive_function_t receive;
    void *user;
};

struct tcp_dev_t* tcp_connect(const char *addr, uint16_t port, ppkt_receive_function_t receive, void *user)
{
    assert(addr);
    assert(receive);

    struct tcp_dev_t *dev = NULL;
    struct hostent *hostent = NULL;

    hostent = gethostbyname(addr);
    if (! hostent)
        return NULL;

    if (! hostent->h_addr_list[0])
        return NULL;

    dev = malloc(sizeof(struct tcp_dev_t));
    if (! dev)
        return NULL;

    dev->fd = socket(AF_INET, SOCK_STREAM, 0);
    dev->user = user;
    dev->receive = receive;
    if (dev->fd == -1)
        goto error;

    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    memcpy(&in.sin_addr, hostent->h_addr_list[0], sizeof(struct in_addr));

    int ret = connect(dev->fd, (struct sockaddr*)&in,
            sizeof(struct sockaddr_in));
    if (ret == -1)
        goto error;

    int flag = 1;
    ret = setsockopt(dev->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if (ret == -1)
        goto error;

    return dev;

error:
    if (dev && dev->fd != -1)
        close(dev->fd);

    free(dev);
    return NULL;
}

void tcp_disconnect(struct tcp_dev_t *dev)
{
    assert(dev);
    close(dev->fd);
    free(dev);
}

err_t tcp_send(struct tcp_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(dev->fd != -1);
    assert(p);

    size_t size = ppkt_chain_size(p);

    size_t pkts = ppkt_chain_count(p);
    assert(pkts >= 1);
    struct iovec *iov = malloc(sizeof(struct iovec) * pkts);
    if (! iov)
        return ERR_NO_MEM;

    for (size_t i = 0; i < pkts; i++)
    {
        iov[i].iov_base = ppkt_payload(p);
        iov[i].iov_len = ppkt_size(p);

        p = ppkt_next(p);
    }

    ssize_t ret = writev(dev->fd, iov, pkts);
    assert(size == (size_t)ret);

    free(iov);

    return ret == -1 ? ERR_SEND_FAILED : ERR_NONE;
}

err_t tcp_poll(struct tcp_dev_t *dev)
{
    assert(dev);
    assert(0); // Not implemented yet
    return ERR_UNKNOWN;
}
