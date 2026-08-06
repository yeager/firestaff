#include "theron_v1_vram_trace_loader.h"
#include "theron_v1_palette.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void test_load_raw(void) {
    Theron_V1_Viewport vp;
    memset(&vp, 0, sizeof(vp));

    uint8_t vram[THERON_VRAM_SIZE];
    uint8_t vce[THERON_VCE_SIZE];
    memset(vram, 0, sizeof(vram));
    memset(vce, 0, sizeof(vce));

    /* BAT words select tile 0/1/2 and palette groups 3/4/5. */
    vram[0] = 0x00; vram[1] = 0x38;
    vram[2] = 0x01; vram[3] = 0x48;
    vram[4] = 0x02; vram[5] = 0x58;
    /* Put a non-zero tile at VRAM byte 0x1000 (tile 0 at word $0800). */
    for (int i = 0; i < 32; i++) vram[0x1000 + i] = (uint8_t)(i + 1);

    /* Put a BGR333 color in VCE entry 1: R=7, G=0, B=0 → 0x0007 LE */
    vce[2] = 0x07;
    vce[3] = 0x00;

    int rc = theron_v1_vram_trace_load_raw(&vp, vram, THERON_VRAM_SIZE,
                                            vce, THERON_VCE_SIZE);
    assert(rc == 0);
    assert(vp.vram_trace_loaded == 1);
    assert(vp.vram_trace_data != NULL);
    assert(vp.vce_trace_data != NULL);

    /* Palette entry 1 should be red (R=7*36=252) */
    uint32_t rgba = vp.palette.entries[1].rgba;
    unsigned r = (rgba >> 16) & 0xFF;
    assert(r == 252);

    theron_v1_vram_trace_unload(&vp);
    assert(vp.vram_trace_loaded == 0);
    assert(vp.vram_trace_data == NULL);
    printf("PASS: test_load_raw\n");
}

static void test_populate_tiles(void) {
    Theron_V1_Viewport vp;
    uint8_t framebuffer[TQR_FB_W * TQR_FB_H];
    memset(&vp, 0, sizeof(vp));
    memset(framebuffer, 0, sizeof(framebuffer));
    vp.fb.data = framebuffer;
    vp.fb.w = TQR_FB_W;
    vp.fb.h = TQR_FB_H;
    vp.fb.stride = TQR_FB_W;

    uint8_t vram[THERON_VRAM_SIZE];
    uint8_t vce[THERON_VCE_SIZE];
    memset(vram, 0, sizeof(vram));
    memset(vce, 0, sizeof(vce));

    /* BAT words select three source tiles with distinct palette groups. */
    vram[0] = 0x00; vram[1] = 0x38;
    vram[2] = 0x01; vram[3] = 0x48;
    vram[4] = 0x02; vram[5] = 0x58;
    /* Create 3 source tiles. */
    for (int t = 0; t < 3; t++)
        for (int i = 0; i < 32; i++)
            vram[0x1000 + t * 32 + i] = (uint8_t)(t + 1);

    int rc = theron_v1_vram_trace_load_raw(&vp, vram, THERON_VRAM_SIZE,
                                            vce, THERON_VCE_SIZE);
    assert(rc == 0);

    int loaded = theron_v1_vram_trace_populate_tiles(&vp, 0, 3, 1);
    assert(loaded == 3);
    assert(vp.palette.tile_count == 3);
    assert(vp.palette.tiles[0].vram_index == 0);
    assert(vp.palette.tiles[0].pal_group == 3);
    assert(vp.palette.tiles[1].vram_index == 1);
    assert(vp.palette.tiles[1].pal_group == 4);
    assert(vp.palette.tiles[2].vram_index == 2);
    assert(vp.palette.tiles[2].pal_group == 5);
    assert(theron_v1_vram_trace_bat_atlas_index(&vp, 0) == 0);
    assert(theron_v1_vram_trace_bat_atlas_index(&vp, 1) == 1);
    assert(theron_v1_vram_trace_bat_atlas_index(&vp, 2) == 2);
    assert(theron_v1_vram_trace_bat_atlas_index(&vp, 3) == -1);
    assert(theron_v1_vram_trace_render_bat_preview(&vp, 0, 3, 1, 0, 0) == 3);
    assert(framebuffer[0] != 0 || framebuffer[8] != 0 || framebuffer[16] != 0);
    assert(theron_v1_vram_trace_render_bat_preview(&vp, 0, 3, 1,
                                                   TQR_FB_W - 16, 0) == -1);

    assert(theron_v1_vram_trace_populate_tiles(&vp, -1, 32, 32) == -1);
    assert(theron_v1_vram_trace_populate_tiles(&vp, 0, 65, 1) == -1);
    assert(theron_v1_vram_trace_populate_tiles(&vp, 1900, 8, 4) == -1);
    assert(theron_v1_vram_trace_bat_atlas_index(&vp, 2048) == -1);

    theron_v1_vram_trace_unload(&vp);
    printf("PASS: test_populate_tiles\n");
}

static void test_bgr333_decode(void) {
    /* BGR333: B[8:6] G[5:3] R[2:0]
     * White: R=7 G=7 B=7 → 0x01FF */
    uint32_t white = tqr_bgr333_to_rgba(0x01FF);
    unsigned r = (white >> 16) & 0xFF;
    unsigned g = (white >> 8) & 0xFF;
    unsigned b = white & 0xFF;
    assert(r == 252 && g == 252 && b == 252);

    /* Black: all zero */
    uint32_t black = tqr_bgr333_to_rgba(0x0000);
    assert((black & 0x00FFFFFF) == 0);

    /* Pure blue: B=7, G=0, R=0 → 0x01C0 */
    uint32_t blue = tqr_bgr333_to_rgba(0x01C0);
    r = (blue >> 16) & 0xFF;
    g = (blue >> 8) & 0xFF;
    b = blue & 0xFF;
    assert(r == 0 && g == 0 && b == 252);

    printf("PASS: test_bgr333_decode\n");
}

static void test_tile_decode_msb_first(void) {
    /* 2bpp tile row: plane0=0x80 (bit7=1), plane1=0x00
     * MSB-first: pixel 0 should be 1, pixels 1-7 should be 0 */
    uint8_t src[2] = {0x80, 0x00};
    uint8_t out[8] = {0};
    tqr_decode_tile_row(out, src, 2);
    assert(out[0] == 1);
    for (int i = 1; i < 8; i++) assert(out[i] == 0);

    /* 2bpp: plane0=0x01 (bit0=1), plane1=0x00
     * MSB-first: pixel 7 should be 1, pixels 0-6 should be 0 */
    src[0] = 0x01; src[1] = 0x00;
    tqr_decode_tile_row(out, src, 2);
    assert(out[7] == 1);
    for (int i = 0; i < 7; i++) assert(out[i] == 0);

    printf("PASS: test_tile_decode_msb_first\n");
}

static void test_4bpp_interleaved(void) {
    /* 4bpp tile: 32 bytes. Set plane 2 bit 7 for row 0.
     * Bytes 16+0*2 = byte 16 = plane 2 for row 0.
     * Result: pixel 0, row 0 should have bit 2 set = value 4 */
    uint8_t src[32];
    memset(src, 0, 32);
    src[16] = 0x80;  /* plane 2, row 0, bit 7 */

    uint8_t out[64];
    memset(out, 0, 64);
    tqr_decode_tile(out, src, 4);
    assert(out[0] == 4);
    for (int i = 1; i < 8; i++) assert(out[i] == 0);

    printf("PASS: test_4bpp_interleaved\n");
}

static void test_null_safety(void) {
    assert(theron_v1_vram_trace_load_raw(NULL, NULL, 0, NULL, 0) == -1);
    theron_v1_vram_trace_unload(NULL);
    assert(theron_v1_vram_trace_populate_tiles(NULL, 0, 0, 0) == -1);
    printf("PASS: test_null_safety\n");
}

int main(void) {
    test_bgr333_decode();
    test_tile_decode_msb_first();
    test_4bpp_interleaved();
    test_load_raw();
    test_populate_tiles();
    test_null_safety();
    printf("All VRAM trace loader tests passed.\n");
    return 0;
}
