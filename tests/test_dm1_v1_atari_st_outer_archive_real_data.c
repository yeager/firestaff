#include "asset_status_m12.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_ATARI_ST_OUTER_ARCHIVE");
    M12_AssetStatus status;
    const M12_AssetVersionStatus *version = NULL;
    size_t i;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Atari ST preservation archive is not staged");
        return 77;
    }
    /* Preservation packages exist in both ZIP -> ZIP -> STX and the
     * current retail ZIP -> STX form.  Exercise the public M12 admission
     * path rather than a hand-written virtual member spelling: this is the
     * exact scanner/reader used by the launcher and keeps all members in
     * memory. */
    memset(&status, 0, sizeof(status));
    M12_AssetStatus_ScanGame(&status, archive, "dm1");
    for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
        const M12_AssetVersionStatus *candidate =
            M12_AssetStatus_GetVersion(&status, "dm1", i);
        if (candidate && candidate->matched &&
            (strcmp(candidate->versionId, "st10a-en") == 0 ||
             strcmp(candidate->versionId, "st12-en") == 0)) {
            version = candidate;
            break;
        }
    }
    expect(version != NULL,
           "the authentic ZIP -> STX or ZIP -> ZIP -> STX original is admitted as Atari ST");
    expect(version && strstr(version->matchedPath,
                             ".stx::GRAPHICS.DAT"),
           "the Atari graphics receipt remains a virtual source path");
    expect(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
           "the authentic Atari preservation archive satisfies DM1 launch requirements");
    expect(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"), archive) == 0,
           "the runtime owner remains the supplied archive without extraction");
    for (i = 0U; i < M12_AssetStatus_GetRequiredFileCount(&status, "dm1"); ++i) {
        const M12_AssetRequiredFileStatus *required =
            M12_AssetStatus_GetRequiredFile(&status, "dm1", i);
        expect(required && required->matched,
               "both required Atari ST files are read from the original STX");
    }
    if (failures != 0) return 1;
    puts("PASS: authentic outer DM1 Atari archive is admitted without extraction");
    return 0;
}
