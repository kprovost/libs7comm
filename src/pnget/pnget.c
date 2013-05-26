#include <stdio.h>
#include <stdint.h>
#include "profinet.h"

int main(int argc, char **argv)
{
    struct profinet_dev *dev = profinet_connect("10.0.3.9");
    if (! dev)
    {
        printf("Failed to connect\n");
        return 1;
    }

    int16_t temp = profinet_read_word(dev, 10, 4);
    printf("Temperature: %f\n", temp / 10.0);

    profinet_disconnect(dev);

    return 0;
}
