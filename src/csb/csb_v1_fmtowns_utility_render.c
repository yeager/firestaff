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

static size_t c06_text_length(const char *text, size_t maximum)
{
    size_t length = 0u;
    while (length < maximum && text[length] != '\0') ++length;
    return length;
}

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

static uint32_t portrait_catalog_hash(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog)
{
    uint32_t hash = 2166136261u;
    uint16_t index;
    if (!catalog || !catalog->valid || catalog->entry_count == 0u) return 0u;
    for (index = 0u; index < catalog->entry_count; ++index) {
        const CSB_V1_FmtownsUtilityPortraitCatalogEntry *entry =
            &catalog->entries[index];
        size_t i;
        for (i = 0u; i < sizeof(entry->filename) && entry->filename[i]; ++i) {
            hash ^= (uint8_t)entry->filename[i];
            hash *= 16777619u;
        }
        hash ^= entry->source_fnv1a;
        hash *= 16777619u;
        hash ^= entry->portrait.pixel_fnv1a;
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int picker_box_contains(int left, int right, int top, int bottom,
                               int x, int y)
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

int csb_v1_fmtowns_utility_file_picker_open(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t initial_index,
    CSB_V1_FmtownsUtilityFilePicker *out_picker)
{
    uint32_t hash;
    if (!out_picker) return 0;
    memset(out_picker, 0, sizeof(*out_picker));
    hash = portrait_catalog_hash(catalog);
    if (!hash || initial_index >= catalog->entry_count ||
        !catalog->entries[initial_index].portrait.valid) return 0;
    out_picker->valid = 1;
    /* F7083 always resets FirstFileNameIndex before drawing the list. */
    out_picker->first_index = 0u;
    out_picker->selected_index = initial_index;
    out_picker->catalog_fnv1a = hash;
    out_picker->catalog = catalog;
    out_picker->source_evidence =
        "ReDMCSB CEDT008.C F7083/F7084; CEDTDATA.C G2285/G7048/G7049";
    return 1;
}

int csb_v1_fmtowns_utility_file_picker_input(
    CSB_V1_FmtownsUtilityFilePicker *picker,
    int16_t source_x, int16_t source_y,
    int *out_command, int *out_catalog_index)
{
    int command = 0;
    int index = -1;
    int row;
    if (out_command) *out_command = 0;
    if (out_catalog_index) *out_catalog_index = -1;
    if (!picker || !picker->valid || !picker->catalog ||
        picker->catalog_fnv1a != portrait_catalog_hash(picker->catalog)) return 0;
    /* F31E CEDTDATA.C: G2285_as_FilePickerDialogButtons[0..4]. */
    if (picker_box_contains(77, 129, 63, 135, source_x, source_y)) {
        command = CSB_V1_FMTOWNS_FILE_PICKER_FILE_LIST;
        row = ((int)source_y - 62) / 8; /* F7084's FileListTopOnScreen. */
        if (row < 0) row = 0;
        if (row > 8) row = 8;
        index = (int)picker->first_index + row;
        if (index >= (int)picker->catalog->entry_count) index = -1;
        else picker->selected_index = (uint16_t)index;
    } else if (picker_box_contains(165, 237, 106, 114, source_x, source_y)) {
        command = CSB_V1_FMTOWNS_FILE_PICKER_NEW_DISK;
    } else if (picker_box_contains(165, 237, 126, 134, source_x, source_y)) {
        command = CSB_V1_FMTOWNS_FILE_PICKER_CANCEL;
    } else if (picker_box_contains(135, 151, 63, 98, source_x, source_y)) {
        command = CSB_V1_FMTOWNS_FILE_PICKER_UP;
        if (picker->first_index > 0u) {
            picker->first_index--;
        }
    } else if (picker_box_contains(135, 151, 100, 135, source_x, source_y)) {
        command = CSB_V1_FMTOWNS_FILE_PICKER_DOWN;
        if ((unsigned int)picker->catalog->entry_count >
            (unsigned int)picker->first_index + 9u) {
            picker->first_index++;
        }
    } else {
        return 0;
    }
    if (out_command) *out_command = command;
    if (out_catalog_index) *out_catalog_index = index;
    return 1;
}

int csb_v1_fmtowns_utility_render_file_picker(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_FmtownsUtilityFilePicker *picker,
    uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt)
{
    static const uint8_t new_disk[] = "NEW DISK";
    static const uint8_t cancel[] = "CANCEL";
    uint8_t arrows[32u * 75u];
    CSB_V1_FmtownsItemDecodeReceipt decoded;
    uint32_t catalog_hash;
    uint16_t row;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!handoff || !font || !picker || !pixels ||
        pixel_capacity < CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS ||
        !handoff->valid || !handoff->static_art_verified ||
        !handoff->file_picker_arrows_fnv1a || !font->valid ||
        !picker->valid || !picker->catalog ||
        handoff->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        font->language != CSB_FMTOWNS_SWITCH_ENGLISH ||
        picker->catalog_fnv1a != (catalog_hash = portrait_catalog_hash(picker->catalog)))
        return 0;
    memset(&decoded, 0, sizeof(decoded));
    if (!csb_v1_fmtowns_img2_decode_strided(
            handoff->file_picker_arrows,
            CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_BYTES,
            31u, 75u, 32u, arrows, sizeof(arrows), &decoded) ||
        !decoded.valid || decoded.stream_bytes_consumed !=
            CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_STREAM_BYTES)
        return 0;

    memset(pixels, C00_BLACK, CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
    filled_box((C06_Box){62, 255, 48, 149}, 1, C00_BLACK, C02_LIGHT_GRAY, pixels);
    filled_box((C06_Box){77, 129, 63, 135}, 1, C00_BLACK, C02_LIGHT_GRAY, pixels);
    blit(arrows, 32, 75, 134, 62, C01_DARK_GRAY, pixels);
    button((C06_Box){165, 237, 106, 114}, new_disk, sizeof(new_disk) - 1u,
           font, pixels);
    button((C06_Box){165, 237, 126, 134}, cancel, sizeof(cancel) - 1u,
           font, pixels);
    for (row = 0u; row < 9u; ++row) {
        uint16_t index = (uint16_t)(picker->first_index + row);
        if (index >= picker->catalog->entry_count) break;
        text(font, 80, 72 + (int)row * 8, C15_WHITE, C00_BLACK,
             (const uint8_t *)picker->catalog->entries[index].filename,
             sizeof(picker->catalog->entries[index].filename), pixels);
    }
    if (picker->selected_index >= picker->first_index &&
        picker->selected_index < picker->first_index + 9u &&
        picker->selected_index < picker->catalog->entry_count) {
        int y = 62 + ((int)picker->selected_index - picker->first_index) * 8;
        /* F7084 inverts the selected row; this indexed equivalent is
         * deliberately limited to the authenticated list rectangle. */
        for (row = 0u; row < 8u; ++row) {
            int x;
            for (x = 77; x <= 129; ++x)
                pixels[(size_t)(y + row) *
                       CSB_V1_FMTOWNS_UTILITY_SCREEN_WIDTH + (size_t)x] ^= 0x0fu;
        }
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->language = CSB_FMTOWNS_SWITCH_ENGLISH;
        out_receipt->source_fnv1a = handoff->executable_fnv1a ^
                                    font->source_fnv1a ^ catalog_hash;
        out_receipt->pixel_fnv1a = fnv1a(
            pixels, CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
        out_receipt->file_picker_first_index = picker->first_index;
        out_receipt->file_picker_selected_index = picker->selected_index;
        out_receipt->source_evidence =
            "ReDMCSB CEDT008.C F7080/F7081/F7083/F7084; "
            "CEDTINCF.C F7082; CEDTDATA.C G2285/G2284/G7046-G7051";
    }
    return 1;
}

int csb_v1_fmtowns_utility_render_editor(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    int edit_field, uint8_t edit_character_index,
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
    static const C06_Box edit_boxes[2] = {
        {15, 59, 87, 95}, {15, 131, 100, 108}
    };
    uint8_t mirror[48u * 41u];
    uint8_t portrait[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
    uint8_t selected_portrait[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
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
    if (selected_champion_index >= champion_count || selected_color_index > 15u ||
        edit_field < -1 || edit_field > 1)
        return 0;
    memset(&decoded, 0, sizeof(decoded));
    if (!csb_v1_fmtowns_img2_decode(handoff->mirror_bitmap,
                                    CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES,
                                    48u, 41u, mirror, sizeof(mirror), &decoded) ||
        !decoded.valid || decoded.stream_bytes_consumed !=
                         CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES ||
        !csb_v1_fmtowns_portrait_decode_planar(
            portraits->source_bytes[selected_champion_index],
            CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES, selected_portrait,
            sizeof(selected_portrait)))
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
    /* ReDMCSB CEDT006.C F7031/F7033 keeps the selected portrait distinct
     * from the top-row iteration.  Reusing the latter's scratch buffer here
     * made the zoom pane show the final champion whenever the party held
     * more than one original MINI.DAT portrait. */
    blit(selected_portrait, 32, 29, 83, 62, C01_DARK_GRAY, pixels);
    for (index = 0u; index < 32u; ++index) {
        unsigned int y;
        for (y = 0u; y < 29u; ++y) {
            uint8_t color = selected_portrait[y * 32u + index];
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
    /* CEDT006.C F7038/F7027: source text fields and the one-pixel cursor.
     * The font and underscores are native F31 bytes, not host widgets. */
    for (index = 0u; index < 2u; ++index) {
        const uint8_t *value = (const uint8_t *)(index == 0u
            ? party->Champions[selected_champion_index].Name
            : party->Champions[selected_champion_index].Title);
        unsigned int maximum = index == 0u ? 7u : 19u;
        fill(edit_boxes[index], C01_DARK_GRAY, pixels);
        text(font, edit_boxes[index].left + 2, edit_boxes[index].bottom - 2,
             C09_GOLD, C01_DARK_GRAY, value, maximum, pixels);
        text(font, edit_boxes[index].left + 2 +
                    (int)c06_text_length((const char *)value, maximum) * 6,
             edit_boxes[index].bottom - 2, C09_GOLD, C01_DARK_GRAY,
             (const uint8_t *)"___________________" + (19u - maximum +
             c06_text_length((const char *)value, maximum)), maximum, pixels);
    }
    if (edit_field >= 0) {
        C06_Box cursor = edit_boxes[edit_field];
        cursor.left = cursor.right = cursor.left + 1 + (int)edit_character_index * 6;
        fill(cursor, C15_WHITE, pixels);
    }
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
        handoff, menu, font, party, portraits, 0u, 0u, -1, 0u, pixels,
        pixel_capacity, out_receipt);
}

int csb_v1_fmtowns_utility_render_save_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt)
{
    static const uint8_t line1[] = "YOU MAY SAVE EITHER";
    static const uint8_t line2[] = "THE WHOLE GAME OR THE";
    static const uint8_t line3[] = "SELECTED CHAMPION'S PORTRAIT";
    static const uint8_t game[] = "GAME";
    static const uint8_t portrait[] = "PORTRAIT";
    static const uint8_t cancel[] = "CANCEL";
    CSB_V1_FmtownsUtilityRenderReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (!csb_v1_fmtowns_utility_render_editor(
            handoff, menu, font, party, portraits, selected_champion_index,
            selected_color_index, -1, 0u, pixels, pixel_capacity, &receipt)) return 0;
    /* CEDT001.C F7001 calls F7072 with G2254/G2261.  The dialog's text and
     * three labels are the original F31E CEDTDATA strings, rendered with
     * the verified F31 font; its button rectangles are G2261 exactly. */
    filled_box((C06_Box){62, 255, 48, 149}, 2, C01_DARK_GRAY, C00_BLACK,
               pixels);
    text(font, 76, 63, C15_WHITE, C01_DARK_GRAY, line1, sizeof(line1), pixels);
    text(font, 70, 71, C15_WHITE, C01_DARK_GRAY, line2, sizeof(line2), pixels);
    text(font, 64, 79, C15_WHITE, C01_DARK_GRAY, line3, sizeof(line3), pixels);
    button((C06_Box){80, 148, 108, 116}, game, sizeof(game), font, pixels);
    button((C06_Box){165, 237, 108, 116}, portrait, sizeof(portrait), font,
           pixels);
    button((C06_Box){123, 196, 128, 136}, cancel, sizeof(cancel), font,
           pixels);
    receipt.pixel_fnv1a = fnv1a(pixels, CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
    receipt.source_evidence =
        "ReDMCSB CEDT001.C F7001_SaveChampions; CEDTDATA.C G2261/G7069/G7064";
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int csb_v1_fmtowns_utility_render_load_dialog(
    const CSB_V1_FmtownsUtilityHandoffReceipt *handoff,
    const CSB_V1_FmtownsUtilityMenuReceipt *menu,
    const CSB_V1_FmtownsUtilityFontReceipt *font,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion_index, uint8_t selected_color_index,
    uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_FmtownsUtilityRenderReceipt *out_receipt)
{
    static const uint8_t line1[] = "YOU MAY LOAD EITHER";
    static const uint8_t line2[] = "A SAVED GAME OR";
    static const uint8_t line3[] = "A CHAMPION PORTRAIT";
    static const uint8_t game[] = "GAME";
    static const uint8_t portrait[] = "PORTRAIT";
    static const uint8_t cancel[] = "CANCEL";
    CSB_V1_FmtownsUtilityRenderReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (!csb_v1_fmtowns_utility_render_editor(
            handoff, menu, font, party, portraits, selected_champion_index,
            selected_color_index, -1, 0u, pixels, pixel_capacity, &receipt)) return 0;
    /* CEDT001.C F7004 calls F7072 with G2254/G2261.  Its only difference
     * from F7001 is G7068's three original F31E text lines. */
    filled_box((C06_Box){62, 255, 48, 149}, 2, C01_DARK_GRAY, C00_BLACK,
               pixels);
    text(font, 76, 63, C15_WHITE, C01_DARK_GRAY, line1, sizeof(line1), pixels);
    text(font, 79, 71, C15_WHITE, C01_DARK_GRAY, line2, sizeof(line2), pixels);
    text(font, 76, 79, C15_WHITE, C01_DARK_GRAY, line3, sizeof(line3), pixels);
    button((C06_Box){80, 148, 108, 116}, game, sizeof(game), font, pixels);
    button((C06_Box){165, 237, 108, 116}, portrait, sizeof(portrait), font,
           pixels);
    button((C06_Box){123, 196, 128, 136}, cancel, sizeof(cancel), font,
           pixels);
    receipt.pixel_fnv1a = fnv1a(pixels, CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS);
    receipt.source_evidence =
        "ReDMCSB CEDT001.C F7004_LoadChampions; CEDTDATA.C G2261/G7068/G7064";
    if (out_receipt) *out_receipt = receipt;
    return 1;
}
