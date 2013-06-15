#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "profinet.h"

static void help(const char *name)
{
    printf("Usage: %s -a <ip> -d <db> -n <number>\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char *ip = NULL;
    int db = -1;
    int num = -1;

    int opt;
    while ((opt = getopt(argc, argv, "a:d:n:h")) != -1)
    {
        switch (opt)
        {
            case 'a':
                ip = optarg;
                break;
            case 'd':
                db = atoi(optarg);
                break;
            case 'n':
                num = atoi(optarg);
                break;
            case 'h':
            default:
                help(argv[0]);
        }
    }

    if (! ip || db == -1 || num == -1)
        help(argv[0]);

    struct profinet_dev_t *dev = profinet_connect(ip);
    if (! dev)
    {
        printf("Failed to connect: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    uint16_t value = 0;
    err_t err = profinet_read_word(dev, db, num, &value);
    if (! OK(err))
    {
        printf("Failed to read\n");
        goto exit;
    }

    printf("Value: 0x%04x\n", value);

exit:
    profinet_disconnect(dev);

    return 0;
}
