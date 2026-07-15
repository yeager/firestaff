#include "dm2_v1_gdat_hud_material_admission_dm2_010.h"

#include <string.h>

static int dm2_v1_gdat_hud_admission_source_valid(
    const DM2_V1_GdatHudAdmissionSource *source,
    int category,
    int entry_type,
    int field)
{
    return source && source->category == category &&
        source->index >= 0 && source->field >= 0 &&
        (field < 0 || source->field == field) &&
        source->entry_type == entry_type && source->bytes &&
        source->byte_count > 0U && source->content_hash != 0U;
}

int dm2_v1_gdat_hud_material_admit_dm2_010(
    const DM2_V1_GdatHudAdmissionRequest *request,
    DM2_V1_GdatHudAdmissionReceipt *out_receipt)
{
    unsigned int rejected = DM2_V1_GDAT_HUD_ADMISSION_REJECT_NONE;
    size_t required_pixels;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request) {
        out_receipt->rejection_mask =
            DM2_V1_GDAT_HUD_ADMISSION_REJECT_LAYOUT |
            DM2_V1_GDAT_HUD_ADMISSION_REJECT_PALETTE |
            DM2_V1_GDAT_HUD_ADMISSION_REJECT_FONT |
            DM2_V1_GDAT_HUD_ADMISSION_REJECT_CHAMPION |
            DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS;
        return 0;
    }

    if (!dm2_v1_gdat_hud_admission_source_valid(
            &request->layout, DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
            DM2_V1_GDAT_HUD_ADMISSION_RAW7_TYPE,
            DM2_V1_GDAT_HUD_ADMISSION_RECT14_FIELD)) {
        rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_LAYOUT;
    }
    if (!dm2_v1_gdat_hud_admission_source_valid(
            &request->palette, DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
            DM2_V1_GDAT_HUD_ADMISSION_PALETTE16_TYPE,
            DM2_V1_GDAT_HUD_ADMISSION_PALETTE16_FIELD)) {
        rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_PALETTE;
    }
    if (!dm2_v1_gdat_hud_admission_source_valid(
            &request->font, DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
            DM2_V1_GDAT_HUD_ADMISSION_RAW7_TYPE,
            DM2_V1_GDAT_HUD_ADMISSION_FONT_FIELD)) {
        rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_FONT;
    }
    if (!dm2_v1_gdat_hud_admission_source_valid(
            &request->champion, DM2_V1_GDAT_HUD_ADMISSION_CHAMPION_CATEGORY,
            DM2_V1_GDAT_HUD_ADMISSION_IMAGE_TYPE,
            -1)) {
        rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_CHAMPION;
    }

    if (request->champion_width <= 0 || request->champion_height <= 0 ||
        request->champion_stride < request->champion_width ||
        (size_t)request->champion_height > (size_t)-1 /
            (size_t)request->champion_stride) {
        rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS;
    } else {
        required_pixels = (size_t)request->champion_height *
            (size_t)request->champion_stride;
        if (request->champion.byte_count < required_pixels) {
            rejected |= DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS;
        }
    }

    out_receipt->rejection_mask = rejected;
    if (rejected != DM2_V1_GDAT_HUD_ADMISSION_REJECT_NONE) return 0;

    out_receipt->admitted = 1;
    out_receipt->champion_width = request->champion_width;
    out_receipt->champion_height = request->champion_height;
    out_receipt->champion_stride = request->champion_stride;
    out_receipt->champion_pixels = request->champion.bytes;
    out_receipt->champion_pixel_count = request->champion.byte_count;
    return 1;
}
