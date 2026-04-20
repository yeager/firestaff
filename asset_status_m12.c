#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int m12_file_exists(const char* path) {
    struct stat st;
    if (!path || path[0] == '\0') {
        return 0;
    }
    return stat(path, &st) == 0;
}

static int m12_join_path(char* out,
                         size_t outSize,
                         const char* dir,
                         const char* leaf) {
    int rc;
    if (!out || outSize == 0U || !dir || !leaf) {
        return 0;
    }
    rc = snprintf(out,
                  outSize,
                  "%s%s%s",
                  dir,
                  (dir[0] != '\0' && dir[strlen(dir) - 1] == '/') ? "" : "/",
                  leaf);
    return rc > 0 && (size_t)rc < outSize;
}

static int m12_any_file_exists(const char* dir, const char* const* names) {
    char path[M12_ASSET_DATA_DIR_CAPACITY + 64];
    size_t i;
    if (!dir || !names) {
        return 0;
    }
    for (i = 0; names[i] != NULL; ++i) {
        if (m12_join_path(path, sizeof(path), dir, names[i]) &&
            m12_file_exists(path)) {
            return 1;
        }
    }
    return 0;
}

static int m12_all_groups_exist(const char* dir,
                                const char* const* const* groups,
                                size_t groupCount) {
    size_t i;
    for (i = 0; i < groupCount; ++i) {
        if (!m12_any_file_exists(dir, groups[i])) {
            return 0;
        }
    }
    return 1;
}

static void m12_copy_data_dir(char* out,
                              size_t outSize,
                              const char* requestedDataDir) {
    const char* envDataDir = getenv("FIRESTAFF_DATA");
    const char* resolved = requestedDataDir;
    if (!out || outSize == 0U) {
        return;
    }
    if (!resolved || resolved[0] == '\0') {
        resolved = envDataDir;
    }
    if (!resolved || resolved[0] == '\0') {
        resolved = ".";
    }
    snprintf(out, outSize, "%s", resolved);
}

void M12_AssetStatus_Scan(M12_AssetStatus* status, const char* requestedDataDir) {
    static const char* const dm1Graphics[] = {"GRAPHICS.DAT", NULL};
    static const char* const dm1Dungeon[] = {"DUNGEON.DAT", NULL};
    static const char* const csbGraphics[] = {"CSBGRAPH.DAT", "CSBGRAPH.DAT", NULL};
    static const char* const csbDungeon[] = {"CSB.DAT", "CSB_DUNGEON.DAT", NULL};
    static const char* const dm2Graphics[] = {"DM2GRAPHICS.DAT", "SKULLKEEP.GFX", NULL};
    static const char* const dm2Dungeon[] = {"DM2DUNGEON.DAT", "SKULLKEEP.DAT", NULL};
    static const char* const* const dm1Groups[] = {dm1Graphics, dm1Dungeon};
    static const char* const* const csbGroups[] = {csbGraphics, csbDungeon};
    static const char* const* const dm2Groups[] = {dm2Graphics, dm2Dungeon};

    if (!status) {
        return;
    }

    memset(status, 0, sizeof(*status));
    m12_copy_data_dir(status->dataDir, sizeof(status->dataDir), requestedDataDir);

    status->dm1Available = m12_all_groups_exist(status->dataDir,
                                                dm1Groups,
                                                sizeof(dm1Groups) / sizeof(dm1Groups[0]));
    status->csbAvailable = m12_all_groups_exist(status->dataDir,
                                                csbGroups,
                                                sizeof(csbGroups) / sizeof(csbGroups[0]));
    status->dm2Available = m12_all_groups_exist(status->dataDir,
                                                dm2Groups,
                                                sizeof(dm2Groups) / sizeof(dm2Groups[0]));
}

int M12_AssetStatus_GameAvailable(const M12_AssetStatus* status,
                                  const char* gameId) {
    if (!status || !gameId) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        return status->dm1Available;
    }
    if (strcmp(gameId, "csb") == 0) {
        return status->csbAvailable;
    }
    if (strcmp(gameId, "dm2") == 0) {
        return status->dm2Available;
    }
    return 0;
}

const char* M12_AssetStatus_GetDataDir(const M12_AssetStatus* status) {
    if (!status || status->dataDir[0] == '\0') {
        return ".";
    }
    return status->dataDir;
}
