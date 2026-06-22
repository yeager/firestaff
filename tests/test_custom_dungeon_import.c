#include "custom_dungeon_m12.h"
#include "dm1_v1_custom_dungeon_loader.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
static int fs_pid(void) { return _getpid(); }
#else
#include <unistd.h>
#define FS_MKDIR(path) mkdir((path), 0755)
static int fs_pid(void) { return (int)getpid(); }
#endif

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static int make_dir(const char* path) {
    if (!path || path[0] == '\0') {
        return 0;
    }
    if (FS_MKDIR(path) == 0) {
        return 1;
    }
    return 1;
}

static int write_bytes(const char* path, const unsigned char* bytes, size_t count) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (count > 0U && fwrite(bytes, 1, count, fp) != count) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_dungeon(const char* path, int map_count, int compressed, size_t total_size) {
    unsigned char bytes[96];
    size_t i;
    if (total_size > sizeof(bytes)) {
        total_size = sizeof(bytes);
    }
    for (i = 0U; i < sizeof(bytes); ++i) {
        bytes[i] = 0;
    }
    if (compressed) {
        bytes[0] = (unsigned char)(DUNGEON_COMPRESSED_SIGNATURE & 0xFFu);
        bytes[1] = (unsigned char)((DUNGEON_COMPRESSED_SIGNATURE >> 8) & 0xFFu);
    } else {
        bytes[0] = 0x34;
        bytes[1] = 0x12;
    }
    bytes[4] = (unsigned char)map_count;
    return write_bytes(path, bytes, total_size);
}

static void join_path(char* out, size_t out_size, const char* a, const char* b) {
    snprintf(out, out_size, "%s/%s", a ? a : "", b ? b : "");
}

static void test_launcher_scan(void) {
    char base[512];
    char custom[512];
    char alpha[512];
    char beta[512];
    char tiny[512];
    char compressed[512];
    char path[512];
    M12_CustomDungeonState state;
    const M12_CustomDungeonEntry* selected;

    snprintf(base, sizeof(base), "/tmp/firestaff-custom-import-m12-%d", fs_pid());
    join_path(custom, sizeof(custom), base, "custom");
    join_path(alpha, sizeof(alpha), custom, "Alpha");
    join_path(beta, sizeof(beta), custom, "Beta");
    join_path(tiny, sizeof(tiny), custom, "Tiny");
    join_path(compressed, sizeof(compressed), custom, "Compressed");

    CHECK(make_dir(base));
    CHECK(make_dir(custom));
    CHECK(make_dir(alpha));
    CHECK(make_dir(beta));
    CHECK(make_dir(tiny));
    CHECK(make_dir(compressed));

    join_path(path, sizeof(path), alpha, "dungeon.dat");
    CHECK(write_dungeon(path, 2, 0, DUNGEON_HEADER_SIZE + 2U * DUNGEON_MAP_DESC_SIZE));
    join_path(path, sizeof(path), beta, "DUNGEON.DAT");
    CHECK(write_dungeon(path, 1, 0, DUNGEON_HEADER_SIZE + DUNGEON_MAP_DESC_SIZE));
    join_path(path, sizeof(path), tiny, "DUNGEON.DAT");
    CHECK(write_dungeon(path, 1, 0, 16U));
    join_path(path, sizeof(path), compressed, "DUNGEON.DAT");
    CHECK(write_dungeon(path, 1, 1, DUNGEON_HEADER_SIZE + DUNGEON_MAP_DESC_SIZE));

    M12_CustomDungeon_Init(&state);
    CHECK(M12_CustomDungeon_Scan(&state, base) == 4);
    CHECK(state.scanned == 1);
    CHECK(state.entryCount == 4);
    CHECK(strcmp(state.entries[0].name, "Alpha") == 0);
    CHECK(state.entries[0].status == CUSTOM_DUNGEON_STATUS_VALID);
    CHECK(state.entries[0].mapCount == 2);
    CHECK(strcmp(state.entries[1].name, "Beta") == 0);
    CHECK(state.entries[1].status == CUSTOM_DUNGEON_STATUS_VALID);
    CHECK(strcmp(state.entries[2].name, "Compressed") == 0);
    CHECK(state.entries[2].status == CUSTOM_DUNGEON_STATUS_COMPRESSED);
    CHECK(strcmp(state.entries[3].name, "Tiny") == 0);
    CHECK(state.entries[3].status == CUSTOM_DUNGEON_STATUS_TOO_SMALL);
    CHECK(M12_CustomDungeon_Select(&state, 0) == 1);
    CHECK(M12_CustomDungeon_Select(&state, 2) == 0);
    selected = M12_CustomDungeon_GetSelected(&state);
    CHECK(selected != NULL && strcmp(selected->name, "Alpha") == 0);
    CHECK(strcmp(M12_CustomDungeon_StatusLabel(CUSTOM_DUNGEON_STATUS_COMPRESSED),
                 "Compressed (unsupported)") == 0);
}

static void test_engine_scan(void) {
    char base[512];
    char one[512];
    char bad[512];
    char path[512];
    M11_CustomDungeonList list;
    int map_count = 0;
    const M11_CustomDungeon* selected;

    snprintf(base, sizeof(base), "/tmp/firestaff-custom-import-m11-%d", fs_pid());
    join_path(one, sizeof(one), base, "One");
    join_path(bad, sizeof(bad), base, "Bad");

    CHECK(make_dir(base));
    CHECK(make_dir(one));
    CHECK(make_dir(bad));

    join_path(path, sizeof(path), one, "Dungeon.dat");
    CHECK(write_dungeon(path, 3, 0, DUNGEON_HEADER_SIZE + 3U * DUNGEON_MAP_DESC_SIZE));
    join_path(path, sizeof(path), one, "GRAPHICS.DAT");
    CHECK(write_bytes(path, (const unsigned char*)"gfx", 3U));
    join_path(path, sizeof(path), bad, "dungeon.dat");
    CHECK(write_dungeon(path, 0, 0, DUNGEON_HEADER_SIZE + DUNGEON_MAP_DESC_SIZE));

    M11_CustomDungeon_Init(&list);
    CHECK(M11_CustomDungeon_Scan(&list, base) == 2);
    CHECK(list.count == 2);
    CHECK(M11_CustomDungeon_Validate(list.entries[0].dungeonDatPath, &map_count) ==
          list.entries[0].valid);
    CHECK((list.entries[0].valid && list.entries[0].mapCount == 3) ||
          (list.entries[1].valid && list.entries[1].mapCount == 3));
    CHECK((list.entries[0].graphicsDatPath[0] != '\0') ||
          (list.entries[1].graphicsDatPath[0] != '\0'));
    list.selectedIndex = list.entries[0].valid ? 0 : 1;
    selected = M11_CustomDungeon_GetSelected(&list);
    CHECK(selected != NULL && selected->valid == 1);
    CHECK(M11_CustomDungeon_Validate(NULL, &map_count) == 0);
    CHECK(map_count == 0);
}

int main(void) {
    test_launcher_scan();
    test_engine_scan();
    if (failures) {
        fprintf(stderr, "%d custom dungeon import failure(s)\n", failures);
        return 1;
    }
    puts("custom_dungeon_import: ok");
    return 0;
}
