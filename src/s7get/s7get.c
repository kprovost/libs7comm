#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "s7comm.h"

static void help(const char *name)
{
    printf("Usage: %s -a <ip> -t [md] -d <db> -n <number>\n", name);
    printf("       m: Merker (bit)\n");
    printf("       d: Data word\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char *ip = NULL;
    int db = -1;
    int num = -1;
    char type = 'd';

    int opt;
    while ((opt = getopt(argc, argv, "t:a:d:n:h")) != -1)
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
            case 't':
                type = optarg[0];
                break;
            case 'h':
            default:
                help(argv[0]);
        }
    }

    if (! ip || db == -1 || num == -1)
        help(argv[0]);

    if (type != 'm' && type != 'd')
    {
        printf("Invalid type %c\n", type);
        help(argv[0]);
    }

    struct s7comm_dev_t *dev = s7comm_connect(ip);
    if (! dev)
    {
        printf("Failed to connect: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    if (type == 'm')
    {
        bool value = 0;
        err_t err = s7comm_read_flag_bit(dev, num, &value);
        if (! OK(err))
        {
            printf("Failed to read\n");
            goto exit;
        }
        printf("Value: 0x%01x\n", value);
    } else if (type == 'd')
    {
        uint16_t value = 0;
        err_t err = s7comm_read_db_word(dev, db, num, &value);
        if (! OK(err))
        {
            printf("Failed to read\n");
            goto exit;
        }

        printf("Value: 0x%04x\n", value);
    }
    else
        assert(false);

exit:
    s7comm_disconnect(dev);

    return 0;
}
