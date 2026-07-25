#include "dm1_v1_f0661_damage_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0661_C014_DAMAGE_TO_CREATURE_PC34 == 14);
    assert(DM1_V1_F0661_C014_WIDTH_PC34 == 88);
    assert(DM1_V1_F0661_C014_HEIGHT_PC34 == 45);
    assert(DM1_V1_F0661_MEDIUM_WIDTH_PC34 == 64);
    assert(DM1_V1_F0661_SMALL_WIDTH_PC34 == 42);
    assert(DM1_V1_F0661_M653_PC34 == 695);
}
static void test_receipt_struct(void) {
    DM1_V1_F0661DamageMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0661_damage_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_receipt_null(void) {
    DM1_V1_F0661DamageMaterialReceiptPc34 r;
    int ok = dm1_v1_f0661_damage_material_receipt_pc34(NULL, NULL, NULL, &r); (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_receipt_struct(); test_fnv1a_null(); test_receipt_null();
    puts("ok: DM1 F0661 damage material (Q-DM1-05) 4 tests passed"); return 0;
}
