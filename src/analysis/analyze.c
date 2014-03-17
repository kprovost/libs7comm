#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include "cotp.h"
#include "tpkt.h"
#include "pcap.h"
#include <s7comm_types.h>
#include <s7comm_debug.h>
#include <ppkt.h>

/*void dump_s7comm_iso_header(const struct s7comm_iso_header *h, const int len)
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

void dump_s7comm_ibh_header(const struct s7comm_ibh_header *ibh)
{
    printf("IBH channel: 0x%04x\n", ibh->channel);
    printf("IBH len: %d\n", ibh->len);
    printf("IBH seq: %d\n", ibh->seq);
    printf("IBH sflags: 0x%04x\n", ntohs(ibh->sflags));
    printf("IBH rflags: 0x%04x\n", ntohs(ibh->rflags));
}

void dump_s7comm_pdu_header(const struct s7comm_pdu_header *pdu)
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

void dump_s7comm_request(const struct s7comm_request *r)
{
    printf("Function: ");
    switch (r->function)
    {
        case s7comm_function_open_connection:
            printf("open connection");
            break;
        case s7comm_function_read:
            printf("read");
            break;
        case s7comm_function_write:
            printf("write");
            break;
        case s7comm_function_download_request:
            printf("download request");
            break;
        case s7comm_function_download_block:
            printf("download block");
            break;
        case s7comm_function_download_ended:
            printf("download ended");
            break;
        case s7comm_function_upload_start:
            printf("upload start");
            break;
        case s7comm_function_upload:
            printf("upload");
            break;
        case s7comm_function_upload_end:
            printf("upload end");
            break;
        case s7comm_function_insert_block:
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
    printf("area_code = %s\n", s7comm_area_to_string(r->area_code));

    uint32_t start_addr = (r->start_addr << 24) | ntohs(r->start_addr_2);
    printf("start_addr = 0x%06x\n", start_addr);
}

void pcap_parse_s7comm_request(u_char *user, const u_char *bytes, const int len)
{
    if (len < sizeof(struct s7comm_request))
    {
        printf("Runt request!\n");
        return;
    }

    struct s7comm_request *r = (struct s7comm_request*)bytes;
    dump_s7comm_request(r);
}

void pcap_parse_s7comm_pdu(u_char *user, const u_char *bytes, const int len)
{
    if (len < sizeof(struct s7comm_pdu_header))
    {
        printf("Runt packet!\n");
        return;
    }

    struct s7comm_pdu_header *pdu = (struct s7comm_pdu_header*)bytes;
    dump_s7comm_pdu_header(pdu);

    int header_len = sizeof(struct s7comm_pdu_header);
    if (pdu->version != 2 && pdu->version != 3)
    {
        // Only version 2 and 3 are 12 bytes. The others are only 10 bytes.
        header_len = sizeof(struct s7comm_pdu_header) - 2;
    }

    pcap_parse_s7comm_request(user, bytes + header_len, len - header_len);
}

void pcap_parse_s7comm(u_char *user, const u_char *bytes, const int len)
{
    struct s7comm_iso_header *iso = (struct s7comm_iso_header*)bytes;
    if (iso->prot != PROFINET_ISO_PROTOCOL)
        return;

    dump_s7comm_iso_header(iso, len);

    if (iso->func == PROFINET_ISO_FUNCTION_PDU_TRANSPORT)
    {
        pcap_parse_s7comm_pdu(user, bytes + sizeof(struct s7comm_iso_header),
                len - sizeof(struct s7comm_iso_header));
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
    if (payload_len < sizeof(struct s7comm_request))
        return;

    if (dst_port == PROFINET_PORT)
    {
        printf("===== REQUEST ==========================\n");
        pcap_parse_s7comm(user, bytes + hdr_len,
                payload_len);
    }
    else if (src_port == PROFINET_PORT)
    {
        printf("===== RESPONSE =========================\n");
        pcap_parse_s7comm(user, bytes + hdr_len,
                payload_len);
    }
    else
        printf("Unknown connection at dest port = %d, src_port = %d\n",
                dst_port, src_port);
}

static void dump_s7comm_function(const enum s7comm_function_t function)
{
    printf("Function: ");
    switch (function)
    {
        case s7comm_function_open_connection:
            printf("open connection");
            break;
        case s7comm_function_read:
            printf("read");
            break;
        case s7comm_function_write:
            printf("write");
            break;
        case s7comm_function_download_request:
            printf("download request");
            break;
        case s7comm_function_download_block:
            printf("download block");
            break;
        case s7comm_function_download_ended:
            printf("download ended");
            break;
        case s7comm_function_upload_start:
            printf("upload start");
            break;
        case s7comm_function_upload:
            printf("upload");
            break;
        case s7comm_function_upload_end:
            printf("upload end");
            break;
        case s7comm_function_insert_block:
            printf("insert block");
            break;
        default:
            printf("unknown function 0x%02x", function);
    }
    printf("\n");
}

static void dump_s7comm_read_response(struct ppkt_t *p, size_t plen, size_t dlen)
{
    assert(p);
    assert(dlen > 0);

    uint32_t *prefix = (uint32_t*)ppkt_payload(p);
    assert(*prefix == htonl(0xff040000));

    ppkt_pull(p, 4);
    assert(dlen == ppkt_size(p));
    dump_bytes(ppkt_payload(p), dlen);
}

static void dump_s7comm_read_request(struct ppkt_t *p, size_t plen, size_t dlen)
{
    assert(p);

    assert(plen == 14);  // Full request (incl. function/empty byte!)
    struct s7comm_read_request_t *req = (struct s7comm_read_request_t*)ppkt_payload(p);

    printf("Read size: %s\n", (req->read_size == 1 ? "bit" : (req->read_size == 2 ? "byte" : "word")));
    printf("read_length: %d bytes\n", ntohs(req->read_length));
    printf("DB: %d\n", ntohs(req->db_num));
    printf("Area: %s\n", s7comm_area_to_string(req->area_code));
    uint32_t start_addr = (req->start_addr << 24) | ntohs(req->start_addr_2);
    printf("Start addr: 0x%06x\n", start_addr);
}*/

static err_t analyze_tpkt_receive(struct ppkt_t *p, void *user)
{
    assert(p);

    printf("tpkt_receive %lu bytes\n", ppkt_chain_size(p));
    ppkt_free(p);

    return ERR_NONE;
}

#if 0
static err_t analyze_pcap_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(! user);
    printf("Packet:\n");
    dump_bytes(ppkt_payload(p), ppkt_size(p));

    // We get TCP payload here
    ppkt_free(p);
    return ERR_NONE;

    if (ppkt_size(p) < sizeof(struct s7comm_hdr_t))
        goto done;

    struct s7comm_hdr_t *hdr = (struct s7comm_hdr_t*)ppkt_payload(p);
    assert(hdr->version == 0x32);

    /*printf("msgtype: %d\n", hdr->msgtype);
    assert(hdr->zero == 0);
    printf("seq: %d\n", ntohs(hdr->seq));
    printf("parameter len: %d\n", ntohs(hdr->plen));
    printf("data len: %d\n", ntohs(hdr->dlen));*/

    size_t seek = sizeof(struct s7comm_hdr_t);
    if (hdr->msgtype == 2 || hdr->msgtype == 3)
        seek += 2; // Result field.

    if (ppkt_size(p) <= seek)
        goto done;

    ppkt_pull(p, seek);

    if (ppkt_size(p) < sizeof(struct s7comm_request_t))
        goto done;

    struct s7comm_request_t *req = (struct s7comm_request_t*)ppkt_payload(p);
    dump_s7comm_function(req->function);

    ppkt_pull(p, sizeof(struct s7comm_request_t));

    uint16_t plen = ntohs(hdr->plen);
    uint16_t dlen = ntohs(hdr->dlen);
    if (req->function == s7comm_function_read)
    {
        if (plen == 14)
            dump_s7comm_read_request(p, plen, dlen);
        else if (plen == 2)
            dump_s7comm_read_response(p, plen, dlen);
        else
            printf("Unknown read function packet\n");
    }

    printf("==================================================\n");

done:
    ppkt_free(p);

    return ERR_NONE;
}
#endif

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <file.pcap>\n", argv[0]);
        return 1;
    }

    struct tpkt_dev_t *tdev = tpkt_connect(argv[1], analyze_tpkt_receive,
            NULL, &pcap_proto);
    if (! tdev)
    {
        printf("Unable to set up tpkt layer\n");
        return 1;
    }

    err_t err = ERR_NONE;
    do
    {
        err = tpkt_poll(tdev);
    } while (OK(err));

    tpkt_disconnect(tdev);

    return 0;
}
