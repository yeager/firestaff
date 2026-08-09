/* CSB Amiga A31E must not be reported launchable from only its base pair.
 * ReDMCSB COMPILE.H:199-213 assigns APPB.FTL to C03_GAME; this deliberately
 * rejects the incompatible A31M TITL/APPA surrogate. */

#include "asset_status_m12.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    M12_AssetStatus status;
    const char* root = getenv("FIRESTAFF_CSB_A31E_DATA_DIR");
    const M12_AssetVersionStatus* a31e;
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
    if (M12_AssetStatus_GameAvailable(&status, "csb") != 0 ||
        strstr(M12_AssetStatus_GetCSBLaunchBlockReason(&status),
               "APPB.FTL C03") == NULL) {
        fputs("FAIL: A31E base pair was falsely reported startable\n", stderr);
        return 1;
    }
    puts("ok: CSB A31E native handoff gate blocks false READY status");
    return 0;
}
