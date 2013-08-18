#ifndef _S7COMM_TYPES_H_
#define _S7COMM_TYPES_H_

#include <stdint.h>

enum s7comm_function_t
{
    s7comm_function_open_connection  = 0xf0,
    s7comm_function_read             = 0x04,
    s7comm_function_write            = 0x05,
    s7comm_function_download_request = 0x1a,
    s7comm_function_download_block   = 0x1b,
    s7comm_function_download_ended   = 0x1c,
    s7comm_function_upload_start     = 0x1d,
    s7comm_function_upload           = 0x1e,
    s7comm_function_upload_end       = 0x1f,
    s7comm_function_insert_block     = 0x28,
};

enum s7comm_area_t
{
    s7comm_area_SysInfo     = 0x3,   /* System info of 200 family */
    s7comm_area_SysFlags    = 0x5,   /* System flags of 200 family */
    s7comm_area_AnaIn       = 0x6,   /* analog inputs of 200 family */
    s7comm_area_AnaOut      = 0x7,   /* analog outputs of 200 family */
    s7comm_area_P           = 0x80,  /* direct peripheral access */
    s7comm_area_Inputs      = 0x81,
    s7comm_area_Outputs     = 0x82,
    s7comm_area_Flags       = 0x83,
    s7comm_area_DB          = 0x84,  /* data blocks */
    s7comm_area_DI          = 0x85,  /* instance data blocks */
    s7comm_area_SysDataS5   = 0x86,  /* system data area ? */
    s7comm_area_V           = 0x87,  /* don't know what it is */
    s7comm_area_Counter     = 0x1c,  /* S7 counters */
    s7comm_area_Timer       = 0x1d,  /* S7 timers */
    s7comm_area_Counter200  = 0x1e,  /* IEC counters (200 family) */
    s7comm_area_Timer200    = 0x1f,  /* IEC timers (200 family) */
    s7comm_area_RawMemoryS5 = 0      /* just the raw memory */
};

#define S7COMM_VERSION 0x32

struct s7comm_hdr_t
{
    /* S7 common header */
    uint8_t version; // Always 0x32
    uint8_t msgtype; // 1, 2, 3, or 7. Type 2 and 3 are two bytes longer (see result)
    uint16_t zero;
    uint16_t seq;
    uint16_t plen; // Length of parameters after this header
    uint16_t dlen; // Length of data after this header

    //uint16_t result; // Optional, only in type 2 and 3 headers
} __attribute__((packed));

struct s7comm_request_t
{
    uint8_t function;
    uint8_t unknown;
} __attribute__((packed));

struct s7comm_open_connection_t
{
    uint16_t unknown1;
    uint16_t unknown2;
    uint16_t unknown3;
} __attribute__((packed));

enum s7comm_read_size_t
{
    s7comm_read_size_bit = 1,
    s7comm_read_size_byte = 2,
    s7comm_read_size_word = 4,
};

struct s7comm_read_request_t
{
    uint16_t prefix;
    uint8_t unknown;
    uint8_t read_size;
    uint16_t read_length;
    uint16_t db_num;
    uint8_t area_code;
    uint8_t start_addr; /* start */
    uint16_t start_addr_2; /* start, part 2, is 3 bytes in total */
} __attribute__((packed));

enum s7comm_read_response_err_t
{
    s7comm_READ_RESPONSE_ERR_NONE = 0xff,
    s7comm_READ_RESPONSE_ERR_NO_ITEM = 0x0a,
};

struct s7comm_read_response_t
{
    uint8_t err; // 0xff == OK
    uint8_t len_type;
    uint16_t len;
};

struct s7comm_write_response_t
{
    uint8_t err; // 0xff == OK
};

struct s7comm_dev
{
    int fd;
};

#endif
