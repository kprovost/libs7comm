#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <linux/if_ether.h>

void pcap_parse_ip4(u_char *user, const u_char *bytes, const int len)
{
    assert(! user);
    printf("IP packet (len %d)\n", len);
}

void pcap_callback(u_char *user, const struct pcap_pkthdr *h,
        const u_char *bytes)
{
    assert(! user);

    if (h->caplen < sizeof(struct ethhdr))
        return;

    struct ethhdr *eh = (struct ethhdr*)bytes;
    uint16_t eth_proto = ntohs(eh->h_proto);
    switch (eth_proto)
    {
        case ETH_P_IP:
            pcap_parse_ip4(user, bytes + sizeof(struct ethhdr), h->caplen - sizeof(struct ethhdr));
            break;
        default:
            printf("Unknown ethernet protocol = %02x\n", eth_proto);
            break;
    }
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
