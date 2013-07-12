#include "s7comm_debug.h"
#include <stdio.h>

const char* s7comm_area_to_string(const enum s7comm_area_t area)
{
    switch (area)
    {
        case s7comm_area_SysInfo:
            return "SysInfo";
        case s7comm_area_SysFlags:
            return "SysFlags";
        case s7comm_area_AnaIn:
            return "Analog in";
        case s7comm_area_AnaOut:
            return "Analog out";
        case s7comm_area_P:
            return "Direct peripheral access";
        case s7comm_area_Inputs:
            return "Inputs";
        case s7comm_area_Outputs:
            return "Outputs";
        case s7comm_area_Flags:
            return "Flags";
        case s7comm_area_DB:
            return "Data blocks";
        case s7comm_area_DI:
            return "Instance data blocks";
        case s7comm_area_SysDataS5:
            return "System data S5";
        case s7comm_area_V:
            return "V";
        case s7comm_area_Counter:
            return "Counter";
        case s7comm_area_Timer:
            return "Timer";
        case s7comm_area_Counter200:
            return "Counter (200 family)";
        case s7comm_area_Timer200:
            return "Timer (200 family)";
        case s7comm_area_RawMemoryS5:
            return "Raw memory S5";
    }

    return "unknown";
}

void dump_bytes(const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        printf("%02x ", bytes[i]);

        if (i % 8 == 7)
            printf("  ");

        if (i % 16 == 15)
            printf("\n");
    }
    if (i % 16 != 15)
        printf("\n");
}
