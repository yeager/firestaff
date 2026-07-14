#include "redmcsb_f0766_blit_to_screen_pc34_compat.h"

void redmcsb_f0766_blit_to_screen_pc34_compat(
    const redmcsb_f0766_renderer_pc34_compat *renderer,
    const void *bitmap,
    const int16_t xyz[4],
    int16_t transparent_color)
{
    renderer->video_blit(
        renderer->context,
        bitmap,
        renderer->screen_bitmap,
        xyz,
        INT16_C(0),
        INT16_C(0),
        renderer->bitmap_pixel_width(renderer->context, bitmap),
        renderer->screen_pixel_width,
        transparent_color,
        REDMCSB_F0766_NO_FLIP_PC34_COMPAT);
}

const char *redmcsb_f0766_blit_to_screen_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:1374-1391, MEDIA463 P20JA/P20JB/I34E/I34M/P31J "
           "branch: F0766 calls F0132_VIDEO_Blit from bitmap to "
           "G0348_Bitmap_Screen with supplied XYZ, source origin (0,0), "
           "M100 bitmap width, C320 screen width, supplied transparency, "
           "and MASK0x0000_NO_FLIP.";
}
