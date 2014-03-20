#ifndef _PROTO_H_
#define _PROTO_H_

#include "ppkt.h"

#define PROTO_NAME_MAX_LEN 32

struct proto_t;
typedef struct proto_t* proto_stack_t;

/**
 * Protocol descriptor.
 *
 * Intended to allow us to stitch protocol layers together more or less
 * arbitrarily.
 **/
struct proto_t
{
    char name[PROTO_NAME_MAX_LEN];

    void* (*proto_connect)(const char *addr, ppkt_receive_function_t func,
            void *user, proto_stack_t *protos);
    void (*proto_disconnect)(void* proto_dev);

    err_t (*proto_receive)(struct ppkt_t *p, void *user);
    err_t (*proto_send)(void *proto_dev, struct ppkt_t *p);
    err_t (*proto_poll)(void *proto_dev);
};

#endif
