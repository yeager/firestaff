#include "m11_game_view.h"
#include "asset_status_m12.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dm1_legacy_scroll_real_check.h"

int main(void) {
    const char *path = getenv("FIRESTAFF_DM1_AMIGA_ARCHIVE");
    M12_AssetStatus *scan;
    M11_GameViewState *state;
    M11_GameLaunchSpec spec = {0};
    const M12_AssetVersionStatus *version;
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    unsigned char raw[65535];
    size_t length = 0, cursor = 0;
    int result = 1, index;
    FILE *media;
    if (!path || !(media = fopen(path, "rb"))) return 77;
    fclose(media);
    scan = calloc(1, sizeof(*scan));
    state = calloc(1, sizeof(*state));
    if (!scan || !state) { free(scan); free(state); return 1; }
    M11_GameView_Init(state);
    M12_AssetStatus_ScanGame(scan, path, "dm1");
    index = M12_AssetStatus_FindVersionIndex("dm1", "amiga20-en");
    version = index < 0 ? NULL : M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
    if (!version || !version->matched ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(scan, "amiga20-en", runtime, sizeof(runtime)))
        goto done;
    spec.title = "Dungeon Master"; spec.gameId = "dm1"; spec.sourceId = "dm1";
    spec.dataDir = runtime; spec.verifiedAssetPath = version->matchedPath;
    spec.verifiedAssetMd5 = version->matchedMd5;
    if (!M11_GameView_Start(state, &spec) || !state->dm1ObjectNameTableValid ||
        !state->assetLoader.legacyDm1 || !state->assetLoader.legacyBigEndian) goto done;
    if (!dm1_v1_legacy_graphics_read_raw(state->assetLoader.legacyData,
            (size_t)state->assetLoader.legacyDataSize, 1, 556, raw, sizeof(raw), &length))
        goto done;
    /* OBJECT.C F0031/MEDIA060: M564 names end with bit 7, not NUL. */
    for (int i = 0; i < 199; ++i) {
        size_t ch = 0;
        unsigned char byte;
        do {
            if (cursor >= length || ch >= sizeof(state->dm1ObjectNames[i]) - 1) goto done;
            byte = raw[cursor++];
            if ((unsigned char)state->dm1ObjectNames[i][ch++] != (byte & 0x7f)) goto done;
        } while (!(byte & 0x80));
        if (state->dm1ObjectNames[i][ch]) goto done;
    }
    puts("PASS: 199 original Amiga M564 names through the selected-edition launcher handoff");
    if (!state->originalFontAvailable ||
        !dm1_v1_legacy_graphics_read_raw(state->assetLoader.legacyData,
            (size_t)state->assetLoader.legacyDataSize, 1, 557, raw, sizeof(raw), &length) ||
        length != M11_FONT_BITMAP_BYTES ||
        memcmp(raw, state->originalFont.bitmap, M11_FONT_BITMAP_BYTES)) goto done;
    puts("PASS: Amiga interface font matches original M653 bytes");
    if (state->audioState.originalSnd3Available || state->audioState.originalSnd3LoadedCount) goto done;
    {
        static const unsigned short records[35] = {
            533,534,535,535,536,537,537,539,540,541,542,543,544,
            546,547,549,545,569,566,552,553,554,555,550,570,551,
            571,572,563,564,565,567,568,573,574
        };
        static const unsigned short periods[35] = {
            112,112,112,145,112,112,112,112,112,112,112,112,112,
            112,112,138,112,138,138,112,112,112,112,112,138,112,
            138,112,138,138,138,138,138,138,150
        };
        if (!state->audioState.initialized && !M11_Audio_Init(&state->audioState)) goto done;
        for (int event = 0; event < 35; ++event) {
            if (!M11_Audio_EmitDm1AmigaSound(&state->audioState,
                    state->assetLoader.legacyData,
                    (size_t)state->assetLoader.legacyDataSize, event) ||
                !dm1_v1_legacy_graphics_read_raw(state->assetLoader.legacyData,
                    (size_t)state->assetLoader.legacyDataSize, 1, records[event],
                    raw, sizeof(raw), &length) || length < 3 ||
                state->audioState.csbAmigaRuntimeSoundByteCount != (int)length - 2) {
                fprintf(stderr, "FAIL: original Amiga sound event %d\n", event);
                goto done;
            }
            {
                unsigned int rate = 3579545u / (72800u / periods[event]);
                int output = (int)(((length - 2u) * M11_AUDIO_SAMPLE_RATE + rate - 1u) / rate);
                if (state->audioState.csbAmigaRuntimePcm.sampleCount != output) goto done;
                for (int p = 0; p < output; ++p) {
                    size_t source = (size_t)p * rate / M11_AUDIO_SAMPLE_RATE;
                    int value;
                    if (source >= length - 2u) source = length - 3u;
                    value = raw[source + 2u];
                    if (value >= 128) value -= 256;
                    if (state->audioState.csbAmigaRuntimePcm.samples[p] != value / 128.0f) goto done;
                }
            }
        }
        if (M11_Audio_EmitDm1AmigaSound(&state->audioState,
                state->assetLoader.legacyData,
                (size_t)state->assetLoader.legacyDataSize, 35)) goto done;
    }
    puts("PASS: all 35 engine sound events select original Amiga PCM records (34 unique effects)");
    if (!check_legacy_scroll_raster(state)) goto done;
    result = 0;
done:
    if (result) fputs("FAIL: original Amiga name/media selection gate\n", stderr);
    M11_GameView_Shutdown(state); free(state); free(scan);
    return result;
}
