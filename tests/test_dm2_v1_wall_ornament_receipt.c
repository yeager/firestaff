/* skproject SKWIN/SkWinCore.cpp DRAW_DOOR and DRAW_WALL_ORNATE only expose
 * a renderer receipt after the selected original GDAT image, local palette,
 * and scalar route all agree. Fixtures model GDAT structure, not artwork. */
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_wall_ornament.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void put16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static void check(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", name);
    }
}

static size_t append_word(uint8_t *raw, size_t cursor, uint16_t value)
{
    put16le(raw + cursor, value);
    return cursor + 2u;
}

static size_t append_u4_img3(uint8_t *raw, size_t cursor, uint8_t seed)
{
    int palette;

    raw[cursor] = 2u;
    raw[cursor + 2u] = 1u;
    raw[cursor + 3u] = 0x80u;
    raw[cursor + 4u] = 4u;
    raw[cursor + 10u] = (uint8_t)(0x12u | (seed & 0x0fu));
    for (palette = 0; palette < 16; ++palette) {
        raw[cursor + 11u + (size_t)palette] =
            (uint8_t)(seed + (uint8_t)palette);
    }
    return cursor + 27u;
}

int main(void)
{
    uint8_t raw[96];
    uint32_t offsets[8];
    uint32_t sizes[8];
    DM2_V1_GdatEntry entries[8];
    DM2_V1_AssetLoader loader;
    DM2_V1_DoorGdatMaterialReceipt door;
    DM2_V1_WallOrnamentReceipt wall;
    size_t cursor = 0u;
    int i;

    memset(raw, 0, sizeof(raw));
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    memset(entries, 0, sizeof(entries));
    memset(&loader, 0, sizeof(loader));

    /* DOORS/4: dtWordValue IMG_COLORKEY_1 then selected dtImage 0. */
    offsets[0] = (uint32_t)cursor;
    cursor = append_word(raw, cursor, 1u);
    sizes[0] = 2u;
    offsets[1] = (uint32_t)cursor;
    cursor = append_u4_img3(raw, cursor, 0x30u);
    sizes[1] = 27u;
    entries[0].cls1 = DM2_GDAT_CATEGORY_DOORS;
    entries[0].cls2 = 4u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = DM2_V1_DOOR_GDAT_COLORKEY_FIELD;
    entries[0].data_index = 1u;
    entries[1].cls1 = DM2_GDAT_CATEGORY_DOORS;
    entries[1].cls2 = 4u;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[1].cls4 = 0u;
    entries[1].data_index = 1u;

    /* WALL_GFX/7: source scalar inputs and selected front dtImage 1. */
    for (i = 0; i < 5; ++i) {
        static const uint16_t values[5] = { 1u, 13u, 0u, 2u, 0x1234u };
        static const uint8_t fields[5] = { 4u, 5u, 7u, 10u, 0xfdu };
        offsets[2 + i] = (uint32_t)cursor;
        cursor = append_word(raw, cursor, values[i]);
        sizes[2 + i] = 2u;
        entries[2 + i].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
        entries[2 + i].cls2 = 7u;
        entries[2 + i].cls3 = i == 4 ? DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET :
                                       DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
        entries[2 + i].cls4 = fields[i];
        entries[2 + i].data_index = values[i];
    }
    offsets[7] = (uint32_t)cursor;
    cursor = append_u4_img3(raw, cursor, 0x50u);
    sizes[7] = 27u;
    entries[7].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[7].cls2 = 7u;
    entries[7].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[7].cls4 = 1u;
    entries[7].data_index = 7u;

    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 8u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 8u;
    loader.data = raw;
    loader.data_size = cursor;

    check(dm2_v1_door_gdat_material_receipt(&loader, 4u, 0u, &door) &&
              door.valid && door.color_key == 1u && door.door_gfx_index == 4u &&
              door.image_field == 0u && door.image_metadata.width == 2u &&
              door.decoded_pixel_count == 2u && door.local_palette16[0] == 0x30u &&
              door.local_palette_hash != 0u && door.material_hash != 0u,
          "door receipt owns selected source GDAT image and palette");
    check(!dm2_v1_door_gdat_material_receipt(&loader, 4u, 1u, &door),
          "door receipt fails closed for a missing selected image");
    entries[0].data_index = 0u;
    check(!dm2_v1_door_gdat_material_receipt(&loader, 4u, 0u, &door),
          "door receipt does not replace a source-disabled material");
    entries[0].data_index = 1u;

    check(dm2_v1_wall_ornament_material_receipt(&loader, 7u, 1u, &wall) &&
              wall.valid && wall.wall_gfx_index == 7u && wall.image_field == 1u &&
              wall.position == 13u && wall.alcove_type == 2u &&
              wall.item_inside_displacement == 0x1234u &&
              wall.image_metadata.width == 2u && wall.decoded_pixel_count == 2u &&
              wall.local_palette16[0] == 0x50u && wall.material_hash != 0u,
          "wall receipt owns the source-selected ornament image and palette");
    check(dm2_v1_wall_ornament_receipt(&loader, 7u, &wall) &&
              wall.image_field == 1u,
          "legacy front ornate entry retains the source front image field");
    check(!dm2_v1_wall_ornament_material_receipt(&loader, 7u, 2u, &wall),
          "wall receipt fails closed instead of selecting another side image");

    fprintf(stderr, "DM2 door/wall GDAT material receipt: %d failure(s)\n",
            failures);
    return failures == 0 ? 0 : 1;
}
