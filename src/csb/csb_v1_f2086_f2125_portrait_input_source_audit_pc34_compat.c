#include "csb_v1_f2086_f2125_portrait_input_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F2086F2125SourceAuditPc34 k_audit[] = {
    NONE(2086), NONE(2087), NONE(2088), NONE(2089), NONE(2090),
    BLOCK(2091, "CEDTINC2.C; CEDT023.C F2091_GetDriveAddress", "fail_closed: no authenticated CSB PC34 drive owner"),
    NONE(2092),
    BLOCK(2093, "CEDT025.C; CEDT006.C; CEDT002.C F2093_GetVerticalBlankCount", "fail_closed: no authenticated CSB PC34 vertical-blank owner"),
    BLOCK(2094, "CEDTINC2.C; CEDT023.C F2094_Call_DOS_FFLUSH", "fail_closed: no CSB PC34 DOS flush substitute"),
    BLOCK(2095, "CEDT005.C; PORTRAIT.C F2095_AllocateMemory", "DM1 portrait owner preserved separately; no CSB PC34 owner"),
    NONE(2096), NONE(2097), NONE(2098), NONE(2099),
    BLOCK(2100, "CEDTINCF.C; UTIO.C F2100_GetChildFiles", "fail_closed: no authenticated CSB PC34 child-file owner"),
    NONE(2101), NONE(2102), NONE(2103),
    BLOCK(2104, "CEDTINCI.C; PORTRAIT.C F2104_CHAMPION_ConvertBitmapToAtariSTPlanar", "DM1 portrait owner preserved separately; no CSB PC34 owner"),
    BLOCK(2105, "CEDTINCE.C; PORTRAIT.C F2105_CHAMPION_ConvertBitmapFromAtariSTPlanar", "DM1 portrait owner preserved separately; no CSB PC34 owner"),
    BLOCK(2106, "CEDTINCI.C; COMMAND.C F2106_InitializeUsioDataFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2107, "CEDTINCI.C; COMMAND.C F2107_UninitializeUsioDataFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2108, "COMMAND.C F2108_AddUsioDataToFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2109, "COMMAND.C F2109_BuildUsioDataFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2110, "CEDT025.C; COMMAND.C F2110_GetFirstUsioDataInFilteredQueueOfSpecifiedTypeIndex", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2111, "COMMAND.C F2111_RemoveUsioDataFromFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    NONE(2112),
    BLOCK(2113, "CEDT025.C; COMMAND.C F2113_GetFirstUsioDataInFilteredQueueOfSpecifiedTypeIndex_KeepInQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    BLOCK(2114, "COMMAND.C F2114_EmptyUsioDataFilteredQueue", "DM1 click-routing owner preserved separately; no CSB PC34 owner"),
    NONE(2115), NONE(2116), NONE(2117), NONE(2118), NONE(2119), NONE(2120), NONE(2121),
    BLOCK(2122, "CEDT019.C; CEDTINCD.C F2122_DecodeAllPortraitsWhileLoading", "DM1 portrait-save owner preserved separately; no CSB PC34 owner"),
    BLOCK(2123, "CEDT019.C; CEDTINC8.C F2123_EncodeAllPortraitsBeforeSaving", "DM1 portrait-save owner preserved separately; no CSB PC34 owner"),
    BLOCK(2124, "CEDT019.C; CEDTINC8.C F2124_DecodeAllPortraitsAfterSaving", "DM1 portrait-save owner preserved separately; no CSB PC34 owner"),
    NONE(2125)
};

#undef BLOCK
#undef NONE

const CSB_V1_F2086F2125SourceAuditPc34 *
csb_v1_f2086_f2125_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2086F2125SourceAuditPc34 *
csb_v1_f2086_f2125_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2086_f2125_source_audit_evidence_pc34(void)
{
    return "ReDMCSB CEDTINC2.C, CEDT023.C, CEDT025.C, CEDT006.C, CEDT002.C, "
           "CEDT005.C, PORTRAIT.C, CEDTINCF.C, UTIO.C, CEDTINCI.C, CEDTINCE.C, "
           "COMMAND.C, CEDT019.C, CEDTINCD.C, and CEDTINC8.C own the identified "
           "F2086-F2125 routes. Existing DM1 portrait, click-routing, and portrait-save "
           "owners remain separate. No CSB PC34 owner is present, so all routes fail closed "
           "without authenticated material. This audit does not render or synthesize portrait, "
           "input, UI, timing, memory, file, or graphics behavior.";
}
