#include "csb_v1_csbwin_layout_0232.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

enum {
    CSBWIN_0232_PARTY_DIRECTION_OFFSET = 376,
    CSBWIN_0232_EYE_BOX_OFFSET = 424,
    CSBWIN_0232_MOUTH_BOX_OFFSET = 432,
    CSBWIN_0232_POISON_BOX_OFFSET = 864,
    CSBWIN_0232_WATER_LABEL_BOX_OFFSET = 872,
    CSBWIN_0232_FOOD_LABEL_BOX_OFFSET = 880,
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
        decoded_graphic + CSBWIN_0232_FOOD_LABEL_BOX_OFFSET,
        &out_layout->food_label_box);
    csb_v1_csbwin_layout_0232_read_rect(
        decoded_graphic + CSBWIN_0232_WATER_LABEL_BOX_OFFSET,
        &out_layout->water_label_box);
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

static int csb_v1_csbwin_layout_0232_append_hud_material(
    CSB_V1_CSBWinHudMaterialPlan0232 *plan,
    CSB_V1_CSBWinHudMaterialKind0232 kind, uint16_t graphic_index,
    uint16_t source_x, const CSB_V1_CSBWinRect0232 *destination)
{
    CSB_V1_CSBWinHudMaterial0232 *entry;

    if (!plan || !destination || plan->count >=
        CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT) return 0;
    entry = &plan->entries[plan->count++];
    entry->kind = kind;
    entry->graphic_index = graphic_index;
    entry->source_x = source_x;
    entry->destination = *destination;
    return 1;
}

int csb_v1_csbwin_layout_0232_build_hud_material_plan(
    const CSB_V1_CSBWinLayout0232 *layout,
    CSB_V1_CSBWinHudMaterialPlan0232 *out_plan)
{
    size_t index;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!layout || !layout->valid ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(
            &layout->food_water_box) ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(
            &layout->food_label_box) ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(
            &layout->water_label_box) ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout->poison_box) ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(
            &layout->movement_box) ||
        !csb_v1_csbwin_layout_0232_rect_is_screen_valid(&layout->magic_box)) {
        return 0;
    }
    for (index = 0; index < 4u; ++index) {
        if (!csb_v1_csbwin_layout_0232_rect_is_screen_valid(
                &layout->party_direction[index]) ||
            !csb_v1_csbwin_layout_0232_append_hud_material(
                out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_DIRECTION, 28u,
                (uint16_t)(index * 19u), &layout->party_direction[index])) {
            memset(out_plan, 0, sizeof(*out_plan));
            return 0;
        }
    }
    if (!csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_FOOD_WATER, 20u, 0u,
            &layout->food_water_box) ||
        !csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_FOOD_LABEL, 30u, 0u,
            &layout->food_label_box) ||
        !csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_WATER_LABEL, 31u, 0u,
            &layout->water_label_box) ||
        !csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_POISON, 32u, 0u,
            &layout->poison_box) ||
        !csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_MOVEMENT, 13u, 0u,
            &layout->movement_box) ||
        !csb_v1_csbwin_layout_0232_append_hud_material(
            out_plan, CSB_V1_CSBWIN_HUD_MATERIAL_MAGIC, 9u, 0u,
            &layout->magic_box)) {
        memset(out_plan, 0, sizeof(*out_plan));
        return 0;
    }
    out_plan->valid = out_plan->count ==
        CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT;
    return out_plan->valid;
}

static uint32_t csb_v1_csbwin_layout_0232_hash_bytes(
    uint32_t hash, const uint8_t *bytes, size_t size)
{
    size_t index;

    for (index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT32_C(16777619);
    }
    return hash;
}

int csb_v1_csbwin_layout_0232_compose_hud(
    const CSB_V1_CSBWinHudMaterialPlan0232 *plan,
    CSB_V1_CSBWinHudPixelResolver0232 resolver, void *resolver_user_data,
    uint8_t *out_pixels, size_t out_size,
    CSB_V1_CSBWinHudCompositionReceipt0232 *out_receipt)
{
    enum { SCREEN_WIDTH = 320, SCREEN_HEIGHT = 200 };
    uint8_t candidate[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t source_hash = UINT32_C(2166136261);
    size_t index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!plan || !plan->valid ||
        plan->count != CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT ||
        !resolver || !out_pixels || out_size < sizeof(candidate)) {
        return 0;
    }
    memcpy(candidate, out_pixels, sizeof(candidate));
    for (index = 0; index < plan->count; ++index) {
        const CSB_V1_CSBWinHudMaterial0232 *entry = &plan->entries[index];
        const uint8_t *source = NULL;
        int source_width = 0;
        int source_height = 0;
        int destination_width;
        int destination_height;
        int row;

        if (!csb_v1_csbwin_layout_0232_rect_is_screen_valid(
                &entry->destination) ||
            !resolver(resolver_user_data, entry->graphic_index, &source,
                      &source_width, &source_height) || !source ||
            source_width <= 0 || source_height <= 0) {
            return 0;
        }
        destination_width = entry->destination.x2 - entry->destination.x1 + 1;
        destination_height = entry->destination.y2 - entry->destination.y1 + 1;
        if (entry->source_x > (uint16_t)source_width ||
            destination_width > source_width - (int)entry->source_x ||
            destination_height > source_height) {
            return 0;
        }
        for (row = 0; row < destination_height; ++row) {
            const uint8_t *source_row = source + (size_t)row *
                (size_t)source_width + entry->source_x;
            uint8_t *destination_row = candidate +
                (size_t)(entry->destination.y1 + row) * SCREEN_WIDTH +
                entry->destination.x1;

            memcpy(destination_row, source_row, (size_t)destination_width);
            source_hash = csb_v1_csbwin_layout_0232_hash_bytes(
                source_hash, source_row, (size_t)destination_width);
        }
    }
    memcpy(out_pixels, candidate, sizeof(candidate));
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->material_count = plan->count;
        out_receipt->source_hash = source_hash;
        out_receipt->composed_hash = csb_v1_csbwin_layout_0232_hash_bytes(
            UINT32_C(2166136261), candidate, sizeof(candidate));
    }
    return 1;
}
