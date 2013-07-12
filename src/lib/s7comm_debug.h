#ifndef _S7COMM_DEBUG_H_
#define _S7COMM_DEBUG_H_

#include <stdint.h>
#include <stddef.h>
#include "s7comm_types.h"

const char* s7comm_area_to_string(const enum s7comm_area_t area);

void dump_bytes(const uint8_t *bytes, size_t size);

#endif
