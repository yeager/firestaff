#include "dm1_v1_fmtowns_font_asset.h"
#include <assert.h>
#include <stdio.h>

static void test_pic_lib_index(void) {
    /* Byte-verified from EDM.EXP INIT_TEXT + GET_MY_DECODED decode. */
    assert(dm1_v1_fmtowns_font_pic_library_index_pc34() == 557u);
    assert(DM1_V1_FMTOWNS_FONT_PIC_LIB_INDEX == 557u);
}

static void test_alloc_bytes(void) {
    /* INIT_TEXT: push 0x300 ; push 1 ; call PUSH_MEM. */
    assert(dm1_v1_fmtowns_font_raster_alloc_bytes_pc34() == 768u);
    assert(DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES == 0x300u);
}

static void test_decode_init_text_arg(void) {
    /* INIT_TEXT pushes 0xffffc22d as the GET_MY_DECODED arg. */
    uint32_t arg = DM1_V1_FMTOWNS_FONT_MY_DECODED_ARG;
    assert(arg == 0xffffc22du);
    /* Low 14 bits = 557 = 0x022d. */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(arg) == 557u);
    /* Bit 15 set. */
    assert(dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(arg) == 1);
    /* Bit 14 set. */
    assert(dm1_v1_fmtowns_font_is_no_size_header_pc34(arg) == 1);
}

static void test_decode_arg_masks(void) {
    /* Verify each mask extracts only its own bits. */
    /* Pure direct flag, no index. */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0x00008000u) == 0);
    assert(dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(0x00008000u) == 1);
    assert(dm1_v1_fmtowns_font_is_no_size_header_pc34(0x00008000u) == 0);
    /* Pure no-header flag, no index. */
    assert(dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(0x00004000u) == 0);
    assert(dm1_v1_fmtowns_font_is_no_size_header_pc34(0x00004000u) == 1);
    /* Pure index, no flags. */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0x00000042u) == 0x42u);
    assert(dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(0x00000042u) == 0);
    assert(dm1_v1_fmtowns_font_is_no_size_header_pc34(0x00000042u) == 0);
}

static void test_mask_boundaries(void) {
    /* Max index that fits in the 14-bit field. */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0x3fffu) == 0x3fffu);
    /* Adjacent bits are properly rejected. */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0x4000u) == 0);
    /* Both flags plus index (matches the exact INIT_TEXT arg shape). */
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0xc22du) == 0x022du);
    assert(dm1_v1_fmtowns_font_decode_pic_index_pc34(0xc22du) == 557u);
}

int main(void) {
    test_pic_lib_index();
    test_alloc_bytes();
    test_decode_init_text_arg();
    test_decode_arg_masks();
    test_mask_boundaries();
    printf("All dm1_v1_fmtowns_font_asset tests passed.\n");
    return 0;
}
