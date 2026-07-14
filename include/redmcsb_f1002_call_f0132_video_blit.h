#ifndef FIRESTAFF_REDMCSB_F1002_CALL_F0132_VIDEO_BLIT_H
#define FIRESTAFF_REDMCSB_F1002_CALL_F0132_VIDEO_BLIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB BASE.C:1202-1212, MEDIA458_P20JA_P20JB implementation of
 * F1002_Call_F0132_VIDEO_Blit.
 *
 * A ReDMCSB bitmap pointer addresses pixel data. The two preceding int16_t
 * values are its pixel width and pixel height (DEFS.H:3444-3445). F1002
 * reads only the width words and forwards all other arguments unchanged.
 */
typedef void (*redmcsb_f1002_video_blit)(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip);

void redmcsb_f1002_call_f0132_video_blit(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t transparent_color,
    int16_t flip,
    redmcsb_f1002_video_blit video_blit);

const char *redmcsb_f1002_call_f0132_video_blit_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1002_CALL_F0132_VIDEO_BLIT_H */
