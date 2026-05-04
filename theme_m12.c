/*
 * theme_m12.c
 *
 * Pre-defined colour palettes for the M12 launcher theme selector.
 */

#include "theme_m12.h"

static const M12_ThemeColors s_themes[M12_THEME_COUNT] = {
    /* M12_THEME_CLASSIC — the original dark-grey menu look */
    {
        /* bg  */ 0x1A, 0x1A, 0x2E,
        /* fg  */ 0xCC, 0xCC, 0xCC,
        /* acc */ 0x33, 0x88, 0xFF,
        /* pnl */ 0x22, 0x22, 0x3A
    },
    /* M12_THEME_DARK — deeper blacks with amber accent */
    {
        /* bg  */ 0x0C, 0x0C, 0x0C,
        /* fg  */ 0xE0, 0xE0, 0xE0,
        /* acc */ 0xFF, 0xA5, 0x00,
        /* pnl */ 0x18, 0x18, 0x18
    },
    /* M12_THEME_AMIGA — warm Amiga Workbench vibe */
    {
        /* bg  */ 0x00, 0x55, 0xAA,
        /* fg  */ 0xFF, 0xFF, 0xFF,
        /* acc */ 0xFF, 0x88, 0x00,
        /* pnl */ 0x00, 0x44, 0x88
    },
    /* M12_THEME_CGA — cyan-magenta CGA palette nod */
    {
        /* bg  */ 0x00, 0x00, 0x00,
        /* fg  */ 0xFF, 0xFF, 0xFF,
        /* acc */ 0xFF, 0x55, 0xFF,
        /* pnl */ 0x00, 0xAA, 0xAA
    }
};

static const char* s_labels[M12_THEME_COUNT] = {
    "Classic",
    "Dark",
    "Amiga",
    "CGA"
};

const M12_ThemeColors* M12_Theme_GetColors(M12_Theme theme) {
    if (theme < 0 || theme >= M12_THEME_COUNT)
        theme = M12_THEME_CLASSIC;
    return &s_themes[theme];
}

const char* M12_Theme_GetLabel(M12_Theme theme) {
    if (theme < 0 || theme >= M12_THEME_COUNT)
        theme = M12_THEME_CLASSIC;
    return s_labels[theme];
}
