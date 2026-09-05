#include "m11_game_view.h"
#include "firestaff_po_loader.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int glyph_matches(const unsigned char* fb, int origin_y,
                         const unsigned char rows[7], unsigned char color)
{
    int x;
    int y;
    /* Bottom C015 row is clipped by the 200-line source framebuffer, exactly
     * like TEXT.C's six-visible-pixel M653 cell. */
    for (y = 0; y < 6; ++y) {
        for (x = 0; x < 5; ++x) {
            unsigned char want =
                (rows[y] & (1u << (4 - x))) ? color : 0u;
            if (fb[(origin_y + y) * 320 + x] != want) return 0;
        }
    }
    return 1;
}

int main(void)
{
    static const unsigned char capital_a_ring[7] = {
        0x04u, 0x0au, 0x0eu, 0x11u, 0x11u, 0x1fu, 0x11u
    };
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    unsigned char* graphics = NULL;
    size_t graphics_size = 0u;
    unsigned char framebuffer[320 * 200];
    M11_GameViewState state;
    const char* translated;

    if (!archive || !archive[0]) {
        puts("SKIP: no DM1 DOS PC 3.4 archive");
        return 0;
    }
    if (firestaff_zip_extract_by_suffix(archive, "DATA/GRAPHICS.DAT",
                                        &graphics, &graphics_size) != 0 ||
        !graphics || graphics_size == 0u) {
        fprintf(stderr, "FAIL: read PC34 GRAPHICS.DAT from ZIP in memory\n");
        free(graphics);
        return 1;
    }

    M11_GameView_Init(&state);
    state.active = 1;
    if (!M11_AssetLoader_InitFromBuffer(&state.assetLoader, graphics,
                                         (long)graphics_size) ||
        !M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                      state.assetLoader.fileState,
                                      state.assetLoader.runtimeState)) {
        fprintf(stderr, "FAIL: bind authentic PC34 M653\n");
        M11_GameView_Shutdown(&state);
        free(graphics);
        return 1;
    }
    state.assetsAvailable = 1;
    state.originalFontAvailable = 1;

    if (fs_po_load(FIRESTAFF_SOURCE_DIR "/po/dm1.sv.po") <= 0) {
        fprintf(stderr, "FAIL: load Swedish DM1 catalog\n");
        M11_GameView_Shutdown(&state);
        free(graphics);
        return 1;
    }
    translated = fs_po_gettext_in_domain("dm1", "RETURN");
    if (!translated || strcmp(translated, "ÅTERGÅ") != 0) {
        fprintf(stderr, "FAIL: resolve Swedish runtime translation\n");
        M11_GameView_Shutdown(&state);
        free(graphics);
        return 1;
    }

    dm1_v1_text_init(&state.dm1V1TextMessage);
    dm1_v1_text_print_message(&state.dm1V1TextMessage,
                              DM1_V1_COLOR_WHITE, translated);
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    /* A no-linefeed F0047 message starts in the bottom C015 row: y=194.
     * The accent bitmap is a presentation extension only; all ASCII glyphs
     * in ÅTERGÅ continue to come from authenticated M653. A byte-oriented
     * renderer would consume C3/85 separately and could not draw Å at x=0. */
    if (!glyph_matches(framebuffer, 194, capital_a_ring,
                       DM1_V1_COLOR_WHITE)) {
        fprintf(stderr, "FAIL: Swedish U+00C5 was not one C015 glyph cell\n");
        M11_GameView_Shutdown(&state);
        free(graphics);
        return 1;
    }

    M11_GameView_Shutdown(&state);
    free(graphics);
    puts("PASS: real PC34 M653 + Unicode-capable Original C015 presentation");
    return 0;
}
