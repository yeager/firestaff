#include "csb_v1_inscription_presentation.h"

#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_viewport_wall_ornament_ordinal_pc34_compat.h"

static int f31j_sjis_lead(uint8_t value)
{
    return (value >= 0x81u && value <= 0x9fu) ||
           (value >= 0xe0u && value <= 0xfcu);
}

static int f31j_sjis_trail(uint8_t value)
{
    return value >= 0x40u && value <= 0xfcu && value != 0x7fu;
}

int csb_v1_f31j_unpack_f0168_text(
    const uint8_t *first_pass, size_t first_pass_size,
    uint8_t *output, size_t output_capacity, size_t *out_size)
{
    size_t source = 0u, destination = 0u;
    if (out_size) *out_size = 0u;
    if (!first_pass || !output || output_capacity == 0u ||
        first_pass_size == 0u) return 0;
    /* DUNGEON.C F0168 preserves one literal prefix byte after a leading A. */
    if (first_pass[source] == 'A') {
        if (++source >= first_pass_size || destination + 1u >= output_capacity)
            return 0;
        output[destination++] = first_pass[source++];
    }
    while (source < first_pass_size && first_pass[source] != 0u) {
        uint8_t value = first_pass[source];
        if (value == 0x81u && source + 1u < first_pass_size &&
            first_pass[source + 1u] == 0u) break;
        if (value >= 'A' && value <= 'P') {
            uint8_t low;
            if (source + 1u >= first_pass_size ||
                (low = first_pass[source + 1u]) < 'A' || low > 'P') return 0;
            value = (uint8_t)(((value - 'A') << 4) | (low - 'A'));
            source += 2u;
        } else {
            ++source;
        }
        if (destination + 1u >= output_capacity) return 0;
        output[destination++] = value;
    }
    if (source >= first_pass_size || destination == 0u) return 0;
    output[destination] = 0u;
    if (out_size) *out_size = destination;
    return 1;
}

int csb_v1_f31j_f0646_printable_substring(
    const uint8_t *source, size_t source_size, size_t *in_out_index,
    int max_width, uint8_t *output, size_t output_capacity,
    CSB_V1_F31JPrintableSubstringReceipt *out_receipt)
{
    size_t at, used = 0u, break_source = (size_t)-1, break_used = 0u;
    int width = 0, break_width = 0, ja = 0, ank = 0;
    int break_ja = 0, break_ank = 0, overflow = 0;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!source || !in_out_index || !output || output_capacity == 0u ||
        *in_out_index >= source_size || max_width <= 0) return 0;
    at = *in_out_index;
    while (at < source_size) {
        uint8_t value = source[at];
        int glyph_width, bytes;
        if (value == 0u || value == '\n') break;
        if (value == '|') {
            ++at;
            break_source = at; break_used = used; break_width = width;
            break_ja = ja; break_ank = ank;
            continue;
        }
        if (value == 0x1bu || value == 0x7cu) {
            glyph_width = 0; bytes = 1;
        } else if (f31j_sjis_lead(value)) {
            if (at + 1u >= source_size || !f31j_sjis_trail(source[at + 1u]))
                return 0;
            glyph_width = 16; bytes = 2;
        } else {
            glyph_width = 8; bytes = 1;
        }
        /* Exact F0646 comparison: a glyph that reaches the limit waits for
         * the next row (width + accumulated >= available). */
        if (width + glyph_width >= max_width) { overflow = 1; break; }
        if (used + (size_t)bytes + 1u > output_capacity) return 0;
        output[used++] = source[at++];
        if (bytes == 2) { output[used++] = source[at++]; ++ja; }
        else if (glyph_width) ++ank;
        width += glyph_width;
    }
    if (overflow) {
        /* MEDIA686 F31J has no X31J's no-break fallback after the source
         * loop. Authentic strings provide '|'; without one the source's
         * result is undefined, so the native host must reject the row. */
        if (break_source == (size_t)-1) return 0;
        at = break_source; used = break_used; width = break_width;
        ja = break_ja; ank = break_ank;
    }
    output[used] = 0u;
    if (used == 0u && width == 0 && at == *in_out_index) return 0;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->source_begin = *in_out_index;
        out_receipt->source_end = at;
        out_receipt->output_size = used;
        out_receipt->pixel_width = width;
        out_receipt->japanese_character_count = ja;
        out_receipt->ank_character_count = ank;
        out_receipt->stopped_at_explicit_break =
            break_source != (size_t)-1 && at == break_source;
    }
    *in_out_index = at;
    return 1;
}

static uint16_t layout_u16(const uint8_t *p, int be)
{ return be ? (uint16_t)((p[0] << 8) | p[1]) : (uint16_t)(p[0] | (p[1] << 8)); }

typedef struct CSB_V1_LayoutRecordView {
    uint16_t type;
    uint16_t parent;
    int16_t data1;
    int16_t data2;
} CSB_V1_LayoutRecordView;

static int layout_record(const uint8_t *layout, size_t size, int be,
                         uint16_t wanted, CSB_V1_LayoutRecordView *out)
{
    uint16_t ranges, r;
    size_t records;
    if (!layout || !out || size < 4 || layout_u16(layout, be) != 0xfc0du)
        return 0;
    ranges = layout_u16(layout + 2, be);
    if (!ranges || 4u + (size_t)ranges * 4u > size) return 0;
    records = 4u + (size_t)ranges * 4u;
    for (r = 0; r < ranges; ++r) {
        uint16_t first = layout_u16(layout + 4u + r * 4u, be);
        uint16_t last = layout_u16(layout + 6u + r * 4u, be);
        uint32_t idx;
        if (last < first) return 0;
        for (idx = first; idx <= last; ++idx, records += 8u) {
            if (records + 8u > size) return 0;
            if (idx == wanted) {
                out->type = layout_u16(layout + records, be);
                out->parent = layout_u16(layout + records + 2u, be);
                out->data1 = (int16_t)layout_u16(layout + records + 4u, be);
                out->data2 = (int16_t)layout_u16(layout + records + 6u, be);
                return 1;
            }
        }
    }
    return 0;
}

int csb_v1_media720_f0635_f31_inventory_rectangles(
    const uint8_t *layout, size_t size, int be,
    CSB_V1_F31InventorySlotRectangle out_rectangles[30])
{
    CSB_V1_LayoutRecordView parent;
    int i;
    if (!out_rectangles ||
        !layout_record(layout, size, be, 105u, &parent) ||
        parent.type != 9u || parent.parent != 4u ||
        parent.data1 != 16 || parent.data2 != 16) return 0;
    for (i = 0; i < 30; ++i) {
        CSB_V1_LayoutRecordView slot;
        if (!layout_record(layout, size, be, (uint16_t)(507 + i), &slot) ||
            slot.type != 1u || slot.parent != 105u) return 0;
        out_rectangles[i].x = slot.data1;
        out_rectangles[i].y = slot.data2;
        out_rectangles[i].width = parent.data1;
        out_rectangles[i].height = parent.data2;
    }
    return 1;
}

int csb_v1_inscription_media720_f0635_lines(
    const uint8_t *layout, size_t size, int be, int out_y[4])
{
    uint16_t ranges, r;
    size_t records;
    int found = 0;
    if (!layout || !out_y || size < 4 || layout_u16(layout, be) != 0xfc0du)
        return 0;
    ranges = layout_u16(layout + 2, be);
    if (!ranges || 4u + (size_t)ranges * 4u > size) return 0;
    records = 4u + (size_t)ranges * 4u;
    for (r = 0; r < ranges; ++r) {
        uint16_t first = layout_u16(layout + 4u + r * 4u, be);
        uint16_t last = layout_u16(layout + 6u + r * 4u, be);
        uint32_t idx;
        if (last < first) return 0;
        for (idx = first; idx <= last; ++idx, records += 8u) {
            uint16_t type, parent;
            if (records + 8u > size) return 0;
            type = layout_u16(layout + records, be);
            parent = layout_u16(layout + records + 2u, be);
            if (idx >= 1000u && idx <= 1003u) {
                if (type != 7u || parent != 4u) return 0;
                out_y[idx - 1000u] = (int16_t)layout_u16(layout + records + 6u, be);
                ++found;
            }
        }
    }
    return found == 4;
}

static void csb_v1_front_square(const CSB_V1_RuntimeProfile *runtime,
                                int *x, int *y)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    *x = runtime->party_x + dx[runtime->party_dir & 3];
    *y = runtime->party_y + dy[runtime->party_dir & 3];
}

static int csb_v1_view_wall_cell(int view_wall_index, int party_dir)
{
    switch (view_wall_index) {
    case 0: case 2: case 7: case 12: /* RIGHT wall ordinal */
        return (party_dir + 1) & 3;
    case 1: case 3: case 8: case 13: /* LEFT wall ordinal */
        return (party_dir + 3) & 3;
    case 4: case 5: case 6: case 9: case 10: case 11: case 14:
        return (party_dir + 2) & 3;  /* FRONT wall ordinal */
    default:
        return -1;
    }
}

static int csb_v1_text_line_count(const char *text)
{
    int lines = 1;
    if (!text || !*text) return 0;
    while (*text) if (*text++ == '\n') ++lines;
    return lines > 4 ? 4 : lines;
}

int csb_v1_unreadable_inscription_shift(
    int view_wall_index, int line_count, int raster_width,
    int *out_shift_x, int *out_shift_y)
{
    static const unsigned char increment[15] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4
    };
    static const unsigned char g0204[15] = {
        5, 8, 13, 7, 13, 20, 5, 12, 19, 10, 17, 27, 11, 22, 33
    };
    int index;
    if (!out_shift_x || !out_shift_y) return 0;
    *out_shift_x = 0;
    *out_shift_y = 0;
    if (view_wall_index < 0 || view_wall_index > 14 ||
        line_count < 1 || line_count > 3 || raster_width <= 0) return 0;
    index = (int)increment[view_wall_index] * 3 + line_count - 1;
    if (index < 0 || index >= 15) return 0;
    *out_shift_x = raster_width;
    *out_shift_y = g0204[index];
    return 1;
}

int csb_v1_wall_aspect_inscription_receipt(
    const CSB_V1_RuntimeProfile *runtime, int map_x, int map_y,
    int view_wall_index, CSB_V1_WallAspectInscriptionReceipt *out)
{
    const CSB_V1_DungeonData *dungeon;
    uint16_t thing, selected = THING_NONE;
    int wall_cell, guard = 0, lines = 0;
    char decoded[256];
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->selected_text_thing = THING_NONE;
    if (!runtime || !(dungeon = runtime->dungeon_handle) ||
        runtime->current_level < 0 ||
        runtime->current_level >= dungeon->level_count ||
        view_wall_index < 0 || view_wall_index > 14 ||
        ((csb_v1_dungeon_get_raw_square(dungeon, runtime->current_level,
                                        map_x, map_y) >> 5) & 7) != 0)
        return 0;
    wall_cell = csb_v1_view_wall_cell(view_wall_index, runtime->party_dir & 3);
    if (wall_cell < 0) return 0;
    thing = (uint16_t)csb_v1_dungeon_get_first_thing(
        dungeon, runtime->current_level, map_x, map_y);
    while (thing != THING_ENDOFLIST && thing != THING_NONE && guard++ < 64) {
        int type = THING_GET_TYPE(thing);
        if (type > 3) break; /* F0172 ordered text/sensor prefix. */
        if (type == THING_TYPE_TEXTSTRING &&
            (int)THING_GET_CELL(thing) == wall_cell &&
            csb_v1_runtime_decode_visible_inscription_text_pc34(
                runtime, thing, decoded, (int)sizeof(decoded))) {
            selected = thing;
            lines = csb_v1_text_line_count(decoded);
            break;
        }
        thing = csb_v1_runtime_next_thing(dungeon, thing);
    }
    if (selected == THING_NONE || lines < 1) return 0;
    out->valid = 1;
    out->view_wall_index = view_wall_index;
    out->wall_cell = wall_cell;
    out->map_x = map_x;
    out->map_y = map_y;
    out->selected_text_thing = selected;
    out->line_count = lines;
    if (lines < 4) {
        int shift_x, shift_y;
        if (!csb_v1_unreadable_inscription_shift(
                view_wall_index, lines, 1, &shift_x, &shift_y)) {
            memset(out, 0, sizeof(*out));
            out->selected_text_thing = THING_NONE;
            return 0;
        }
        out->unreadable_shift_x_is_raster_width = 1;
        out->unreadable_shift_y = shift_y;
    }
    return 1;
}

int csb_v1_wall_aspect_inscription_ordinal_callback(
    void *user_data, int map_x, int map_y, int view_wall_index,
    int *out_inscription_line_count)
{
    CSB_V1_WallAspectInscriptionReceipt receipt;
    if (out_inscription_line_count) *out_inscription_line_count = 0;
    /* D1C readable M648 replaces the unreadable M615 plaque in F0107;
     * M11 consumes that front receipt inside the same candidate page. */
    if (view_wall_index == DM1_V1_VIEW_WALL_D1C_FRONT_PC34) return -1;
    if (!csb_v1_wall_aspect_inscription_receipt(
            (const CSB_V1_RuntimeProfile *)user_data, map_x, map_y,
            view_wall_index, &receipt)) return -1;
    if (out_inscription_line_count)
        *out_inscription_line_count = receipt.line_count;
    return 1; /* F0173: global M615 inscription index 0 -> ordinal 1. */
}

int csb_v1_visible_front_inscription_receipt(
    const CSB_V1_RuntimeProfile *runtime,
    CSB_V1_VisibleInscriptionReceipt *out)
{
    const CSB_V1_DungeonData *dungeon;
    uint16_t thing;
    uint16_t selected = THING_NONE;
    int front = 0, x, y, guard = 0;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->selected_text_thing = THING_NONE;
    if (!runtime || !(dungeon = runtime->dungeon_handle) ||
        runtime->current_level < 0 || runtime->current_level >= dungeon->level_count)
        return 0;
    csb_v1_front_square(runtime, &x, &y);
    if (csb_v1_dungeon_get_raw_square(
            dungeon, runtime->current_level, x, y) < 0 ||
        ((csb_v1_dungeon_get_raw_square(
              dungeon, runtime->current_level, x, y) >> 5) & 7) != 0)
        return 0;
    thing = (uint16_t)csb_v1_dungeon_get_first_thing(
        dungeon, runtime->current_level, x, y);
    while (thing != THING_ENDOFLIST && thing != THING_NONE && guard++ < 64) {
        int type = THING_GET_TYPE(thing);
        char decoded[256];
        if (type > 3) break; /* F0172's ordered text/sensor prefix. */
        if (type == THING_TYPE_TEXTSTRING &&
            csb_v1_runtime_decode_visible_inscription_text_pc34(
                runtime, thing, decoded, (int)sizeof(decoded))) {
            /* F0172 publishes G0290 for every visible C02 (BUG0_76), while
             * the facing ornament itself belongs to cell direction + 2. */
            selected = thing;
            if ((((int)THING_GET_CELL(thing) - runtime->party_dir) & 3) == 2)
                front = 1;
        }
        thing = csb_v1_runtime_next_thing(dungeon, thing);
    }
    if (!front || selected == THING_NONE ||
        !csb_v1_runtime_decode_visible_inscription_text_pc34(
            runtime, selected, out->source_text,
            (int)sizeof(out->source_text))) return 0;
    out->valid = 1;
    out->front_wall_has_inscription = 1;
    out->map_x = x;
    out->map_y = y;
    out->selected_text_thing = selected;
    return 1;
}

int csb_v1_inscription_presentation_plan(
    CSB_V1_VariantId variant,
    CSB_V1_InscriptionPresentationPlan *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->glyph_width = 8;
    out->glyph_height = 8;
    out->transparent_colour = 10;
    out->line_y[0] = 48;
    out->line_y[1] = 59;
    out->line_y[2] = 75;
    out->line_y[3] = 86;
    switch (variant) {
        /* DEFS.H MEDIA720: A31/A35/F31 use M648=258.  English releases
         * consume the decoded C02 bytes as 8x8 cells in F0107. */
        case CSB_V1_VARIANT_AMIGA31_EN:
        case CSB_V1_VARIANT_AMIGA31_MULTI:
        case CSB_V1_VARIANT_AMIGA35_EN:
        case CSB_V1_VARIANT_AMIGA35_MULTI:
        case CSB_V1_VARIANT_FMTOWNS_EN:
            out->font_graphic = 258u;
            out->fixed_geometry = 0; /* F0635 zone geometry, source tables. */
            out->valid = 1;
            return 1;
        /* DEFS.H MEDIA020: F20E uses M648=120 and the original fixed
         * G0203 geometry. */
        case CSB_V1_VARIANT_ST_F20E:
            out->font_graphic = 120u;
            out->fixed_geometry = 1;
            out->valid = 1;
            return 1;
        /* S20/S21 are MEDIA020 as well. */
        case CSB_V1_VARIANT_ST20_EN:
        case CSB_V1_VARIANT_ST21_EN:
            out->font_graphic = 120u;
            out->fixed_geometry = 1;
            out->valid = 1;
            return 1;
        /* F31J uses F0644 and a Japanese font/substring pipeline, not M648.
         * Fail closed until that selected-media material is represented. */
        case CSB_V1_VARIANT_FMTOWNS_JA:
        default:
            return 0;
    }
}
