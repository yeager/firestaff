#include "vga_palette_pc34_compat.h"

#include <stdio.h>

static int same_rgb(const unsigned char *a, const unsigned char *b)
{
    return a && b && a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

int main(void)
{
    const unsigned char *presents_white = F9011_VGA_GetSpecialColorRgb_Compat(
        15u, VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS);
    const unsigned char *title_red = F9011_VGA_GetSpecialColorRgb_Compat(
        12u, VGA_PALETTE_PC34_SPECIAL_TITLE);
    const unsigned char *csb_presents = F9011_VGA_GetSpecialColorRgb_Compat(
        15u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS);
    const unsigned char *csb_title = F9011_VGA_GetSpecialColorRgb_Compat(
        12u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS);

    if (!presents_white || presents_white[0] != 255u ||
        presents_white[1] != 255u || presents_white[2] != 255u ||
        !title_red || same_rgb(presents_white, title_red) ||
        !csb_presents || !csb_title ||
        (same_rgb(presents_white, csb_presents) &&
         same_rgb(title_red, csb_title)) ||
        F9011_VGA_GetSpecialColorRgb_Compat(
            16u, VGA_PALETTE_PC34_SPECIAL_TITLE) != NULL) {
        return 1;
    }
    puts("ok: DM1 title PC34 palette RGB upload rejects CSB substitution");
    return 0;
}
