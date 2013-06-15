#ifndef _PROFINET_DEBUG_H_
#define _PROFINET_DEBUG_H_

#include <stdint.h>
#include <stddef.h>
#include "profinet_types.h"

const char* profinet_area_to_string(const enum profinet_area_t area);

void dump_bytes(const uint8_t *bytes, size_t size);

#endif
