#include "asset_status_m12.h"
#include "asset_find_by_hash.h"

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
    const char *archive = getenv("FIRESTAFF_DM1_ATARI_ST_OUTER_ARCHIVE");
    M12_AssetStatus status;
    const M12_AssetVersionStatus *version = NULL;
    char graphics_path[1536];
    char dungeon_path[1536];
    uint8_t *graphics = NULL;
    uint8_t *dungeon = NULL;
    size_t graphics_size = 0U;
    size_t dungeon_size = 0U;
    size_t i;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Atari ST preservation archive is not staged");
        return 77;
    }
    snprintf(graphics_path, sizeof(graphics_path),
             "%s::Dungeon Master (1987)(FTL)[!].zip::Dungeon Master (1987)(FTL)[!].stx::GRAPHICS.DAT",
             archive);
    snprintf(dungeon_path, sizeof(dungeon_path),
             "%s::Dungeon Master (1987)(FTL)[!].zip::Dungeon Master (1987)(FTL)[!].stx::DUNGEON.DAT",
             archive);
    expect(asset_read_virtual_path_alloc(graphics_path, &graphics, &graphics_size) &&
               graphics_size > 0U,
           "the nested STX reader opens GRAPHICS.DAT in bounded memory");
    expect(asset_read_virtual_path_alloc(dungeon_path, &dungeon, &dungeon_size) &&
               dungeon_size > 0U,
           "the nested STX reader opens DUNGEON.DAT in bounded memory");
    free(graphics);
    free(dungeon);
    memset(&status, 0, sizeof(status));
    M12_AssetStatus_ScanGame(&status, archive, "dm1");
    for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
        const M12_AssetVersionStatus *candidate =
            M12_AssetStatus_GetVersion(&status, "dm1", i);
        if (candidate && candidate->matched &&
            (strcmp(candidate->versionId, "st10a-en") == 0 ||
             strcmp(candidate->versionId, "st12-en") == 0)) {
            version = candidate;
            break;
        }
    }
    expect(version != NULL,
           "the authentic ZIP -> ZIP -> STX original is admitted as Atari ST");
    expect(version && strstr(version->matchedPath,
                             "Dungeon Master (1987)(FTL)[!].stx::GRAPHICS.DAT"),
           "the Atari graphics receipt remains a virtual source path");
    expect(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
           "the authentic Atari preservation archive satisfies DM1 launch requirements");
    expect(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"), archive) == 0,
           "the runtime owner remains the supplied archive without extraction");
    for (i = 0U; i < M12_AssetStatus_GetRequiredFileCount(&status, "dm1"); ++i) {
        const M12_AssetRequiredFileStatus *required =
            M12_AssetStatus_GetRequiredFile(&status, "dm1", i);
        expect(required && required->matched,
               "both required Atari ST files are read from the original STX");
    }
    if (failures != 0) return 1;
    puts("PASS: authentic outer DM1 Atari archive is admitted without extraction");
    return 0;
}
