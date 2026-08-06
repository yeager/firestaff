#include "dm1_v1_fmtowns_text_geometry.h"
#include <string.h>

/* Source-locked FM Towns DM1 text/screen geometry constants.
 * Every value is a byte-verified read of EDM.EXP's initialised
 * data section at the SYM1-declared vaddr; do not modify without
 * matching evidence. */

int dm1_v1_fmtowns_text_pixel_width_pc34(int char_count) {
    if (char_count <= 0) return 0;
    return char_count * DM1_V1_FMTOWNS_CHAR_X_WID;
}

int dm1_v1_fmtowns_text_pixel_height_pc34(int line_count) {
    if (line_count <= 0) return 0;
    return line_count * DM1_V1_FMTOWNS_CHAR_Y_HYT;
}

uint32_t dm1_v1_fmtowns_text_geometry_vaddr_pc34(const char *name) {
    static const struct { const char *n; uint32_t v; } k[] = {
        { "CHAR_X_SIZE",    0x26c8a },
        { "CHAR_Y_SIZE",    0x26c8c },
        { "CHAR_X_SPC",     0x26c8e },
        { "CHAR_Y_SPC",     0x26c90 },
        { "CHAR_DESCENDER", 0x26c92 },
        { "CHAR_X_WID",     0x26c94 },
        { "CHAR_Y_HYT",     0x26c96 },
        { "SCR_X_SIZE",     0x26c68 },
        { "ICON_SIZE",      0x26c76 },
        { "ICON_X_SIZE",    0x26c78 },
        { "ICON_Y_SIZE",    0x26c7a },
    };
    size_t i;
    if (!name) return 0u;
    for (i = 0; i < sizeof(k) / sizeof(k[0]); ++i) {
        if (strcmp(name, k[i].n) == 0) return k[i].v;
    }
    return 0u;
}
