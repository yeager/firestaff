#include "dm1_v1_f0352_eye_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_V1_F0352_C018_ARROW_PC34 == 18);
    assert(DM1_V1_F0352_C019_EYE_PC34 == 19);
    assert(DM1_V1_F0352_M653_PC34 == 695);
    assert(DM1_V1_F0352_C503_ZONE_PC34 == 503);
}
static void test_source_struct(void) {
    DM1_V1_F0352SourceSurfacePc34 s; memset(&s, 0, sizeof(s));
    assert(s.graphicsDatOwned == 0); assert(s.indexedPixels == NULL);
}
static void test_receipt_struct(void) {
    DM1_V1_F0352EyeMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.materialFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0352_eye_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_fnv1a_data(void) {
    unsigned char d[] = {1,2,3};
    uint32_t h = dm1_v1_f0352_eye_material_fnv1a_pc34(d, 3); (void)h; assert(h != 0u);
}
static void test_receipt_empty(void) {
    DM1_V1_F0352EyeMaterialReceiptPc34 r;
    int ok = dm1_v1_f0352_eye_material_receipt_pc34(NULL, 0, NULL, 0, &r); (void)ok; assert(ok == 0);
}
int main(void) {
    test_constants(); test_source_struct(); test_receipt_struct();
    test_fnv1a_null(); test_fnv1a_data(); test_receipt_empty();
    puts("ok: DM1 F0352 eye material (Q-DM1-06) 6 tests passed"); return 0;
}
