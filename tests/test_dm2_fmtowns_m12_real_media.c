/* Opt-in real-media regression for the DM2 FM Towns launcher receipt.
 *
 * The archive remains user-owned.  This test only scans it and asserts the
 * M12 receipt keeps the virtual source path; it never extracts a game file to
 * disk.  Set FIRESTAFF_DM2_FMTOWNS_ROOT to either the global data root (with
 * dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip below it) or the dm2
 * directory itself. */

#include "asset_status_m12.h"

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
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;
    int versionIndex;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_FMTOWNS_ROOT is not set");
        return 0;
    }

    memset(&status, 0, sizeof(status));
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
    expect(graphics && graphics->matched &&
               strcmp(graphics->matchedHash,
                      "027ff3b8ddc2c4c4cdda7ada0b0bc46c") == 0 &&
               strstr(graphics->matchedPath,
                      ".zip::DATA/GRAPHICS.DAT") != NULL,
           "required GRAPHICS.DAT remains a virtual archive member");
    expect(dungeon && dungeon->matched &&
               strcmp(dungeon->matchedHash,
                      "74c7549f174574201988bf936385841a") == 0 &&
               strstr(dungeon->matchedPath,
                      ".zip::DATA/DUNGEON.DAT") != NULL,
           "required DUNGEON.DAT remains a virtual archive member");
    expect(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm2"), root) == 0,
           "M12 passes the unchanged archive root to the memory-owned boot path");

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 FM Towns M12 real-media receipt stays in the original ZIP");
    return 0;
}
