#include "redmcsb_f0903_draw_error_message_pc34_compat.h"

#include <string.h>

int redmcsb_f0903_draw_error_message_pc34_compat(
    const uint8_t *const source_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34],
    uint8_t *const destination_planes[REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34])
{
    size_t plane_index;

    if (!source_planes || !destination_planes) {
        return 0;
    }

    for (plane_index = 0;
         plane_index < REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34;
         ++plane_index) {
        if (!source_planes[plane_index] || !destination_planes[plane_index]) {
            return 0;
        }
    }

    for (plane_index = 0;
         plane_index < REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANES_PC34;
         ++plane_index) {
        memcpy(destination_planes[plane_index], source_planes[plane_index],
               REDMCSB_F0903_DRAW_ERROR_MESSAGE_PLANE_BYTES_PC34);
    }

    return 1;
}

const char *redmcsb_f0903_draw_error_message_source_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C:2208-2215 F0903_DrawErrorMessage; "
           "four CopyMem calls from G0747_aui_Bitmap_ErrorMessages[0..3] "
           "to ScreenBitMap->Planes[0..3], each M091_BITPLANE_SIZE(320, 200).";
}
