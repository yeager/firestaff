#include "dm1_v1_fmtowns_layout_crop.h"

#include <string.h>

typedef struct DM1_FmtownsLayoutRecord {
    uint8_t type;
    uint8_t parent;
    int16_t data1;
    int16_t data2;
} DM1_FmtownsLayoutRecord;

/* ReDMCSB COORD.C G3019_s_LayoutData04 records 700..717, MEDIA508's
 * static F20E/F20J layout chain. The wall-zone records share parent 4;
 * G3016 records 3 and 4 define the 224x136 viewport and its origin. */
static const DM1_FmtownsLayoutRecord g_zone_records_700_717[18] = {
    {11, 4,   0,  0}, {14, 4,   0,  0}, {11, 4,   0, 25},
    {12, 4,   0, 25}, {15, 4,   0, 25}, {11, 4,   0, 25},
    {12, 4,   0, 25}, {15, 4,   0, 20}, {11, 4,   0, 20},
    {12, 4,   0, 20}, {15, 4,   0,  9}, {11, 4,   0,  9},
    {12, 4,   0,  9}, { 1, 3,   0,  0}, {11, 4,   0,  0},
    {12, 4,   0,  0}, {11, 4,  12, 28}, {12, 4, -12, 28}
};

int dm1_v1_fmtowns_wall_panel_route(unsigned int pc34_graphic_index,
                                   int flip_walls,
                                   DM1_V1_FmtownsWallPanelRoute *out_route)
{
    DM1_V1_FmtownsWallPanelRoute route;
    switch (pc34_graphic_index) {
    case 93u: route.source_graphic_base = 84u; route.layout_zone = 717u; break;
    case 94u: route.source_graphic_base = 85u; route.layout_zone = 716u; break;
    case 95u: route.source_graphic_base = 86u; route.layout_zone = 714u; break;
    case 96u: route.source_graphic_base = 86u; route.layout_zone = 713u; break;
    case 97u: route.source_graphic_base = 86u; route.layout_zone = 712u; break;
    case 98u: route.source_graphic_base = 87u; route.layout_zone = 708u; break;
    case 99u: route.source_graphic_base = 87u; route.layout_zone = 707u; break;
    case 100u: route.source_graphic_base = 87u; route.layout_zone = 711u; break;
    case 101u: route.source_graphic_base = 87u; route.layout_zone = 710u; break;
    case 102u: route.source_graphic_base = 87u; route.layout_zone = 709u; break;
    case 103u: route.source_graphic_base = 89u; route.layout_zone = 703u; break;
    case 104u: route.source_graphic_base = 89u; route.layout_zone = 702u; break;
    case 105u: route.source_graphic_base = 88u; route.layout_zone = 706u; break;
    case 106u: route.source_graphic_base = 88u; route.layout_zone = 705u; break;
    case 107u: route.source_graphic_base = 88u; route.layout_zone = 704u; break;
    default:
        if (out_route) memset(out_route, 0, sizeof(*out_route));
        return 0;
    }

    route.flip_horizontally = 0;
    if (pc34_graphic_index == 103u) {
        /* F20 F0128 uses the startup-created mirrored M663 for C703. */
        route.flip_horizontally = 1;
    } else if (flip_walls && pc34_graphic_index != 104u) {
        if (pc34_graphic_index == 93u || pc34_graphic_index == 94u) {
            /* STARTUP2.C swaps the D0L/D0R sources in the flipped family. */
            route.source_graphic_base = pc34_graphic_index == 93u ? 85u : 84u;
            route.flip_horizontally = 1;
        } else {
            route.flip_horizontally = 1;
        }
    }
    if (out_route) *out_route = route;
    return 1;
}

static int dm1_crop_axis(int anchor, int extent, int alignment,
                         int *out_origin, int *out_source)
{
    int origin;
    switch (alignment) {
    case 1: origin = anchor; break;
    case 2: origin = anchor - (extent - 1); break;
    case 3: origin = anchor - ((extent + 1) >> 1); break;
    default: return 0;
    }
    *out_origin = origin < 0 ? 0 : origin;
    *out_source = origin < 0 ? -origin : 0;
    return 1;
}

int dm1_v1_fmtowns_layout_resolve_crop(unsigned int zone_index,
                                      int bitmap_width, int bitmap_height,
                                      DM1_V1_FmtownsLayoutCrop *out_crop)
{
    const DM1_FmtownsLayoutRecord *record;
    int mode;
    int origin_x;
    int origin_y;
    int source_x;
    int source_y;
    int clipped_width;
    int clipped_height;

    if (out_crop) memset(out_crop, 0, sizeof(*out_crop));
    if (!out_crop || zone_index < 700u || zone_index > 717u ||
        bitmap_width <= 0 || bitmap_height <= 0 ||
        bitmap_width > UINT16_MAX || bitmap_height > UINT16_MAX) return 0;
    record = &g_zone_records_700_717[zone_index - 700u];
    if (record->type >= 10u) {
        mode = (int)record->type - 10;
    } else if (record->type == 1u) {
        mode = 1;
    } else {
        return 0;
    }
    /* Record 4 is (type=1,parent=3,0,0); record 3 is the 224x136
     * viewport. Its dimensions supply the offsets used by F0635's
     * left/right/center and top/bottom anchor cases. */
    if (record->parent != 3u && record->parent != 4u) return 0;
    if (mode == 1 || mode == 4 || mode == 8) {
        origin_x = record->data1;
    } else if (mode == 2 || mode == 3 || mode == 6) {
        origin_x = record->data1 + DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH - 1;
    } else if (mode == 5 || mode == 7) {
        origin_x = record->data1 +
            ((DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH + 1) >> 1);
    } else return 0;
    if (mode == 1 || mode == 2 || mode == 5) {
        origin_y = record->data2;
    } else if (mode == 3 || mode == 4 || mode == 7) {
        origin_y = record->data2 + DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT - 1;
    } else if (mode == 6 || mode == 8) {
        origin_y = record->data2 +
            ((DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT + 1) >> 1);
    } else return 0;

    if (!dm1_crop_axis(origin_x, bitmap_width,
                       (mode == 2 || mode == 3 || mode == 6) ? 2 :
                       (mode == 5 || mode == 7) ? 3 : 1,
                       &origin_x, &source_x) ||
        !dm1_crop_axis(origin_y, bitmap_height,
                       (mode == 3 || mode == 4 || mode == 7) ? 2 :
                       (mode == 6 || mode == 8) ? 3 : 1,
                       &origin_y, &source_y)) {
        return 0;
    }
    clipped_width = bitmap_width - source_x;
    clipped_height = bitmap_height - source_y;
    if (clipped_width > DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH - origin_x)
        clipped_width = DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH - origin_x;
    if (clipped_height > DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT - origin_y)
        clipped_height = DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT - origin_y;
    if (clipped_width <= 0 || clipped_height <= 0) return 0;
    out_crop->source_x = source_x;
    out_crop->source_y = source_y;
    out_crop->destination_x = origin_x;
    out_crop->destination_y = origin_y;
    out_crop->width = clipped_width;
    out_crop->height = clipped_height;
    return 1;
}

int dm1_v1_fmtowns_layout_blit(uint8_t *destination,
                              size_t destination_size,
                              int destination_stride,
                              const uint8_t *source,
                              size_t source_size, int source_stride,
                              int bitmap_width, int bitmap_height,
                              unsigned int zone_index,
                              int transparent_index)
{
    DM1_V1_FmtownsLayoutCrop crop;
    size_t dest_required;
    size_t source_required;
    int row;
    int column;

    if (!destination || !source || destination_stride <
            DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH || source_stride < bitmap_width ||
        bitmap_width <= 0 || bitmap_height <= 0 || transparent_index < -1 ||
        transparent_index > 255 ||
        !dm1_v1_fmtowns_layout_resolve_crop(zone_index, bitmap_width,
                                             bitmap_height, &crop)) return 0;
    if ((size_t)(DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT - 1) >
            (SIZE_MAX - DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH) /
                (size_t)destination_stride ||
        (size_t)(bitmap_height - 1) >
            (SIZE_MAX - (size_t)bitmap_width) / (size_t)source_stride) {
        return 0;
    }
    dest_required = (size_t)(DM1_V1_FMTOWNS_LAYOUT_VIEW_HEIGHT - 1) *
        (size_t)destination_stride + DM1_V1_FMTOWNS_LAYOUT_VIEW_WIDTH;
    source_required = (size_t)(bitmap_height - 1) * (size_t)source_stride +
        (size_t)bitmap_width;
    if (destination_size < dest_required || source_size < source_required)
        return 0;
    for (row = 0; row < crop.height; ++row) {
        uint8_t *dst = destination +
            (size_t)(crop.destination_y + row) * (size_t)destination_stride +
            (size_t)crop.destination_x;
        const uint8_t *src = source +
            (size_t)(crop.source_y + row) * (size_t)source_stride +
            (size_t)crop.source_x;
        if (transparent_index < 0) {
            memcpy(dst, src, (size_t)crop.width);
        } else {
            for (column = 0; column < crop.width; ++column) {
                if (src[column] != (uint8_t)transparent_index)
                    dst[column] = src[column];
            }
        }
    }
    return 1;
}
