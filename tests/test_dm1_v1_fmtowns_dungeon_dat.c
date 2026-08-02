#include "dm1_v1_fmtowns_dungeon_dat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_probe_null_rejects(void) {
    assert(dm1_v1_fmtowns_dungeon_probe(NULL, 0) == 0);
}

static void test_probe_wrong_size_rejects(void) {
    uint8_t buf[100];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x63; /* map count = 99 */
    assert(dm1_v1_fmtowns_dungeon_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_wrong_map_count_rejects(void) {
    uint8_t buf[33423];
    memset(buf, 0, sizeof(buf));
    buf[0] = 100; /* wrong map count */
    assert(dm1_v1_fmtowns_dungeon_probe(buf, sizeof(buf)) == 0);
}

static void test_probe_en_size_accepts(void) {
    uint8_t buf[33423];
    memset(buf, 0, sizeof(buf));
    buf[0] = 99; /* map count = 99 */
    assert(dm1_v1_fmtowns_dungeon_probe(buf, sizeof(buf)) == 1);
}

static void test_probe_jp_size_accepts(void) {
    uint8_t buf[33931];
    memset(buf, 0, sizeof(buf));
    buf[0] = 99;
    assert(dm1_v1_fmtowns_dungeon_probe(buf, sizeof(buf)) == 1);
}

static void test_receipt_null_rejects(void) {
    assert(dm1_v1_fmtowns_dungeon_receipt(NULL, 0, NULL) == -1);
}

static void test_receipt_en_size(void) {
    uint8_t buf[33423];
    memset(buf, 0, sizeof(buf));
    buf[0] = 99;
    DM1_V1_FmtownsDungeonReceipt r;
    int rc = dm1_v1_fmtowns_dungeon_receipt(buf, sizeof(buf), &r);
    assert(rc == 0);
    assert(r.is_fmtowns == 1);
    assert(r.map_count == 99);
    assert(r.file_size == 33423);
    assert(r.lang == 0); /* synthetic data won't match known MD5 */
}

int main(void) {
    test_probe_null_rejects();
    test_probe_wrong_size_rejects();
    test_probe_wrong_map_count_rejects();
    test_probe_en_size_accepts();
    test_probe_jp_size_accepts();
    test_receipt_null_rejects();
    test_receipt_en_size();
    printf("All dm1_v1_fmtowns_dungeon_dat tests passed.\n");
    return 0;
}
