/* Opt-in M12 receipt for the original DM2 Amiga AGA installer.
 *
 * FIRESTAFF_DM2_AMIGA_ROOT may name the original ZIP directly, or a directory
 * containing it. The scan reads nested media in RAM and preserves the archive
 * as the boot handoff. */

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
    const char *root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    const M12_AssetVersionStatus *version;
    const M12_AssetRequiredFileStatus *graphics;
    const M12_AssetRequiredFileStatus *dungeon;
    M12_AssetStatus status;
    char selectedRuntime[512];
    int versionIndex;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 0;
    }
    memset(&status, 0, sizeof(status));
    memset(selectedRuntime, 0, sizeof(selectedRuntime));
    M12_AssetStatus_ScanGame(&status, root, "dm2");
    versionIndex = M12_AssetStatus_FindVersionIndex("dm2", "amiga-en");
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&status, "dm2", (size_t)versionIndex)
        : NULL;
    graphics = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0u);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1u);

    expect(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
           "M12 launch-admits the original Amiga installer only as a complete pair");
    expect(version && version->matched &&
               strcmp(version->matchedMd5,
                      "1c940ea95703eaea0ecdf84d17e954b9") == 0 &&
               strstr(version->matchedPath,
                      ".zip::DM2_archive.LZX/GRAPHICS.DAT") != NULL,
           "M12 records the original Amiga GDAT as nested virtual provenance");
    /* Required rows describe the scan default.  A shared root can quite
     * legitimately select DOS there; selected-edition ownership is tested
     * through the resolver.  With an explicit Amiga archive there is no
     * competing default, so retain the direct virtual-payload assertion. */
    if (strstr(root, "Dungeon-Master-II-Skullkeep_Amiga_EN.zip") != NULL) {
        expect(graphics && graphics->matched &&
                   strcmp(graphics->matchedHash,
                          "1c940ea95703eaea0ecdf84d17e954b9") == 0 &&
                   dungeon && dungeon->matched &&
                   strcmp(dungeon->matchedHash,
                          "719ae78bc124027806c65491a256827d") == 0,
               "direct Amiga required rows are backed by the verified LZX payload");
    }
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               &status, "dm2", "amiga-en", selectedRuntime,
               sizeof(selectedRuntime)) &&
               strstr(selectedRuntime,
                      "Dungeon-Master-II-Skullkeep_Amiga_EN.zip") != NULL,
           "selected Amiga edition retains its original archive handoff");
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 Amiga M12 receipt keeps original installer media in RAM");
    return 0;
}
