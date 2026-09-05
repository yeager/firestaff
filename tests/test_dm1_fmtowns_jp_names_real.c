#include "m11_game_view.h"
#include "asset_find_by_hash.h"
#include "csb_v1_boot.h"
#include "firestaff_cp932.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "dm1_v1_fmtowns_dyna_buttons_ja.h"
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
    result = 0;
done:
    free(bytes);
    M11_GameView_Shutdown(state);
    free(state);
    return result;
}
