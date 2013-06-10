#ifndef _ERR_H_
#define _ERR_H_

typedef enum
{
    ERR_NONE,
    ERR_NO_MEM,
    ERR_SEND_FAILED,
    ERR_RECV_FAILED,
    ERR_CONNECTION_CLOSED,
    ERR_TIMEOUT,
    ERR_NOT_SUPPORTED,
    ERR_UNKNOWN
} err_t;
#define OK(x) (x == ERR_NONE)

#endif
