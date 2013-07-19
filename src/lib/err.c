#include "err.h"
#include <assert.h>
#include <stddef.h>

const char* err_to_string(const err_t err)
{
    switch (err)
    {
        case ERR_NONE:
            return "No error";
        case ERR_NO_MEM:
            return "Out of memory";
        case ERR_SEND_FAILED:
            return "Send failed";
        case ERR_RECV_FAILED:
            return "Receive failed";
        case ERR_CONNECTION_CLOSED:
            return "Connection closed";
        case ERR_TIMEOUT:
            return "Timeout";
        case ERR_READ_FAILURE:
            return "Read failure";
        case ERR_WRITE_FAILURE:
            return "Write failure";
        case ERR_NO_SUCH_VALUE:
            return "Variable does not exist";
        case ERR_NOT_SUPPORTED:
            return "Not supported";
        case ERR_UNKNOWN:
            return "Unknown";
    }
    assert(0);
    return NULL;
}

