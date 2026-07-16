#ifndef FIRESTAFF_CSB_V1_F0474_F0477_F0478_F0479_F0488_F0490_STARTUP_GRAPHICS_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0474_F0477_F0478_F0479_F0488_F0490_STARTUP_GRAPHICS_BOUNDARIES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum CSB_V1_StartupGraphicsBoundaryStage_PC34 {
    CSB_V1_STARTUP_GRAPHICS_F0474_LOAD_GRAPHIC_PC34 = 1u << 0,
    CSB_V1_STARTUP_GRAPHICS_F0477_OPEN_GRAPHICS_DAT_PC34 = 1u << 1,
    CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34 = 1u << 2,
    CSB_V1_STARTUP_GRAPHICS_F0479_READ_HEADER_PC34 = 1u << 3,
    CSB_V1_STARTUP_GRAPHICS_F0488_EXPAND_BITMAP_PC34 = 1u << 4,
    CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34 = 1u << 5
};

#define CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34 \
    (CSB_V1_STARTUP_GRAPHICS_F0474_LOAD_GRAPHIC_PC34 | \
     CSB_V1_STARTUP_GRAPHICS_F0477_OPEN_GRAPHICS_DAT_PC34 | \
     CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34 | \
     CSB_V1_STARTUP_GRAPHICS_F0479_READ_HEADER_PC34 | \
     CSB_V1_STARTUP_GRAPHICS_F0488_EXPAND_BITMAP_PC34 | \
     CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34)

typedef struct CSB_V1_StartupGraphicsBoundaryReceipt_PC34 {
    uint32_t attempted_stage_mask;
    uint32_t completed_stage_mask;
    uint32_t blocked_stage_mask;
    uint32_t no_synthetic_fallback_mask;
} CSB_V1_StartupGraphicsBoundaryReceipt_PC34;

void csb_v1_f047x_startup_graphics_boundary_receipt_init_pc34(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);

int F0474_MEMORY_LoadGraphic_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);
int F0477_MEMORY_OpenGraphicsDat_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);
void F0478_MEMORY_CloseGraphicsDat_CPSDF(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);
int F0479_MEMORY_ReadGraphicsDatHeader(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);
int F0488_MEMORY_ExpandGraphicToBitmap(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);
int F0490_MEMORY_LoadDecompressAndExpandGraphic(
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt);

const char *csb_v1_f0474_load_graphic_source_evidence_pc34(void);
const char *csb_v1_f0477_open_graphics_dat_source_evidence_pc34(void);
const char *csb_v1_f0478_close_graphics_dat_source_evidence_pc34(void);
const char *csb_v1_f0479_read_graphics_dat_header_source_evidence_pc34(void);
const char *csb_v1_f0488_expand_graphic_source_evidence_pc34(void);
const char *csb_v1_f0490_load_decompress_expand_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0474_F0477_F0478_F0479_F0488_F0490_STARTUP_GRAPHICS_BOUNDARIES_PC34_COMPAT_H */
