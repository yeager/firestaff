#include "dm1_v1_f0675_scaled_material_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_source_struct(void) {
    DM1_V1_F0675SourceSurfacePc34 s; memset(&s, 0, sizeof(s));
    assert(s.graphicsDatOwned == 0); assert(s.pixelsFNV1a == 0);
}
static void test_receipt_struct(void) {
    DM1_V1_F0675ScaledMaterialReceiptPc34 r; memset(&r, 0, sizeof(r));
    assert(r.valid == 0); assert(r.sourceFingerprint == 0);
}
static void test_fnv1a_null(void) {
    uint32_t h = dm1_v1_f0675_scaled_material_fnv1a_pc34(NULL, 0); (void)h; assert(h == 0u);
}
static void test_fnv1a_data(void) {
    unsigned char d[] = {0xCC, 0xDD};
    uint32_t h = dm1_v1_f0675_scaled_material_fnv1a_pc34(d, 2); (void)h; assert(h != 0u);
}
static void test_receipt_null(void) {
    DM1_V1_F0675ScaledMaterialReceiptPc34 r;
    int ok = dm1_v1_f0675_scaled_material_receipt_pc34(NULL, 0, 0, NULL, 0, &r);
    (void)ok; assert(ok == 0);
}
int main(void) {
    test_source_struct(); test_receipt_struct(); test_fnv1a_null();
    test_fnv1a_data(); test_receipt_null();
    puts("ok: DM1 F0675 scaled material (Q-DM1-03) 5 tests passed"); return 0;
}
