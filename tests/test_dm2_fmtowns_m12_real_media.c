/* Opt-in real-media regression for the DM2 FM Towns launcher receipt.
 *
 * The archive remains user-owned.  This test only scans it and asserts the
 * M12 receipt keeps the virtual source path; it never extracts a game file to
 * disk.  Set FIRESTAFF_DM2_FMTOWNS_ROOT to either the global data root (with
 * dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip below it) or the dm2
 * directory itself. */

#include "asset_status_m12.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char* root = getenv("FIRESTAFF_DM2_FMTOWNS_ROOT");
    const char* english_companion =
        getenv("FIRESTAFF_DM2_ENGLISH_COMPANION");
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;
    char selectedRuntime[512];
    int versionIndex;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_FMTOWNS_ROOT is not set");
        return 0;
    }

    memset(&status, 0, sizeof(status));
    memset(selectedRuntime, 0, sizeof(selectedRuntime));
    M12_AssetStatus_ScanGame(&status, root, "dm2");
    versionIndex = M12_AssetStatus_FindVersionIndex("dm2", "fmtowns-ja");
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&status, "dm2", (size_t)versionIndex)
        : NULL;
    graphics = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);

    expect(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
           "FM Towns original ZIP is launch-admitted by M12");
    expect(version && version->matched &&
               strcmp(version->matchedMd5,
                      "027ff3b8ddc2c4c4cdda7ada0b0bc46c") == 0 &&
               strstr(version->matchedPath,
                      ".zip::DATA/GRAPHICS.DAT") != NULL,
           "M12 records the verified FM Towns GDAT as virtual provenance");
    /* Required-file rows are the scan's default launch pair.  In a shared
     * root that may correctly be PC-DOS, so edition-specific provenance is
     * asserted through the resolver below.  A direct archive request has no
     * competing edition and must publish its two virtual members directly. */
    if (strstr(root, "Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip") != NULL) {
        expect(graphics && graphics->matched &&
                   strcmp(graphics->matchedHash,
                          "027ff3b8ddc2c4c4cdda7ada0b0bc46c") == 0 &&
                   strstr(graphics->matchedPath,
                          ".zip::DATA/GRAPHICS.DAT") != NULL,
               "direct FM Towns GRAPHICS.DAT remains a virtual archive member");
        expect(dungeon && dungeon->matched &&
                   strcmp(dungeon->matchedHash,
                          "74c7549f174574201988bf936385841a") == 0 &&
                   strstr(dungeon->matchedPath,
                          ".zip::DATA/DUNGEON.DAT") != NULL,
               "direct FM Towns DUNGEON.DAT remains a virtual archive member");
    }
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               &status, "dm2", "fmtowns-ja", selectedRuntime,
               sizeof(selectedRuntime)) &&
               strstr(selectedRuntime,
                      "Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip") != NULL,
           "selected FM Towns edition retains its original archive handoff");

    /* The Japanese CD is still the session owner. English is admitted only
     * through this separately selected canonical PC corpus; both sources are
     * read in memory and the ZIP is never unpacked to disk. */
    if (english_companion && english_companion[0] != '\0') {
        DM2_V1_BootStartupLaunch launch;
        DM2_V1_BootStartupLaunch missing_companion;
        const uint8_t* text;
        size_t text_size = 0u;
        memset(&missing_companion, 0, sizeof(missing_companion));
        expect(dm2_v1_boot_startup_launch_alloc_with_language(
                   selectedRuntime, NULL, 0, &missing_companion) == 0,
               "FM Towns English refuses to launch without its real companion corpus");
        memset(&launch, 0, sizeof(launch));
        expect(dm2_v1_boot_startup_launch_alloc_with_language(
                   selectedRuntime, english_companion, 0, &launch) == 1,
               "FM Towns English requires and accepts the explicit PC-English companion");
        expect(launch.profile &&
                   launch.profile->platform == DM2_PLATFORM_FMTOWNS_JA &&
                   dm2_v1_runtime_i18n_ready(),
               "FM Towns runtime keeps Japanese CD ownership and binds English text only");
        text = dm2_v1_runtime_i18n_text(0x07, 0x00, 0x00, &text_size);
        expect(text && text_size >= 7u && memcmp(text, "FIGHTER", 7u) == 0,
               "English text comes from the authenticated PC GDAT companion");
        dm2_v1_boot_startup_launch_cleanup(&launch);
    } else {
        puts("SKIP: FIRESTAFF_DM2_ENGLISH_COMPANION is not set");
    }

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 FM Towns M12 real-media receipt stays in the original ZIP");
    return 0;
}
