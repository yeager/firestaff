#include "dm1_v1_f0659_shield_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0659_C037_SHIELD_PC34 == 37);
    assert(DM1_V1_F0659_C038_FIRE_SHIELD_PC34 == 38);
    assert(DM1_V1_F0659_C039_SPELL_SHIELD_PC34 == 39);
    assert(DM1_V1_F0659_M653_PC34 == 695);
}
static void test_receipt_struct(void) {
    DM1_V1_F0659ShieldMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0659_shield_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_fnv1a_data(void) {
    unsigned char d[] = {5,6,7};
    uint32_t h = dm1_v1_f0659_shield_material_fnv1a_pc34(d, 3); (void)h; assert(h != 0u);
}
static void test_receipt_empty(void) {
    DM1_V1_F0659ShieldMaterialReceiptPc34 r;
    int ok = dm1_v1_f0659_shield_material_receipt_pc34(NULL, 0, NULL, 0, &r); (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_receipt_struct(); test_fnv1a_null(); test_fnv1a_data(); test_receipt_empty();
    puts("ok: DM1 F0659 shield material (Q-DM1-07) 5 tests passed"); return 0;
}
