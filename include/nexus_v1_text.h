
#ifndef NEXUS_V1_TEXT_H
#define NEXUS_V1_TEXT_H
#include <stdint.h>

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_s2d_text_layout.h"

/* Source-bound text extraction from Nexus Saturn data.
 * ASCII and half-width katakana are converted. Unsupported JIS X 0208
 * double-byte text returns -1; no replacement glyph is synthesized. */

int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max);
int nexus_v1_extract_strings(const uint8_t *data, int size,
    char **out_strings, int max_strings);

/* Minimal runtime screen-text binding for Nexus FONT256.S2D.
 *
 * This path intentionally stays small: callers provide the already-loaded
 * flat 1bpp glyph font plus the parsed SCR section table. The map/layout
 * object remains available to the data-free fixture probe; the production
 * draw seam is fail-closed until Saturn page/tilemap/attribute ownership and
 * screen placement are authenticated.
 */
typedef struct {
    int      glyphs_drawn;
    int      chars_skipped;
    int      chars_clipped;
    int      newline_count;
    int      tab_count;
    int      final_cursor_x;
    int      final_cursor_y;
    long     framebuffer_writes;
    int      map_range_count;
    int      map_char_count;
    uint64_t framebuffer_hash;
} Nexus_V1_ScreenTextReceipt;

typedef struct {
    Nexus_V1_S2D_SectionGlyphMap glyph_map;
    Nexus_V1_S2D_TextLayout      layout;
    Nexus_V1_ScreenTextReceipt   last_receipt;
    int                          initialized;
} Nexus_V1_ScreenTextRuntime;

int nexus_v1_screen_text_init(
    Nexus_V1_ScreenTextRuntime *runtime,
    const Nexus_V1_Font *font,
    const Nexus_V1_FontSections *sections,
    const Nexus_V1_S2D_TextLayoutConfig *config);

void nexus_v1_screen_text_reset(Nexus_V1_ScreenTextRuntime *runtime);
void nexus_v1_screen_text_free(Nexus_V1_ScreenTextRuntime *runtime);

int nexus_v1_screen_text_draw(
    Nexus_V1_ScreenTextRuntime *runtime,
    Nexus_Framebuffer *framebuffer,
    int x,
    int y,
    const char *text,
    Nexus_V1_ScreenTextReceipt *out_receipt);

const Nexus_V1_ScreenTextReceipt *nexus_v1_screen_text_last_receipt(
    const Nexus_V1_ScreenTextRuntime *runtime);

#endif
