/* Shared real-media DM1 catalog and runtime-owner checks. */

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    const char *archive;
    const char *version_id;
    const char *owner_hint;
} DM1CatalogCase;

static int failures;
static int checked;

static void check_version(const M12_AssetStatus *status,
                          const char *root,
                          const DM1CatalogCase *test_case)
{
    char archive_path[M12_ASSET_DATA_DIR_CAPACITY];
    char resolved[M12_ASSET_DATA_DIR_CAPACITY];
    struct stat info;
    int index;
    const M12_AssetVersionStatus *version;

    if (snprintf(archive_path, sizeof(archive_path), "%s/dm1/%s",
                 root, test_case->archive) >= (int)sizeof(archive_path)) {
        fprintf(stderr, "FAIL: DM1 archive path is too long: %s\n",
                test_case->archive);
        ++failures;
        return;
    }
    if (stat(archive_path, &info) != 0)
        return;

    ++checked;
    index = M12_AssetStatus_FindVersionIndex("dm1", test_case->version_id);
    version = index >= 0
        ? M12_AssetStatus_GetVersion(status, "dm1", (size_t)index)
        : NULL;
    if (!version || !version->matched) {
        fprintf(stderr, "FAIL: installed DM1 %s was not catalogued\n",
                test_case->version_id);
        ++failures;
        return;
    }
    if (strstr(version->matchedPath, test_case->owner_hint) == NULL) {
        fprintf(stderr,
                "FAIL: DM1 %s matched the wrong owner: %s (expected %s)\n",
                test_case->version_id, version->matchedPath,
                test_case->owner_hint);
        ++failures;
        return;
    }

    memset(resolved, 0, sizeof(resolved));
    if (!M12_AssetStatus_ResolveRuntimeDataDirForVersion(
            status, "dm1", test_case->version_id,
            resolved, sizeof(resolved)) ||
        strstr(resolved, test_case->owner_hint) == NULL) {
        fprintf(stderr,
                "FAIL: DM1 %s runtime resolution lost owner %s: %s\n",
                test_case->version_id, test_case->owner_hint, resolved);
        ++failures;
        return;
    }
    printf("PASS: shared DM1 root selects %s from %s\n",
           test_case->version_id, test_case->owner_hint);
}

int main(void)
{
    static const DM1CatalogCase cases[] = {
        {"Dungeon-Master_FM-Towns_JA-EN.zip", "fmtowns-en",
         "Dungeon-Master_FM-Towns_JA-EN.zip"},
        {"Dungeon-Master_FM-Towns_JA-EN.zip", "fmtowns-ja",
         "Dungeon-Master_FM-Towns_JA-EN.zip"},
        {"Dungeon-Master_DOS_EN_Version-34.zip", "pc34-en",
         "dos_extract/Dungeon-Master_DOS_EN_Version-34"},
        {"Dungeon-Master_DOS_FR_EUDATA.zip", "pc34-multi",
         "Dungeon-Master_DOS_FR_EUDATA.zip"},
        {"Dungeon-Master_Atari-ST_EN_Version-12.zip", "st12-en",
         "Dungeon-Master_Atari-ST_EN_Version-12.zip"},
        {"Dungeon-Master_Atari-ST_DE_Version-12.zip", "st12-de",
         "Game,Dungeon_Master,Atari_ST,Software.7z"},
        {"Dungeon-Master_Atari-ST_FR_Version-13.zip", "st13-fr",
         "Game,Dungeon_Master,Atari_ST,Software.7z"},
        {"Dungeon-Master_Amiga_EN_Version-20.zip", "amiga20-en",
         "Dungeon-Master_Amiga_EN_Version-20.zip"}
    };
    const char *root = getenv("FIRESTAFF_DM1_SHARED_ROOT");
    M12_AssetStatus status;
    size_t i;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM1_SHARED_ROOT is not set");
        return 77;
    }

    memset(&status, 0, sizeof(status));
    M12_AssetStatus_ScanGame(&status, root, "dm1");
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i)
        check_version(&status, root, &cases[i]);

    if (checked == 0) {
        puts("SKIP: no catalogued DM1 original archives are installed");
        return 77;
    }
    if (failures)
        return 1;
    printf("PASS: shared DM1 catalog retained original owners for %d installed editions\n",
           checked);
    return 0;
}
