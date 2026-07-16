#ifndef FIRESTAFF_CSB_V1_F0902_DRAW_FTL_LOGO_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0902_DRAW_FTL_LOGO_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0902_FTL_LOGO_WIDTH_PC34 = 320,
    CSB_V1_F0902_FTL_LOGO_HEIGHT_PC34 = 200,
    CSB_V1_F0902_FTL_LOGO_PACKED_STRIDE_BYTES_PC34 = 160,
    CSB_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34 = 16
};

typedef struct CSB_V1_FtlLogoFacts_PC34 {
    int valid;
    int source_ftl_logo_bitmap_bound;
    int source_ftl_logo_palette_bound;
    int width;
    int height;
    int packed_stride_bytes;
    int palette_color_count;
    uint32_t source_bitmap_hash;
    uint32_t source_palette_hash;
    int before_swoosh_sound_init;
    int title_not_started_yet;
    int no_synthetic_graphic_bytes;
    int no_synthetic_palette_data;
    int no_legacy_logo_wrapper;
} CSB_V1_FtlLogoFacts_PC34;

typedef struct CSB_V1_FtlLogoReceipt_PC34 {
    int valid;
    int source_bitmap_consumed;
    int source_palette_consumed;
    int width;
    int height;
    int packed_stride_bytes;
    int palette_color_count;
    uint32_t source_bitmap_hash;
    uint32_t source_palette_hash;
    int before_swoosh_sound_init;
    int title_not_started_yet;
    int no_synthetic_graphic_bytes;
    int no_synthetic_palette_data;
    int no_legacy_logo_wrapper;
    const char *source_evidence;
} CSB_V1_FtlLogoReceipt_PC34;

void csb_v1_ftl_logo_receipt_init_pc34(
    CSB_V1_FtlLogoReceipt_PC34 *receipt);

int F0902_DrawFTLLogo(
    const CSB_V1_FtlLogoFacts_PC34 *facts,
    CSB_V1_FtlLogoReceipt_PC34 *out_receipt);

const char *csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0902_DRAW_FTL_LOGO_PC34_COMPAT_H */
