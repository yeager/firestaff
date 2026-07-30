#include "csb_v1_csbwin_layout_0232.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

enum {
    CSBWIN_0232_PARTY_DIRECTION_OFFSET = 376,
    CSBWIN_0232_EYE_BOX_OFFSET = 424,
    CSBWIN_0232_MOUTH_BOX_OFFSET = 432,
    CSBWIN_0232_POISON_BOX_OFFSET = 864,
    CSBWIN_0232_FOOD_WATER_BOX_OFFSET = 904,
    CSBWIN_0232_ICON_DISPLAY_OFFSET = 914,
    CSBWIN_0232_OBJECT_GRAPHIC_FIRST_OFFSET = 1218,
    CSBWIN_0232_DEFAULT_GRAPHIC_LIST_OFFSET = 1534,
    CSBWIN_0232_MOVEMENT_BOX_OFFSET = 1802,
    CSBWIN_0232_MAGIC_BOX_OFFSET = 1818,
    CSBWIN_0232_RECT_SIZE = 8
};

static int16_t csb_v1_csbwin_layout_0232_read_be16(const uint8_t *bytes)
{
    return (int16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint16_t csb_v1_csbwin_layout_0232_read_u16be(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static void csb_v1_csbwin_layout_0232_read_rect(
    const uint8_t *bytes, CSB_V1_CSBWinRect0232 *out_rect)
{
    out_rect->x1 = csb_v1_csbwin_layout_0232_read_be16(bytes);
    out_rect->x2 = csb_v1_csbwin_layout_0232_read_be16(bytes + 2);
    out_rect->y1 = csb_v1_csbwin_layout_0232_read_be16(bytes + 4);
    out_rect->y2 = csb_v1_csbwin_layout_0232_read_be16(bytes + 6);
}

int csb_v1_csbwin_layout_0232_rect_is_screen_valid(
    const CSB_V1_CSBWinRect0232 *rect)
{
    return rect && rect->x1 >= 0 && rect->x1 <= rect->x2 &&
        rect->x2 < 320 && rect->y1 >= 0 && rect->y1 <= rect->y2 &&
        rect->y2 < 200;
}

int csb_v1_csbwin_layout_0232_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinLayout0232 *out_layout)
{
    size_t index;

    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!decoded_graphic || decoded_size != CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE) {
        return 0;
    }
    for (index = 0; index < 4u; ++index) {
        csb_v1_csbwin_layout_0232_read_rect(
            decoded_graphic + CSBWIN_0232_PARTY_DIRECTION_OFFSET +
                index * CSBWIN_0232_RECT_SIZE,
            &out_layout->party_direction[index]);
    }
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_EYE_BOX_OFFSET, &out_layout->eye_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_MOUTH_BOX_OFFSET, &out_layout->mouth_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_POISON_BOX_OFFSET, &out_layout->poison_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_FOOD_WATER_BOX_OFFSET,
        &out_layout->food_water_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_MOVEMENT_BOX_OFFSET,
        &out_layout->movement_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_MAGIC_BOX_OFFSET, &out_layout->magic_box);
    for (index = 0; index < CSB_V1_CSBWIN_LAYOUT_0232_ICON_COUNT; ++index) {
        const uint8_t *entry = decoded_graphic +
            CSBWIN_0232_ICON_DISPLAY_OFFSET + index * 6u;
        out_layout->icon_display[index].pixel_x =
            csb_v1_csbwin_layout_0232_read_be16(entry);
        out_layout->icon_display[index].pixel_y =
            csb_v1_csbwin_layout_0232_read_be16(entry + 2u);
        out_layout->icon_display[index].object_type =
            csb_v1_csbwin_layout_0232_read_be16(entry + 4u);
    }
    for (index = 0; index < CSB_V1_CSBWIN_LAYOUT_0232_OBJECT_GRAPHIC_GROUPS;
         ++index) {
        out_layout->object_graphic_first[index] =
            csb_v1_csbwin_layout_0232_read_u16be(decoded_graphic +
                CSBWIN_0232_OBJECT_GRAPHIC_FIRST_OFFSET + index * 2u);
    }
    for (index = 0; index < CSB_V1_CSBWIN_LAYOUT_0232_DEFAULT_GRAPHIC_COUNT;
         ++index) {
        out_layout->default_graphic_list[index] =
            csb_v1_csbwin_layout_0232_read_u16be(decoded_graphic +
                CSBWIN_0232_DEFAULT_GRAPHIC_LIST_OFFSET + index * 2u);
    }
    out_layout->valid = 1;
    return 1;
}

int csb_v1_csbwin_layout_0232_read_graphics_dat(
    const char *graphics_dat_path, CSB_V1_CSBWinLayout0232 *out_layout)
{
    CSB_AtariStLoader loader;
    uint8_t *decoded = NULL;
    int ok = 0;

    if (!out_layout) return 0;
    memset(out_layout, 0, sizeof(*out_layout));
    if (!graphics_dat_path || !graphics_dat_path[0]) return 0;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, graphics_dat_path) ||
        loader.item_count != 563u || loader.items[0x232u].decompressed_size !=
            CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE) {
        goto done;
    }
    decoded = (uint8_t *)malloc(CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE);
    if (!decoded || csb_atari_st_graphics_loader_read_item(
            &loader, 0x232u, decoded,
            CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE) !=
            (int)CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE) {
        goto done;
    }
    ok = csb_v1_csbwin_layout_0232_decode(
        decoded, CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE, out_layout);
done:
    free(decoded);
    csb_atari_st_graphics_loader_close(&loader);
    return ok;
}
