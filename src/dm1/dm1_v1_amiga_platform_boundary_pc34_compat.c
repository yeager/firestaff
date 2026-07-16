#include "dm1_v1_amiga_platform_boundary_pc34_compat.h"

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0513 = {
    "F0513_DIALOG_DrawGameReadyToPlay_Unreferenced",
    "AMIGA.H:304; DIALOG.C",
    "Amiga-host dialog",
    0,
    "The ReDMCSB callable audit identifies this as an Amiga dialog host-facing "
    "unreferenced route, with no reviewed Firestaff implementation and no "
    "evidenced I34E/I34M PC3.4 runtime dialog contract."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0535 = {
    "F0535_MEMORY_GetGraphicsDatFileSize",
    "AMIGA.H:336; MEMORY.C; STARTUP1.C",
    "Amiga-host graphics.dat sizing",
    0,
    "The reviewed callable and platform-boundary audits leave this as an "
    "Amiga host-facing GRAPHICS.DAT size query with no source-backed portable "
    "PC3.4 file-size contract. Firestaff must use its existing asset-loader "
    "routes rather than this symbol as a synthetic file-size API."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0551 = {
    "F0551_VIDEO_HatchBox_Unreferenced",
    "AMIGA.H:352; BLITFILL.C:437",
    "Amiga video hatch-box helper",
    0,
    "ReDMCSB marks this hatch-box route unreferenced on the reviewed source "
    "path. Firestaff keeps it as an explicit no-portability boundary instead "
    "of fabricating a hatch fill effect."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0552 = {
    "F0552_BASE_DisplayError",
    "AMIGA.H:353; BASE.C:1085",
    "Amiga base error display",
    0,
    "The source routine is a host error display/termination surface. PC34 "
    "error behavior is represented by narrower source-mapped callables such "
    "as F0019, so no generic host error dialog is synthesized here."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0557 = {
    "F0557_SCROLLER_Initialize",
    "AMIGA.H:358; SCRLMGMT.C; STARTUP1.C",
    "Amiga-host scroller lifecycle",
    0,
    "The ReDMCSB audits provide no exact Firestaff implementation claim for "
    "this Amiga scroller lifecycle entry. DM1 text-message and scroll content "
    "routes remain separate; no host scroller task is synthesized here."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0558 = {
    "F0558_SCROLLER_CancelInitialize",
    "AMIGA.H:359; SCRLMGMT.C",
    "Amiga-host scroller lifecycle",
    0,
    "The ReDMCSB audits provide no exact Firestaff implementation claim for "
    "this Amiga scroller initialization-cancel entry. No portable scroller "
    "thread or host callback is synthesized."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0559 = {
    "F0559_SCROLLER_Deinitialize",
    "AMIGA.H:360; SCRLMGMT.C; STARTUP1.C",
    "Amiga-host scroller lifecycle",
    0,
    "The ReDMCSB audits provide no exact Firestaff implementation claim for "
    "this Amiga scroller teardown entry. Text-message cleanup remains owned by "
    "the DM1 text runtime, not by a fabricated host scroller."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0562 = {
    "F0562_SCROLLER_Task",
    "AMIGA.H:363; SCRLTASK.C",
    "Amiga-host scroller task",
    0,
    "The ReDMCSB audits provide no exact Firestaff implementation claim for "
    "the Amiga scroller task. Firestaff does not invent a portable background "
    "scroller task for PC3.4."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF0563 = {
    "F0563_SCROLLER_UpdateMessageArea",
    "AMIGA.H:364; SCRLTASK.C",
    "Amiga-host scroller task",
    0,
    "The ReDMCSB audits provide no exact Firestaff implementation claim for "
    "this Amiga scroller message-area updater. Existing DM1 TEXT.C message "
    "area routes remain separate and source-named."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1111 = {
    "F1111_CPSX",
    "AMIGINIT.C:551; FLOPPYAM.C; FIO1.C; HINT001.C; UTIO.C; CNFG.C",
    "Amiga-host CPSX boundary",
    0,
    "The callable audit reports no exact Firestaff implementation claim for "
    "this Amiga CPSX entry. No copy-protection, floppy, file, or hint-oracle "
    "side effect is synthesized."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1133 = {
    "F1133_AddCopperInterrupt",
    "AMIGINIT.C:550; COPERINT.C",
    "Amiga Copper interrupt boundary",
    0,
    "The source route installs an Amiga Copper interrupt path. Firestaff does "
    "not create a portable interrupt handler or display-list substitute here."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1134 = {
    "F1134_RemoveCopperInterrupt",
    "AMIGINIT.C:668; COPERINT.C",
    "Amiga Copper interrupt boundary",
    0,
    "The source route removes an Amiga Copper interrupt path. Firestaff does "
    "not synthesize host interrupt teardown or Copper state."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1135 = {
    "F1135_CopperInterrupt_CPSX",
    "COPERINT.C:9",
    "Amiga Copper interrupt boundary",
    0,
    "The source route is the Amiga Copper interrupt CPSX entry. It has no "
    "portable PC3.4 interrupt contract in the reviewed audits."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1140 = {
    "F1140_InitializeColorPaletteFullBlack",
    "AMIGAVID.C:32",
    "Amiga video palette boundary",
    0,
    "The reviewed audits leave this as an Amiga video palette initialization "
    "surface with no exact Firestaff implementation claim. PC34 black-palette "
    "and curtain behavior remains owned by verified video-driver routes."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1148 = {
    "F1148_CustomExceptCode_CPSX",
    "COPYPROE.C:14",
    "Amiga copy-protection exception boundary",
    0,
    "The source route is copy-protection exception code. Firestaff does not "
    "invent portable exception behavior, disk checks, or protection state."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1149 = {
    "F1149_Init_CPSX",
    "AMIGINIT.C:553; COPYPROE.C",
    "Amiga copy-protection lifecycle",
    0,
    "The source route initializes Amiga copy-protection/platform state. No "
    "portable CPSX initialization or host device state is synthesized."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1150 = {
    "F1150_Free_CPSX",
    "AMIGINIT.C:665; COPYPROE.C",
    "Amiga copy-protection lifecycle",
    0,
    "The source route frees Amiga copy-protection/platform state. No portable "
    "CPSX teardown or host device state is synthesized."
};

static const DM1_V1_AmigaPlatformBoundaryPc34 kF1157 = {
    "F1157_BackupA5",
    "AMIGINIT.C:48",
    "Amiga register/ABI boundary",
    0,
    "The source route is an Amiga register/ABI helper. Firestaff does not "
    "claim a portable A5-register backup contract."
};

const DM1_V1_AmigaPlatformBoundaryPc34*
F0513_DIALOG_DrawGameReadyToPlay_Unreferenced_PlatformBoundaryPc34(void) {
    return &kF0513;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0535_MEMORY_GetGraphicsDatFileSize_PlatformBoundaryPc34(void) {
    return &kF0535;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0551_VIDEO_HatchBox_Unreferenced_PlatformBoundaryPc34(void) {
    return &kF0551;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0552_BASE_DisplayError_PlatformBoundaryPc34(void) {
    return &kF0552;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0557_SCROLLER_Initialize_PlatformBoundaryPc34(void) {
    return &kF0557;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0558_SCROLLER_CancelInitialize_PlatformBoundaryPc34(void) {
    return &kF0558;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0559_SCROLLER_Deinitialize_PlatformBoundaryPc34(void) {
    return &kF0559;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0562_SCROLLER_Task_PlatformBoundaryPc34(void) {
    return &kF0562;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F0563_SCROLLER_UpdateMessageArea_PlatformBoundaryPc34(void) {
    return &kF0563;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1111_CPSX_PlatformBoundaryPc34(void) {
    return &kF1111;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1133_AddCopperInterrupt_PlatformBoundaryPc34(void) {
    return &kF1133;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1134_RemoveCopperInterrupt_PlatformBoundaryPc34(void) {
    return &kF1134;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1135_CopperInterrupt_CPSX_PlatformBoundaryPc34(void) {
    return &kF1135;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1140_InitializeColorPaletteFullBlack_PlatformBoundaryPc34(void) {
    return &kF1140;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1148_CustomExceptCode_CPSX_PlatformBoundaryPc34(void) {
    return &kF1148;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1149_Init_CPSX_PlatformBoundaryPc34(void) {
    return &kF1149;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1150_Free_CPSX_PlatformBoundaryPc34(void) {
    return &kF1150;
}

const DM1_V1_AmigaPlatformBoundaryPc34*
F1157_BackupA5_PlatformBoundaryPc34(void) {
    return &kF1157;
}

int dm1_v1_amiga_platform_boundary_is_portable_pc34(const DM1_V1_AmigaPlatformBoundaryPc34* boundary) {
    if (!boundary) return 0;
    return boundary->has_portable_pc34_equivalent != 0;
}

const char* dm1_v1_amiga_platform_boundary_source_evidence_pc34(void) {
    return "REDMCSB_CALLABLE_SYMBOL_AUDIT.tsv; "
           "REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv; "
           "REDMCSB_MISSING_PLATFORM_BOUNDARIES.tsv";
}
