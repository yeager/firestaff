#include "dm1_v1_f0732_f0735_fill_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0732_SPELL_BACKGROUND_GRAPHIC_PC34 == 9);
    assert(DM1_V1_F0735_INVENTORY_GRAPHIC_PC34 == 17);
    assert(DM1_V1_F0732_SCREEN_WIDTH_PC34 == 320);
    assert(DM1_V1_F0732_SCREEN_HEIGHT_PC34 == 200);
    assert(DM1_V1_F0732_SPELL_LEFT_PC34 == 224);
    assert(DM1_V1_F0735_VIEWPORT_WIDTH_PC34 == 224);
    assert(DM1_V1_F0735_VIEWPORT_HEIGHT_PC34 == 136);
}
static void test_receipt_struct(void) {
    DM1_V1_F0732F0735ReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.sourceFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0732_f0735_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_fnv1a_data(void) {
    unsigned char d[] = {0x11, 0x22};
    uint32_t h = dm1_v1_f0732_f0735_fnv1a_pc34(d, 2); (void)h; assert(h != 0u);
}
static void test_box_struct(void) {
    DM1_V1_F0732F0735BoxPc34 b; memset(&b, 0, sizeof(b));
    assert(b.left == 0); assert(b.right == 0);
}
static void test_clear_spell_null(void) {
    DM1_V1_F0732F0735ReceiptPc34 r;
    int ok = dm1_v1_f0732_clear_spell_area_pc34(NULL, NULL, 0, NULL, NULL, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_receipt_struct(); test_fnv1a_null();
    test_fnv1a_data(); test_box_struct(); test_clear_spell_null();
    puts("ok: DM1 F0732-F0735 fill material (Q-DM1-07) 6 tests passed"); return 0;
}
