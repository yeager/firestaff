#include "dm1_v1_f1006_f1025_source_ownership_pc34_compat.h"

static const DM1_V1_F1006F1025OwnershipPc34 kOwnership[] = {
    {1006, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1006_", "BASE.C:1572", "Unnamed bitmap helper; no authenticated PC34 route is evidenced."},
    {1007, DM1_V1_F1006_F1025_SOURCE_OWNER_PC34, "F1007_AddMemoryChunk", "MEMORY.C:177-188", "Existing source-locked memory-chunk owner; caller owns authentic memory region."},
    {1008, DM1_V1_F1006_F1025_SOURCE_OWNER_PC34, "F1008_GetLargestAvailableMemoryChunk", "MEMORY.C:191-220", "Existing source-locked memory-chunk selection owner."},
    {1009, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1009_", "STARTUP2.C:1010-1014", "X68000 startup helper; no PC34 route is evidenced."},
    {1010, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1010_LoadX68000BorderGraphics", "IMAGE.C:58-138", "X68000 video-memory route; existing boundary fails closed."},
    {1011, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1011_", "ENDGAME.C:70-76", "Platform-specific endgame helper; no PC34 route is evidenced."},
    {1012, DM1_V1_F1006_F1025_SOURCE_OWNER_PC34, "F1012_PALETTE_SetCurtain", "DRAWVIEW.C:665-679; TITLE.C:319-324", "Existing PC34 VGA palette owner; only caller-supplied original palette bytes are admissible."},
    {1013, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1013_Blit_Amiga", "BLIT.C:2105; BLITAMIG.C", "Amiga bitplane blitter route; no PC34 substitute is admitted."},
    {1014, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1014_Unreferenced", "PALETTE.C:21", "Unreferenced palette entry; no authenticated PC34 call route."},
    {1015, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1015_Unreferenced", "PALETTE.C:24", "Unreferenced palette entry; no authenticated PC34 call route."},
    {1016, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1016_SetPalette", "PALETTE.C:399", "No independent PC34 palette material receipt; use F1012's authenticated owner or fail closed."},
    {1017, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1017_Malloc", "CEDT018.C:208", "X68000 native allocation; existing boundary returns NULL."},
    {1018, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1018_Mfree", "CEDT018.C:216", "X68000 native free; existing boundary fails closed."},
    {1019, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1019_", "STARTUP2.C:1565-1569", "X68000 startup helper; no PC34 route is evidenced."},
    {1020, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1020_InitializeX68000", "STARTUP2.C:1361-1671", "X68000 IOCS/VDEO route; existing boundary fails closed."},
    {1021, DM1_V1_F1006_F1025_LOCAL_SYMBOL_PC34, "L1021_i_Unreferenced", "CHEST.C:19 F0333", "Function-local CHEST storage; no standalone F1021 route."},
    {1022, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1022_PrintCharacter", "IO2.C:239-248", "X68000 DOS PRINT route; no host console/UI replacement is admitted."},
    {1023, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1023_PrintString", "IO2.C:250-258", "X68000 DOS PRINT route; no host console/UI replacement is admitted."},
    {1024, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1024_SetTrap14VectorErrorProcessing", "FILE.C:806-836;1091-1126", "X68000 DOS TRAP 14 vector route; existing boundary fails closed."},
    {1025, DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34, "F1025_GetFloppyDriveStatus", "FILE.C:1128-1151", "X68000 IOCS B_DRVCHK route; existing boundary fails closed."},
};

const DM1_V1_F1006F1025OwnershipPc34 *
dm1_v1_f1006_f1025_source_ownership_pc34(unsigned int number)
{
    if (number < 1006U || number > 1025U) return 0;
    return &kOwnership[number - 1006U];
}

int dm1_v1_f1006_f1025_admits_authentic_route_pc34(unsigned int number)
{
    const DM1_V1_F1006F1025OwnershipPc34 *entry =
        dm1_v1_f1006_f1025_source_ownership_pc34(number);
    return entry != 0 && entry->kind == DM1_V1_F1006_F1025_SOURCE_OWNER_PC34;
}

int dm1_v1_f1006_f1025_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1006_f1025_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:177-220; IMAGE.C:58-138; DRAWVIEW.C:665-679; "
           "TITLE.C:319-324; BLIT.C:2105; PALETTE.C:21,24,399; "
           "CEDT018.C:208,216; STARTUP2.C:1010-1014,1361-1671; "
           "IO2.C:239-258; FILE.C:806-836,1091-1151. No generated UI, "
           "graphics, timing, palette, console, or platform substitute.";
}
