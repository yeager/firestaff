#include "dm1_v1_f0693_f0699_video_material_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_video_constants(void)
{
    assert(DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 == 320);
    assert(DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34 == 200);
    assert(DM1_V1_F0693_F0699_VIDEO_PIXEL_COUNT_PC34 == 64000);
}

static void test_raster_struct(void)
{
    DM1_V1_F0693F0699VideoRasterPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.graphicsDatOwned == 0);
    assert(r.indexedPixels == NULL);
    assert(r.indexedPixelsFNV1a == 0);
}

static void test_receipt_struct(void)
{
    DM1_V1_F0693F0699VideoReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticFallback == 0);
    assert(r.rasterFingerprint == 0);
}

static void test_fnv1a_null(void)
{
    uint32_t h = dm1_v1_f0693_f0699_video_fnv1a_pc34(NULL, 0);
    (void)h;
    assert(h == 0u);
}

static void test_fnv1a_data(void)
{
    unsigned char data[] = {0x10, 0x20, 0x30};
    uint32_t h = dm1_v1_f0693_f0699_video_fnv1a_pc34(data, 3);
    (void)h;
    assert(h != 0u);
}

static void test_present_null_receipt(void)
{
    int ok = dm1_v1_f0693_f0699_video_present_pc34(NULL, NULL, NULL, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_present_null_raster(void)
{
    DM1_V1_F0693F0699VideoReceiptPc34 r;
    int ok = dm1_v1_f0693_f0699_video_present_pc34(NULL, NULL, NULL, &r);
    (void)ok;
    assert(ok == 0);
    assert(r.valid == 0);
}

int main(void)
{
    test_video_constants();
    test_raster_struct();
    test_receipt_struct();
    test_fnv1a_null();
    test_fnv1a_data();
    test_present_null_receipt();
    test_present_null_raster();

    puts("ok: DM1 F0693/F0699 video material (Q-DM1-03) 7 tests passed");
    return 0;
}
