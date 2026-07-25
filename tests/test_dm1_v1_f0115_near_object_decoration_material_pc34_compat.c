#include "dm1_v1_f0115_near_object_decoration_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_kind_enum(void) {
    assert(DM1_V1_F0115_NEAR_FLOOR_PC34 == 1);
    assert(DM1_V1_F0115_NEAR_CEILING_PC34 == 2);
    assert(DM1_V1_F0115_NEAR_FLOOR_ORNAMENT_PC34 == 3);
    assert(DM1_V1_F0115_NEAR_NORMAL_OBJECT_PC34 == 4);
}
static void test_provenance_struct(void) {
    DM1_V1_F0115NearDungeonProvenancePc34 p; memset(&p, 0, sizeof(p));
    assert(p.dungeonDatOwned == 0); assert(p.rawBytesFNV1a == 0);
}
static void test_receipt_struct(void) {
    DM1_V1_F0115NearMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.sourcePixelsFNV1a == 0);
}
static void test_provenance_invalid(void) {
    DM1_V1_F0115NearDungeonProvenancePc34 p; memset(&p, 0, sizeof(p));
    int v = dm1_v1_f0115_near_dungeon_provenance_is_valid_pc34(&p);
    (void)v; assert(v == 0);
}
static void test_receipt_null(void) {
    DM1_V1_F0115NearMaterialReceiptPc34 r;
    int ok = dm1_v1_f0115_near_object_decoration_material_receipt_pc34(NULL, NULL, 0, NULL, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_kind_enum(); test_provenance_struct(); test_receipt_struct();
    test_provenance_invalid(); test_receipt_null();
    puts("ok: DM1 F0115 near object decoration material (Q-DM1-03) 5 tests passed"); return 0;
}
