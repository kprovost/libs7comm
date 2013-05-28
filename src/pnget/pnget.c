#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "profinet.h"

int main(int argc, char **argv)
{
    struct profinet_dev *dev = profinet_connect("10.0.3.9");
    if (! dev)
    {
        printf("Failed to connect: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    uint16_t temp = 0;
    profinet_err_t err = profinet_read_word(dev, 10, 4, &temp);
    if (! PROFINET_OK(err))
    {
        printf("Failed to read temperature\n");
        goto exit;
    }

    printf("Temperature: %f\n", ((int16_t)temp) / 10.0);

    uint16_t setpoint = 0;
    err = profinet_read_word(dev, 10, 6, &setpoint);
    if (! PROFINET_OK(err))
    {
        printf("Failed to read temperature\n");
        goto exit;
    }
    printf("Temperature setpoint: %f\n", ((int16_t)setpoint) / 10.0);

exit:
    profinet_disconnect(dev);

    return 0;
}
