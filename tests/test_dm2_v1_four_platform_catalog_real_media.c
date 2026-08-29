/* Shared DM2 data-root catalog receipt.
 *
 * This is deliberately a launcher-level test: each edition must remain
 * selectable from one .firestaff/data/dm2 root, and the resolver must return
 * that edition's original owner instead of the scan's first match. */

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int ok, const char *message)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void check_version(const M12_AssetStatus *status,
                          const char *version_id,
                          const char *owner_hint)
{
    int index = M12_AssetStatus_FindVersionIndex("dm2", version_id);
    const M12_AssetVersionStatus *version = index >= 0
        ? M12_AssetStatus_GetVersion(status, "dm2", (size_t)index)
        : NULL;
    char resolved[M12_ASSET_DATA_DIR_CAPACITY];

    expect(version && version->matched, version_id);
    memset(resolved, 0, sizeof(resolved));
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               status, "dm2", version_id, resolved, sizeof(resolved)),
           "version resolver returns an owner");
    expect(strstr(resolved, owner_hint) != NULL,
           "version resolver retains the selected platform owner");
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_SHARED_ROOT");
    M12_AssetStatus status;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_SHARED_ROOT is not set");
        return 77;
    }
    memset(&status, 0, sizeof(status));
    M12_AssetStatus_ScanGame(&status, root, "dm2");

    /* All four retail editions are selected directly from their supplied
     * archives.  The resolver must retain each original container rather
     * than manufacturing an extracted data directory or borrowing the
     * first scan match. */
    check_version(&status, "pc-en", "Dungeon-Master-II-Skullkeep_DOS_EN.zip");
    check_version(&status, "amiga-en", "Dungeon-Master-II-Skullkeep_Amiga_EN.zip");
    check_version(&status, "fmtowns-ja", "Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip");
    check_version(&status, "mac-en-retail", "Dungeon-Master-II-Skullkeep_Mac_EN");

    if (failures) return 1;
    puts("PASS: shared DM2 root selects DOS, Amiga, FM Towns and Mac owners");
    return 0;
}
