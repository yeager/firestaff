#include "dm1_v1_graphics_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_GFX_MAX_BITMAPS == 600);
    assert(DM1_GFX_LZW_MAX_CODE == 4096);
    assert(DM1_GFX_LZW_MAX_BITS == 12);
    assert(DM1_GFX_LZW_CLEAR_CODE == 256);
    assert(DM1_GFX_LZW_END_CODE == 257);
    assert(DM1_GFX_LZW_FIRST_CODE == 258);
}

static void test_init(void) {
    DM1_V1_GFX_LoaderStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_GFX_InitPc34Compat(&state);
    assert(state.loaded == false);
    assert(state.bitmap_count == 0);
}

static void test_open_nonexistent(void) {
    DM1_V1_GFX_LoaderStatePc34 state;
    DM1_V1_GFX_InitPc34Compat(&state);
    bool ok = DM1_V1_GFX_OpenDatPc34Compat(&state,
        "/nonexistent_path_12345.dat");
    (void)ok;
    assert(ok == false);
}

static void test_close_unloaded(void) {
    DM1_V1_GFX_LoaderStatePc34 state;
    DM1_V1_GFX_InitPc34Compat(&state);
    DM1_V1_GFX_ClosePc34Compat(&state);
    /* Should not crash; state remains clean */
    assert(state.loaded == false);
}

static void test_free_bitmap_null_data(void) {
    DM1_V1_GFX_BitmapPc34 bmp;
    memset(&bmp, 0, sizeof(bmp));
    bmp.data = NULL;
    bmp.allocated = false;
    DM1_V1_GFX_FreeBitmapPc34Compat(&bmp);
    /* Should not crash */
    assert(bmp.data == NULL);
}

static void test_lzw_state_struct(void) {
    DM1_V1_GFX_LZWStatePc34 lzw;
    memset(&lzw, 0, sizeof(lzw));
    assert(lzw.next_code == 0);
    assert(lzw.code_bits == 0);
}

int main(void) {
    test_constants();
    test_init();
    test_open_nonexistent();
    test_close_unloaded();
    test_free_bitmap_null_data();
    test_lzw_state_struct();
    puts("ok: DM1 graphics loader (Q-DM1-08) 6 tests passed");
    return 0;
}
