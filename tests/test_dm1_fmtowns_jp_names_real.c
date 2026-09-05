#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "csb_v1_boot.h"
#include "firestaff_cp932.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "dm1_v1_fmtowns_dyna_buttons_ja.h"
#include "csb_v1_audio_runtime_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    M11_GameViewState *state;
    M11_GameLaunchSpec spec = {0};
    unsigned char *bytes = NULL, raw[65535];
    size_t size = 0, length = 0, offset = 0;
    int result = 1;
    FILE *media;
    if (!archive || !archive[0]) return 77;
    media = fopen(archive, "rb");
    if (!media) return 77;
    fclose(media);
    state = calloc(1, sizeof(*state));
    if (!state) return 1;
    M11_GameView_Init(state);
    spec.title = "Dungeon Master";
    spec.gameId = "dm1";
    spec.sourceId = "dm1";
    spec.dataDir = archive;
    spec.dm1Fmtowns = 1;
    spec.dm1FmtownsJapanese = 1;
    if (!M11_GameView_Start(state, &spec) ||
        !state->dm1FmtownsStartupReceiptValid ||
        state->dm1FmtownsStartupReceipt.language != DM1_FMTOWNS_LANG_JP) {
        fprintf(stderr, "FAIL: JDM startup names=%d receipt=%d language=%d\n",
                state->dm1ObjectNameTableValid, state->dm1FmtownsStartupReceiptValid,
                state->dm1FmtownsStartupReceipt.language);
        goto done;
    }
    size = (size_t)state->assetLoader.legacyDataSize;
    bytes = malloc(size);
    if (bytes) memcpy(bytes, state->assetLoader.legacyData, size);
    if (!bytes ||
        !dm1_v1_legacy_graphics_read_raw(bytes, size, 0, 556,
            raw, sizeof(raw), &length)) {
        fprintf(stderr, "FAIL: source M564 read size=%zu path=%s\n", size,
                state->assetLoader.graphicsDatPath);
        goto done;
    }
    /* OBJECT.C F0031 MEDIA574/F20J: 199 consecutive NUL-terminated names.
     * Walk original M564 independently of M11's framing loop. No fixture
     * names or English fallback are accepted as the Japanese source. */
    for (int i = 0; i < 199; ++i) {
        const unsigned char *end;
        char expected[256];
        if (offset >= length) goto done;
        end = memchr(raw + offset, 0, length - offset);
        if (!end || firestaff_cp932_to_utf8((const char *)raw + offset,
                (size_t)(end - raw - offset), expected, sizeof(expected)) < 0 ||
            strcmp(expected, state->dm1ObjectNames[i]) != 0) {
            fprintf(stderr, "FAIL: authentic F20J M564 name index %d\n", i);
            goto done;
        }
        offset = (size_t)(end - raw) + 1;
    }
    if (!state->dm1ObjectNameTableValid) goto done;
    {
        char full[96], clipped[97];
        unsigned short thing = (unsigned short)(THING_TYPE_WEAPON << 10);
        if (!state->world.things || state->world.things->weaponCount <= 0 ||
            !DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing) ||
            !DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(state, full, sizeof(full)))
            goto done;
        for (size_t capacity = 1; capacity <= strlen(full) + 1; ++capacity) {
            size_t expected = capacity - 1;
            while (expected && ((unsigned char)full[expected] & 0xc0u) == 0x80u)
                --expected;
            memset(clipped, 0x55, sizeof(clipped));
            (void)DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(
                state, clipped, (int)capacity);
            if (strlen(clipped) != expected || memcmp(clipped, full, expected) ||
                clipped[capacity] != 0x55) {
                fprintf(stderr, "FAIL: Japanese hand name clipping at capacity %zu\n", capacity);
                goto done;
            }
        }
        puts("PASS: original Japanese weapon hand label clips only at UTF-8 boundaries");
    }
    if (state->dm1FmtownsStartupReceipt.game_action_name_count != 44) goto done;
    for (unsigned int i = 0; i < 44; ++i) {
        char utf8[48];
        const char *original = state->dm1FmtownsStartupReceipt.game_action_names[i];
        if (strcmp(original, dm1_v1_fmtowns_dyna_button_label_ja_pc34(i)) ||
            firestaff_cp932_to_utf8(original, strlen(original), utf8, sizeof(utf8)) < 0) {
            fprintf(stderr, "FAIL: original JDM action index %u\n", i);
            goto done;
        }
    }
    puts("PASS: all 44 original JDM action names match the reviewed source pool");
    puts("PASS: all 199 authentic F20J names retain their source indices and UTF-8 keys");
    if (!state->originalFontAvailable ||
        !dm1_v1_legacy_graphics_read_raw(bytes, size, 0, 557, raw, sizeof(raw), &length) ||
        length != M11_FONT_BITMAP_BYTES ||
        memcmp(raw, state->originalFont.bitmap, M11_FONT_BITMAP_BYTES)) goto done;
    puts("PASS: FM Towns M653 interface font bytes match original media (not system Kanji glyphs)");
    if (!state->audioState.initialized && !M11_Audio_Init(&state->audioState)) goto done;
    for (int language = 0; language < 2; ++language) {
      if (language == 1) {
        free(bytes);
        bytes = NULL;
        M11_GameView_Shutdown(state);
        M11_GameView_Init(state);
        spec.dm1FmtownsJapanese = 0;
        if (!M11_GameView_Start(state, &spec) ||
            !state->dm1FmtownsStartupReceiptValid ||
            state->dm1FmtownsStartupReceipt.language != DM1_FMTOWNS_LANG_EN) goto done;
        size = (size_t)state->assetLoader.legacyDataSize;
        bytes = malloc(size);
        if (!bytes) goto done;
        memcpy(bytes, state->assetLoader.legacyData, size);
        if (!state->audioState.initialized && !M11_Audio_Init(&state->audioState)) goto done;
      }
      for (int event = 0; event < 35; ++event) {
        if (state->audioState.originalSnd3Available || state->audioState.originalSnd3LoadedCount) goto done;
        int index = M11_Audio_Dm1AtariSoundIndex(event);
        int accepted = M11_Audio_EmitDm1FmtownsSound(&state->audioState,
            bytes, size, event, 127);
        const CsbV1AtariStSoundSpec* sound =
            csb_v1_audio_runtime_atari_st_sound_spec((int16_t)index);
        size_t samples;
        if (accepted != (index >= 0)) {
            fprintf(stderr, "FAIL: F20 PCM event %d\n", event);
            goto done;
        }
        if (!accepted) continue;
        if (!sound || !dm1_v1_legacy_graphics_read_raw(bytes, size, 0,
                sound->graphicIndex, raw, sizeof(raw), &length) || length < 2) goto done;
        samples = ((size_t)raw[0] << 8) | raw[1];
        if (samples > 31936) samples = 31936;
        if (samples > length - 2 || state->audioState.csbFmtownsRuntimePcm.sampleCount !=
                (int)((samples * M11_AUDIO_SAMPLE_RATE + 5499) / 5500)) goto done;
        for (int p = 0; p < state->audioState.csbFmtownsRuntimePcm.sampleCount; ++p) {
            size_t source = (size_t)p * 5500 / M11_AUDIO_SAMPLE_RATE;
            float expected;
            if (source >= samples) source = samples - 1;
            expected = ((int)raw[source + 2] - 128) / 128.0f;
            if (state->audioState.csbFmtownsRuntimePcm.samples[p] != expected) goto done;
        }
    }
      }
    puts("PASS: original F20 EN/JP unsigned PCM matches every resampled output sample at 5500 Hz");
    result = 0;
done:
    free(bytes);
    M11_GameView_Shutdown(state);
    free(state);
    return result;
}
