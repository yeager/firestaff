#include "redmcsb_f0803_draw_magic_map_icon_pc34_compat.h"

enum {
    REDMCSB_F0803_COLOR_FLESH_PC34 = 10
};

void redmcsb_f0803_draw_magic_map_icon_pc34_compat(
    const RedmcsbF0803DrawMagicMapIconHooksPc34 *hooks,
    int16_t magic_map_icons_graphic_index,
    void *viewport_bitmap,
    int16_t magic_map_icon_index,
    int16_t x,
    int16_t y,
    int16_t magic_map_icon_width,
    int16_t magic_map_icon_height)
{
    int16_t xyz[4];
    const void *magic_map_icons;

    magic_map_icons = hooks->get_native_bitmap(
        hooks->context, magic_map_icons_graphic_index);
    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = magic_map_icon_width;
    xyz[3] = magic_map_icon_height;
    hooks->video_blit(
        hooks->context,
        magic_map_icons,
        viewport_bitmap,
        xyz,
        (int16_t)(magic_map_icon_width * (magic_map_icon_index & 0x0f)),
        (int16_t)(magic_map_icon_height * (magic_map_icon_index >> 4)),
        REDMCSB_F0803_COLOR_FLESH_PC34);
}

const char *redmcsb_f0803_draw_magic_map_icon_source_evidence_pc34(void)
{
    return "ReDMCSB PANEL.C:534-543; F0489 lookup, F0787 XYZ zone, F0654/F0132 icon blit";
}
