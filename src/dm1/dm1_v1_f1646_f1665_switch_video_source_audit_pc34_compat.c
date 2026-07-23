#include "dm1_v1_f1646_f1665_switch_video_source_audit_pc34_compat.h"

static const DM1_V1_F1646F1665SourceAuditPc34 k_audit[] = {
    { 1646u, "no numbered F1646 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1647u, "no numbered F1647 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1648u, "no numbered F1648 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1649u, "no numbered F1649 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1650u, "SWITCHMM.C:9 F1650_AllocateMemory", "fail_closed: Atari ST switch allocator", 1, 1, 1, 1 },
    { 1651u, "SWITCHMM.C:18 F1651_FreeMemory", "fail_closed: Atari ST switch allocator", 1, 1, 1, 1 },
    { 1652u, "SWITCHMM.C:23 F1652_GetAllocatedMemoryBlockCount", "fail_closed: Atari ST switch allocator state", 1, 1, 1, 1 },
    { 1653u, "SWITCHMM.C:29 F1653_ConvertCoordinates", "fail_closed: Atari ST switch coordinate route", 1, 1, 1, 1 },
    { 1654u, "SWITCHMM.C:36 F1654_Unreferenced", "fail_closed: source unreferenced switch helper", 1, 1, 1, 1 },
    { 1655u, "no numbered F1655 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1656u, "no numbered F1656 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1657u, "no numbered F1657 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1658u, "VDEO2.C:137 F1658_IsShiftKeyPressed_Unrerefenced", "fail_closed: X68000 IOCS keyboard route", 1, 1, 1, 1 },
    { 1659u, "VDEO2.C:273 F1659_", "fail_closed: X68000 video-mode route", 1, 1, 1, 1 },
    { 1660u, "VDEO2.C:325 F1660_", "fail_closed: X68000 video-mode restore", 1, 1, 1, 1 },
    { 1661u, "VDEO2.C:348 F1661_", "fail_closed: X68000 bitmap/video allocation", 1, 1, 1, 1 },
    { 1662u, "VDEO1.C:1251 F1662_InstallVerticalBlankInterruptHandler", "fail_closed: X68000 vertical-blank interrupt", 1, 1, 1, 1 },
    { 1663u, "VDEO1.C:1284 F1663_UninstallVerticalBlankInterruptHandler", "fail_closed: X68000 vertical-blank interrupt", 1, 1, 1, 1 },
    { 1664u, "no numbered F1664 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1665u, "INT1.C:50 F1665_InstallVerticalBlankHandlerManager", "fail_closed: X68000 interrupt manager", 1, 1, 1, 1 }
};

const DM1_V1_F1646F1665SourceAuditPc34 *
dm1_v1_f1646_f1665_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1646F1665SourceAuditPc34 *
dm1_v1_f1646_f1665_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1646_f1665_source_audit_evidence_pc34(void)
{
    return "ReDMCSB SWITCHMM.C, VDEO1.C, VDEO2.C, and INT1.C are the authority "
           "for F1646-F1665. F1646-F1649, F1655-F1657, and F1664 have no numbered "
           "source body. Atari ST and X68000 memory, video, and interrupt routes "
           "remain fail closed without authentic PC34 material. The audit does not "
           "render or synthesize UI or timing paths.";
}
