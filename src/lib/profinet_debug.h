#ifndef _PROFINET_DEBUG_H_
#define _PROFINET_DEBUG_H_

const char* profinet_area_to_string(const enum profinet_area_t area)
{
    switch (area)
    {
        case profinet_area_SysInfo:
            return "SysInfo";
        case profinet_area_SysFlags:
            return "SysFlags";
        case profinet_area_AnaIn:
            return "Analog in";
        case profinet_area_AnaOut:
            return "Analog out";
        case profinet_area_P:
            return "Direct peripheral access";
        case profinet_area_Inputs:
            return "Inputs";
        case profinet_area_Outputs:
            return "Outputs";
        case profinet_area_Flags:
            return "Flags";
        case profinet_area_DB:
            return "Data blocks";
        case profinet_area_DI:
            return "Instance data blocks";
        case profinet_area_SysDataS5:
            return "System data S5";
        case profinet_area_V:
            return "V";
        case profinet_area_Counter:
            return "Counter";
        case profinet_area_Timer:
            return "Timer";
        case profinet_area_Counter200:
            return "Counter (200 family)";
        case profinet_area_Timer200:
            return "Timer (200 family)";
        case profinet_area_RawMemoryS5:
            return "Raw memory S5";
    }

    return "unknown";
}

#endif
