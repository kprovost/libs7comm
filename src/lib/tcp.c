#include "tcp.h"

#include <stddef.h>
#include <stdlib.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define TPKT_PORT 102

struct tcp_dev_t
{
    int fd;
    ppkt_receive_function_t receive;
    void *user;
};

void* tcp_connect(const char *addr, ppkt_receive_function_t receive,
        void *user, struct proto_t *lower_layer)
{
    assert(addr);
    assert(receive);
    assert(! lower_layer);

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
    in.sin_port = htons(TPKT_PORT);
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

void tcp_disconnect(void *d)
{
    assert(d);
    struct tcp_dev_t *dev = (struct tcp_dev_t*)d;
    close(dev->fd);
    free(dev);
}

err_t tcp_send(void *d, struct ppkt_t *p)
{
    assert(d);
    struct tcp_dev_t *dev = (struct tcp_dev_t*)d;
    assert(dev->fd != -1);
    assert(p);

    size_t size = ppkt_chain_size(p);

    size_t pkts = ppkt_chain_count(p);
    assert(pkts >= 1);
    struct iovec *iov = malloc(sizeof(struct iovec) * pkts);
    if (! iov)
        return ERR_NO_MEM;

    struct ppkt_t *it = p;
    for (size_t i = 0; i < pkts; i++)
    {
        iov[i].iov_base = ppkt_payload(it);
        iov[i].iov_len = ppkt_size(it);

        it = ppkt_next(it);
    }

    ssize_t ret = writev(dev->fd, iov, pkts);
    assert(size == (size_t)ret);

    free(iov);

    ppkt_free(p);

    return ret == -1 ? ERR_SEND_FAILED : ERR_NONE;
}

err_t tcp_poll(void *d)
{
    assert(d);
    struct tcp_dev_t *dev = (struct tcp_dev_t*)d;

    struct pollfd pfd = {
        .fd = dev->fd,
        .events = POLLIN
    };
    int ret = poll(&pfd, 1, -1);
    assert(ret >= 0);

    if (! pfd.revents)
        return ERR_TIMEOUT;

    if (pfd.revents & (POLLHUP | POLLERR))
        return ERR_CONNECTION_CLOSED;

    struct ppkt_t *p = ppkt_alloc(512);

    ret = recv(dev->fd, ppkt_payload(p), ppkt_size(p), 0);
    if (ret <= 0)
    {
        ppkt_free(p);
        return ERR_RECV_FAILED;
    }

    // Ensure the ppkt size matches the real data size
    ppkt_cut(p, ppkt_size(p) - ret);
    assert(ppkt_size(p) == ret);

    return dev->receive(p, dev->user);
}

struct proto_t tcp_proto = {
    .name = "TCP",
    .proto_connect = tcp_connect,
    .proto_disconnect = tcp_disconnect,
    .proto_receive = NULL,
    .proto_send = tcp_send,
    .proto_poll = tcp_poll
};
