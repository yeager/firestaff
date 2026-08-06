#include "dm1_v1_fmtowns_title.h"

#include <string.h>

static void blit_scaled(const uint8_t *source, uint16_t source_width,
                        uint16_t source_height, uint8_t *destination,
                        int dst_x, int dst_y, int dst_width, int dst_height,
                        int src_x, int src_y, int src_width, int src_height)
{
    int x;
    int y;
    for (y = 0; y < dst_height; ++y) {
        int source_y = src_y + y * src_height / dst_height;
        if (dst_y + y < 0 || dst_y + y >= (int)DM1_FMTOWNS_TITLE_HEIGHT ||
            source_y < 0 || source_y >= (int)source_height) continue;
        for (x = 0; x < dst_width; ++x) {
            int source_x = src_x + x * src_width / dst_width;
            if (dst_x + x < 0 || dst_x + x >= (int)DM1_FMTOWNS_TITLE_WIDTH ||
                source_x < 0 || source_x >= (int)source_width) continue;
            destination[(size_t)(dst_y + y) * DM1_FMTOWNS_TITLE_WIDTH +
                        (size_t)(dst_x + x)] =
                source[(size_t)source_y * source_width + (size_t)source_x];
        }
    }
}

int dm1_v1_fmtowns_title_compose_frame(
    const DM1_V1_FmtownsStartupReceipt *startup,
    const uint8_t *title_pixels, uint16_t title_width, uint16_t title_height,
    unsigned int frame, uint8_t *out_pixels, size_t out_capacity)
{
    unsigned int zoom_width;
    unsigned int zoom_height;
    int zoom_x;
    int zoom_y;

    if (!startup || !startup->valid ||
        !startup->game_title_animation_plan_verified || !title_pixels ||
        !out_pixels || out_capacity <
            (size_t)DM1_FMTOWNS_TITLE_WIDTH * DM1_FMTOWNS_TITLE_HEIGHT ||
        title_width < DM1_FMTOWNS_TITLE_WIDTH ||
        title_height < 153u || frame > DM1_FMTOWNS_TITLE_FINAL_FRAME ||
        startup->game_title_graphic_index != 1u ||
        startup->game_title_zoom_step_count != DM1_FMTOWNS_TITLE_ZOOM_FRAME_COUNT ||
        startup->game_title_zoom_start_width != DM1_FMTOWNS_TITLE_WIDTH ||
        startup->game_title_zoom_start_height != 80u ||
        startup->game_title_zoom_width_step != 16u ||
        startup->game_title_zoom_height_step != 4u) return 0;

    memset(out_pixels, 0,
           (size_t)DM1_FMTOWNS_TITLE_WIDTH * DM1_FMTOWNS_TITLE_HEIGHT);

    /* EDM.EXP TITLE_PRESENTS: source (0,137), destination (0,90..105). */
    blit_scaled(title_pixels, title_width, title_height, out_pixels,
                0, 90, 320, 16, 0,
                (int)startup->game_title_presents_source_y, 320, 16);

    if (frame == DM1_FMTOWNS_TITLE_PRESENTS_FRAME) {
        /* EDM.EXP + 0xc3f0 draws TITLE_PRESENTS and flips that page before
         * DO_TITLE_ANIMATION prepares and presents any zoom bitmap. */
        return 1;
    }

    if (frame < DM1_FMTOWNS_TITLE_FINAL_FRAME) {
        /* EDM.EXP DO_TITLE_ANIMATION: BX starts at 80, SI at 320, then
         * decrements by 4/16 for 18 prepared bitmaps.  It presents them in
         * reverse order, from the 48x12 centre frame to 320x80 at y=40. */
        unsigned int zoom_step = frame - DM1_FMTOWNS_TITLE_ZOOM_FIRST_FRAME;
        zoom_width = 48u + zoom_step * 16u;
        zoom_height = 12u + zoom_step * 4u;
        zoom_x = ((int)DM1_FMTOWNS_TITLE_WIDTH - (int)zoom_width) / 2;
        zoom_y = 40 + (80 - (int)zoom_height) / 2;
        blit_scaled(title_pixels, title_width, title_height, out_pixels,
                    zoom_x, zoom_y, (int)zoom_width, (int)zoom_height,
                    0, 0, 320, 80);
        return 1;
    }

    /* EDM.EXP TITLE_MASTER: source y=80, destination y=118..174. */
    blit_scaled(title_pixels, title_width, title_height, out_pixels,
                0, 118, 320, 57, 0,
                (int)startup->game_title_master_source_y, 320, 57);
    return 1;
}
