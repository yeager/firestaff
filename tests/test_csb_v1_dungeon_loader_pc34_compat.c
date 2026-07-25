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

int main(void)
{
    test_load_null_path();
    test_load_nonexistent();
    test_load_null_data();
    test_load_too_small();
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

    puts("ok: CSB dungeon loader (Q-CSB-04) 16 tests passed");
    return 0;
}
