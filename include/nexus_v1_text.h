
#ifndef NEXUS_V1_TEXT_H
#define NEXUS_V1_TEXT_H
#include <stdint.h>

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_s2d_text_layout.h"

/* Shift-JIS text extraction from Nexus Saturn data.
 * Japanese text in DM.BIN, SLEV*.BIN, and DGN files. */

int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max);
int nexus_v1_extract_strings(const uint8_t *data, int size,
    char **out_strings, int max_strings);

/* Minimal runtime screen-text binding for Nexus FONT256.S2D.
 *
 * This path intentionally stays small: callers provide the already-loaded
 * flat 1bpp glyph font plus the parsed SCR section table. The runtime object
 * builds the section -> glyph range map once, then draws ASCII text through
 * nexus_v1_s2d_text_layout into the live 320x200 Nexus indexed framebuffer.
 * It is a runtime-facing handoff, not a full Saturn text-system claim: no
 * Shift-JIS double-byte layout, proportional metrics, or real-screen parity.
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
