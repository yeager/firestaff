#include "dm1_v1_amiga_platform_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void assert_boundary(const DM1_V1_AmigaPlatformBoundaryPc34* boundary,
                            const char* symbol,
                            const char* source_anchor,
                            const char* rationale_fragment) {
                                (void)rationale_fragment;
                                (void)source_anchor;
                                (void)symbol;
                                (void)boundary;
    assert(boundary != 0);
    assert(strcmp(boundary->symbol, symbol) == 0);
    assert(strstr(boundary->source_anchor, source_anchor) != 0);
    assert(boundary->has_portable_pc34_equivalent == 0);
    assert(dm1_v1_amiga_platform_boundary_is_portable_pc34(boundary) == 0);
    assert(strstr(boundary->rationale, rationale_fragment) != 0);
}

int main(void) {
    const DM1_V1_AmigaPlatformBoundaryPc34* boundary;
    (void)boundary;
    const char* evidence;
    (void)evidence;

    boundary = F0513_DIALOG_DrawGameReadyToPlay_Unreferenced_PlatformBoundaryPc34();
    assert(boundary != 0);
    assert(strcmp(boundary->symbol, "F0513_DIALOG_DrawGameReadyToPlay_Unreferenced") == 0);
    assert(strstr(boundary->source_anchor, "AMIGA.H:304") != 0);
    assert(strstr(boundary->platform_partition, "Amiga-host") != 0);
    assert(boundary->has_portable_pc34_equivalent == 0);
    assert(dm1_v1_amiga_platform_boundary_is_portable_pc34(boundary) == 0);
    assert(strstr(boundary->rationale, "unreferenced route") != 0);
    assert(strstr(boundary->rationale, "no evidenced I34E/I34M PC3.4") != 0);

    assert_boundary(F0535_MEMORY_GetGraphicsDatFileSize_PlatformBoundaryPc34(),
                    "F0535_MEMORY_GetGraphicsDatFileSize",
                    "AMIGA.H:336",
                    "no source-backed portable");

    boundary = F0551_VIDEO_HatchBox_Unreferenced_PlatformBoundaryPc34();
    assert(boundary != 0);
    assert(strcmp(boundary->symbol, "F0551_VIDEO_HatchBox_Unreferenced") == 0);
    assert(strstr(boundary->source_anchor, "AMIGA.H:352") != 0);
    assert(strstr(boundary->source_anchor, "BLITFILL.C:437") != 0);
    assert(boundary->has_portable_pc34_equivalent == 0);
    assert(strstr(boundary->rationale, "unreferenced") != 0);
    assert(strstr(boundary->rationale, "fabricating a hatch fill") != 0);

    boundary = F0552_BASE_DisplayError_PlatformBoundaryPc34();
    assert(boundary != 0);
    assert(strcmp(boundary->symbol, "F0552_BASE_DisplayError") == 0);
    assert(strstr(boundary->source_anchor, "AMIGA.H:353") != 0);
    assert(strstr(boundary->source_anchor, "BASE.C:1085") != 0);
    assert(boundary->has_portable_pc34_equivalent == 0);
    assert(strstr(boundary->rationale, "host error display") != 0);
    assert(strstr(boundary->rationale, "F0019") != 0);

    assert_boundary(F0557_SCROLLER_Initialize_PlatformBoundaryPc34(),
                    "F0557_SCROLLER_Initialize",
                    "AMIGA.H:358",
                    "no exact Firestaff implementation");
    assert_boundary(F0558_SCROLLER_CancelInitialize_PlatformBoundaryPc34(),
                    "F0558_SCROLLER_CancelInitialize",
                    "AMIGA.H:359",
                    "No portable scroller");
    assert_boundary(F0559_SCROLLER_Deinitialize_PlatformBoundaryPc34(),
                    "F0559_SCROLLER_Deinitialize",
                    "AMIGA.H:360",
                    "not by a fabricated host scroller");
    assert_boundary(F0562_SCROLLER_Task_PlatformBoundaryPc34(),
                    "F0562_SCROLLER_Task",
                    "AMIGA.H:363",
                    "does not invent");
    assert_boundary(F0563_SCROLLER_UpdateMessageArea_PlatformBoundaryPc34(),
                    "F0563_SCROLLER_UpdateMessageArea",
                    "AMIGA.H:364",
                    "TEXT.C message");

    assert_boundary(F1111_CPSX_PlatformBoundaryPc34(),
                    "F1111_CPSX",
                    "AMIGINIT.C:551",
                    "No copy-protection");
    assert_boundary(F1133_AddCopperInterrupt_PlatformBoundaryPc34(),
                    "F1133_AddCopperInterrupt",
                    "AMIGINIT.C:550",
                    "portable interrupt handler");
    assert_boundary(F1134_RemoveCopperInterrupt_PlatformBoundaryPc34(),
                    "F1134_RemoveCopperInterrupt",
                    "AMIGINIT.C:668",
                    "host interrupt teardown");
    assert_boundary(F1135_CopperInterrupt_CPSX_PlatformBoundaryPc34(),
                    "F1135_CopperInterrupt_CPSX",
                    "COPERINT.C:9",
                    "no portable PC3.4");
    assert_boundary(F1140_InitializeColorPaletteFullBlack_PlatformBoundaryPc34(),
                    "F1140_InitializeColorPaletteFullBlack",
                    "AMIGAVID.C:32",
                    "verified video-driver routes");
    assert_boundary(F1148_CustomExceptCode_CPSX_PlatformBoundaryPc34(),
                    "F1148_CustomExceptCode_CPSX",
                    "COPYPROE.C:14",
                    "does not invent");
    assert_boundary(F1149_Init_CPSX_PlatformBoundaryPc34(),
                    "F1149_Init_CPSX",
                    "AMIGINIT.C:553",
                    "No portable CPSX initialization");
    assert_boundary(F1150_Free_CPSX_PlatformBoundaryPc34(),
                    "F1150_Free_CPSX",
                    "AMIGINIT.C:665",
                    "No portable CPSX teardown");
    assert_boundary(F1157_BackupA5_PlatformBoundaryPc34(),
                    "F1157_BackupA5",
                    "AMIGINIT.C:48",
                    "A5-register backup");

    assert(dm1_v1_amiga_platform_boundary_is_portable_pc34(0) == 0);

    evidence = dm1_v1_amiga_platform_boundary_source_evidence_pc34();
    assert(evidence != 0);
    assert(strstr(evidence, "REDMCSB_CALLABLE_SYMBOL_AUDIT") != 0);
    assert(strstr(evidence, "REDMCSB_MISSING_PLATFORM_BOUNDARIES") != 0);
    return 0;
}
