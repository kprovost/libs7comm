#include <assert.h>
#include <pcap/pcap.h>
#include <stdio.h>

void pcap_callback(u_char *user, const struct pcap_pkthdr *h,
        const u_char *bytes)
{
    assert(! user);
    printf("Packet!\n");
}

int main(int argc, char** argv)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    if (argc < 2)
    {
        printf("Usage: %s <file.pcap>\n", argv[0]);
        return 1;
    }

    pcap_t *p = pcap_open_offline(argv[1], errbuf);
    if (! p)
    {
        printf("Failed to open file %s: %s\n", argv[1], errbuf);
        return 1;
    }

    int ret = pcap_loop(p, 0, pcap_callback, NULL);
    if (ret < 0)
    {
        printf("pcap_loop error %d\n", ret);
        return 1;
    }

    return 0;
}
