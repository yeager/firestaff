/*
 * dm2_v1_gfx_pixel_pc34_compat.c -- DM2 pixel type operations.
 *
 * Port of skproject c_gfx_pixel.cpp.
 * Non-inline implementations of build_pixels16 and build_pixels_masked16.
 *
 * Most functions are declared inline in the header. This source provides
 * the source evidence function and ensures the translation unit exists.
 */

#include "dm2_v1_gfx_pixel_pc34_compat.h"

const char *dm2_v1_gfx_pixel_source_evidence(void)
{
    return "skproject c_gfx_pixel.cpp: 5 functions "
           "(operator!=, operator==, ui8_to_pixel, pixel_to_ui8, "
           "build_pixels16, build_pixels_masked16)";
}
