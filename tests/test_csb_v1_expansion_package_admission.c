#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
static int fs_pid(void) { return _getpid(); }
#else
#include <sys/stat.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir((path), 0700)
static int fs_pid(void) { return (int)getpid(); }
#endif

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static int write_file(const char *path)
{
    static const unsigned char bytes[] = { 1, 0, 3, 3, 0, 0, 0, 0 };
    FILE *file = fopen(path, "wb");
    if (!file) {
        return 0;
    }
    if (fwrite(bytes, 1, sizeof(bytes), file) != sizeof(bytes)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int main(void)
{
    char root[256];
    char active_path[320];
    char dm1_named_bonus_path[320];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData active_dungeon;

    snprintf(root, sizeof(root), "/tmp/firestaff-csb-expansion-admission-%d",
             fs_pid());
    snprintf(active_path, sizeof(active_path), "%s/DUNGEON.DAT", root);
    snprintf(dm1_named_bonus_path, sizeof(dm1_named_bonus_path),
             "%s/DUNGEONB.DAT", root);
    check(FS_MKDIR(root) == 0, "create expansion fixture directory");
    check(write_file(active_path), "write selected CSB package placeholder");
    check(write_file(dm1_named_bonus_path), "write neighboring DUNGEONB placeholder");

    check(!csb_v1_runtime_bonus_dungeon_candidate_admitted(
              dm1_named_bonus_path),
          "filename-only DUNGEONB candidate is not an admitted CSB package");
    check(!csb_v1_runtime_bonus_dungeon_candidate_admitted(NULL),
          "missing candidate fails closed");

    memset(&profile, 0, sizeof(profile));
    memset(&active_dungeon, 0, sizeof(active_dungeon));
    profile.load_bonus_dungeon = 1;
    profile.dungeon_path = active_path;
    profile.dungeon_asset.path = active_path;
    profile.dungeon_handle = &active_dungeon;
    check(!csb_v1_runtime_bonus_dungeon_active_owner_admitted(&profile),
          "unverified active dungeon cannot own a bonus-package handoff");
    profile.dungeon_asset.path = dm1_named_bonus_path;
    check(!csb_v1_runtime_bonus_dungeon_active_owner_admitted(&profile),
          "stale active asset path cannot own a bonus-package handoff");
    profile.dungeon_asset.path = active_path;
    check(csb_v1_runtime_try_load_bonus_dungeon(&profile) == 0,
          "unverified package owner cannot load a neighboring CSB dungeon");
    check(profile.dungeon_handle == &active_dungeon &&
              profile.dungeon_path == active_path &&
              profile.bonus_dungeon_path[0] == '\0',
          "rejected package leaves the active CSB package and save identity intact");

    remove(dm1_named_bonus_path);
    remove(active_path);
    rmdir(root);
    printf("csb_v1_expansion_package_admission: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
