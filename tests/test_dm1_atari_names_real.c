#include "m11_game_view.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include "asset_find_by_hash.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "csb_v1_audio_runtime_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
    M11_GameViewState *state;
    M11_GameLaunchSpec spec = {0};
    char selected[ASSET_PATH_MAX];
    char container[ASSET_PATH_MAX];
    DM1_V1_AtariStGraphicsDat dat;
    unsigned char raw[65535];
    unsigned char *pixels = NULL;
    int count, result = 1;
    size_t cursor = 0;
    FILE *media;
    if (!path || !(media = fopen(path, "rb"))) return 77;
    fclose(media);
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    spec.title = "Dungeon Master";
    spec.gameId = "dm1";
    spec.sourceId = "dm1";
    spec.dataDir = path;
    if (argc == 2) {
        if (!asset_find_by_md5(path, argv[1], selected, sizeof(selected), 8)) goto done;
        spec.verifiedAssetPath = selected;
        spec.verifiedAssetMd5 = argv[1];
        snprintf(container, sizeof(container), "%s", selected);
        {
            char *part = container, *last = NULL;
            while ((part = strstr(part, "::")) != NULL) { last = part; part += 2; }
            if (!last) goto done;
            *last = '\0';
        }
        spec.dataDir = container;
    }
    if (!M11_GameView_Start(state, &spec) || !state->assetLoader.atariStDm1) {
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
    if (state->audioState.originalSnd3Available || state->audioState.originalSnd3LoadedCount) goto done;
    for (unsigned int i = 0; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        int expanded = dm1_v1_atari_st_graphics_read(&dat, (uint16_t)i, raw, sizeof(raw));
        if (expanded != dat.records[i].expanded_size) {
            fprintf(stderr, "FAIL: Atari record %u expanded=%d expected=%u\n",
                    i, expanded, dat.records[i].expanded_size);
            goto done;
        }
    }
    puts("PASS: all 563 authentic Atari graphics records decode to original table lengths");
    if (!state->originalFontAvailable ||
        dm1_v1_atari_st_graphics_read(&dat, 557, raw, sizeof(raw)) != M11_FONT_BITMAP_BYTES ||
        memcmp(raw, state->originalFont.bitmap, M11_FONT_BITMAP_BYTES)) {
        fputs("FAIL: Atari interface font does not match original M653\n", stderr);
        goto done;
    }
    pixels = malloc(1024u * 1024u);
    if (!pixels) goto done;
    for (unsigned int i = 0; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        uint16_t width = 0, height = 0;
        if (!dm1_v1_legacy_graphics_is_bitmap_index((uint16_t)i)) {
            if (dm1_v1_atari_st_graphics_decode(&dat, (uint16_t)i, pixels,
                    1024u * 1024u, &width, &height)) {
                fprintf(stderr, "FAIL: non-raster Atari record %u accepted as image\n", i);
                goto done;
            }
            continue;
        }
        if (!dm1_v1_atari_st_graphics_decode(&dat, (uint16_t)i, pixels,
                1024u * 1024u, &width, &height)) {
            fprintf(stderr, "FAIL: original Atari bitmap %u\n", i);
            goto done;
        }
        for (size_t p = 0; p < (size_t)width * height; ++p) {
            if (pixels[p] > 15) {
                fprintf(stderr, "FAIL: Atari bitmap %u non-4bpp pixel\n", i);
                goto done;
            }
        }
    }
    puts("PASS: all 532 original Atari bitmap records decode to 4bpp pixels");
    for (int i = 0; i < CSB_V1_ATARI_ST_SOUND_COUNT; ++i) {
        CsbV1AtariStSoundPayload sound = {0};
        CsbV1StSoundDecodeResult decodedSound = {0};
        int decodeStatus;
        if (!csb_v1_audio_runtime_load_atari_st_sound_payload(
                state->assetLoader.graphicsDatPath, (int16_t)i, &sound)) {
            fprintf(stderr, "FAIL: original Atari SND1 source index %d\n", i);
            goto done;
        }
        count = dm1_v1_atari_st_graphics_read(&dat, sound.spec.graphicIndex,
                                            raw, sizeof(raw));
        if (count < 0 || (size_t)count != sound.byteCount ||
            memcmp(raw, sound.bytes, sound.byteCount)) {
            csb_v1_audio_runtime_atari_st_sound_payload_free(&sound);
            fputs("FAIL: Atari audio transport changed original record bytes\n", stderr);
            goto done;
        }
        decodeStatus = csb_v1_audio_runtime_decode_st_sound(sound.bytes,
                sound.byteCount, 0, pixels, 1024u * 1024u, &decodedSound);
        /* Characterization, not playback parity: these three original records
         * exhaust their bounded stream before the declared sample count.
         * SOUND.C F0060/F0061 reads from runtime RAM; do not invent padding.
         * Keep the strict rejection until that RAM boundary is captured. */
        if (decodeStatus != ((i == 1 || i == 12 || i == 16) ? -2 : 0)) {
            fprintf(stderr, "FAIL: unexpected Atari SND1 decode index %d status %d\n",
                    i, decodeStatus);
            csb_v1_audio_runtime_atari_st_sound_payload_free(&sound);
            goto done;
        }
        csb_v1_audio_runtime_atari_st_sound_payload_free(&sound);
    }
    puts("PASS: 22 Atari sound entries preserve source bytes; 19 decode, 3 known short streams reject safely (not playback parity)");
    {
        static const int expected[35] = {
            0, 1, 2, 2, 4, 5, 20, 6, 8, 9, 10, 11, 12, 16, 17, 18,
            13, -1, -1, 14, 15, 19, 21, 3, -1, 7, -1, -1,
            -1, -1, -1, -1, -1, -1, -1
        };
        if (!state->audioState.initialized && !M11_Audio_Init(&state->audioState)) goto done;
        for (int i = 0; i < 35; ++i) {
            int index = expected[i];
            int playable = index >= 0 && index != 1 && index != 12 && index != 16;
            int markers = state->audioState.playedMarkerCount;
            if (M11_Audio_Dm1AtariSoundIndex(i) != index ||
                M11_Audio_EmitDm1AtariSound(&state->audioState,
                    state->assetLoader.graphicsDatPath, i, 3) != playable ||
                state->audioState.playedMarkerCount != markers) {
                fprintf(stderr, "FAIL: original Atari event transport %d\n", i);
                goto done;
            }
            if (playable && (state->audioState.lastSoundIndex != i ||
                state->audioState.csbAtariStSoundPeriod != (i == 3 ? 145 : i == 15 ? 138 : 112))) goto done;
        }
        if (M11_Audio_Dm1AtariSoundIndex(-1) != -1 ||
            M11_Audio_Dm1AtariSoundIndex(35) != -1) goto done;
    }
    puts("PASS: DM1 event indices select original Atari samples and Timer-A periods without generated markers");
    result = 0;
done:
    free(pixels);
    M11_GameView_Shutdown(state);
    free(state);
    return result;
}
