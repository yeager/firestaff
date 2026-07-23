#include "csb_v1_f1006_f1025_source_boundary_pc34_compat.h"

#include <string.h>

typedef struct { CSB_V1_F1006F1025SourceKindPc34 kind; const char *symbol; const char *anchor; const char *reason; } Spec;
#define LOCAL(symbol, anchor, reason) { CSB_V1_F1006_F1025_LOCAL_OR_UNNUMBERED_PC34, symbol, anchor, reason }
#define OWNER(symbol, anchor, reason) { CSB_V1_F1006_F1025_EXISTING_OWNER_NO_CSB_ADMISSION_PC34, symbol, anchor, reason }
#define PLATFORM(symbol, anchor, reason) { CSB_V1_F1006_F1025_PLATFORM_NONAPPLICABLE_PC34, symbol, anchor, reason }
static const Spec kSpecs[] = {
    LOCAL("F1006_", "BASE.C:1572", "Unnumbered bitmap helper; no authenticated CSB PC34 route."),
    OWNER("F1007_AddMemoryChunk", "MEMORY.C:177-188", "Existing memory owner; no CSB package-backed consumer is proven."),
    OWNER("F1008_GetLargestAvailableMemoryChunk", "MEMORY.C:191-220", "Existing memory owner; no CSB package-backed consumer is proven."),
    PLATFORM("F1009_", "STARTUP2.C:1010-1014", "X68000 startup helper; no PC34 route."),
    PLATFORM("F1010_LoadX68000BorderGraphics", "IMAGE.C:58-138", "X68000 video-memory route; no PC34 branch."),
    PLATFORM("F1011_", "ENDGAME.C:70-76", "Platform-specific endgame helper; no PC34 route."),
    OWNER("F1012_PALETTE_SetCurtain", "DRAWVIEW.C:665-679; TITLE.C:319-324", "Existing palette owner; no authenticated CSB PC34 palette receipt."),
    PLATFORM("F1013_Blit_Amiga", "BLIT.C:2105; BLITAMIG.C", "Amiga bitplane route; no PC34 substitute."),
    PLATFORM("F1014_Unreferenced", "PALETTE.C:21", "No authenticated PC34 call route."),
    PLATFORM("F1015_Unreferenced", "PALETTE.C:24", "No authenticated PC34 call route."),
    PLATFORM("F1016_SetPalette", "PALETTE.C:399", "No independent authenticated CSB PC34 palette receipt."),
    PLATFORM("F1017_Malloc", "CEDT018.C:208", "X68000 native allocation; no PC34 branch."),
    PLATFORM("F1018_Mfree", "CEDT018.C:216", "X68000 native free; no PC34 branch."),
    PLATFORM("F1019_", "STARTUP2.C:1565-1569", "X68000 startup helper; no PC34 route."),
    PLATFORM("F1020_InitializeX68000", "STARTUP2.C:1361-1671", "X68000 IOCS/VDEO route; no PC34 branch."),
    LOCAL("L1021_i_Unreferenced", "CHEST.C:19 F0333", "Function-local chest storage; no standalone CSB route."),
    PLATFORM("F1022_PrintCharacter", "IO2.C:239-248", "X68000 DOS PRINT route; no host console substitute."),
    PLATFORM("F1023_PrintString", "IO2.C:250-258", "X68000 DOS PRINT route; no host console substitute."),
    PLATFORM("F1024_SetTrap14VectorErrorProcessing", "FILE.C:806-836;1091-1126", "X68000 DOS trap route; no PC34 branch."),
    PLATFORM("F1025_GetFloppyDriveStatus", "FILE.C:1128-1151", "X68000 IOCS route; no PC34 branch."),
};

int csb_v1_f1006_f1025_source_boundary_admit_pc34(unsigned int number, CSB_V1_F1006F1025SourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1006F1025SourceBoundaryReceiptPc34 receipt;
    const Spec *spec;
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1006u || number > 1025u) return 0;
    spec = &kSpecs[number - 1006u];
    receipt.function_number = number;
    receipt.source_kind = spec->kind;
    receipt.symbol = spec->symbol;
    receipt.source_anchor = spec->anchor;
    receipt.owner_or_rationale = spec->reason;
    receipt.authentic_pc34_material_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1006_f1025_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB F1006-F1025 has no authenticated CSB PC34 package-backed runtime consumer; all routes fail closed.";
}
