#include "custom_dungeon_m12.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static int mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static int write_dungeon(const char* path, unsigned char mapCount, int compressed, size_t bytes) {
    FILE* fp;
    unsigned char data[96];
    if (bytes > sizeof(data)) return 0;
    memset(data, 0, sizeof(data));
    if (compressed) {
        data[0] = 0x04;
        data[1] = 0x81;
    } else {
        data[0] = 0x34;
        data[1] = 0x12;
    }
    data[4] = mapCount;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(data, 1, bytes, fp) != bytes) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static void cleanup_fixture(const char* root) {
    char path[512];
    snprintf(path, sizeof(path), "%s/custom/zeta_too_small/DUNGEON.DAT", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/custom/beta_valid/dUnGeOn.DaT", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/custom/alpha_compressed/DUNGEON.DAT", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/custom/zeta_too_small", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/custom/beta_valid", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/custom/alpha_compressed", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/custom", root);
    rmdir(path);
    rmdir(root);
}

int main(void) {
    char root[256];
    char path[512];
    M12_CustomDungeonState state;
    const M12_CustomDungeonEntry* selected;

    snprintf(root, sizeof(root), "/tmp/firestaff_custom_dungeon_m12_%ld", (long)getpid());
    cleanup_fixture(root);

    check(mkdir_one(root), "created fixture root");
    snprintf(path, sizeof(path), "%s/custom", root);
    check(mkdir_one(path), "created custom root");
    snprintf(path, sizeof(path), "%s/custom/beta_valid", root);
    check(mkdir_one(path), "created valid dungeon dir");
    snprintf(path, sizeof(path), "%s/custom/alpha_compressed", root);
    check(mkdir_one(path), "created compressed dungeon dir");
    snprintf(path, sizeof(path), "%s/custom/zeta_too_small", root);
    check(mkdir_one(path), "created too-small dungeon dir");

    snprintf(path, sizeof(path), "%s/custom/beta_valid/dUnGeOn.DaT", root);
    check(write_dungeon(path, 1u, 0, 60u), "wrote mixed-case valid DUNGEON.DAT");
    snprintf(path, sizeof(path), "%s/custom/alpha_compressed/DUNGEON.DAT", root);
    check(write_dungeon(path, 1u, 1, 60u), "wrote compressed-signature DUNGEON.DAT");
    snprintf(path, sizeof(path), "%s/custom/zeta_too_small/DUNGEON.DAT", root);
    check(write_dungeon(path, 1u, 0, 44u), "wrote too-small DUNGEON.DAT");

    M12_CustomDungeon_Init(&state);
    check(M12_CustomDungeon_Scan(&state, root) == 3, "scan finds three real path entries");
    check(state.scanned == 1, "scan flag set");
    check(strcmp(state.entries[0].name, "alpha_compressed") == 0,
          "entries sorted alphabetically");
    check(state.entries[0].status == CUSTOM_DUNGEON_STATUS_COMPRESSED,
          "compressed save signature rejected");
    check(strcmp(state.entries[1].name, "beta_valid") == 0,
          "mixed-case DUNGEON.DAT entry sorted into middle");
    check(state.entries[1].status == CUSTOM_DUNGEON_STATUS_VALID,
          "mixed-case real path validates");
    check(state.entries[1].mapCount == 1, "map count parsed from valid entry");
    check(strstr(state.entries[1].path, "dUnGeOn.DaT") != NULL,
          "matched mixed-case path preserved");
    check(state.entries[2].status == CUSTOM_DUNGEON_STATUS_TOO_SMALL,
          "too-small dungeon rejected");

    check(M12_CustomDungeon_Select(&state, 0) == 0,
          "invalid compressed entry cannot be selected");
    check(M12_CustomDungeon_Select(&state, 1) == 1,
          "valid entry can be selected");
    selected = M12_CustomDungeon_GetSelected(&state);
    check(selected != NULL && strcmp(selected->name, "beta_valid") == 0,
          "selected valid entry returned");
    check(M12_CustomDungeon_StatusLabel(CUSTOM_DUNGEON_STATUS_VALID) != NULL,
          "status labels remain available");

    cleanup_fixture(root);

    if (failures) {
        printf("test_custom_dungeon_m12_real_path: FAIL %d\n", failures);
        return 1;
    }
    puts("test_custom_dungeon_m12_real_path: PASS");
    return 0;
}
