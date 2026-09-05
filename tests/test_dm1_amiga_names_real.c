#include "m11_game_view.h"
#include "asset_status_m12.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    result = 0;
done:
    if (result) fputs("FAIL: original Amiga name/media selection gate\n", stderr);
    M11_GameView_Shutdown(state); free(state); free(scan);
    return result;
}
