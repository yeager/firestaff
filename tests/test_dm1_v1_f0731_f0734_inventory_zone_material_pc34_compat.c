#include "dm1_v1_f0731_f0734_inventory_zone_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0731_SPELL_BACKGROUND_GRAPHIC_PC34 == 9);
    assert(DM1_V1_F0734_INVENTORY_GRAPHIC_PC34 == 17);
    assert(DM1_V1_F0734_DARKEST_GRAY_PC34 == 12);
    assert(DM1_V1_F0734_VIEWPORT_WIDTH_PC34 == 224);
    assert(DM1_V1_F0734_VIEWPORT_HEIGHT_PC34 == 136);
}
static void test_zone_constants(void) {
    assert(DM1_V1_F0734_ZONE_SAVE_PC34 == 562);
    assert(DM1_V1_F0734_ZONE_REST_PC34 == 564);
    assert(DM1_V1_F0734_ZONE_CLOSE_PC34 == 566);
    assert(DM1_V1_F0734_ZONE_MUSIC_PC34 == 568);
}
static void test_receipt_struct(void) {
    DM1_V1_F0731F0734ReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.sourceFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0731_f0734_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_invert_null(void) {
    DM1_V1_F0731F0734ReceiptPc34 r;
    int ok = dm1_v1_f0731_invert_spell_caster_zone_pc34(NULL, NULL, NULL, NULL, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_zone_constants(); test_receipt_struct();
    test_fnv1a_null(); test_invert_null();
    puts("ok: DM1 F0731-F0734 inventory zone material (Q-DM1-06) 5 tests passed"); return 0;
}
