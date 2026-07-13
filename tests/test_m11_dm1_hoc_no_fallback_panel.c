/* ReDMCSB REVIVE.C F0281/F0282: C027/C040 are original panel graphics.
 * Missing PC34 art must not produce an M11 substitute panel. */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(void) {
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    int x;
    int y;
    int substitutePixels = 0;

    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state.inventoryPanelActive = 1;
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorRenameActive = 1;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    /* No GRAPHICS.DAT session: C027/C040 must fail closed. */
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));

    M11_GameView_Draw(&state, framebuffer, 320, 200);
    /* C101 occupies the 224x136 M11 viewport at (12,33). */
    for (y = 33; y < 33 + 136; ++y) {
        for (x = 12; x < 12 + 224; ++x) {
            unsigned char pixel = framebuffer[y * 320 + x];
            /* The retired substitute used a green fill and orange border.
             * Other M11 chrome may legitimately overlap C101. */
            if (pixel == 6 || pixel == 9) {
                substitutePixels++;
            }
        }
    }
    if (substitutePixels != 0) {
        fprintf(stderr,
                "FAIL test_m11_dm1_hoc_no_fallback_panel substitute_pixels=%d\n",
                substitutePixels);
        return 1;
    }
    printf("PASS test_m11_dm1_hoc_no_fallback_panel substitute_pixels=0\n");
    return 0;
}
