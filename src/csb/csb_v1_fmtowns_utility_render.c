#include "csb_v1_fmtowns_utility_render.h"

#include "csb_v1_fmtowns_graphics_dat.h"
#include "csb_v1_fmtowns_portrait.h"

#include <string.h>

enum {
    C00_BLACK = 0,
    C01_DARK_GRAY = 1,
    C02_LIGHT_GRAY = 2,
    C03_DARK_BROWN = 3,
    C09_GOLD = 9,
    C15_WHITE = 15,
    C06_FONT_GLYPHS = 70,
    C06_FONT_ROWS = 6,
    C06_FONT_BITS = 5
};

typedef struct C06_Box { int left, right, top, bottom; } C06_Box;

static uint32_t fnv1a(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static void fill(C06_Box box, uint8_t color, uint8_t *pixels)
{
    int x, y;
    if (box.left < 0) box.left = 0;
    if (box.top < 0) box.top = 0;
    if (box.right >= (int)CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH)
        box.right = (int)CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH - 1;
    if (box.bottom >= (int)CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT)
        box.bottom = (int)CSB_V1_FMTOWNS_UTILITY_SCREEN_HEIGHT - 1;
    for (y = box.top; y <= box.bottom; ++y)
        for (x = box.left; x <= box.right; ++x)
            pixels[(size_t)y * CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH + x] = color;
}

/* ReDMCSB CEDT006.C F7030: black full box then four width-n fill strips. */
static void filled_box(C06_Box box, int width, uint8_t fill_color,
                       uint8_t line_color, uint8_t *pixels)
{
    C06_Box inner;
    fill(box, line_color, pixels);
    if (width <= 0) return;
    inner.left = box.left + width;
    inner.right = box.right - width;
    inner.top = box.top + width;
    inner.bottom = box.bottom - width;
    fill(inner, fill_color, pixels);
}

/* ReDMCSB CEDT030.C F7338: six 5-bit rows separated by 0x46 (=70) bytes. */
static void text(const CSB_V1_FmtownsUtilityFontReceipt *font, int x, int y,
                 uint8_t foreground, uint8_t background,
                 const uint8_t *string, size_t max_bytes, uint8_t *pixels)
{
    size_t index;
    if (!font || !string) return;
    for (index = 0u; index < max_bytes && string[index] != 0u; ++index) {
        uint8_t character = string[index];
        int row, column;
        if (character < 0x20u || character >= 0x20u + C06_FONT_GLYPHS) {
            x += 6;
            continue;
        }
        for (row = 0; row < C06_FONT_ROWS; ++row) {
            uint8_t bits = font->source_bytes[(size_t)row * C06_FONT_GLYPHS +
                                               character - 0x20u];
            for (column = 0; column < C06_FONT_BITS; ++column) {
                C06_Box pixel = { x + column, x + column, y - 4 + row,
                                  y - 4 + row };
                fill(pixel, (bits & (uint8_t)(0x10u >> column)) ? foreground :
                     background, pixels);
            }
        }
        x += 6;
    }
}

static const uint8_t *menu_label(const CSB_V1_FmtownsUtilityMenuReceipt *menu,
                                 unsigned int index, size_t *out_max)
{
    uint16_t offset;
    if (!menu || index >= CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT) return NULL;
    offset = menu->label_offsets[index];
    if (offset >= menu->source_size) return NULL;
    if (out_max) *out_max = menu->source_size - offset;
    return menu->source_bytes + offset;
}

static void button(C06_Box box, const uint8_t *label, size_t label_max,
                   const CSB_V1_FmtownsUtilityFontReceipt *font,
                   uint8_t *pixels)
{
    size_t chars = 0u;
    while (chars < label_max && label[chars] != 0u) ++chars;
    filled_box(box, 2, C02_LIGHT_GRAY, C00_BLACK, pixels);
    text(font, (box.left + box.right) / 2 - (int)(chars * 6u) / 2,
         box.bottom - 2, C15_WHITE, C00_BLACK, label, chars, pixels);
}

static void blit(const uint8_t *source, int width, int height, int x, int y,
                 int transparent, uint8_t *pixels)
{
    int source_x, source_y;
    for (source_y = 0; source_y < height; ++source_y) {
        for (source_x = 0; source_x < width; ++source_x) {
            uint8_t color = source[(size_t)source_y * width + source_x];
            if (color != transparent)
                fill((C06_Box){x + source_x, x + source_x, y + source_y,
                                y + source_y}, color, pixels);
        }
    }
}

int csb_v1_fmtowns_utility_render_editor(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt)
{
    static const C06_Box boxes[6] = {
        {2, 92, 186, 194}, {102, 192, 186, 194}, {202, 316, 186, 194},
        {156, 196, 159, 167}, {225, 253, 159, 167}, {288, 316, 5, 13}
    };
    static const C06_Box portrait_boxes[4] = {
        {11, 42, 14, 42}, {78, 109, 14, 42}, {145, 176, 14, 42}, {211, 242, 14, 42}
    };
    static const C06_Box name_boxes[4] = {
        {4, 48, 3, 11}, {71, 115, 3, 11}, {138, 182, 3, 11}, {205, 249, 3, 11}
    };
    uint8_t mirror[48u * 41u];
    uint8_t portrait[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
    CSB_V1_FmtownsItemDecodeReceipt decoded;
    unsigned int index, champion_count;
    size_t max_bytes;
    const uint8_t *label;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!handoff || !menu || !font || !party || !portraits || !pixels ||
        pixel_capacity < CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS ||
        !handoff->valid || !handoff->static_art_verified || !menu->valid ||
        !font->valid || !portraits->valid ||
        handoff->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        menu->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        font->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        portraits->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        party->ChampionCount < 1 || party->ChampionCount > CSB_V1_MAX_CHAMPIONS)
        return 0;
    champion_count = (unsigned int)party->ChampionCount;
    if (selected_champion_index >= champion_count || selected_color_index > 15u)
        return 0;
    memset(&decoded, 0, sizeof(decoded));
    if (!csb_v1_fmtowns_img2_decode(handoff->mirror_bitmap,
                                    CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES,
                                    48u, 41u, mirror, sizeof(mirror), &decoded) ||
        !decoded.valid || decoded.stream_bytes_consumed !=
                         CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES ||
        !csb_v1_fmtowns_portrait_decode_planar(
            portraits->source_bytes[selected_champion_index],
            CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES, portrait, sizeof(portrait)))
        return 0;

    memset(pixels, C00_BLACK, CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
    for (index = 0u; index < 6u; ++index) {
        label = menu_label(menu, index, &max_bytes);
        if (!label) return 0;
        button(boxes[index], label, max_bytes, font, pixels);
    }
    filled_box((C06_Box){157, 252, 60, 146}, 3, C03_DARK_BROWN, C00_BLACK, pixels);
    filled_box((C06_Box){284, 305, 41, 171}, 2, C02_LIGHT_GRAY, C00_BLACK, pixels);
    for (index = 0u; index < champion_count; ++index) {
        if (!csb_v1_fmtowns_portrait_decode_planar(portraits->source_bytes[index],
                                                   CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES,
                                                   portrait, sizeof(portrait))) return 0;
        fill(name_boxes[index], C01_DARK_GRAY, pixels);
        text(font, name_boxes[index].left + 2, 9,
             index == selected_champion_index ? C09_GOLD : C15_WHITE,
             C01_DARK_GRAY,
             (const uint8_t *)party->Champions[index].Name,
             CSB_V1_MAX_NAME_LEN, pixels);
        filled_box(portrait_boxes[index], 1,
                   index == selected_champion_index ? C15_WHITE : C00_BLACK,
                   C01_DARK_GRAY, pixels);
        blit(portrait, 32, 29, portrait_boxes[index].left,
             portrait_boxes[index].top, -1, pixels);
    }
    blit(mirror, 48, 41, 77, 56, -1, pixels);
    blit(portrait, 32, 29, 83, 62, C01_DARK_GRAY, pixels);
    for (index = 0u; index < 32u; ++index) {
        unsigned int y;
        for (y = 0u; y < 29u; ++y) {
            uint8_t color = portrait[y * 32u + index];
            fill((C06_Box){157 + (int)index * 3, 159 + (int)index * 3,
                            60 + (int)y * 3, 62 + (int)y * 3}, color, pixels);
        }
    }
    for (index = 1u; index < 16u; ++index)
        fill((C06_Box){286, 303, 43 + (int)index * 8,
                       49 + (int)index * 8}, (uint8_t)index, pixels);
    /* CEDT006.C F7035/F7036: the selected swatch gets the source colour as
     * its line and white as its two-pass inner box. */
    filled_box((C06_Box){286, 303, 43 + (int)selected_color_index * 8,
                         49 + (int)selected_color_index * 8}, 1,
               C15_WHITE, selected_color_index, pixels);
    filled_box((C06_Box){286, 303, 43 + (int)selected_color_index * 8,
                         49 + (int)selected_color_index * 8}, 1,
               C15_WHITE, selected_color_index, pixels);

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->language = CSB_FMTOWNS_SWITCH_ENGLISH;
        out_receipt->source_fnv1a = handoff->executable_fnv1a ^
                                    menu->source_fnv1a ^ font->source_fnv1a ^
                                    portraits->source_fnv1a;
        out_receipt->pixel_fnv1a = fnv1a(pixels,
                                         CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
        out_receipt->rendered_champion_count = (uint16_t)champion_count;
        out_receipt->selected_champion_index = selected_champion_index;
        out_receipt->selected_color_index = selected_color_index;
        out_receipt->source_evidence =
            "ReDMCSB CEDT006.C F7030/F7031/F7032/F7033/F7034/F7042; "
            "CEDT018.C F0689; CEDT019.C F2124; CEDT030.C F7338";
    }
    return 1;
}

int csb_v1_fmtowns_utility_render_initial(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt)
{
    return csb_v1_fmtowns_utility_render_editor(
        handoff, menu, font, party, portraits, 0u, 0u, pixels,
        pixel_capacity, out_receipt);
}
