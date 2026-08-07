#ifndef FIRESTAFF_DM1_V2_FILTERS_H
#define FIRESTAFF_DM1_V2_FILTERS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V2_PALETTE_LEVELS 6

/* Compatibility-only V2.0 filter API. ReDMCSB owns palette selection and
 * presentation; no PC34 route supports CRT rows, neighbourhood rewriting,
 * interpolated brightness or unsharp masking. The four pixel entries return
 * success for valid buffers without changing them. Palette-LUT construction
 * copies the authenticated six-level VGA table exactly, regardless of host
 * gamma, brightness or contrast arguments. */
int dm1_v2_filter_palette_interpolate_indexed(unsigned char* fb, int w, int h,
                                               int strength_pct);
int dm1_v2_filter_palette_build_lut(int gamma100,
                                    int brightness,
                                    int contrast,
                                    unsigned char out_lut[DM1_V2_PALETTE_LEVELS][16][3]);
int dm1_v2_filter_dither_cleanup_indexed(unsigned char* fb, int w, int h);
int dm1_v2_filter_sharpen_rgba(unsigned char* rgba, int w, int h,
                               int strength_pct);
int dm1_v2_filter_crt_scanlines_rgba(unsigned char* rgba, int w, int h,
                                     int strength_pct);

#ifdef __cplusplus
}
#endif

#endif
