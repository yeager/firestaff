#include "csb_v1_f1486_f1525_switch_vdi_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1486F1525SourceAuditPc34 k_audit[] = {
    NONE(1486), NONE(1487), NONE(1488), NONE(1489), NONE(1490), NONE(1491), NONE(1492), NONE(1493),
    NONE(1494), NONE(1495), NONE(1496), NONE(1497), NONE(1498), NONE(1499), NONE(1500),
    BLOCK(1501, "SWITCH.C:53 F1501_GetSwitchOptionBitmapByteCount", "fail_closed: no authenticated CSB PC34 switch-option bitmap owner"),
    NONE(1502),
    BLOCK(1503, "SWITCH.C:78 F1503_ReadFTLHeaders", "fail_closed: no authenticated CSB PC34 FTL-header receipt"),
    BLOCK(1504, "SWITCH.C:151 F1504_LoadSegment", "fail_closed: no authenticated CSB PC34 switch segment owner"),
    NONE(1505),
    BLOCK(1506, "SWITCH.C:230 F1506_SetMouseInputFromSwitchOptions", "fail_closed: no authenticated CSB PC34 switch mouse-input owner"),
    BLOCK(1507, "SWITCH.C:256 F1507_DrawSwitchOptionBitmap", "fail_closed: no authenticated CSB PC34 switch bitmap owner"),
    BLOCK(1508, "SWITCH.C:298 F1508_FreeMemoryOfAllSwitchOptionGraphics", "fail_closed: no authenticated CSB PC34 switch graphics owner"),
    BLOCK(1509, "SWITCH.C:310 F1509_Initialization", "fail_closed: no authenticated CSB PC34 switch lifecycle owner"),
    BLOCK(1510, "SWITCH.C:329 F1510_LoadDataFile", "fail_closed: no authenticated CSB PC34 switch data-file owner"),
    BLOCK(1511, "SWITCH.C:404 F1511_Unreferenced", "fail_closed: source marks this switch route unreferenced"),
    NONE(1512),
    BLOCK(1513, "SWITCH.C:412 F1513_MainLoop", "fail_closed: no authenticated CSB PC34 switch main-loop owner"),
    NONE(1514), NONE(1515), NONE(1516), NONE(1517), NONE(1518), NONE(1519), NONE(1520), NONE(1521), NONE(1522), NONE(1523),
    BLOCK(1524, "UTSTWKS.C:18 F1524_VerticalBlankClearPalette", "fail_closed: no authenticated CSB PC34 VDI palette owner"),
    BLOCK(1525, "UTSTWKS.C; CEDTINCI.C; ANIM.C; SWITCH.C F1525_OpenVDIWorkstation", "fail_closed: no authenticated CSB PC34 VDI workstation owner")
};

#undef BLOCK
#undef NONE

const CSB_V1_F1486F1525SourceAuditPc34 *
csb_v1_f1486_f1525_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1486F1525SourceAuditPc34 *
csb_v1_f1486_f1525_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1486_f1525_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWITCH.C:53-412 and UTSTWKS.C:18 are the identified "
           "source bodies for F1486-F1525. Existing CSB frontend and runtime "
           "owners remain separate. Switch-option and VDI routes fail closed "
           "without authenticated PC34 material; this audit does not render or "
           "synthesize UI, graphics, timing, input, or files.";
}
