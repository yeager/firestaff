#include "dm1_v1_f0115_f0219_creature_item_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_kind_enum(void) {
    assert(DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34 == 1);
    assert(DM1_V1_F0115_F0219_MATERIAL_CREATURE_PC34 == 2);
}
static void test_provenance_struct(void) {
    DM1_V1_F0115F0219DungeonProvenancePc34 p; memset(&p, 0, sizeof(p));
    assert(p.dungeonDatOwned == 0); assert(p.rawBytesFNV1a == 0);
}
static void test_request_struct(void) {
    DM1_V1_F0115F0219MaterialRequestPc34 r; memset(&r, 0, sizeof(r));
    assert(r.kind == 0); assert(r.creatureType == 0);
}
static void test_receipt_struct(void) {
    DM1_V1_F0115F0219MaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.sourcePixelsFNV1a == 0);
}
static void test_provenance_invalid(void) {
    DM1_V1_F0115F0219DungeonProvenancePc34 p; memset(&p, 0, sizeof(p));
    int v = dm1_v1_f0115_f0219_dungeon_provenance_is_valid_pc34(&p);
    (void)v; assert(v == 0);
}
static void test_receipt_null(void) {
    DM1_V1_F0115F0219MaterialReceiptPc34 r;
    int ok = dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(NULL, NULL, 0, NULL, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_kind_enum(); test_provenance_struct(); test_request_struct();
    test_receipt_struct(); test_provenance_invalid(); test_receipt_null();
    puts("ok: DM1 F0115-F0219 creature item material (Q-DM1-03) 6 tests passed"); return 0;
}
