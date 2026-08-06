/* Nexus V1 FONT256 runtime boundary.
 *
 * The source-format SCR parser and indexed glyph writer live only in explicit
 * probes.  M10/M11 still owns a small font field in the engine lifecycle, so
 * retail linking needs these bounded no-op lifecycle symbols until Saturn
 * page/tilemap/attribute and VDP2 placement capture exists.
 */

#include "nexus_v1_saturn_font.h"
#include <string.h>

int nexus_v1_font_load_from_s2d(Nexus_V1_Font *font,
                                const uint8_t *data, int data_size,
                                const Nexus_V1_FontS2dDecodeResult *decoded) {
    if (!font || !data || data_size <= 0 || !decoded) return -1;
    memset(font, 0, sizeof(*font));
    return -1;
}

void nexus_v1_font_free(Nexus_V1_Font *font) {
    if (!font) return;
    /* The retail route never owns a decoded host glyph buffer. */
    memset(font, 0, sizeof(*font));
}
