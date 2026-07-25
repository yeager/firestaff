#include "dm1_v1_f0355_inventory_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0355_C017_INVENTORY_PC34 == 17);
    assert(DM1_V1_F0355_C033_SLOT_PC34 == 33);
    assert(DM1_V1_F0355_M653_PC34 == 695);
}
static void test_receipt_struct(void) {
    DM1_V1_F0355InventoryMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0355_inventory_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_fnv1a_data(void) {
    unsigned char d[] = {0xAA, 0xBB};
    uint32_t h = dm1_v1_f0355_inventory_material_fnv1a_pc34(d, 2); (void)h; assert(h != 0u);
}
static void test_receipt_empty(void) {
    DM1_V1_F0355InventoryMaterialReceiptPc34 r;
    int ok = dm1_v1_f0355_inventory_material_receipt_pc34(NULL, 0, NULL, 0, &r); (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_receipt_struct(); test_fnv1a_null(); test_fnv1a_data(); test_receipt_empty();
    puts("ok: DM1 F0355 inventory material (Q-DM1-06) 5 tests passed"); return 0;
}
