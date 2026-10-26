#include "dm1_v1_fmtowns_hud_layout.h"

#include <string.h>

/* ReDMCSB COORD.C:1646-1677 G3016_s_LayoutData01,
 * 217-230 G3038_s_LayoutData23, 382-459 G3034_s_LayoutData19 and
 * 470-711 G3033_s_LayoutData18.
 * These are the static MEDIA508 F20 tables. F20 does not populate these
 * zones by loading C696 at runtime; COORD.C F0634/F0635 walks this chain. */
static const DM1_V1_FmtownsHudLayoutRecord g_f20_hud_records[] = {
    {1,   9,   0, 320, 200},
    {2,   1,   1,   0,   0},
    {3,   9,   0, 224, 136},
    {4,   1,   3,   0,   0},
    {110, 9,   2,  39,  29},
    {111, 2, 110, 319,   0},
    {112, 9, 111,  19,  14},
    {113, 1, 112,   0,   0},
    {114, 1, 112,  20,   0},
    {115, 1, 112,  20,  15},
    {116, 1, 112,   0,  15},
    {150, 9,   0,  67,  29},
    {151, 1, 150,   0,   0},
    {152, 1, 150,  69,   0},
    {153, 1, 150, 138,   0},
    {154, 1, 150, 207,   0},
    {155, 9, 151,  43,   7},
    {156, 9, 152,  43,   7},
    {157, 9, 153,  43,   7},
    {158, 9, 154,  43,   7},
    {159, 1, 155,   0,   0},
    {160, 1, 156,   0,   0},
    {161, 1, 157,   0,   0},
    {162, 1, 158,   0,   0},
    {163,18, 159,   1,   0},
    {164,18, 160,   1,   0},
    {165,18, 161,   1,   0},
    {166,18, 162,   1,   0},
    {167,10, 159,   0,   0},
    {168,10, 160,   0,   0},
    {169,10, 161,   0,   0},
    {170,10, 162,   0,   0},
    {171, 9, 151,  32,  29},
    {172, 9, 152,  32,  29},
    {173, 9, 153,  32,  29},
    {174, 9, 154,  32,  29},
    {175, 1, 171,   7,   0},
    {176, 1, 172,   7,   0},
    {177, 1, 173,   7,   0},
    {178, 1, 174,   7,   0},
    {179,10, 175,   0,   0},
    {180,10, 176,   0,   0},
    {181,10, 177,   0,   0},
    {182,10, 178,   0,   0},
    {183, 9, 151,  24,  29},
    {184, 9, 152,  24,  29},
    {185, 9, 153,  24,  29},
    {186, 9, 154,  24,  29},
    {187, 1, 183,  43,   0},
    {188, 1, 184,  43,   0},
    {189, 1, 185,  43,   0},
    {190, 1, 186,  43,   0},
    {191, 9, 187,   4,  25},
    {192, 9, 188,   4,  25},
    {193, 9, 189,   4,  25},
    {194, 9, 190,   4,  25},
    {195, 7, 191,   5,  26},
    {196, 7, 192,   5,  26},
    {197, 7, 193,   5,  26},
    {198, 7, 194,   5,  26},
    {199, 7, 191,  12,  26},
    {200, 7, 192,  12,  26},
    {201, 7, 193,  12,  26},
    {202, 7, 194,  12,  26},
    {203, 7, 191,  19,  26},
    {204, 7, 192,  19,  26},
    {205, 7, 193,  19,  26},
    {206, 7, 194,  19,  26},
    {207, 9, 151,  16,  16},
    {208, 9, 152,  16,  16},
    {209, 9, 153,  16,  16},
    {210, 9, 154,  16,  16},
    {211, 1, 207,   4,  10},
    {212, 1, 207,  24,  10},
    {213, 1, 208,   4,  10},
    {214, 1, 208,  24,  10},
    {215, 1, 209,   4,  10},
    {216, 1, 209,  24,  10},
    {217, 1, 210,   4,  10},
    {218, 1, 210,  24,  10},
    {553, 4,   4,   3,   7},
    {554, 4,   4, 104, 132},
    {555, 3,   4, 218, 132}
};

static int supported_zone(unsigned int zone)
{
    return (zone >= 113u && zone <= 116u) ||
           (zone >= 151u && zone <= 162u) ||
           (zone >= 163u && zone <= 170u) ||
           (zone >= 175u && zone <= 218u) ||
           (zone >= 553u && zone <= 555u);
}

static const DM1_V1_FmtownsHudLayoutRecord *find_record(unsigned int zone)
{
    size_t i;
    for (i = 0; i < sizeof(g_f20_hud_records) /
                        sizeof(g_f20_hud_records[0]); ++i) {
        if (g_f20_hud_records[i].zone_index == zone)
            return &g_f20_hud_records[i];
    }
    return NULL;
}

int dm1_v1_fmtowns_hud_layout_chain(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    DM1_V1_FmtownsHudLayoutChain *out_chain)
{
    DM1_V1_FmtownsHudLayoutChain result;
    unsigned int current = zone_index;
    size_t depth;

    if (out_chain) memset(out_chain, 0, sizeof(*out_chain));
    if (!out_chain ||
        (edition != DM1_V1_FMTOWNS_HUD_EDITION_F20E &&
         edition != DM1_V1_FMTOWNS_HUD_EDITION_F20J) ||
        !supported_zone(zone_index)) return 0;

    memset(&result, 0, sizeof(result));
    for (depth = 0; depth < DM1_V1_FMTOWNS_HUD_LAYOUT_MAX_CHAIN; ++depth) {
        const DM1_V1_FmtownsHudLayoutRecord *record = find_record(current);
        size_t i;
        if (!record || record->record_type > 18u)
            return 0;
        for (i = 0; i < result.record_count; ++i) {
            if (result.records[i].zone_index == record->zone_index) return 0;
        }
        result.records[result.record_count++] = *record;
        if (record->parent_index == 0u) {
            *out_chain = result;
            return 1;
        }
        current = record->parent_index;
    }
    return 0;
}

static int f0635_anchor_offset(int type, int data1, int data2,
                               int parent_width, int parent_height,
                               int *out_x, int *out_y)
{
    if (!out_x || !out_y || parent_width <= 0 || parent_height <= 0)
        return 0;
    switch (type) {
    case 0: *out_x = data1 - ((parent_width + 1) >> 1);
            *out_y = data2 - ((parent_height + 1) >> 1); break;
    case 1: *out_x = data1; *out_y = data2; break;
    case 2: *out_x = data1 - (parent_width - 1); *out_y = data2; break;
    case 3: *out_x = data1 - (parent_width - 1);
            *out_y = data2 - (parent_height - 1); break;
    case 4: *out_x = data1; *out_y = data2 - (parent_height - 1); break;
    case 5: *out_x = data1 - ((parent_width + 1) >> 1); *out_y = data2; break;
    case 6: *out_x = data1 - (parent_width - 1);
            *out_y = data2 - ((parent_height + 1) >> 1); break;
    case 7: *out_x = data1 - ((parent_width + 1) >> 1);
            *out_y = data2 - (parent_height - 1); break;
    case 8: *out_x = data1; *out_y = data2 - ((parent_height + 1) >> 1); break;
    default: return 0;
    }
    return 1;
}

static int f0635_relative_anchor_offset(int type, int *x, int *y,
                                        int parent_width,
                                        int parent_height)
{
    if (!x || !y || parent_width <= 0 || parent_height <= 0) return 0;
    switch (type) {
    case 10: *y += (parent_height + 1) >> 1; /* fall through */
    case 15: *x += (parent_width + 1) >> 1; break;
    case 13: *y += parent_height - 1; /* fall through */
    case 12: *x += parent_width - 1; break;
    case 16: *x += parent_width - 1; /* fall through */
    case 18: *y += (parent_height + 1) >> 1; break;
    case 17: *x += (parent_width + 1) >> 1; /* fall through */
    case 14: *y += parent_height - 1; /* fall through */
    case 11: break;
    default: return 0;
    }
    return 1;
}

static int f0635_place_anchor(int type, int anchor_x, int anchor_y,
                              int width, int height,
                              int *out_x, int *out_y)
{
    if (!out_x || !out_y || width <= 0 || height <= 0) return 0;
    switch (type) {
    case 0: *out_x = anchor_x - ((width + 1) >> 1);
            *out_y = anchor_y - ((height + 1) >> 1); break;
    case 1: *out_x = anchor_x; *out_y = anchor_y; break;
    case 2: *out_x = anchor_x - (width - 1); *out_y = anchor_y; break;
    case 3: *out_x = anchor_x - (width - 1);
            *out_y = anchor_y - (height - 1); break;
    case 4: *out_x = anchor_x; *out_y = anchor_y - (height - 1); break;
    case 5: *out_x = anchor_x - ((width + 1) >> 1); *out_y = anchor_y; break;
    case 6: *out_x = anchor_x - (width - 1);
            *out_y = anchor_y - ((height + 1) >> 1); break;
    case 7: *out_x = anchor_x - ((width + 1) >> 1);
            *out_y = anchor_y - (height - 1); break;
    case 8: *out_x = anchor_x; *out_y = anchor_y - ((height + 1) >> 1); break;
    default: return 0;
    }
    return 1;
}

int dm1_v1_fmtowns_hud_layout_rect(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    int bitmap_width,
    int bitmap_height,
    DM1_V1_FmtownsHudRect *out_rect)
{
    const DM1_V1_FmtownsHudLayoutRecord *leaf;
    const DM1_V1_FmtownsHudLayoutRecord *parent;
    int clip_left = 0;
    int clip_top = 0;
    int clip_right = 20000;
    int clip_bottom = 20000;
    int anchor_x;
    int anchor_y;
    int alignment;
    int shifted = 0;
    int x;
    int y;
    int right;
    int bottom;
    int source_x = 0;
    int source_y = 0;
    unsigned int current;
    size_t depth;
    DM1_V1_FmtownsHudLayoutChain chain;

    if (out_rect) memset(out_rect, 0, sizeof(*out_rect));
    if (!out_rect || !dm1_v1_fmtowns_hud_layout_chain(
            edition, zone_index, &chain))
        return 0;
    leaf = find_record(zone_index);
    if (!leaf || leaf->record_type == 9u) return 0;
    alignment = leaf->record_type <= 8u ? leaf->record_type
                                       : (int)leaf->record_type - 10;
    if (alignment < 0 || alignment > 8) return 0;
    anchor_x = leaf->record_type <= 8u ? leaf->data1 : 0;
    anchor_y = leaf->record_type <= 8u ? leaf->data2 : 0;

    if (bitmap_width <= 0 || bitmap_height <= 0 ||
        bitmap_width > 20000 || bitmap_height > 20000) return 0;

    current = zone_index;
    for (depth = 0; depth < DM1_V1_FMTOWNS_HUD_LAYOUT_MAX_CHAIN; ++depth) {
        const DM1_V1_FmtownsHudLayoutRecord *record = find_record(current);
        if (!record) return 0;
        if (record->parent_index == 0u) break;
        parent = find_record(record->parent_index);
        if (!parent) return 0;

        if (record->record_type >= 10u && record->record_type <= 18u) {
            const DM1_V1_FmtownsHudLayoutRecord *grand =
                find_record(parent->parent_index);
            int parent_type = parent->record_type;
            if (!grand || parent_type > 8 || grand->data1 <= 0 ||
                grand->data2 <= 0) return 0;
            x = parent->data1;
            y = parent->data2;
            if (!f0635_anchor_offset(parent_type, x, y,
                                     grand->data1, grand->data2, &x, &y))
                return 0;
            clip_left += x;
            if (clip_left < x) clip_left = x;
            clip_right += x;
            if (clip_right >= grand->data1 + x)
                clip_right = grand->data1 + x - 1;
            clip_top += y;
            if (clip_top < y) clip_top = y;
            clip_bottom += y;
            if (clip_bottom >= grand->data2 + y)
                clip_bottom = grand->data2 + y - 1;
            if (!f0635_relative_anchor_offset(record->record_type, &x, &y,
                                               grand->data1, grand->data2))
                return 0;
            anchor_x += x + record->data1;
            anchor_y += y + record->data2;
            current = grand->zone_index;
        } else {
            if (parent->record_type == 1u) {
                x = parent->data1;
                y = parent->data2;
                anchor_x += x;
                anchor_y += y;
                clip_left += x;
                if (clip_left < x) clip_left = x;
                clip_right += x;
                clip_top += y;
                if (clip_top < y) clip_top = y;
                clip_bottom += y;
            } else if (parent->record_type == 9u) {
                if (!f0635_anchor_offset(record->record_type,
                                         record->data1, record->data2,
                                         parent->data1, parent->data2,
                                         &x, &y)) return 0;
                if (shifted) {
                    shifted = 0;
                    anchor_x += x;
                    anchor_y += y;
                    clip_left += x;
                    if (clip_left < x) clip_left = x;
                    clip_right += x;
                    clip_top += y;
                    if (clip_top < y) clip_top = y;
                    clip_bottom += y;
                }
                if (clip_left < x) clip_left = x;
                if (clip_right >= parent->data1 + x)
                    clip_right = parent->data1 + x - 1;
                if (clip_top < y) clip_top = y;
                if (clip_bottom >= parent->data2 + y)
                    clip_bottom = parent->data2 + y - 1;
            } else if (parent->record_type <= 8u) {
                shifted = 1;
            } else {
                return 0;
            }
            current = parent->zone_index;
        }
        if (depth + 1u == DM1_V1_FMTOWNS_HUD_LAYOUT_MAX_CHAIN) return 0;
    }

    if (!f0635_place_anchor(alignment, anchor_x, anchor_y,
                            bitmap_width, bitmap_height, &x, &y)) return 0;
    right = clip_right < x + bitmap_width - 1
        ? clip_right : x + bitmap_width - 1;
    bottom = clip_bottom < y + bitmap_height - 1
        ? clip_bottom : y + bitmap_height - 1;
    if (clip_left - x > 0) {
        source_x = clip_left - x;
        x += source_x;
    }
    if (clip_top - y > 0) {
        source_y = clip_top - y;
        y += source_y;
    }
    if (x > right || y > bottom) return 0;
    out_rect->x = x;
    out_rect->y = y;
    out_rect->width = right - x + 1;
    out_rect->height = bottom - y + 1;
    out_rect->source_x = source_x;
    out_rect->source_y = source_y;
    return 1;
}

int dm1_v1_fmtowns_hud_blit_indexed(
    DM1_V1_FmtownsHudEdition edition,
    unsigned int zone_index,
    const uint8_t *source,
    int source_stride,
    int source_height,
    int bitmap_width,
    int bitmap_height,
    int source_x,
    int source_y,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int transparent_index)
{
    DM1_V1_FmtownsHudRect rect;
    int x;
    int y;
    if (!source || !destination || source_stride <= 0 || source_height <= 0 ||
        bitmap_width <= 0 || bitmap_height <= 0 ||
        source_stride > 20000 || source_height > 20000 ||
        bitmap_width > 20000 || bitmap_height > 20000 ||
        source_x < 0 || source_y < 0 || destination_width <= 0 ||
        destination_height <= 0 || destination_width > 20000 ||
        destination_height > 20000 ||
        (long long)source_x + bitmap_width > source_stride ||
        (long long)source_y + bitmap_height > source_height ||
        (long long)destination_width * destination_height > 200000000LL ||
        !dm1_v1_fmtowns_hud_layout_rect(edition, zone_index,
            bitmap_width, bitmap_height, &rect) ||
        rect.source_x + rect.width > bitmap_width ||
        rect.source_y + rect.height > bitmap_height) return 0;

    for (y = 0; y < rect.height; ++y) {
        int destination_y = rect.y + y;
        int source_row = source_y + rect.source_y + y;
        if (destination_y < 0 || destination_y >= destination_height) continue;
        for (x = 0; x < rect.width; ++x) {
            int destination_x = rect.x + x;
            uint8_t pixel;
            if (destination_x < 0 || destination_x >= destination_width) continue;
            pixel = source[source_row * source_stride + source_x +
                           rect.source_x + x];
            if (transparent_index < 0 || pixel != (uint8_t)transparent_index)
                destination[destination_y * destination_width + destination_x] =
                    pixel;
        }
    }
    return 1;
}

int dm1_v1_fmtowns_hud_champion_icon_color(
    DM1_V1_FmtownsHudEdition edition,
    int champion_slot,
    int invisibility_active,
    uint8_t *out_color)
{
    static const uint8_t colors[4] = {7u, 11u, 8u, 14u};
    (void)invisibility_active;
    if (out_color) *out_color = 0u;
    if (!out_color ||
        (edition != DM1_V1_FMTOWNS_HUD_EDITION_F20E &&
         edition != DM1_V1_FMTOWNS_HUD_EDITION_F20J) ||
        champion_slot < 0 || champion_slot >= 4) return 0;
    *out_color = colors[champion_slot];
    return 1;
}
