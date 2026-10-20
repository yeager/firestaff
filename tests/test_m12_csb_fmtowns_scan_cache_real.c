/* Verify cold and warm admission of the authentic CSB FM Towns CD image. */
#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int fullImageReads;
    unsigned int cachedReceipts;
} ScanEvidence;

static int count_scan_evidence(const M12_AssetScanProgress* progress,
                               void* userData)
{
    ScanEvidence* evidence = (ScanEvidence*)userData;
    if (!progress || !evidence || strcmp(progress->currentGameId, "csb") != 0)
        return 1;
    if (strcmp(progress->currentTask,
               "authenticating CSB FM Towns image") == 0)
        ++evidence->fullImageReads;
    if (strcmp(progress->currentTask,
               "reusing verified FM Towns media receipt") == 0)
        ++evidence->cachedReceipts;
    return 1;
}

static int scan_once(const char* root, M12_AssetStatus* status,
                     ScanEvidence* evidence)
{
    M12_AssetStatusScanOptions options;
    memset(&options, 0, sizeof(options));
    options.honorRequestedDataDir = 1;
    options.progressFn = count_scan_evidence;
    options.progressUserData = evidence;
    return M12_AssetStatus_ScanWithOptions(status, root, &options);
}

int main(void)
{
    const char* archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char* home = getenv("HOME");
    char cachePath[4096];
    M12_AssetStatus cold, warm;
    ScanEvidence coldEvidence = {0U, 0U};
    ScanEvidence warmEvidence = {0U, 0U};
    const int english = M12_AssetStatus_FindVersionIndex("csb", "fmtowns-en");
    const int japanese = M12_AssetStatus_FindVersionIndex("csb", "fmtowns-ja");
    FILE* media;

    if (!archive || !archive[0] || !(media = fopen(archive, "rb"))) {
        puts("SKIP: authentic CSB FM Towns archive is not staged");
        return 77;
    }
    fclose(media);
    if (!home || !FSP_CreateDirectoryRecursive(home)) {
        fputs("FAIL: create isolated scan-cache home\n", stderr);
        return 1;
    }
    if (snprintf(cachePath, sizeof(cachePath),
                 "%s/.firestaff/cache/asset_scan_cache.dat", home) >=
        (int)sizeof(cachePath)) {
        fputs("FAIL: scan-cache path is too long\n", stderr);
        return 1;
    }
    /* CTest may rerun this target with its isolated HOME intact. Force the
     * first measured scan to be cold so the integration check is repeatable. */
    (void)remove(cachePath);
    memset(&cold, 0, sizeof(cold));
    memset(&warm, 0, sizeof(warm));
    if (!scan_once(archive, &cold, &coldEvidence) || english < 0 || japanese < 0 ||
        !M12_AssetStatus_GetVersion(&cold, "csb", (size_t)english)->matched ||
        !M12_AssetStatus_GetVersion(&cold, "csb", (size_t)japanese)->matched ||
        coldEvidence.fullImageReads == 0U) {
        fputs("FAIL: cold scan did not authenticate both original CSB FM Towns editions\n",
              stderr);
        return 1;
    }
    if (!scan_once(archive, &warm, &warmEvidence) ||
        !M12_AssetStatus_GetVersion(&warm, "csb", (size_t)english)->matched ||
        !M12_AssetStatus_GetVersion(&warm, "csb", (size_t)japanese)->matched ||
        warmEvidence.fullImageReads != 0U || warmEvidence.cachedReceipts == 0U) {
        fputs("FAIL: warm scan did not reuse the verified original CD receipt\n",
              stderr);
        return 1;
    }
    puts("PASS: authentic CSB FM Towns cold scan verified both editions; warm scan reused metadata without rereading the CD image");
    return 0;
}
