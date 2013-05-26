#ifndef _PROFINET_TYPES_H_
#define _PROFINET_TYPES_H_

#include <stdint.h>

#define PROFINET_PORT 102

#define PROFINET_ISO_PROTOCOL 0x03

#define PROFINET_ISO_FUNCTION_PDU_TRANSPORT 0xf0
#define PROFINET_ISO_FUNCTION_CONNECT       0xe0

enum profinet_function_t
{
    profinet_function_open_connection  = 0xf0,
    profinet_function_read             = 0x04,
    profinet_function_write            = 0x05,
    profinet_function_download_request = 0x1a,
    profinet_function_download_block   = 0x1b,
    profinet_function_download_ended   = 0x1c,
    profinet_function_upload_start     = 0x1d,
    profinet_function_upload           = 0x1e,
    profinet_function_upload_end       = 0x1f,
    profinet_function_insert_block     = 0x28,
};

enum profinet_area_t
{
    profinet_area_SysInfo     = 0x3,   /* System info of 200 family */
    profinet_area_SysFlags    = 0x5,   /* System flags of 200 family */
    profinet_area_AnaIn       = 0x6,   /* analog inputs of 200 family */
    profinet_area_AnaOut      = 0x7,   /* analog outputs of 200 family */
    profinet_area_P           = 0x80,  /* direct peripheral access */
    profinet_area_Inputs      = 0x81,
    profinet_area_Outputs     = 0x82,
    profinet_area_Flags       = 0x83,
    profinet_area_DB          = 0x84,  /* data blocks */
    profinet_area_DI          = 0x85,  /* instance data blocks */
    profinet_area_SysDataS5   = 0x86,  /* system data area ? */
    profinet_area_V           = 0x87,  /* don't know what it is */
    profinet_area_Counter     = 0x1c,  /* S7 counters */
    profinet_area_Timer       = 0x1d,  /* S7 timers */
    profinet_area_Counter200  = 0x1e,  /* IEC counters (200 family) */
    profinet_area_Timer200    = 0x1f,  /* IEC timers (200 family) */
    profinet_area_RawMemoryS5 = 0      /* just the raw memory */
};

struct profinet_iso_header
{
    uint8_t prot;
    uint8_t ch1;
    uint8_t ch2;
    uint8_t len;
    uint8_t xxxx1;
    uint8_t func;
    uint8_t xxxx2;
} __attribute__((packed));

struct profinet_ibh_header
{
    uint16_t channel;
    uint8_t len;
    uint8_t seq;
    uint16_t sflags;
    uint16_t rflags;
} __attribute__((packed));

struct profinet_pdu_header
{
    uint8_t unknown;
    uint8_t version;
    uint16_t unknown2;
    uint16_t unknown3;
    uint16_t plen;
    uint16_t dlen;
    uint16_t res;
} __attribute__((packed));

struct profinet_request
{
    uint8_t function;
    uint8_t unknown3;
    uint16_t prefix;
    uint8_t unknown4;
    uint8_t read_size;
    uint16_t bytes;
    uint16_t db_num;
    uint8_t area_code;
    uint8_t start_addr; /* start */
    uint16_t start_addr_2; /* start, part 2, is 3 bytes in total */
} __attribute__((packed));

#endif
