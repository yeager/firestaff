#include "dm1_v1_f0662_invisibility_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0662_C028_CHAMPION_ICONS_PC34 == 28);
    assert(DM1_V1_F0662_C028_WIDTH_PC34 == 76);
    assert(DM1_V1_F0662_C028_HEIGHT_PC34 == 14);
    assert(DM1_V1_F0662_M653_PC34 == 695);
    assert(DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34 == 16);
}
static void test_receipt_struct(void) {
    DM1_V1_F0662InvisibilityMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_palette_changes(void) {
    const unsigned char* pc = dm1_v1_f0662_invisibility_palette_changes_pc34();
    assert(pc != NULL);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0662_invisibility_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_receipt_null(void) {
    DM1_V1_F0662InvisibilityMaterialReceiptPc34 r;
    int ok = dm1_v1_f0662_invisibility_material_receipt_pc34(NULL, NULL, NULL, 0, &r); (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_receipt_struct(); test_palette_changes(); test_fnv1a_null(); test_receipt_null();
    puts("ok: DM1 F0662 invisibility material (Q-DM1-07) 5 tests passed"); return 0;
}
