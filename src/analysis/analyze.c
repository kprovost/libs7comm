#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include "pcap.h"
#include <profinet_types.h>
#include <profinet_debug.h>

/*void dump_profinet_iso_header(const struct profinet_iso_header *h, const int len)
{
    printf("Protocol = 0x%02x\n", h->prot);
    assert(h->prot == PROFINET_ISO_PROTOCOL);

    printf("Length = %d (packet length %d)\n", h->len, len);
    switch (h->func)
    {
        case PROFINET_ISO_FUNCTION_PDU_TRANSPORT:
            printf("Function = PDU Transport\n");
            break;
        case PROFINET_ISO_FUNCTION_CONNECT:
            printf("Function = Connect to rack\n");
            break;
        default:
            printf("Protocol = UNKNOWN (0x%02x)\n", h->func);
    }
}

void dump_profinet_ibh_header(const struct profinet_ibh_header *ibh)
{
    printf("IBH channel: 0x%04x\n", ibh->channel);
    printf("IBH len: %d\n", ibh->len);
    printf("IBH seq: %d\n", ibh->seq);
    printf("IBH sflags: 0x%04x\n", ntohs(ibh->sflags));
    printf("IBH rflags: 0x%04x\n", ntohs(ibh->rflags));
}

void dump_profinet_pdu_header(const struct profinet_pdu_header *pdu)
{
    printf("PDU unknown: 0x%02x\n", pdu->unknown);
    printf("PDU version: %d\n", pdu->version);
    if (pdu->version == 1 || pdu->version == 3)
        printf("PDU result: 0x%04x\n", ntohs(pdu->res));
    printf("PDU unknown2: 0x%04x\n", ntohs(pdu->unknown2));
    printf("PDU unknown3: 0x%04x\n", ntohs(pdu->unknown3));
    printf("PDU plen: 0x%02x\n", ntohs(pdu->plen));
    printf("PDU dlen: 0x%02x\n", ntohs(pdu->dlen));
}

void dump_profinet_request(const struct profinet_request *r)
{
    printf("Function: ");
    switch (r->function)
    {
        case profinet_function_open_connection:
            printf("open connection");
            break;
        case profinet_function_read:
            printf("read");
            break;
        case profinet_function_write:
            printf("write");
            break;
        case profinet_function_download_request:
            printf("download request");
            break;
        case profinet_function_download_block:
            printf("download block");
            break;
        case profinet_function_download_ended:
            printf("download ended");
            break;
        case profinet_function_upload_start:
            printf("upload start");
            break;
        case profinet_function_upload:
            printf("upload");
            break;
        case profinet_function_upload_end:
            printf("upload end");
            break;
        case profinet_function_insert_block:
            printf("insert block");
            break;
        default:
            printf("unknown function 0x%02x", r->function);
    }
    printf("\n");

    printf("Function = 0x%02x\n", r->function);

    printf("Prefix = 0x%04x\n", ntohs(r->prefix));
    printf("Read size = 0x%02x\n", r->read_size);
    printf("bytes = 0x%02x\n", r->bytes);
    printf("db_num = 0x%04x\n", ntohs(r->db_num));
    printf("area_code = %s\n", profinet_area_to_string(r->area_code));

    uint32_t start_addr = (r->start_addr << 24) | ntohs(r->start_addr_2);
    printf("start_addr = 0x%06x\n", start_addr);
}

void pcap_parse_profinet_request(u_char *user, const u_char *bytes, const int len)
{
    if (len < sizeof(struct profinet_request))
    {
        printf("Runt request!\n");
        return;
    }

    struct profinet_request *r = (struct profinet_request*)bytes;
    dump_profinet_request(r);
}

void pcap_parse_profinet_pdu(u_char *user, const u_char *bytes, const int len)
{
    if (len < sizeof(struct profinet_pdu_header))
    {
        printf("Runt packet!\n");
        return;
    }

    struct profinet_pdu_header *pdu = (struct profinet_pdu_header*)bytes;
    dump_profinet_pdu_header(pdu);

    int header_len = sizeof(struct profinet_pdu_header);
    if (pdu->version != 2 && pdu->version != 3)
    {
        // Only version 2 and 3 are 12 bytes. The others are only 10 bytes.
        header_len = sizeof(struct profinet_pdu_header) - 2;
    }

    pcap_parse_profinet_request(user, bytes + header_len, len - header_len);
}

void pcap_parse_profinet(u_char *user, const u_char *bytes, const int len)
{
    struct profinet_iso_header *iso = (struct profinet_iso_header*)bytes;
    if (iso->prot != PROFINET_ISO_PROTOCOL)
        return;

    dump_profinet_iso_header(iso, len);

    if (iso->func == PROFINET_ISO_FUNCTION_PDU_TRANSPORT)
    {
        pcap_parse_profinet_pdu(user, bytes + sizeof(struct profinet_iso_header),
                len - sizeof(struct profinet_iso_header));
    }
}

void pcap_parse_tcp(u_char *user, const u_char *bytes, const int len)
{
    assert(! user);

    if (len < sizeof(struct tcphdr))
        return;

    struct tcphdr *tcph = (struct tcphdr*)bytes;
    uint16_t dst_port = ntohs(tcph->dest);
    uint16_t src_port = ntohs(tcph->source);
    int hdr_len = tcph->doff * 4;
    assert(hdr_len >= sizeof(struct tcphdr));

    int payload_len = len - hdr_len;
    if (payload_len < sizeof(struct profinet_request))
        return;

    if (dst_port == PROFINET_PORT)
    {
        printf("===== REQUEST ==========================\n");
        pcap_parse_profinet(user, bytes + hdr_len,
                payload_len);
    }
    else if (src_port == PROFINET_PORT)
    {
        printf("===== RESPONSE =========================\n");
        pcap_parse_profinet(user, bytes + hdr_len,
                payload_len);
    }
    else
        printf("Unknown connection at dest port = %d, src_port = %d\n",
                dst_port, src_port);
}*/

static void dump_bytes(const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x ", bytes[i]);

        if (i % 8 == 7)
            printf("  ");

        if (i % 16 == 15)
            printf("\n");
    }
}

static err_t analyze_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(! user);
    printf("Packet:\n");
    dump_bytes(ppkt_payload(p), 32);

    return ERR_NONE;
}

int main(int argc, char** argv)
{
    struct pcap_dev_t *pdev;

    if (argc < 2)
    {
        printf("Usage: %s <file.pcap>\n", argv[0]);
        return 1;
    }

    pdev = pcap_connect(argv[1], &analyze_receive, NULL);

    err_t err = ERR_NONE;
    do
    {
        err = pcap_poll(pdev);
    } while (OK(err));

    pcap_disconnect(pdev);

    return 0;
}
