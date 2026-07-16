#include "csb_v1_f0902_draw_ftl_logo_pc34_compat.h"

#include <string.h>

void csb_v1_ftl_logo_receipt_init_pc34(
    CSB_V1_FtlLogoReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0902_DrawFTLLogo(
    const CSB_V1_FtlLogoFacts_PC34 *facts,
    CSB_V1_FtlLogoReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0902_draw_ftl_logo_source_evidence_pc34();

    csb_v1_ftl_logo_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->source_ftl_logo_bitmap_bound ||
        !facts->source_ftl_logo_palette_bound ||
        facts->width != CSB_V1_F0902_FTL_LOGO_WIDTH_PC34 ||
        facts->height != CSB_V1_F0902_FTL_LOGO_HEIGHT_PC34 ||
        facts->packed_stride_bytes !=
            CSB_V1_F0902_FTL_LOGO_PACKED_STRIDE_BYTES_PC34 ||
        facts->palette_color_count !=
            CSB_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34 ||
        facts->source_bitmap_hash == 0u ||
        facts->source_palette_hash == 0u ||
        !facts->before_swoosh_sound_init ||
        !facts->title_not_started_yet ||
        !facts->no_synthetic_graphic_bytes ||
        !facts->no_synthetic_palette_data ||
        !facts->no_legacy_logo_wrapper) {
        if (out_receipt) {
            out_receipt->no_synthetic_graphic_bytes = 1;
            out_receipt->no_synthetic_palette_data = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->source_bitmap_consumed = 1;
    out_receipt->source_palette_consumed = 1;
    out_receipt->width = facts->width;
    out_receipt->height = facts->height;
    out_receipt->packed_stride_bytes = facts->packed_stride_bytes;
    out_receipt->palette_color_count = facts->palette_color_count;
    out_receipt->source_bitmap_hash = facts->source_bitmap_hash;
    out_receipt->source_palette_hash = facts->source_palette_hash;
    out_receipt->before_swoosh_sound_init = 1;
    out_receipt->title_not_started_yet = 1;
    out_receipt->no_synthetic_graphic_bytes = 1;
    out_receipt->no_synthetic_palette_data = 1;
    out_receipt->no_legacy_logo_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0902_draw_ftl_logo_source_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C F0902_DrawFTLLogo consumes the source "
           "Graphic_FTLLogo 320x200 packed frame with 160-byte rows and its "
           "16-color palette before F0908_InitSound starts the swoosh path; "
           "CSB accepts only caller-bound original logo data";
}
