#include "pcap.h"
#include "ppkt.h"
#include "err.h"

#include <assert.h>
#include <stdlib.h>
#include <pcap/pcap.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

struct pcap_dev_t
{
    pcap_t *pcap;
    ppkt_receive_function_t receive;
};

struct pcap_dev_t* pcap_connect(const char *filename, ppkt_receive_function_t receive)
{
    assert(filename);
    assert(receive);

    char errbuf[PCAP_ERRBUF_SIZE];

    struct pcap_dev_t *dev = malloc(sizeof(struct pcap_dev_t));    
    if (! dev)
        return NULL;

    dev->receive = receive;
    dev->pcap = pcap_open_offline(filename, errbuf);
    if (! dev->pcap)
    {
        free(dev);
        return NULL;
    }

    return dev;
}

void pcap_disconnect(struct pcap_dev_t *dev)
{
    assert(dev);
    pcap_close(dev->pcap);
    free(dev);
}

static void pcap_receive_tcp(struct pcap_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(p);
}

static void pcap_receive_ipv4(struct pcap_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(p);

    if (ppkt_size(p) < sizeof(struct iphdr))
        return;

    struct iphdr *iph = (struct iphdr*)ppkt_payload(p);
    assert(iph->version == 4);

    uint8_t ip_proto = iph->protocol;
    uint8_t header_len = iph->ihl * 4;

    ppkt_pull(p, header_len);

    if (ip_proto == IPPROTO_TCP)
        pcap_receive_tcp(dev, p);
}

static void pcap_receive(u_char *user, const struct pcap_pkthdr *h,
        const u_char *bytes)
{
    assert(user);
    struct pcap_dev_t *dev = (struct pcap_dev_t*)user;

    // Turn this into a ppkt_t for future processing
    struct ppkt_t *p = ppkt_create((uint8_t*)bytes, h->caplen);

    if (ppkt_size(p) < sizeof(struct ethhdr))
        goto done;
    
    struct ethhdr *eh = (struct ethhdr*)ppkt_payload(p);
    uint16_t eth_proto = ntohs(eh->h_proto);
    if (eth_proto != ETH_P_IP)
        goto done;

    ppkt_pull(p, sizeof(struct ethhdr));

    pcap_receive_ipv4(dev, p);

done:
    ppkt_free(p);
}

err_t pcap_poll(struct pcap_dev_t *dev)
{
    assert(dev);
    assert(dev->pcap);

    int ret = pcap_dispatch(dev->pcap, 1, pcap_receive, (void*)dev);
    if (ret <= 0)
        return ERR_CONNECTION_CLOSED;

    return ERR_NONE;
}

