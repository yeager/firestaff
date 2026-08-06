#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_load_null_path(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    int rc = csb_v1_dungeon_load_from_file(&d, NULL);
    (void)rc;
    assert(rc != 0);
}

static void test_load_nonexistent(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    int rc = csb_v1_dungeon_load_from_file(&d, "/no/such/file.dat");
    (void)rc;
    assert(rc != 0);
}

static void test_load_null_data(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    int rc = csb_v1_dungeon_load(&d, NULL, 0);
    (void)rc;
    assert(rc != 0);
}

static void test_load_too_small(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    uint8_t buf[8] = {0};
    int rc = csb_v1_dungeon_load(&d, buf, 8);
    (void)rc;
}

static void test_source_bytes_reject_legacy_fixture(void)
{
    /* The historical 16-bit fixture shape starts with a LE level count.
     * It remains accepted only by the compatibility loader, never by a
     * production in-memory original-data handoff. */
    uint8_t bytes[16] = {1u, 0u, 16u, 0u, 1u, 1u, 10u, 0u};
    CSB_V1_DungeonData dungeon;
    memset(&dungeon, 0, sizeof(dungeon));
    assert(csb_v1_dungeon_load_source_bytes(
               &dungeon, bytes, (int)sizeof(bytes)) != 0);
    assert(dungeon.raw_data == NULL && dungeon.level_count == 0);
}

static void test_get_current_default(void)
{
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    (void)d;
}

static void test_get_current_mutable_default(void)
{
    CSB_V1_DungeonData *d = csb_v1_dungeon_get_current_mutable();
    (void)d;
}

static void test_set_current_null(void)
{
    csb_v1_dungeon_set_current(NULL);
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    (void)d;
    assert(d == NULL);
}

static void test_unload_idempotent(void)
{
    csb_v1_dungeon_unload();
    csb_v1_dungeon_unload();
    const CSB_V1_DungeonData *d = csb_v1_dungeon_get_current();
    (void)d;
    assert(d == NULL);
}

static void test_current_level_default(void)
{
    int lv = csb_v1_dungeon_get_current_level();
    (void)lv;
    assert(lv >= 0);
}

static void test_set_current_level(void)
{
    csb_v1_dungeon_set_current_level(3);
    int lv = csb_v1_dungeon_get_current_level();
    (void)lv;
    assert(lv == 3);
    csb_v1_dungeon_set_current_level(0);
}

static void test_decode_square_wall(void)
{
    CSB_V1_DecodedSquare sq;
    memset(&sq, 0xFF, sizeof(sq));
    uint16_t raw = 0x0000;
    csb_v1_dungeon_decode_square(raw, &sq);
    assert(sq.type == 0);
    assert(sq.has_things == 0 || sq.has_things == 1);
}

static void test_decode_square_open(void)
{
    CSB_V1_DecodedSquare sq;
    uint16_t raw = 0x0001;
    csb_v1_dungeon_decode_square(raw, &sq);
    assert(sq.type == 1);
}

static void test_decode_tile_null(void)
{
    CSB_V1_DecodedSquare sq;
    int rc = csb_v1_dungeon_decode_tile(NULL, 0, 0, 0, &sq);
    (void)rc;
    assert(rc == -1);
}

static void test_free_null(void)
{
    csb_v1_dungeon_free(NULL);
}

static void test_get_thing_record_null(void)
{
    const uint8_t *rec = csb_v1_dungeon_get_thing_record(NULL, 0, NULL, NULL, NULL);
    (void)rec;
    assert(rec == NULL);
}

static void test_source_evidence(void)
{
    const char *ev = csb_v1_dungeon_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_real_header_initial_party_pose(void)
{
    uint8_t bytes[90] = {0};
    CSB_V1_DungeonData dungeon;
    int map_index = -1;
    int x = -1;
    int y = -1;
    int direction = -1;

    /* One real-format 10x1 map. InitialPartyLocation = 0x0809 encodes
     * x=9, y=0, direction=2 exactly as ReDMCSB LOADSAVE.C F0435. */
    bytes[4] = 1;
    bytes[8] = 0x09;
    bytes[9] = 0x08;
    bytes[44 + 8] = 0x40; /* width - 1 in MAP.A bits 6..10 */
    bytes[44 + 9] = 0x02;
    bytes[80] = 0x20;
    memset(&dungeon, 0, sizeof(dungeon));
    assert(csb_v1_dungeon_load_source_bytes(
               &dungeon, bytes, (int)sizeof(bytes)) == 0);
    assert(csb_v1_dungeon_initial_party_pose_pc34(
               &dungeon, &map_index, &x, &y, &direction) == 1);
    assert(map_index == 0 && x == 9 && y == 0 && direction == 2);
    csb_v1_dungeon_free(&dungeon);
}

static void test_map_floor_wall_door_nibble_decode(void)
{
    uint8_t bytes[90] = {0};
    CSB_V1_DungeonData dungeon;

    /* MAP.D final little-endian word: Floor=3, Wall=4, Door0=10, Door1=11.
     * ReDMCSB DEFS.H MAP.C and DUNVIEW.C F0094/F0095 consume the first
     * pair before the door selectors. */
    bytes[4] = 1;
    bytes[44 + 8] = 0x40;
    bytes[44 + 9] = 0x02;
    bytes[44 + 14] = 0x43;
    bytes[44 + 15] = 0xBA;
    bytes[80] = 0x20;
    memset(&dungeon, 0, sizeof(dungeon));
    assert(csb_v1_dungeon_load(&dungeon, bytes, (int)sizeof(bytes)) == 0);
    assert(dungeon.map_floor_set[0] == 3);
    assert(dungeon.map_wall_set[0] == 4);
    assert(dungeon.map_door_set0[0] == 10);
    assert(dungeon.map_door_set1[0] == 11);
    csb_v1_dungeon_free(&dungeon);
}

static void test_map_coordinate_origin_uses_source_offsets(void)
{
    uint8_t bytes[90] = {0};
    CSB_V1_DungeonData dungeon;

    /* ReDMCSB DEFS.H MAP: bytes 4/5 are bUnreferenced; OffsetMapX/Y are
     * bytes 6/7. Keep distinct values so a padding-byte substitution cannot
     * pass this source-format regression. */
    bytes[4] = 1;
    bytes[44 + 4] = 0x7e;
    bytes[44 + 5] = 0x7f;
    bytes[44 + 6] = 17;
    bytes[44 + 7] = 14;
    bytes[44 + 8] = 0x40;
    bytes[44 + 9] = 0x02;
    bytes[80] = 0x20;
    memset(&dungeon, 0, sizeof(dungeon));
    assert(csb_v1_dungeon_load(&dungeon, bytes, (int)sizeof(bytes)) == 0);
    assert(dungeon.map_offset_x[0] == 17);
    assert(dungeon.map_offset_y[0] == 14);
    csb_v1_dungeon_free(&dungeon);
}

int main(void)
{
    test_load_null_path();
    test_load_nonexistent();
    test_load_null_data();
    test_load_too_small();
    test_source_bytes_reject_legacy_fixture();
    test_get_current_default();
    test_get_current_mutable_default();
    test_set_current_null();
    test_unload_idempotent();
    test_current_level_default();
    test_set_current_level();
    test_decode_square_wall();
    test_decode_square_open();
    test_decode_tile_null();
    test_free_null();
    test_get_thing_record_null();
    test_source_evidence();
    test_real_header_initial_party_pose();
    test_map_floor_wall_door_nibble_decode();
    test_map_coordinate_origin_uses_source_offsets();

    puts("ok: CSB dungeon loader (Q-CSB-04) 17 tests passed");
    return 0;
}
