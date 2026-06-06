#include "dm1_v1_ceiling_pit_viewport_pc34_compat.h"

#include <stddef.h>

/* ReDMCSB DUNVIEW.C:568-570, Graphic558 frame format:
 * { X1, X2, Y1, Y2, ByteWidth, Height, 0, 0 }.  F0112:4341-4470
 * delegates to F0104/F0105, which skip C10 transparent pixels while blitting
 * the selected ceiling-pit bitmap into the frame/zone rectangle. */
static const DM1V1CeilingPitViewportRectPc34 s_d2l = {0, 19, 80, 5};
static const DM1V1CeilingPitViewportRectPc34 s_d2c = {64, 19, 96, 5};
static const DM1V1CeilingPitViewportRectPc34 s_d2r = {144, 19, 80, 5};

static int graphic_is_d2l(int graphic)
{
    return graphic == DM1_V1_GRAPHIC_CEILING_PIT_D2L_PC34 ||
           graphic == DM1_V1_GRAPHIC_CEILING_PIT_D2L_I34E;
}

static int graphic_is_d2c(int graphic)
{
    return graphic == DM1_V1_GRAPHIC_CEILING_PIT_D2C_PC34 ||
           graphic == DM1_V1_GRAPHIC_CEILING_PIT_D2C_I34E;
}

const DM1V1CeilingPitViewportRectPc34 *
dm1_v1_ceiling_pit_viewport_rect_pc34(int ceiling_pit_graphic,
                                      int zone,
                                      int parity_flag)
{
    if ((zone == DM1_V1_ZONE_CEILING_PIT_D2L_PC34 && graphic_is_d2l(ceiling_pit_graphic)) ||
        (zone == DM1_V1_ZONE_CEILING_PIT_D2L_I34E && graphic_is_d2l(ceiling_pit_graphic) && !parity_flag)) {
        return &s_d2l;
    }
    if ((zone == DM1_V1_ZONE_CEILING_PIT_D2C_PC34 && graphic_is_d2c(ceiling_pit_graphic)) ||
        (zone == DM1_V1_ZONE_CEILING_PIT_D2C_I34E && graphic_is_d2c(ceiling_pit_graphic))) {
        return &s_d2c;
    }
    if ((zone == DM1_V1_ZONE_CEILING_PIT_D2R_PC34 && graphic_is_d2l(ceiling_pit_graphic) && parity_flag) ||
        (zone == DM1_V1_ZONE_CEILING_PIT_D2R_I34E && graphic_is_d2l(ceiling_pit_graphic) && parity_flag)) {
        return &s_d2r;
    }
    return NULL;
}

int dm1_v1_ceiling_pit_viewport_draw_pc34(int ceiling_pit_graphic,
                                          int zone,
                                          int map_x,
                                          int map_y,
                                          int parity_flag,
                                          unsigned char *dest,
                                          int dest_width,
                                          int dest_height,
                                          int dest_stride,
                                          const unsigned char *source,
                                          int source_width,
                                          int source_height)
{
    const DM1V1CeilingPitViewportRectPc34 *rect;
    int writes = 0;
    int y;

    (void)map_x;
    (void)map_y;

    rect = dm1_v1_ceiling_pit_viewport_rect_pc34(ceiling_pit_graphic, zone, parity_flag);
    if (!rect || !dest || !source || dest_width <= 0 || dest_height <= 0 ||
        dest_stride < dest_width || source_width <= 0 || source_height <= 0) {
        return 0;
    }

    for (y = 0; y < rect->height && y < source_height; ++y) {
        int dest_y = rect->y + y;
        int x;
        if (dest_y < 0 || dest_y >= dest_height) {
            continue;
        }
        for (x = 0; x < rect->width && x < source_width; ++x) {
            int source_x = parity_flag ? (rect->width - 1 - x) : x;
            int dest_x = rect->x + x;
            unsigned char pixel;
            if (source_x < 0 || source_x >= source_width || dest_x < 0 || dest_x >= dest_width) {
                continue;
            }
            pixel = source[y * source_width + source_x];
            if (pixel == DM1_V1_CEILING_PIT_TRANSPARENT_PC34) {
                continue;
            }
            dest[dest_y * dest_stride + dest_x] = pixel;
            ++writes;
        }
    }

    return writes;
}
