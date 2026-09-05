#include "m11_game_view.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *path = getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
    M11_GameViewState *state;
    DM1_V1_AtariStGraphicsDat dat;
    unsigned char raw[65535];
    int count, result = 1;
    size_t cursor = 0;
    FILE *media;
    if (!path || !(media = fopen(path, "rb"))) return 77;
    fclose(media);
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    if (!M11_GameView_StartDm1(state, path) || !state->assetLoader.atariStDm1) {
        fputs("FAIL: authentic Atari M564 name binding\n", stderr);
        goto done;
    }
    if (!dm1_v1_atari_st_graphics_open(state->assetLoader.atariStData,
            (size_t)state->assetLoader.atariStDataSize, &dat)) goto done;
    count = dm1_v1_atari_st_graphics_read(&dat, 556, raw, sizeof(raw));
    fprintf(stderr, "Atari M564 bytes=%d compressed=%u expanded=%u valid=%d\n", count,
            dat.records[556].compressed_size, dat.records[556].expanded_size,
            state->dm1ObjectNameTableValid);
    if (count <= 0) goto done;
    /* OBJECT.C F0031 MEDIA060: last source character has bit 7 set.
     * Compare every admitted M564 row, without a substitute name catalog. */
    for (int i = 0; i < 199; ++i) {
        size_t ch = 0;
        unsigned char byte;
        do {
            if (cursor >= (size_t)count || ch >= sizeof(state->dm1ObjectNames[i]) - 1)
                goto done;
            byte = raw[cursor++];
            if ((unsigned char)state->dm1ObjectNames[i][ch++] != (byte & 0x7f))
                goto done;
        } while (!(byte & 0x80));
        if (state->dm1ObjectNames[i][ch]) goto done;
    }
    puts("PASS: all 199 authentic Atari DM1 names retain M564 source indices");
    for (unsigned int i = 0; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        int expanded = dm1_v1_atari_st_graphics_read(&dat, (uint16_t)i, raw, sizeof(raw));
        if (expanded != dat.records[i].expanded_size) {
            fprintf(stderr, "FAIL: Atari record %u expanded=%d expected=%u\n",
                    i, expanded, dat.records[i].expanded_size);
            goto done;
        }
    }
    puts("PASS: all 563 authentic Atari graphics records decode to original table lengths");
    result = 0;
done:
    M11_GameView_Shutdown(state);
    free(state);
    return result;
}
