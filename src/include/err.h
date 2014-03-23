#ifndef _ERR_H_
#define _ERR_H_

typedef enum
{
    ERR_NONE,
    ERR_SEND_FAILED,
    ERR_RECV_FAILED,
    ERR_CONNECTION_FAILED,
    ERR_CONNECTION_CLOSED,
    ERR_TIMEOUT,
    ERR_READ_FAILURE,
    ERR_WRITE_FAILURE,
    ERR_NO_SUCH_VALUE,
    ERR_NOT_SUPPORTED,
    ERR_UNKNOWN
} err_t;
#define OK(x) (x == ERR_NONE)

const char* err_to_string(const err_t err);

#endif
