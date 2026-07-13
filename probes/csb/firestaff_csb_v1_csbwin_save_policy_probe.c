/* Opt-in real CSBWin save policy probe.
 * Source: CSBWin SaveGame.cpp:1928-2034; no fixture or fallback route. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    CSB_V1_RuntimeProfile profile;
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSBWIN_SAVE");
    uint32_t delete_duplicate;
    uint32_t debugging;
    uint32_t csbgraphics;
    uint32_t graphics;
    uint32_t version;

    if (!path || !path[0]) {
        printf("SKIP: set FIRESTAFF_CSBWIN_SAVE to an original CSBWin save\n");
        return 0;
    }
    csb_v1_runtime_init(&profile, NULL);
    if (csb_v1_runtime_apply_csbwin_resume_file(&profile, path, 0u) != 0 ||
        csb_v1_runtime_get_csbwin_save_policy(
            &profile, &delete_duplicate, &debugging, &csbgraphics,
            &graphics, &version) != 0) {
        fprintf(stderr, "FAIL: original CSBWin save did not reach policy handoff\n");
        csb_v1_runtime_cleanup(&profile);
        return 1;
    }
    printf("PASS: original CSBWin save reached post-palette EXPOOL handoff\n");
    printf("delete_duplicate_timers=%u debugging=0x%08x csbgraphics=0x%08x "
           "graphics=0x%08x version=0x%08x saves_disabled=%d\n",
           (unsigned)delete_duplicate, (unsigned)debugging,
           (unsigned)csbgraphics, (unsigned)graphics, (unsigned)version,
           csb_v1_runtime_csbwin_saves_disabled(&profile));
    csb_v1_runtime_cleanup(&profile);
    return 0;
}
