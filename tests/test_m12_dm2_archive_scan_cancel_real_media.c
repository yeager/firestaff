#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int sawDiscImage;
    unsigned int maximumPercent;
} CancelOnDm2Image;

static int cancel_during_dm2_disc_image(
    const M12_AssetScanProgress* progress, void* userData) {
    CancelOnDm2Image* probe = (CancelOnDm2Image*)userData;
    if (!progress || !probe) return 1;
    if (strcmp(progress->currentGameId, "dm2") == 0 &&
        strcmp(progress->currentTask,
               "reading original DM2 disc image") == 0) {
        probe->sawDiscImage = 1;
        if (progress->currentStepPercent > probe->maximumPercent)
            probe->maximumPercent = progress->currentStepPercent;
        return progress->currentStepPercent == 0U;
    }
    return 1;
}

int main(void) {
    const char* archive = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    M12_AssetStatus status;
    M12_AssetStatusScanOptions options;
    CancelOnDm2Image probe;
    int result;

    if (!archive || !archive[0] || !FSP_FileExists(archive)) {
        puts("SKIP: authentic DM2 Macintosh retail ZIP is unavailable");
        return 77;
    }
    memset(&status, 0, sizeof(status));
    memset(&options, 0, sizeof(options));
    memset(&probe, 0, sizeof(probe));
    options.progressFn = cancel_during_dm2_disc_image;
    options.progressUserData = &probe;

    result = M12_AssetStatus_ScanWithOptions(&status, archive, &options);
    if (result != 0 || !probe.sawDiscImage ||
        probe.maximumPercent == 0U ||
        !status.scanProgress.cancelled || status.scanProgress.complete ||
        status.scanProgress.active ||
        M12_AssetStatus_GameAvailable(&status, "dm2")) {
        fprintf(stderr,
                "FAIL: original DM2 archive cancellation result=%d "
                "saw-image=%d progress=%u cancelled=%d complete=%d active=%d "
                "available=%d\n",
                result, probe.sawDiscImage, probe.maximumPercent,
                status.scanProgress.cancelled, status.scanProgress.complete,
                status.scanProgress.active,
                M12_AssetStatus_GameAvailable(&status, "dm2"));
        return 1;
    }
    printf("PASS: M12 cancelled original DM2 disc-image inflation at %u%% "
           "without publishing game availability\n",
           probe.maximumPercent);
    return 0;
}
