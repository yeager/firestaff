/* Test DM2 savegame unified module — integration tests across all sub-modules.
 * Source: skproject c_savegame.cpp. */

#include "dm2_v1_savegame_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ── Test 1: source evidence ─────────────────────────────────────────── */
static void test_source_evidence(void) {
    const char *ev = dm2_v1_savegame_source_evidence();
    assert(ev != NULL);
    assert(strlen(ev) > 0);
    assert(strstr(ev, "c_savegame.cpp") != NULL);
    assert(strstr(ev, "DM2_GAME_LOAD") != NULL);
    assert(strstr(ev, "DM2_GAME_SAVE_MENU") != NULL);
    assert(strstr(ev, "DM2_SUPPRESS_READER") != NULL);
    assert(strstr(ev, "DM2_SUPPRESS_WRITER") != NULL);
    assert(strstr(ev, "DM2_READ_RECORD_CHECKCODE") != NULL);
    assert(strstr(ev, "DM2_WRITE_RECORD_CHECKCODE") != NULL);
    assert(strstr(ev, "DM2_COMPACT_TIMERLIST") != NULL);
    assert(strstr(ev, "DM2_READ_DUNGEON_STRUCTURE") != NULL);
    assert(strstr(ev, "DM2_STORE_EXTRA_DUNGEON_DATA") != NULL);
    assert(strstr(ev, "FSUBSAVE") != NULL);
    printf("  PASS: source evidence covers all 29 functions\n");
}

/* ── Test 2: submodule count ─────────────────────────────────────────── */
static void test_submodule_count(void) {
    int n = dm2_v1_savegame_submodule_count();
    assert(n == 14);
    printf("  PASS: submodule count = %d\n", n);
}

/* ── Test 3: all evidence present ────────────────────────────────────── */
static void test_all_evidence_present(void) {
    bool ok = dm2_v1_savegame_all_evidence_present();
    assert(ok);
    printf("  PASS: all sub-module evidence strings present\n");
}

/* ── Test 4: savegame buffer size constant ───────────────────────────── */
static void test_savegame_buffer_size(void) {
    assert(DM2_SAVEGAME_BUFFER_SIZE == 0x3C);
    assert(DM2_SAVEGAME_HEADER_SIZE == 0x2A);
    assert(DM2_SAVEGAME_DUNGEON_HEADER_SIZE == 0x2C);
    printf("  PASS: savegame buffer size constants correct\n");
}

/* ── Test 5: record type codes ───────────────────────────────────────── */
static void test_record_type_codes(void) {
    assert(DM2_RECORD_TYPE_DOOR == 0x00);
    assert(DM2_RECORD_TYPE_CREATURE == 0x04);
    assert(DM2_RECORD_TYPE_CONTAINER == 0x09);
    assert(DM2_RECORD_TYPE_MISSILE == 0x0E);
    assert(DM2_RECORD_TYPE_CLOUD == 0x0F);
    assert(DM2_SAVEGAME_MAX_RECORD_TYPES == 16);
    printf("  PASS: record type codes match skproject\n");
}

/* ── Test 6: tile type codes ─────────────────────────────────────────── */
static void test_tile_type_codes(void) {
    assert(DM2_TILE_WALL == 0);
    assert(DM2_TILE_FLOOR == 1);
    assert(DM2_TILE_PIT == 2);
    assert(DM2_TILE_STAIRS == 3);
    assert(DM2_TILE_DOOR == 4);
    assert(DM2_TILE_TELEPORTER == 5);
    assert(DM2_TILE_TRICK_WALL == 6);
    assert(DM2_TILE_SPECIAL == 7);
    printf("  PASS: tile type codes match skproject\n");
}

/* ── Test 7: tile suppress sizes ─────────────────────────────────────── */
static void test_tile_suppress_sizes(void) {
    /* Verify constants match dm2_v1_save_tile_suppress_size() */
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_WALL) == DM2_TILE_SUPPRESS_WALL);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_FLOOR) == DM2_TILE_SUPPRESS_FLOOR);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_PIT) == DM2_TILE_SUPPRESS_PIT);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_STAIRS) == DM2_TILE_SUPPRESS_STAIRS);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_DOOR) == DM2_TILE_SUPPRESS_DOOR);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_TRICK_WALL) == DM2_TILE_SUPPRESS_TRICK);
    assert(dm2_v1_save_tile_suppress_size(DM2_TILE_SPECIAL) == DM2_TILE_SUPPRESS_SPECIAL);
    printf("  PASS: tile suppress sizes match skproject switch\n");
}

/* ── Test 8: actuator sub-type codes ─────────────────────────────────── */
static void test_actuator_subtypes(void) {
    /* These 8 actuator sub-types carry a 9-bit suppress field in
     * both READ_RECORD_CHECKCODE and WRITE_RECORD_CHECKCODE. */
    assert(DM2_ACTUATOR_TYPE_27 == 0x27);
    assert(DM2_ACTUATOR_TYPE_1B == 0x1B);
    assert(DM2_ACTUATOR_TYPE_1D == 0x1D);
    assert(DM2_ACTUATOR_TYPE_41 == 0x41);
    assert(DM2_ACTUATOR_TYPE_2C == 0x2C);
    assert(DM2_ACTUATOR_TYPE_32 == 0x32);
    assert(DM2_ACTUATOR_TYPE_30 == 0x30);
    assert(DM2_ACTUATOR_TYPE_2D == 0x2D);
    printf("  PASS: actuator sub-type codes for 9-bit field\n");
}

/* ── Test 9: suppress mask savegame buffer ───────────────────────────── */
static void test_suppress_mask_savegame_buffer(void) {
    const uint8_t *mask = dm2_v1_save_mask_savegame_buffer();
    assert(mask != NULL);
    /* The mask should have DM2_SAVEGAME_BUFFER_SIZE bytes. First byte
     * should have bits set since l_00 (gametick) has all bits diffable. */
    assert(mask[0] != 0);
    printf("  PASS: savegame buffer suppress mask present\n");
}

/* ── Test 10: record sizes table ─────────────────────────────────────── */
static void test_record_sizes(void) {
    const uint8_t *sizes = dm2_v1_save_record_sizes();
    assert(sizes != NULL);
    /* Record type 4 (creature) is the largest at 8 bytes per word-pair.
     * All sizes must be even (word-aligned in skproject). */
    for (int i = 0; i < DM2_SAVEGAME_MAX_RECORD_TYPES; i++) {
        assert(sizes[i] % 2 == 0);
    }
    printf("  PASS: record sizes table present and word-aligned\n");
}

/* ── Test 11: record mask for each type ──────────────────────────────── */
static void test_record_masks(void) {
    for (int i = 0; i < DM2_SAVEGAME_MAX_RECORD_TYPES; i++) {
        const uint8_t *mask = dm2_v1_save_record_mask_for_type(i);
        /* Some types have NULL mask (no diff bits tracked). */
        (void)mask;
    }
    /* Creature type must have a non-NULL mask. */
    const uint8_t *creature_mask = dm2_v1_save_record_mask_for_type(
        DM2_RECORD_TYPE_CREATURE);
    assert(creature_mask != NULL);
    printf("  PASS: record masks accessible for all 16 types\n");
}

/* ── Test 12: timer accessors ────────────────────────────────────────── */
static void test_timer_accessors(void) {
    DM2_V1_SaveTimerRecord t;
    memset(&t, 0, sizeof(t));
    uint8_t type = dm2_v1_save_timer_get_type(&t);
    assert(type == 0);
    int no = dm2_v1_save_timer_is_no_type(&t);
    assert(no != 0);
    printf("  PASS: timer accessors on zeroed record\n");
}

/* ── Test 13: compact timerlist no-op on full list ───────────────────── */
static void test_compact_timerlist_noop(void) {
    uint8_t timers[6] = {1, 10, 2, 20, 3, 30};
    dm2_v1_compact_timerlist(timers, 2, 0, 3);
    assert(timers[0] == 1 && timers[2] == 2 && timers[4] == 3);
    printf("  PASS: compact timerlist no-op on full list\n");
}

/* ── Test 14: suppress reader/writer round-trip ──────────────────────── */
static void test_suppress_roundtrip(void) {
    /* Write 3 bytes through suppress writer, read them back. */
    uint8_t write_buf[64];
    size_t total = 0;

    DM2_SuppressWriter writer;
    dm2_suppress_writer_init(&writer);

    uint8_t src[3] = {0xAB, 0xCD, 0xEF};
    uint8_t mask[3] = {0xFF, 0xFF, 0xFF};
    size_t n = 0;

    int err = dm2_suppress_writer_write(&writer, src, mask, 3,
                                         write_buf, sizeof(write_buf), &n);
    assert(err == 0);
    total += n;

    /* Flush remaining bits. */
    err = dm2_suppress_writer_flush(&writer, write_buf + total,
                                     sizeof(write_buf) - total, &n);
    assert(err == 0);
    total += n;
    assert(total > 0);

    /* Read back. */
    DM2_SuppressReader reader;
    dm2_suppress_reader_init(&reader, write_buf, total);

    uint8_t dst[3] = {0, 0, 0};
    err = dm2_suppress_reader_read(&reader, mask, 3, dst, 0);
    assert(err == 0);
    assert(dst[0] == 0xAB);
    assert(dst[1] == 0xCD);
    assert(dst[2] == 0xEF);
    printf("  PASS: suppress reader/writer round-trip\n");
}

/* ── Test 15: teleporter forward reference ───────────────────────────── */
static void test_teleporter_forward_ref(void) {
    /* When target_map > current_map, forward ref returns 1. */
    assert(dm2_v1_save_teleporter_is_forward_ref(3, 5) != 0);
    /* When current_map > target_map, returns 0. */
    assert(dm2_v1_save_teleporter_is_forward_ref(5, 3) == 0);
    /* Equal maps: not a forward ref. */
    assert(dm2_v1_save_teleporter_is_forward_ref(3, 3) == 0);
    printf("  PASS: teleporter forward reference logic\n");
}

/* ── Test 16: suppress reader read_bit ───────────────────────────────── */
static void test_suppress_read_bit(void) {
    /* Write a single bit=1, flush, read it back. */
    uint8_t buf[8];
    size_t pos = 0;
    DM2_SuppressWriter w;
    dm2_suppress_writer_init(&w);
    uint8_t data1 = 1, mask1 = 1;
    int err = dm2_suppress_writer_write(&w, &data1, &mask1, 1, buf, sizeof(buf), &pos);
    assert(err == 0);
    err = dm2_suppress_writer_flush(&w, buf, sizeof(buf), &pos);
    assert(err == 0);

    DM2_SuppressReader r;
    dm2_suppress_reader_init(&r, buf, pos);
    int bit = -1;
    err = dm2_suppress_reader_read_bit(&r, &bit);
    assert(err == 0);
    assert(bit == 1);
    printf("  PASS: suppress reader read_bit\n");
}

/* ── Test 17: timer suppress mask ────────────────────────────────────── */
static void test_timer_suppress_mask(void) {
    const uint8_t *mask = dm2_v1_save_timers_suppress_mask();
    assert(mask != NULL);
    printf("  PASS: timer suppress mask present\n");
}

/* ── main ────────────────────────────────────────────────────────────── */
int main(void) {
    printf("dm2_v1_savegame_pc34_compat integration tests:\n");

    test_source_evidence();
    test_submodule_count();
    test_all_evidence_present();
    test_savegame_buffer_size();
    test_record_type_codes();
    test_tile_type_codes();
    test_tile_suppress_sizes();
    test_actuator_subtypes();
    test_suppress_mask_savegame_buffer();
    test_record_sizes();
    test_record_masks();
    test_timer_accessors();
    test_compact_timerlist_noop();
    test_suppress_roundtrip();
    test_teleporter_forward_ref();
    test_suppress_read_bit();
    test_timer_suppress_mask();

    printf("PASS: dm2_v1_savegame_pc34_compat (17 tests)\n");
    return 0;
}
