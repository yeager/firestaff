/*
 * ReDMCSB PANEL.C F0803_DrawMagicMapIcon, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0803_DRAW_MAGIC_MAP_ICON_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0803_DRAW_MAGIC_MAP_ICON_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const void *(*RedmcsbF0803GetNativeBitmapPc34)(
    void *context,
    int16_t graphic_index);

typedef void (*RedmcsbF0803VideoBlitPc34)(
    void *context,
    const void *source,
    void *destination,
    const int16_t source_rectangle[4],
    int16_t source_x,
    int16_t source_y,
    int16_t transparent_color);

typedef struct RedmcsbF0803DrawMagicMapIconHooksPc34 {
    void *context;
    RedmcsbF0803GetNativeBitmapPc34 get_native_bitmap;
    RedmcsbF0803VideoBlitPc34 video_blit;
} RedmcsbF0803DrawMagicMapIconHooksPc34;

/*
 * Calls the real F0489 bitmap lookup and F0654/F0132 blit services supplied
 * by the host. It does not decode, allocate, or synthesize icon pixels.
 */
void redmcsb_f0803_draw_magic_map_icon_pc34_compat(
    const RedmcsbF0803DrawMagicMapIconHooksPc34 *hooks,
    int16_t magic_map_icons_graphic_index,
    void *viewport_bitmap,
    int16_t magic_map_icon_index,
    int16_t x,
    int16_t y,
    int16_t magic_map_icon_width,
    int16_t magic_map_icon_height);

const char *redmcsb_f0803_draw_magic_map_icon_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
