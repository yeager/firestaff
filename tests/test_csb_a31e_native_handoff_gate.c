/* CSB Amiga A31E owns a direct C03_GAME APPB.FTL route.  It must admit only
 * when the selected original ADF also supplies A31E's own APPB/BJELoad pair;
 * A31M's TITL/APPA/KAOS chain is not an interchangeable fallback.
 * ReDMCSB COMPILE.H:199-213. */

#include "asset_status_m12.h"
#include "asset_find_by_hash.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    M12_AssetStatus status;
    const char* root = getenv("FIRESTAFF_CSB_A31E_DATA_DIR");
    const M12_AssetVersionStatus* a31e;
    char runtime_dir[M12_ASSET_DATA_DIR_CAPACITY];
    char appb_path[M12_ASSET_DATA_DIR_CAPACITY];
    char launcher_path[M12_ASSET_DATA_DIR_CAPACITY];
    int a31eIndex;

    if (!root || root[0] == '\0') {
        puts("SKIP: set FIRESTAFF_CSB_A31E_DATA_DIR to original A31E media");
        return 77;
    }
    M12_AssetStatus_ScanGame(&status, root, "csb");
    a31eIndex = M12_AssetStatus_FindVersionIndex("csb", "amiga31-en");
    a31e = a31eIndex >= 0
        ? M12_AssetStatus_GetVersion(&status, "csb", (size_t)a31eIndex) : NULL;
    /* A31M is the EN/FR/DE disk and has a different native route:
     * COMPILE.H:246-269 gives APPB.FTL C08_LANG and KAOS.FTL C03_GAME.
     * Do not turn that valid, multilingual corpus into a false A31E gate
     * regression merely because its archive name mentions version 3.1.
     * The A31E fixture is the separate English-only disk (COMPILE.H:
     * 199-213), whose APPB.FTL itself owns C03_GAME. */
    if (!a31e || !a31e->matched) {
        puts("SKIP: supplied media is not the English-only A31E corpus");
        return 77;
    }
    if (!M12_AssetStatus_GameAvailable(&status, "csb") ||
        !M12_AssetStatus_MaterializeCSBRuntimeVersion(
            &status, "amiga31-en", runtime_dir, sizeof(runtime_dir)) ||
        snprintf(appb_path, sizeof(appb_path), "%s/APPB.FTL", runtime_dir) >=
            (int)sizeof(appb_path) ||
        snprintf(launcher_path, sizeof(launcher_path), "%s/BJELoad_R",
                 runtime_dir) >= (int)sizeof(launcher_path) ||
        !asset_file_matches_md5(appb_path,
                                "af50ff33c61c22e20784d74266d81d1e") ||
        !asset_file_matches_md5(launcher_path,
                                "2788ef63f246b4d3c74c64dff9dcdbde")) {
        fputs("FAIL: A31E did not retain its native C03 program receipt\n", stderr);
        return 1;
    }
    puts("ok: CSB A31E native C03 handoff program receipt is ready");
    return 0;
}
