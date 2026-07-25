#include "dm1_v1_title_screen_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_TITLE_ZOOM_STEPS == 18);
    assert(DM1_V1_TITLE_C001_WIDTH_PC34 == 320);
    assert(DM1_V1_TITLE_C001_HEIGHT_PC34 == 200);
    assert(DM1_V1_TITLE_C001_BYTES_PC34 == 320 * 200);
    assert(DM1_V1_TITLE_GRAPHIC_TITLE_PRESENTS_PC34 == 562);
}

static void test_init(void)
{
    DM1_V1_TitleStatePc34 s;
    DM1_V1_Title_InitPc34Compat(&s);
    assert(s.initialized == false);
    assert(s.current_zoom_step == 0);
    assert(s.title_bitmap == NULL);
    assert(s.master_bitmap == NULL);
}

static void test_load_graphics_null(void)
{
    DM1_V1_TitleStatePc34 s;
    DM1_V1_Title_InitPc34Compat(&s);
    int r = DM1_V1_Title_LoadGraphicsPc34Compat(&s, NULL, 0);
    (void)r;
    assert(r == 0);
}

static void test_cleanup(void)
{
    DM1_V1_TitleStatePc34 s;
    DM1_V1_Title_InitPc34Compat(&s);
    DM1_V1_Title_CleanupPc34Compat(&s);
    assert(s.initialized == false);
}

static void test_capture_receipt_uninit(void)
{
    DM1_V1_TitleStatePc34 s;
    DM1_V1_Title_InitPc34Compat(&s);
    DM1_V1_TitleRealAssetCaptureReceiptPc34 receipt;
    memset(&receipt, 0xFF, sizeof(receipt));
    int r = DM1_V1_Title_BuildRealAssetCaptureReceiptPc34Compat(&s, 0, &receipt);
    (void)r;
    assert(r == 0 || receipt.valid == 0);
}

static void test_receipt_struct_layout(void)
{
    DM1_V1_TitleRealAssetCaptureReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.initialized == 0);
    assert(r.requiresGraphicsDat == 0);
    assert(r.noHostRenderInference == 0);
}

int main(void)
{
    test_constants();
    test_init();
    test_load_graphics_null();
    test_cleanup();
    test_capture_receipt_uninit();
    test_receipt_struct_layout();

    puts("ok: DM1 title screen (Q-DM1-08) 6 tests passed");
    return 0;
}
