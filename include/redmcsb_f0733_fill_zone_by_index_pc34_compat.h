/*
 * ReDMCSB BLITFILL.C F0733_FillZoneByIndex, PC 3.4 (I34E/I34M) route.
 *
 * BLITFILL.C:225-238 resolves the supplied zone through F0638_GetZone and
 * passes that result directly to F0135_VIDEO_FillBox for the 320x200 screen.
 */
#ifndef FIRESTAFF_REDMCSB_F0733_FILL_ZONE_BY_INDEX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0733_FILL_ZONE_BY_INDEX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t *(*redmcsb_f0733_get_zone_pc34_compat)(
    void *context,
    int16_t zone_index,
    int16_t zone_xyz[4]);

typedef void (*redmcsb_f0733_fill_box_pc34_compat)(
    void *context,
    int16_t *zone_xyz,
    int16_t color,
    int16_t screen_pixel_width,
    int16_t screen_pixel_height);

typedef struct {
    redmcsb_f0733_get_zone_pc34_compat get_zone;
    redmcsb_f0733_fill_box_pc34_compat fill_box;
    void *context;
} redmcsb_f0733_graphics_pc34_compat;

/*
 * Executes the PC 3.4 F0733 sequence exactly:
 * F0135_VIDEO_FillBox(G0348_Bitmap_Screen,
 *                     F0638_GetZone(zone_index, local_xyz), color, 320, 200).
 *
 * As in the source, the graphics callbacks must be valid. A null zone result
 * is forwarded to fill_box; F0733 itself contains no zone validation.
 */
void redmcsb_f0733_fill_zone_by_index_pc34_compat(
    const redmcsb_f0733_graphics_pc34_compat *graphics,
    int16_t zone_index,
    int16_t color);

const char *redmcsb_f0733_fill_zone_by_index_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
