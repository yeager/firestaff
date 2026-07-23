#include "dm1_v1_f1986_f2025_editor_hint_source_audit_pc34_compat.h"

static const DM1_V1_F1986F2025SourceAuditPc34 k_audit[] = {
    {1986u,"no numbered F1986 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {1987u,"CEDTINCI.C:225 F1987_Checksum","fail_closed: editor checksum",1,1,1,1},
    {1988u,"CEDTINCI.C:243 F1988_Checksum","fail_closed: editor checksum",1,1,1,1},
    {1989u,"CEDT002.C:174 F1989_InitializeRandomNumber","fail_closed: editor random state",1,1,1,1},
    {1990u,"no numbered F1990 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {1991u,"UTIO.C:264 F1991_","fail_closed: original floppy route",1,1,1,1},
    {1992u,"CEDTINCJ.C:5 F1992_RequestUtilityDiskInDrive","fail_closed: editor utility disk route",1,1,1,1},
    {1993u,"CEDTINC6.C:187 F1993_","fail_closed: editor save header route",1,1,1,1},
    {1994u,"CEDTINC7.C:58 F1994_","fail_closed: editor file route",1,1,1,1},
    {1995u,"CEDTINCC.C:5 F1995_","fail_closed: editor disk route",1,1,1,1},
    {1996u,"CEDTINCH.C:50 F1996_","fail_closed: editor game route",1,1,1,1},
    {1997u,"no numbered F1997 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {1998u,"CEDTINCK.C:5 F1998_PrintSpacePaddedText_Unreferenced","fail_closed: source-unreferenced editor text",1,1,1,1},
    {1999u,"CEDTINC6.C:6 F1999_FILE_Read","fail_closed: editor file transport",1,1,1,1},
    {2000u,"CEDTINC6.C:23 F2000_FILE_Write","fail_closed: editor file transport",1,1,1,1},
    {2001u,"no numbered F2001 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {2002u,"no numbered F2002 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {2003u,"no numbered F2003 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {2004u,"HINT003.C:56 F2004_","fail_closed: optional HINT graphics helper",1,1,1,1},
    {2005u,"UTDEBUG.C:93 F2005_","fail_closed: original debug route",1,1,1,1},
    {2006u,"HINTIORQ.C:231 F2006_","fail_closed: optional HINT I/O route",1,1,1,1},
    {2007u,"UTIO.C:171 F2007_SetFloppyDriveIndex","fail_closed: original floppy selection",1,1,1,1},
    {2008u,"UTIO.C:431 F2008_IsLeftMouseButtonDown","dm1_v1_input_command_queue_pc34_compat",1,1,1,1},
    {2009u,"UTIO.C:437 F2009_GetMouseX","dm1_v1_input_command_queue_pc34_compat",1,1,1,1},
    {2010u,"UTIO.C:443 F2010_GetMouseY","dm1_v1_input_command_queue_pc34_compat",1,1,1,1},
    {2011u,"COMMAND.C:3112 F2011_ProcessClicksAndKeyPresses","dm1_v1_click_routing_pc34_compat",1,1,1,1},
    {2012u,"HINTFLOP.C:6 F2012_","fail_closed: optional HINT floppy route",1,1,1,1},
    {2013u,"HINTFLOP.C:18 F2013_","fail_closed: optional HINT floppy route",1,1,1,1},
    {2014u,"HINT001.C:8 F2014_ConvertStringToLowerCase","dm1_v1_hint_string_helpers_pc34_compat",1,1,1,1},
    {2015u,"HINT001.C:14 F2015_","fail_closed: optional HINT screen buffer",1,1,1,1},
    {2016u,"HINT001.C:20 F2016_","fail_closed: optional HINT screen buffer",1,1,1,1},
    {2017u,"HINT001.C:30 F2017_","fail_closed: optional HINT screen route",1,1,1,1},
    {2018u,"HINT001.C:34 F2018_","fail_closed: optional HINT screen route",1,1,1,1},
    {2019u,"HINT001.C:39 F2019_","fail_closed: optional HINT screen route",1,1,1,1},
    {2020u,"no numbered F2020 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {2021u,"no numbered F2021 body in ReDMCSB corpus","fail_closed: no source owner",0,1,1,1},
    {2022u,"HINT001.C:74 F2022_F1889_SubForInit","fail_closed: optional HINT initialization",1,1,1,1},
    {2023u,"HINT001.C:197 F2023_F1889_SubForUninit","fail_closed: optional HINT teardown",1,1,1,1},
    {2024u,"HINT001.C:219 F2024_IsLeftMouseButtonDown","dm1_v1_input_command_queue_pc34_compat",1,1,1,1},
    {2025u,"HINT001.C:225 F2025_","fail_closed: optional HINT input route",1,1,1,1}
};

const DM1_V1_F1986F2025SourceAuditPc34 *
dm1_v1_f1986_f2025_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1986F2025SourceAuditPc34 *
dm1_v1_f1986_f2025_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1986_f2025_source_audit_evidence_pc34(void)
{
    return "ReDMCSB CEDT, UTIO, HINT, and COMMAND sources are the authority for "
           "F1986-F2025. Existing input, text, and click-routing owners are retained. "
           "Editor, HINT, floppy, and screen routes remain fail closed without authentic "
           "raw PC34 material. The audit does not render or synthesize UI or timing paths.";
}
