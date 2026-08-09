#include "csb_v1_viewport_surface_pc34_compat.h"

static const char s_source_evidence[] =
    "ReDMCSB VIEWPORT.C:21-23,62-96 M091_BITPLANE_SIZE(224,136); "
    "CSBWin Viewport.cpp source screen rectangle (48,33,224,136).";

int csb_v1_viewport_screen_surface_pc34(
    uint8_t *screen_pixels, size_t screen_byte_count,
    int screen_width, int screen_height,
    uint8_t **out_viewport_pixels, int *out_stride)
{
    size_t required_bytes;
    size_t offset;

    if (out_viewport_pixels) *out_viewport_pixels = NULL;
    if (out_stride) *out_stride = 0;
    if (!screen_pixels || !out_viewport_pixels || !out_stride ||
        screen_width <= 0 || screen_height <= 0 ||
        screen_width < CSB_V1_VIEWPORT_SCREEN_X_PC34 +
            CSB_V1_VIEWPORT_SCREEN_WIDTH_PC34 ||
        screen_height < CSB_V1_VIEWPORT_SCREEN_Y_PC34 +
            CSB_V1_VIEWPORT_SCREEN_HEIGHT_PC34) {
        return 0;
    }
    required_bytes = (size_t)screen_width * (size_t)screen_height;
    if (screen_byte_count < required_bytes) return 0;
    offset = (size_t)CSB_V1_VIEWPORT_SCREEN_Y_PC34 *
             (size_t)screen_width + CSB_V1_VIEWPORT_SCREEN_X_PC34;
    *out_viewport_pixels = screen_pixels + offset;
    *out_stride = screen_width;
    return 1;
}

const char *csb_v1_viewport_screen_surface_source_evidence_pc34(void)
{
    return s_source_evidence;
}
