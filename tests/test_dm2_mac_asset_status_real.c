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

static int check_archive(const char *path, const char *version_id)
{
    M12_AssetStatus status;
    char runtime[1024];
    int version_index;
    const M12_AssetVersionStatus *version;
    size_t i;

    memset(&status, 0, sizeof(status));
    memset(runtime, 0, sizeof(runtime));
    M12_AssetStatus_ScanGame(&status, path, "dm2");
    version_index = M12_AssetStatus_FindVersionIndex("dm2", version_id);
    version = version_index >= 0
        ? M12_AssetStatus_GetVersion(&status, "dm2", (size_t)version_index)
        : NULL;
    expect(version && version->matched,
           "the selected authentic Mac archive is admitted as its Mac edition");
    expect(version && strstr(version->matchedPath, "::HFS/DMFiles/Graphics.dat") != NULL,
           "Mac admission retains the virtual HFS Graphics.dat provenance");
    expect(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
           "the selected authentic Mac archive is launch-ready");
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               &status, "dm2", version_id, runtime, sizeof(runtime)) == 1 &&
               strcmp(runtime, path) == 0,
           "Mac runtime resolution keeps the original ZIP path");
    for (i = 0; i < M12_AssetStatus_GetRequiredFileCount(&status, "dm2"); ++i) {
        const M12_AssetRequiredFileStatus *required =
            M12_AssetStatus_GetRequiredFile(&status, "dm2", i);
        if (required && required->required) {
            expect(required->matched,
                   "all required DM2 files are satisfied by the selected Mac archive");
        }
    }
    return failures == 0 ? 0 : 1;
}

int main(void)
{
    const char *demo = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    const char *retail = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    const char *root = getenv("FIRESTAFF_DM2_MAC_DATA_ROOT");
    M12_AssetStatus combined;
    const M12_AssetVersionStatus *demo_version;
    const M12_AssetVersionStatus *retail_version;
    int demo_index;
    int retail_index;

    if (!demo || !demo[0] || !retail || !retail[0]) {
        puts("SKIP: DM2 Mac archive environment is not set");
        return 0;
    }
    check_archive(demo, "mac-en-demo");
    check_archive(retail, "mac-en-retail");
    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_DATA_ROOT is not set; combined-root check skipped");
    } else {
        memset(&combined, 0, sizeof(combined));
        M12_AssetStatus_ScanGame(&combined, root, "dm2");
        demo_index = M12_AssetStatus_FindVersionIndex("dm2", "mac-en-demo");
        retail_index = M12_AssetStatus_FindVersionIndex("dm2", "mac-en-retail");
        demo_version = demo_index >= 0
            ? M12_AssetStatus_GetVersion(&combined, "dm2", (size_t)demo_index)
            : NULL;
        retail_version = retail_index >= 0
            ? M12_AssetStatus_GetVersion(&combined, "dm2", (size_t)retail_index)
            : NULL;
        expect(demo_version && demo_version->matched,
               "a shared DM2 data root retains the authentic Mac demo edition");
        expect(retail_version && retail_version->matched,
               "a shared DM2 data root retains the authentic Mac retail edition");
    }
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 Mac archive admission preserves both authentic editions");
    return 0;
}
