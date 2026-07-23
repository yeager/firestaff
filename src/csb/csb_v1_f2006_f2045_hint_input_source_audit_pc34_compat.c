#include "csb_v1_f2006_f2045_hint_input_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F2006F2045SourceAuditPc34 k_audit[] = {
    NONE(2006),
    BLOCK(2007, "UTIO.C F2007_SetFloppyDriveIndex", "fail_closed: no authenticated CSB PC34 floppy owner"),
    BLOCK(2008, "CEDTINCF.C; UTIO.C; CEDT006.C F2008_IsLeftMouseButtonDown", "DM1 mouse accessor owner preserved separately; no CSB PC34 owner"),
    BLOCK(2009, "CEDTINCF.C; UTIO.C; CEDT006.C F2009_GetMouseX", "DM1 mouse accessor owner preserved separately; no CSB PC34 owner"),
    BLOCK(2010, "CEDTINCF.C; UTIO.C; CEDT006.C F2010_GetMouseY", "DM1 mouse accessor owner preserved separately; no CSB PC34 owner"),
    BLOCK(2011, "COMMAND.C F2011_ProcessClicksAndKeyPresses", "fail_closed: no authenticated CSB PC34 command-input owner"),
    NONE(2012), NONE(2013),
    BLOCK(2014, "HINTTEXT.C; HINT001.C F2014_ConvertStringToLowerCase", "DM1 string owner preserved separately; no CSB PC34 owner"),
    NONE(2015), NONE(2016), NONE(2017), NONE(2018), NONE(2019), NONE(2020), NONE(2021),
    BLOCK(2022, "HINT001.C; HINTSCR.C F2022_F1889_SubForInit", "fail_closed: no authenticated CSB PC34 hint lifecycle owner"),
    BLOCK(2023, "HINT001.C; HINTSCR.C F2023_F1889_SubForUninit", "fail_closed: no authenticated CSB PC34 hint lifecycle owner"),
    BLOCK(2024, "HINT001.C; HINTINPT.C F2024_IsLeftMouseButtonDown", "DM1 mouse accessor owner preserved separately; no CSB PC34 owner"),
    NONE(2025),
    BLOCK(2026, "HINTGRAP.C; HINTGTXT.C F2026_GetGraphicByteCount", "fail_closed: no authenticated CSB PC34 hint graphic owner"),
    NONE(2027),
    BLOCK(2028, "HINTGTXT.C; HINT001.C F2028_LoadHintOracleTexts", "fail_closed: no authenticated CSB PC34 hint text owner"),
    BLOCK(2029, "HINTGTXT.C F2029_GetHintOracleText_Unreferenced", "fail_closed: source marks this hint text route unreferenced"),
    BLOCK(2030, "HINTGTXT.C; HINT001.C F2030_FreeHintOracleTexts", "fail_closed: no authenticated CSB PC34 hint text owner"),
    NONE(2031), NONE(2032), NONE(2033), NONE(2034), NONE(2035), NONE(2036), NONE(2037), NONE(2038), NONE(2039), NONE(2040), NONE(2041),
    BLOCK(2042, "UTAMSCR.C; HINT001.C; UTIO.C F2042_InitializeScreen", "fail_closed: no authenticated CSB PC34 screen owner"),
    BLOCK(2043, "UTAMSCR.C; HINT001.C; UTIO.C F2043_UninitializeScreen", "fail_closed: no authenticated CSB PC34 screen owner"),
    NONE(2044),
    BLOCK(2045, "FILLBOX.C; CEDTINCI.C; UTAMSCR.C; UTIO.C; HINTPAL.C F2045_FadeToPalette", "fail_closed: no authenticated CSB PC34 palette owner")
};

#undef BLOCK
#undef NONE

const CSB_V1_F2006F2045SourceAuditPc34 *
csb_v1_f2006_f2045_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2006F2045SourceAuditPc34 *
csb_v1_f2006_f2045_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2006_f2045_source_audit_evidence_pc34(void)
{
    return "ReDMCSB UTIO.C, CEDTINCF.C, CEDT006.C, COMMAND.C, HINTTEXT.C, "
           "HINT001.C, HINTSCR.C, HINTINPT.C, HINTGRAP.C, HINTGTXT.C, UTAMSCR.C, "
           "FILLBOX.C, CEDTINCI.C, and HINTPAL.C own the identified F2006-F2045 "
           "routes. Existing DM1 mouse/string owners remain separate. No CSB PC34 "
           "owner is present, so all routes fail closed without authenticated material. "
           "This audit does not render or synthesize input, text, graphics, UI, timing, or files.";
}
