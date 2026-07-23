#include "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat.h"

#include <string.h>

static int dm1_v1_non_empty_pixels_pc34(const uint8_t *pixels, size_t size)
{
    size_t index;
    for (index = 0u; index < size; ++index) {
        if (pixels[index] != 0u) return 1;
    }
    return 0;
}

static int dm1_v1_graphic_valid_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *graphic,
    DM1_V1_StartupVisualGraphicRolePc34 role,
    unsigned int width,
    unsigned int height)
{
    const size_t required_bytes = (size_t)width * height;

    return graphic && graphic->role == role && graphic->pixels &&
        graphic->pixel_byte_count >= required_bytes && graphic->width == width &&
        graphic->height == height && graphic->graphics_dat_record_fingerprint != 0u &&
        graphic->decoded_from_original_graphics_dat && graphic->raw_record_verified &&
        graphic->indexed_pixels_verified && graphic->no_synthetic_wrapper &&
        graphic->no_replacement_font &&
        dm1_v1_non_empty_pixels_pc34(graphic->pixels, required_bytes);
}

static int dm1_v1_palette_valid_pc34(const DM1_V1_StartupVisualPalettePc34 *palette)
{
    size_t index;

    if (!palette || !palette->rgb6_bytes ||
        palette->byte_count != DM1_V1_STARTUP_PALETTE_BYTE_COUNT_PC34 ||
        palette->source_fingerprint == 0u ||
        !palette->decoded_from_original_graphics_dat || !palette->raw_record_verified ||
        !palette->no_synthetic_palette) {
        return 0;
    }
    for (index = 0u; index < palette->byte_count; ++index) {
        if (palette->rgb6_bytes[index] > 63u) return 0;
    }
    return dm1_v1_non_empty_pixels_pc34(palette->rgb6_bytes, palette->byte_count);
}

static int dm1_v1_region_has_pixels_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *graphic,
    unsigned int left,
    unsigned int top,
    unsigned int width,
    unsigned int height)
{
    unsigned int row;

    if (!graphic || left + width > graphic->width || top + height > graphic->height) {
        return 0;
    }
    for (row = 0u; row < height; ++row) {
        if (dm1_v1_non_empty_pixels_pc34(
                graphic->pixels + (size_t)(top + row) * graphic->width + left,
                width)) {
            return 1;
        }
    }
    return 0;
}

int dm1_v1_f0437_title_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c001_title,
    const DM1_V1_StartupVisualPalettePc34 *presents_palette,
    const DM1_V1_StartupVisualPalettePc34 *title_palette,
    DM1_V1_F0437TitleMaterialReceiptPc34 *out_receipt)
{
    const int title_valid = dm1_v1_graphic_valid_pc34(
        c001_title, DM1_V1_STARTUP_VISUAL_C001_TITLE_PC34,
        DM1_V1_STARTUP_C001_WIDTH_PC34, DM1_V1_STARTUP_C001_HEIGHT_PC34);
    const int presents = title_valid && dm1_v1_region_has_pixels_pc34(
        c001_title, 0u, 137u, 320u, 16u);
    const int dungeon = title_valid && dm1_v1_region_has_pixels_pc34(
        c001_title, 0u, 0u, 320u, 80u);
    const int master = title_valid && dm1_v1_region_has_pixels_pc34(
        c001_title, 0u, 80u, 320u, 57u);

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!title_valid || !presents || !dungeon || !master ||
        !dm1_v1_palette_valid_pc34(presents_palette) ||
        !dm1_v1_palette_valid_pc34(title_palette)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->c001_title_bound = 1;
        out_receipt->presents_region_bound = 1;
        out_receipt->dungeon_region_bound = 1;
        out_receipt->master_region_bound = 1;
        out_receipt->presents_palette_bound = 1;
        out_receipt->title_palette_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0438_door_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c004_entrance,
    const DM1_V1_StartupVisualGraphicPc34 *c002_left_doors,
    const DM1_V1_StartupVisualGraphicPc34 *c003_right_doors,
    const DM1_V1_StartupVisualPalettePc34 *entrance_palette,
    DM1_V1_F0438DoorMaterialReceiptPc34 *out_receipt)
{
    const int entrance_valid = dm1_v1_graphic_valid_pc34(
        c004_entrance, DM1_V1_STARTUP_VISUAL_C004_ENTRANCE_PC34,
        DM1_V1_STARTUP_C004_WIDTH_PC34, DM1_V1_STARTUP_C004_HEIGHT_PC34);
    const int left_valid = dm1_v1_graphic_valid_pc34(
        c002_left_doors, DM1_V1_STARTUP_VISUAL_C002_LEFT_DOORS_PC34,
        DM1_V1_STARTUP_DOOR_WIDTH_PC34, DM1_V1_STARTUP_DOOR_HEIGHT_PC34);
    const int right_valid = dm1_v1_graphic_valid_pc34(
        c003_right_doors, DM1_V1_STARTUP_VISUAL_C003_RIGHT_DOORS_PC34,
        DM1_V1_STARTUP_DOOR_WIDTH_PC34, DM1_V1_STARTUP_DOOR_HEIGHT_PC34);

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!entrance_valid || !left_valid || !right_valid ||
        !dm1_v1_palette_valid_pc34(entrance_palette)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->c004_entrance_bound = 1;
        out_receipt->c002_left_door_bound = 1;
        out_receipt->c003_right_door_bound = 1;
        out_receipt->entrance_palette_bound = 1;
        out_receipt->source_step_count = 31u;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0439_entrance_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c004_entrance,
    const DM1_V1_StartupVisualPalettePc34 *entrance_palette,
    DM1_V1_F0439EntranceMaterialReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_graphic_valid_pc34(
            c004_entrance, DM1_V1_STARTUP_VISUAL_C004_ENTRANCE_PC34,
            DM1_V1_STARTUP_C004_WIDTH_PC34, DM1_V1_STARTUP_C004_HEIGHT_PC34) ||
        !dm1_v1_palette_valid_pc34(entrance_palette)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->c004_entrance_bound = 1;
        out_receipt->entrance_palette_bound = 1;
        out_receipt->no_title_material_substitution = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34();
    }
    return 1;
}

const char *dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34(void)
{
    return "ReDMCSB TITLE.C:309-410 F0437 uses GRAPHICS.DAT C001 regions "
           "(DUNGEON 0,0 320x80; MASTER 0,80 320x57; PRESENTS 0,137 320x16); "
           "ENTRANCE.C:93-360 F0438 composites C002/C003 door strips for 31 "
           "steps over C004; ENTRANCE.C:371-597 F0439 expands C004 and fades "
           "only to the original entrance palette.";
}
