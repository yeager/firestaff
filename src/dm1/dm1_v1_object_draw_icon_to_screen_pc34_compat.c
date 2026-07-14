#include "dm1_v1_object_draw_icon_to_screen_pc34_compat.h"

static int dm1_v1_object_icon_box_fits_pc34(
    const DM1_V1_ObjectDrawIconSurfacePc34 *surface,
    int x,
    int y)
{
    return x >= 0 && y >= 0 &&
           x <= surface->screen_width - DM1_V1_OBJECT_ICON_WIDTH_PC34 &&
           y <= surface->screen_height - DM1_V1_OBJECT_ICON_HEIGHT_PC34;
}

int dm1_v1_object_draw_icon_to_screen_f0037_pc34(
    const DM1_V1_ObjectDrawIconSurfacePc34 *surface,
    int icon_index,
    int x,
    int y)
{
    const unsigned char *bitmap_icon;
    int bitmap_row_bytes;
    int row;

    if (!surface || !surface->lookup_icon || !surface->screen_pixels ||
        surface->screen_width <= 0 || surface->screen_height <= 0 ||
        surface->screen_row_bytes < surface->screen_width || icon_index < 0 ||
        !dm1_v1_object_icon_box_fits_pc34(surface, x, y)) {
        return 0;
    }

    /* F0036 extraction must succeed before F0037 constructs its blit box. */
    bitmap_icon = surface->lookup_icon(surface->lookup_context, icon_index,
                                       &bitmap_row_bytes);
    if (!bitmap_icon || bitmap_row_bytes < DM1_V1_OBJECT_ICON_WIDTH_PC34) {
        return 0;
    }

    for (row = 0; row < DM1_V1_OBJECT_ICON_HEIGHT_PC34; ++row) {
        const unsigned char *source =
            bitmap_icon + row * bitmap_row_bytes;
        unsigned char *destination = surface->screen_pixels +
            (y + row) * surface->screen_row_bytes + x;
        int column;

        for (column = 0; column < DM1_V1_OBJECT_ICON_WIDTH_PC34; ++column) {
            if (source[column] != (unsigned char)surface->transparent_color) {
                destination[column] = source[column];
            }
        }
    }
    return 1;
}
