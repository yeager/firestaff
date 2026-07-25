#include "dm1_v1_f0342_object_description_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0342_M653_BYTES_PC34 == 768);
    assert(DM1_V1_F0342_C020_PANEL_PC34 == 20);
    assert(DM1_V1_F0342_C029_CIRCLE_PC34 == 29);
    assert(DM1_V1_F0342_M653_PC34 == 695);
}
static void test_operation_kinds(void) {
    assert(DM1_V1_F0342_PANEL_BACKGROUND_PC34 == 1);
    assert(DM1_V1_F0342_OBJECT_CIRCLE_PC34 == 2);
    assert(DM1_V1_F0342_OBJECT_NAME_PC34 == 3);
    assert(DM1_V1_F0342_OBJECT_BODY_PC34 == 4);
}
static void test_receipt_struct(void) {
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0); assert(r.operationCount == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0342_object_description_material_fnv1a_pc34(NULL, 0);
    (void)h; assert(h == 0u);
}
static void test_receipt_empty(void) {
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34 r;
    int ok = dm1_v1_f0342_object_description_material_receipt_pc34(NULL, 0, NULL, 0, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_operation_kinds(); test_receipt_struct();
    test_fnv1a_null(); test_receipt_empty();
    puts("ok: DM1 F0342 object description material (Q-DM1-06) 5 tests passed"); return 0;
}
