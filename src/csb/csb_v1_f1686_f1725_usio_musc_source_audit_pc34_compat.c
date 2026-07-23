#include "csb_v1_f1686_f1725_usio_musc_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1686F1725SourceAuditPc34 k_audit[] = {
    NONE(1686),
    BLOCK(1687, "USIO2.C; USIO1.C F1687_UninstallMouseInterruptHandler", "fail_closed: no authenticated CSB PC34 mouse-interrupt owner"),
    NONE(1688), NONE(1689),
    BLOCK(1690, "USIO2.C F1690_GetASCIICode", "fail_closed: no authenticated CSB PC34 keyboard-code owner"),
    BLOCK(1691, "USIO2.C F1691_Cconis", "fail_closed: no authenticated CSB PC34 console-input owner"),
    BLOCK(1692, "USIO2.C F1692_Crawcin", "fail_closed: no authenticated CSB PC34 console-input owner"),
    NONE(1693),
    BLOCK(1694, "USIO1.C F1694_AddMouseInputToQueue", "fail_closed: no authenticated CSB PC34 mouse-queue owner"),
    NONE(1695), NONE(1696), NONE(1697), NONE(1698), NONE(1699), NONE(1700), NONE(1701), NONE(1702), NONE(1703), NONE(1704),
    NONE(1705), NONE(1706), NONE(1707), NONE(1708), NONE(1709), NONE(1710), NONE(1711), NONE(1712), NONE(1713), NONE(1714),
    BLOCK(1715, "MUSCMAIN.C; MUSCMIDI.C F1715_MUSC_07_", "fail_closed: no authenticated CSB PC34 MIDI owner"),
    BLOCK(1716, "MUSCMAIN.C; MUSCMIDI.C F1716_MUSC_05_StopMIDIMusic", "fail_closed: no authenticated CSB PC34 MIDI owner"),
    BLOCK(1717, "MUSCMAIN.C; MUSCMIDI.C F1717_MUSC_04_PlayMIDIMusic", "fail_closed: no authenticated CSB PC34 MIDI owner"),
    BLOCK(1718, "MUSCMAIN.C; MUSCMIDI.C F1718_MUSC_03_Expunge", "fail_closed: no authenticated CSB PC34 MIDI owner"),
    NONE(1719), NONE(1720),
    BLOCK(1721, "MUSCMAIN.C; MUSCOPM.C F1721_MUSC_06_", "fail_closed: no authenticated CSB PC34 music-platform owner"),
    NONE(1722), NONE(1723), NONE(1724), NONE(1725)
};

#undef BLOCK
#undef NONE

const CSB_V1_F1686F1725SourceAuditPc34 *
csb_v1_f1686_f1725_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1686F1725SourceAuditPc34 *
csb_v1_f1686_f1725_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1686_f1725_source_audit_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C, USIO2.C, MUSCMAIN.C, MUSCMIDI.C, and MUSCOPM.C "
           "own the identified F1686-F1725 input and music vectors. Existing DM1 "
           "input owners remain separate. No CSB PC34 owner is present, so all "
           "routes fail closed without authenticated PC34 material. This audit does "
           "not render or synthesize input, audio, UI, or timing paths.";
}
