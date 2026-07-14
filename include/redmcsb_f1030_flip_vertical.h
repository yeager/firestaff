#ifndef FIRESTAFF_REDMCSB_F1030_FLIP_VERTICAL_H
#define FIRESTAFF_REDMCSB_F1030_FLIP_VERTICAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The bitmap payload is preceded by two native-endian int16_t values: pixel
 * width followed by pixel height. ReDMCSB F1030 forwards both values and the
 * payload unchanged to F0131_VIDEO_FlipVertical.
 */
typedef void (*redmcsb_f1030_flip_vertical_primitive_fn)(
    uint8_t *bitmap,
    int16_t pixel_width,
    int16_t pixel_height);

void redmcsb_f1030_flip_vertical(
    redmcsb_f1030_flip_vertical_primitive_fn flip_vertical_primitive,
    uint8_t *bitmap);

const char *redmcsb_f1030_flip_vertical_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1030_FLIP_VERTICAL_H */
