/*
 * DM1 V1 save-slot core regression gate.
 *
 * ReDMCSB evidence: DM1 V1 itself uses one current save file plus one
 * backup (LOADSAVE.C F0433 lines 803-805, F0435 lines 2437-2441 and
 * 2901-2906). Firestaff's M11 slot wrapper is a native layer around that
 * source-locked header shape, so this test locks its deterministic slot state
 * bookkeeping before a UI multiple-slot flow is wired.
 */

#include "dm1_v1_save_load_system_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
#define FS_RMDIR(path) _rmdir(path)
#define FS_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir(path, 0700)
#define FS_RMDIR(path) rmdir(path)
#define FS_GETPID() getpid()
#endif

static void remove_slot_file(const char* dir, unsigned slot) {
    char path[512];
    snprintf(path, sizeof(path), "%s/save_%02u.dat", dir, slot);
    remove(path);
}

static void cleanup_slot_dir(const char* dir) {
    unsigned i;
    for (i = 0; i < DM1_SL_MAX_SLOTS; i++) {
        remove_slot_file(dir, i);
    }
    FS_RMDIR(dir);
}

static int make_temp_dir(char* dir, size_t dir_size) {
    const char* base = getenv("TMPDIR");
    int rc;
    if (!base || base[0] == 0) base = "/tmp";
    rc = snprintf(dir, dir_size, "%s/firestaff-save-slots-%ld",
                  base, (long)FS_GETPID());
    if (rc <= 0 || (size_t)rc >= dir_size) return 0;
    cleanup_slot_dir(dir);
    return FS_MKDIR(dir) == 0;
}

static M11_SL_SaveHeader make_header(uint16_t level, int16_t x, int16_t y,
                                     uint32_t game_time) {
    M11_SL_SaveHeader header;
    memset(&header, 0, sizeof(header));
    header.game_id = 0x3456789Au;
    header.dungeon_id = 0x1234u;
    header.platform = 9;
    header.format = 5;
    header.current_level = level;
    header.party_x = x;
    header.party_y = y;
    header.party_facing = 2;
    header.game_time = game_time;
    return header;
}

static int expect_int(const char* label, long got, long expected) {
    if (got != expected) {
        printf("FAIL: %s got %ld expected %ld\n", label, got, expected);
        return 0;
    }
    return 1;
}

int main(void) {
    char dir[512];
    M11_SL_State state;
    M11_SL_State scanned;
    M11_SL_State uninformed;
    M11_SL_SaveHeader header;
    M11_SL_SaveHeader loaded_header;
    uint8_t data_a[] = { 1, 2, 3, 4 };
    uint8_t data_b[] = { 9, 8, 7 };
    uint8_t out[4] = { 0, 0, 0, 0 };
    size_t actual = 0;

    if (!make_temp_dir(dir, sizeof(dir))) {
        printf("FAIL: could not create temp save-slot directory\n");
        return 1;
    }

    m11_sl_init(&state, dir);

    header = make_header(3, 10, 11, 1000);
    if (!m11_sl_save(&state, 2, &header, data_a, sizeof(data_a))) {
        printf("FAIL: save slot 2 with data failed\n");
        cleanup_slot_dir(dir);
        return 1;
    }
    if (!expect_int("slot count after first save", state.slot_count, 1) ||
        !m11_sl_slot_occupied(&state, 2)) {
        cleanup_slot_dir(dir);
        return 1;
    }

    header = make_header(5, 12, 13, 2000);
    if (!m11_sl_save(&state, 2, &header, data_b, sizeof(data_b))) {
        printf("FAIL: overwrite slot 2 failed\n");
        cleanup_slot_dir(dir);
        return 1;
    }
    if (!expect_int("slot count after overwrite", state.slot_count, 1)) {
        cleanup_slot_dir(dir);
        return 1;
    }
    if (!m11_sl_load_header(&state, 2, &loaded_header) ||
        !expect_int("overwritten level", loaded_header.current_level, 5) ||
        !expect_int("overwritten data size", loaded_header.data_size, 3)) {
        cleanup_slot_dir(dir);
        return 1;
    }
    if (!m11_sl_load_data(&state, 2, out, sizeof(out), &actual) ||
        !expect_int("exact load size", (long)actual, 3) ||
        memcmp(out, data_b, sizeof(data_b)) != 0) {
        printf("FAIL: exact slot data load mismatch\n");
        cleanup_slot_dir(dir);
        return 1;
    }
    actual = 0;
    if (m11_sl_load_data(&state, 2, out, 2, &actual) ||
        !expect_int("too-small buffer reports required size", (long)actual, 3)) {
        printf("FAIL: too-small slot load should fail without truncation\n");
        cleanup_slot_dir(dir);
        return 1;
    }

    header = make_header(6, 14, 15, 3000);
    if (!m11_sl_save(&state, 5, &header, NULL, 0)) {
        printf("FAIL: header-only slot save failed\n");
        cleanup_slot_dir(dir);
        return 1;
    }
    if (!expect_int("slot count after header-only save", state.slot_count, 2) ||
        !m11_sl_load_header(&state, 5, &loaded_header) ||
        !expect_int("header-only data size", loaded_header.data_size, 0)) {
        cleanup_slot_dir(dir);
        return 1;
    }

    m11_sl_init(&scanned, dir);
    if (!m11_sl_scan_slots(&scanned) ||
        !expect_int("scanned slot count", scanned.slot_count, 2) ||
        !m11_sl_slot_occupied(&scanned, 2) ||
        !m11_sl_slot_occupied(&scanned, 5)) {
        printf("FAIL: scan did not recover occupied slots\n");
        cleanup_slot_dir(dir);
        return 1;
    }

    header = make_header(7, 16, 17, 4000);
    if (!m11_sl_save(&state, 7, &header, data_a, sizeof(data_a))) {
        printf("FAIL: save slot 7 for uninformed delete failed\n");
        cleanup_slot_dir(dir);
        return 1;
    }
    m11_sl_init(&uninformed, dir);
    if (!m11_sl_delete(&uninformed, 7) ||
        !expect_int("uninformed delete slot count", uninformed.slot_count, 0)) {
        cleanup_slot_dir(dir);
        return 1;
    }

    if (!m11_sl_delete(&state, 2) || m11_sl_slot_occupied(&state, 2)) {
        printf("FAIL: delete known occupied slot failed\n");
        cleanup_slot_dir(dir);
        return 1;
    }

    cleanup_slot_dir(dir);
    printf("DM1_V1_SAVE_LOAD_SLOT_CORE_VERIFIED\n");
    return 0;
}
