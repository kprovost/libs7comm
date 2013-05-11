#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#define PROFINET_PORT 102

void pcap_parse_profinet_request(u_char *user, const u_char *bytes, const int len)
{

}

void pcap_parse_profinet_response(u_char *user, const u_char *bytes, const int len)
{

}

void pcap_parse_tcp(u_char *user, const u_char *bytes, const int len)
{
    assert(! user);

    if (len < sizeof(struct tcphdr))
        return;

    struct tcphdr *tcph = (struct tcphdr*)bytes;
    uint16_t dst_port = ntohs(tcph->dest);
    uint16_t src_port = ntohs(tcph->source);

    if (dst_port == PROFINET_PORT)
        pcap_parse_profinet_request(user, bytes + sizeof(struct tcphdr),
                len - sizeof(struct tcphdr));
    else if (src_port == PROFINET_PORT)
        pcap_parse_profinet_response(user, bytes + sizeof(struct tcphdr),
                len - sizeof(struct tcphdr));
    else
        printf("Unknown connection at dest port = %d, src_port = %d\n",
                dst_port, src_port);
}

void pcap_parse_ip4(u_char *user, const u_char *bytes, const int len)
{
    assert(! user);

    if (len < sizeof(struct iphdr))
        return;

    struct iphdr *iph = (struct iphdr*)bytes;
    assert(iph->version == 4);

    uint8_t ip_proto = iph->protocol;
    uint8_t header_len = iph->ihl * 4;

    switch (ip_proto)
    {
        case IPPROTO_TCP:
            pcap_parse_tcp(user, bytes + header_len, len - header_len);
            break;
        case IPPROTO_UDP:
        default:
            printf("Unknown IP protocol %d\n", ip_proto);
    }
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
        case ETH_P_ARP:
            // We don't care about arp.
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
