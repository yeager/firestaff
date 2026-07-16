#include "csb_v1_f0474_f0477_f0478_f0479_f0488_f0490_startup_graphics_boundaries_pc34_compat.h"

#include <string.h>

static void csb_v1_startup_graphics_mark_blocked_pc34(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt, uint32_t stage)
{
    if (!receipt) return;
    receipt->attempted_stage_mask |= stage;
    receipt->blocked_stage_mask |= stage;
    receipt->no_synthetic_fallback_mask |= stage;
}

void csb_v1_f047x_startup_graphics_boundary_receipt_init_pc34(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0474_MEMORY_LoadGraphic_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    csb_v1_startup_graphics_mark_blocked_pc34(
        receipt, CSB_V1_STARTUP_GRAPHICS_F0474_LOAD_GRAPHIC_PC34);
    return 0;
}

int F0477_MEMORY_OpenGraphicsDat_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    csb_v1_startup_graphics_mark_blocked_pc34(
        receipt, CSB_V1_STARTUP_GRAPHICS_F0477_OPEN_GRAPHICS_DAT_PC34);
    return 0;
}

void F0478_MEMORY_CloseGraphicsDat_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    if (!receipt) return;
    receipt->attempted_stage_mask |=
        CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34;
    receipt->completed_stage_mask |=
        CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34;
    receipt->no_synthetic_fallback_mask |=
        CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34;
}

int F0479_MEMORY_ReadGraphicsDatHeader(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    csb_v1_startup_graphics_mark_blocked_pc34(
        receipt, CSB_V1_STARTUP_GRAPHICS_F0479_READ_HEADER_PC34);
    return 0;
}

int F0488_MEMORY_ExpandGraphicToBitmap(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    csb_v1_startup_graphics_mark_blocked_pc34(
        receipt, CSB_V1_STARTUP_GRAPHICS_F0488_EXPAND_BITMAP_PC34);
    return 0;
}

int F0490_MEMORY_LoadDecompressAndExpandGraphic(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    csb_v1_startup_graphics_mark_blocked_pc34(
        receipt, CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34);
    return 0;
}

const char *csb_v1_f0474_load_graphic_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:707-731 F0474_MEMORY_LoadGraphic_CPSDF reads a "
           "selected GRAPHICS.DAT archive member; CSB startup consumes the "
           "real data route in csb_v1_startup_runtime_surfaces_pc34_compat.c:"
           "102-105 and does not fabricate compressed bytes";
}

const char *csb_v1_f0477_open_graphics_dat_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:1212-1285 F0477_MEMORY_OpenGraphicsDat_CPSDF "
           "opens and reference-counts GRAPHICS.DAT; CSB startup calls the "
           "PC34 compat open at csb_v1_startup_runtime_surfaces_pc34_compat.c:"
           "82-84 only when a real verified path is present";
}

const char *csb_v1_f0478_close_graphics_dat_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:1287-1295 F0478_MEMORY_CloseGraphicsDat_CPSDF "
           "closes GRAPHICS.DAT after reference-count teardown; CSB startup "
           "closes the compat file state at "
           "csb_v1_startup_runtime_surfaces_pc34_compat.c:133";
}

const char *csb_v1_f0479_read_graphics_dat_header_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:1330-1351 F0479_MEMORY_ReadGraphicsDatHeader "
           "opens GRAPHICS.DAT and reads the header tables; CSB startup "
           "initializes and consumes those tables at "
           "csb_v1_startup_runtime_surfaces_pc34_compat.c:82-91";
}

const char *csb_v1_f0488_expand_graphic_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:2474-2503 F0488_MEMORY_ExpandGraphicToBitmap "
           "expands an already loaded graphic to a bitmap; CSB startup keeps "
           "this boundary explicit because C001-C005 use the PC3.4/CSBWin "
           "IMG3 four-plane decode at "
           "csb_v1_startup_runtime_surfaces_pc34_compat.c:105-122";
}

const char *csb_v1_f0490_load_decompress_expand_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:2583-2654 "
           "F0490_MEMORY_LoadDecompressAndExpandGraphic owns the "
           "open/select/load/decompress/expand sequence; CSB startup mirrors "
           "that real archive/LZW boundary at "
           "csb_v1_startup_runtime_surfaces_pc34_compat.c:82-133";
}
