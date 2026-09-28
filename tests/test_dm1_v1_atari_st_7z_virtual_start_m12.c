#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int file_hash_matches(const char *path, const char *expected_md5)
{
    char actual_md5[33];
    return path && asset_file_md5_hex(path, actual_md5) &&
           strcmp(actual_md5, expected_md5) == 0;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_ATARI_ST_7Z");
    M12_AssetStatus status;
    const M12_AssetVersionStatus *version = NULL;
    const M12_AssetVersionStatus *launch_version = NULL;
    const M12_AssetRequiredFileStatus *graphics = NULL;
    const M12_AssetRequiredFileStatus *dungeon = NULL;
    char dungeon_md5[33];
    size_t version_count;
    size_t required_count;
    int launch_version_index;
    int auto_version_index;
    size_t i;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Atari ST multi-member 7z is not configured");
        return 77;
    }

    memset(&status, 0, sizeof(status));
    M12_AssetStatus_ScanGameWithOptions(&status, archive, "dm1", NULL);
    launch_version_index = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
        &status, "dm1", M12_ARCH_ATARI_ST);
    if (launch_version_index >= 0) {
        launch_version = M12_AssetStatus_GetVersion(
            &status, "dm1", (size_t)launch_version_index);
    }
    auto_version_index = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
        &status, "dm1", M12_ARCH_AUTO);
    version_count = M12_AssetStatus_GetVersionCount("dm1");
    for (i = 0; i < version_count; ++i) {
        const M12_AssetVersionStatus *candidate =
            M12_AssetStatus_GetVersion(&status, "dm1", i);
        if (candidate && candidate->versionId &&
            strcmp(candidate->versionId, "st11-en") == 0) {
            version = candidate;
            break;
        }
    }
    required_count = M12_AssetStatus_GetRequiredFileCount(&status, "dm1");
    for (i = 0; i < required_count; ++i) {
        const M12_AssetRequiredFileStatus *candidate =
            M12_AssetStatus_GetRequiredFile(&status, "dm1", i);
        if (!candidate || !candidate->roleId) continue;
        if (strcmp(candidate->roleId, "graphics") == 0) graphics = candidate;
        if (strcmp(candidate->roleId, "dungeon") == 0) dungeon = candidate;
    }

    if (!M12_AssetStatus_GameAvailable(&status, "dm1") || !version ||
        !version->matched ||
        strcmp(version->matchedMd5, "5095a13692702235d2e74f6b2b1367a9") != 0 ||
        !launch_version || !launch_version->versionId ||
        strcmp(launch_version->versionId, "st11-en") != 0 ||
        auto_version_index != launch_version_index ||
        !graphics || !graphics->matched || !dungeon || !dungeon->matched ||
        !file_hash_matches(graphics->matchedPath,
                           "5095a13692702235d2e74f6b2b1367a9") ||
        !asset_file_md5_hex(dungeon->matchedPath, dungeon_md5) ||
        strncmp(graphics->matchedPath, archive, strlen(archive)) != 0 ||
        strncmp(dungeon->matchedPath, archive, strlen(archive)) != 0) {
        fprintf(stderr,
                "FAIL: DM1 Atari ST 1.1 did not resolve and select authentic virtual 7z members\n"
                "  available=%d version=%s matched=%d launchVersionIndex=%d autoVersionIndex=%d graphics=%s dungeon=%s\n",
                M12_AssetStatus_GameAvailable(&status, "dm1"),
                version && version->versionId ? version->versionId : "missing",
                version ? version->matched : 0,
                launch_version_index,
                auto_version_index,
                graphics && graphics->matchedPath[0] ? graphics->matchedPath
                                                     : "missing",
                dungeon && dungeon->matchedPath[0] ? dungeon->matchedPath
                                                   : "missing");
        return 1;
    }

    puts("DM1 Atari ST 1.1 authentic multi-member 7z startup media: PASS");
    return 0;
}
