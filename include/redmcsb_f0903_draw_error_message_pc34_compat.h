/*
 * ReDMCSB SWSH.C F0903_DrawErrorMessage source-locked adapter.
 */
#ifndef FIRESTAFF_REDMCSB_F0903_DRAW_ERROR_MESSAGE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0903_DRAW_ERROR_MESSAGE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F0903_DRAW_ERROR_MESSAGE_WIDTH_PC34 320u
#define REDMCSB_F0903_DRAW_ERROR_MESSAGE_HEIGHT_PC34 200u
#define REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34 4u
#define REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANE_BYTES_PC34 \
    ((REDMCSB_F0903_DRAW_ERROR_MESSAGE_WIDTH_PC34 / 8u) * \
     REDMCSB_F0903_DRAW_ERROR_MESSAGE_HEIGHT_PC34)

/*
 * Copies the four original 320x200 bitplanes from
 * G0747_aui_Bitmap_ErrorMessages to the screen bitplanes.  No decoding,
 * text generation, palette work, or fallback drawing is performed here.
 * Returns 1 after all four complete copies, otherwise 0 without writing.
 */
int redmcsb_f0903_draw_error_message_pc34_compat(
    const uint8_t *const source_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34],
    uint8_t *const destination_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34]);

const char *redmcsb_f0903_draw_error_message_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0903_DRAW_ERROR_MESSAGE_PC34_COMPAT_H */
