#include "dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static DM1_V1_StartupVisualGraphicPc34 make_graphic(
    DM1_V1_StartupVisualGraphicRolePc34 role, const uint8_t *pixels,
    size_t size, unsigned int width, unsigned int height, uint32_t fingerprint)
{
    DM1_V1_StartupVisualGraphicPc34 graphic;
    memset(&graphic, 0, sizeof(graphic));
    graphic.role = role;
    graphic.pixels = pixels;
    graphic.pixel_byte_count = size;
    graphic.width = width;
    graphic.height = height;
    graphic.graphics_dat_record_fingerprint = fingerprint;
    graphic.decoded_from_original_graphics_dat = 1;
    graphic.raw_record_verified = 1;
    graphic.indexed_pixels_verified = 1;
    graphic.no_synthetic_wrapper = 1;
    graphic.no_replacement_font = 1;
    return graphic;
}

static DM1_V1_StartupVisualPalettePc34 make_palette(const uint8_t *bytes, uint32_t fingerprint)
{
    DM1_V1_StartupVisualPalettePc34 palette;
    memset(&palette, 0, sizeof(palette));
    palette.rgb6_bytes = bytes;
    palette.byte_count = DM1_V1_STARTUP_PALETTE_BYTE_COUNT_PC34;
    palette.source_fingerprint = fingerprint;
    palette.decoded_from_original_graphics_dat = 1;
    palette.raw_record_verified = 1;
    palette.no_synthetic_palette = 1;
    return palette;
}

int main(void)
{
    const size_t title_size = (size_t)DM1_V1_STARTUP_C001_WIDTH_PC34 * DM1_V1_STARTUP_C001_HEIGHT_PC34;
    const size_t entrance_size = (size_t)DM1_V1_STARTUP_C004_WIDTH_PC34 * DM1_V1_STARTUP_C004_HEIGHT_PC34;
    const size_t door_size = (size_t)DM1_V1_STARTUP_DOOR_WIDTH_PC34 * DM1_V1_STARTUP_DOOR_HEIGHT_PC34;
    uint8_t *title = (uint8_t *)calloc(title_size, 1u);
    uint8_t *entrance = (uint8_t *)calloc(entrance_size, 1u);
    uint8_t *left = (uint8_t *)calloc(door_size, 1u);
    uint8_t *right = (uint8_t *)calloc(door_size, 1u);
    uint8_t palette_bytes[DM1_V1_STARTUP_PALETTE_BYTE_COUNT_PC34] = {0};
    DM1_V1_StartupVisualGraphicPc34 c001;
    DM1_V1_StartupVisualGraphicPc34 c004;
    DM1_V1_StartupVisualGraphicPc34 c002;
    DM1_V1_StartupVisualGraphicPc34 c003;
    DM1_V1_StartupVisualPalettePc34 palette;
    DM1_V1_F0437TitleMaterialReceiptPc34 title_receipt;
    DM1_V1_F0438DoorMaterialReceiptPc34 door_receipt;
    DM1_V1_F0439EntranceMaterialReceiptPc34 entrance_receipt;

    CHECK(title && entrance && left && right);
    if (!title || !entrance || !left || !right) return 1;
    title[0] = 1u;
    title[80u * DM1_V1_STARTUP_C001_WIDTH_PC34] = 2u;
    title[137u * DM1_V1_STARTUP_C001_WIDTH_PC34] = 3u;
    entrance[0] = 4u;
    left[0] = 5u;
    right[0] = 6u;
    palette_bytes[1] = 63u;
    c001 = make_graphic(DM1_V1_STARTUP_VISUAL_C001_TITLE_PC34, title, title_size, 320u, 200u, 1u);
    c004 = make_graphic(DM1_V1_STARTUP_VISUAL_C004_ENTRANCE_PC34, entrance, entrance_size, 320u, 200u, 2u);
    c002 = make_graphic(DM1_V1_STARTUP_VISUAL_C002_LEFT_DOORS_PC34, left, door_size, 256u, 161u, 3u);
    c003 = make_graphic(DM1_V1_STARTUP_VISUAL_C003_RIGHT_DOORS_PC34, right, door_size, 256u, 161u, 4u);
    palette = make_palette(palette_bytes, 5u);

    CHECK(strstr(dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34(), "F0439") != NULL);
    CHECK(dm1_v1_f0437_title_material_admission_pc34(&c001, &palette, &palette, &title_receipt));
    CHECK(title_receipt.accepted && title_receipt.presents_region_bound &&
          title_receipt.dungeon_region_bound && title_receipt.master_region_bound &&
          title_receipt.suppress_synthetic_fallback);
    CHECK(dm1_v1_f0438_door_material_admission_pc34(&c004, &c002, &c003, &palette, &door_receipt));
    CHECK(door_receipt.accepted && door_receipt.source_step_count == 31u &&
          door_receipt.c002_left_door_bound && door_receipt.c003_right_door_bound);
    CHECK(dm1_v1_f0439_entrance_material_admission_pc34(&c004, &palette, &entrance_receipt));
    CHECK(entrance_receipt.accepted && entrance_receipt.no_title_material_substitution);

    c001.no_replacement_font = 0;
    CHECK(!dm1_v1_f0437_title_material_admission_pc34(&c001, &palette, &palette, &title_receipt));
    c001.no_replacement_font = 1;
    c003.raw_record_verified = 0;
    CHECK(!dm1_v1_f0438_door_material_admission_pc34(&c004, &c002, &c003, &palette, &door_receipt));
    c003.raw_record_verified = 1;
    palette_bytes[0] = 64u;
    CHECK(!dm1_v1_f0439_entrance_material_admission_pc34(&c004, &palette, &entrance_receipt));

    free(right);
    free(left);
    free(entrance);
    free(title);
    printf("test_dm1_v1_f0437_f0438_f0439_startup_visual_admission_pc34_compat: %d assertions, %d failures\\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
