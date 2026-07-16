#ifndef FIRESTAFF_REDMCSB_F1033_HATCH_BOX_H
#define FIRESTAFF_REDMCSB_F1033_HATCH_BOX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB F1033 binds F1032's bitmap and width to the screen globals. */
typedef void (*redmcsb_f1033_hatch_box_primitive_fn)(
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width);

void redmcsb_f1033_hatch_box(
    redmcsb_f1033_hatch_box_primitive_fn hatch_box_primitive,
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width);

void F1033_HatchBox_Unreferenced(
    redmcsb_f1033_hatch_box_primitive_fn hatch_box_primitive,
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width);

const char *redmcsb_f1033_hatch_box_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1033_HATCH_BOX_H */
