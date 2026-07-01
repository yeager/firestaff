/* Scan all Hall-of-Champions cells for C127 sensors on the front
 * square, in every direction.  Standalone — no Firestaff runtime. */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* dataDir = argc > 1 ? argv[1] : "/Users/bosse/.firestaff/data/dm1";
    M12_StartupMenuState menu;
    M11_GameViewState game;
    if (argc < 2) { fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]); return 2; }
    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "Cannot open game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Scan a 5x9 grid centered on the Hall corridor. */
    int xMin = 0, xMax = 6;
    int yMin = 0, yMax = 12;
    int hits[24] = {0};
    int seenOrdinals = 0;
    int matchAll[] = {0};
    int matchedMask = 0;

    for (int y = yMin; y <= yMax; ++y) {
        for (int x = xMin; x <= xMax; ++x) {
            for (int d = 0; d < 4; ++d) {
                game.world.party.mapIndex = 0;
                game.world.party.mapX = x;
                game.world.party.mapY = y;
                game.world.party.direction = d;
                int ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                if (ord >= 0 && ord < 24) {
                    if (hits[ord] == 0) seenOrdinals++;
                    hits[ord]++;
                    printf("  HIT  cell=(%d,%d) dir=%d ordinal=%d\n", x, y, d, ord);
                    if (ord == 23) {
                        matchAll[0]++;
                    }
                    matchedMask |= (1 << ord);
                }
            }
        }
    }
    printf("\nSummary: %d distinct ordinals found across %d tested cells x 4 dirs = %d poses\n",
           seenOrdinals, (xMax - xMin + 1) * (yMax - yMin + 1),
           (xMax - xMin + 1) * (yMax - yMin + 1) * 4);
    printf("Ordinal counts:\n");
    for (int i = 0; i < 24; ++i) {
        if (hits[i] > 0) printf("  ordinal=%d -> %d poses\n", i, hits[i]);
    }
    printf("Ordinal 23 hits: %d\n", matchAll[0]);

    M11_GameView_Shutdown(&game);
    return 0;
}
