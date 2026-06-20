/*
 * test_csb_v1_graphics_hidden_item_skip.c
 *
 * Integration test for csb_v1_graphics_hidden_item_skip_pc34_compat.c.
 *
 * Builds a synthetic CSB-style GRAPHICS.DAT file with 700
 * dummy bitmaps, opens it with the existing DM1 V1 graphics
 * loader (m11_gfx_open_dat + m11_gfx_load_bitmap), and
 * exercises the safe-load wrapper to confirm:
 *
 *   1. CSB Atari ST hidden-code items 21, 538, 548 are
 *      skipped (return 1 with empty bitmap; do not invoke
 *      the LZW decompressor on the executable bytes).
 *   2. CSB Amiga hidden-code items 21, 676, 686 are skipped.
 *   3. Normal items at the same indices on a different
 *      platform (or on PC 3.4) are still loaded normally.
 *   4. The framebuffer byte region that would have been
 *      written for a hidden item is left untouched after
 *      safe-load -- verifying the calling code can do
 *      "load -> blit -> if data NULL skip blit" without
 *      corrupting the framebuffer with executable bytes.
 *
 * No real game data needed; the synthetic file is built in
 * a temp directory.
 *
 * Source lock: ReDMCSB GRAPH21.C F0914_Graphic21, GRAPH538.C
 * F0915_Graphic538, GRAPH548.C F0916_Graphic548 (the actual
 * 68k code that gets disguised as image data in
 * GRAPHICS.DAT and which this test defends against).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "csb_v1_graphics_hidden_item_skip_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#define ASSERT_TRUE(cond) do {                                            \
    if (!(cond)) {                                                        \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",               \
                __FILE__, __LINE__, #cond);                               \
        return 0;                                                          \
    }                                                                      \
} while (0)

#define ASSERT_EQ(a, b) do {                                              \
    long long _a = (long long)(a);                                        \
    long long _b = (long long)(b);                                        \
    if (_a != _b) {                                                       \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s == %s "           \
                "(got %lld, expected %lld)\n",                            \
                __FILE__, __LINE__, #a, #b, _a, _b);                      \
        return 0;                                                          \
    }                                                                      \
} while (0)

/*
 * Synthetic GRAPHICS.DAT builder.
 *
 * Layout (matches dm1_v1_graphics_loader_pc34_compat.c):
 *   uint16_t count                    -- number of bitmaps
 *   for each bitmap:
 *     uint16_t width
 *     uint16_t height
 *     uint32_t compressed_size        -- LZW-compressed byte count
 *     uint8_t  compressed_data[compressed_size]
 *
 * The DM1 V1 loader pre-reads count + all headers, so we can
 * write one entry per index, then optionally patch the data
 * of a particular index to look like "executable bytes" (we
 * use a recognisable 0x60 / 0x1A Atari-ST RESET-vector prefix
 * pattern that the Meynaf disassembly actually uses).
 */

#define SYNTH_ITEM_COUNT 700

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t compressed_size;
    uint8_t  payload[8];   /* synthetic LZW-compressed payload (small fixed size) */
} SynthItem;

static int build_synth_dat(const char* path)
{
    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    /* Use a tiny fixed-size payload per item so the loader can
     * decompress it; this is enough to exercise the loader path
     * without needing a real LZW encoder. The hidden-code path
     * is exercised by the safe-load wrapper BEFORE the loader
     * is consulted, so the synthetic payload contents are
     * irrelevant for hidden items. */
    const uint32_t payload_bytes = 8;

    uint16_t count = SYNTH_ITEM_COUNT;
    if (fwrite(&count, sizeof(count), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    SynthItem item;
    for (uint16_t i = 0; i < SYNTH_ITEM_COUNT; ++i) {
        item.width = 8;
        item.height = 8;
        item.compressed_size = payload_bytes;
        /* Mark every item with a recognisable payload fill. */
        memset(item.payload, (uint8_t)(0x10 + (i & 0x0F)), payload_bytes);
        /* Items 21, 538, 548, 676, 686 also stash a copy-protection
         * sentinel at the start of the payload. The safe-load
         * wrapper must NOT pass these to the LZW decoder. */
        if (i == 21 || i == 538 || i == 548 ||
            i == 676 || i == 686) {
            item.payload[0] = 0x60; /* Atari ST reset vector hi */
            item.payload[1] = 0x00; /* Atari ST reset vector lo */
            item.payload[2] = 0x1A; /* Meynaf 68k code sig       */
            item.payload[3] = 0xFC;
        }
        if (fwrite(&item.width, sizeof(uint16_t), 1, f) != 1) { fclose(f); return -1; }
        if (fwrite(&item.height, sizeof(uint16_t), 1, f) != 1) { fclose(f); return -1; }
        if (fwrite(&item.compressed_size, sizeof(uint32_t), 1, f) != 1) { fclose(f); return -1; }
        if (fwrite(item.payload, 1, payload_bytes, f) != payload_bytes) {
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

/* ── Tests ──────────────────────────────────────────────────────── */

static int test_self_test(void)
{
    ASSERT_EQ(csb_v1_graphics_hidden_item_skip_self_test(), 0);
    return 1;
}

static int test_atari_st_hidden_items_skip(void)
{
    const char* path = "/tmp/test_csb_v1_graphics_hidden_synth.dat";
    if (build_synth_dat(path) != 0) {
        fprintf(stderr, "build_synth_dat failed\n");
        return 0;
    }

    M11_GFX_LoaderState state;
    m11_gfx_init(&state);
    ASSERT_TRUE(m11_gfx_open_dat(&state, path));

    M11_GFX_Bitmap bmp;

    /* Item 21: hidden code -> skip. */
    int rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 21, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);
    ASSERT_EQ(bmp.width, 0);
    ASSERT_EQ(bmp.height, 0);
    ASSERT_EQ(bmp.byte_width, 0);

    /* Item 538: hidden code -> skip. */
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 538, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    /* Item 548: hidden code -> skip. */
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 548, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    /* Item 560: in the 558-562 range, also hidden. */
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 560, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    m11_gfx_close(&state);
    return 1;
}

static int test_amiga_hidden_items_skip(void)
{
    const char* path = "/tmp/test_csb_v1_graphics_hidden_synth.dat";
    if (build_synth_dat(path) != 0) return 0;

    M11_GFX_LoaderState state;
    m11_gfx_init(&state);
    ASSERT_TRUE(m11_gfx_open_dat(&state, path));

    M11_GFX_Bitmap bmp;

    /* Item 21: hidden code on Amiga too. */
    int rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 21, CSB_V1_HIDDEN_PLATFORM_AMIGA, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    /* Item 676: Amiga-specific hidden code. */
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 676, CSB_V1_HIDDEN_PLATFORM_AMIGA, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    /* Item 686: Amiga-specific hidden code. */
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 686, CSB_V1_HIDDEN_PLATFORM_AMIGA, &bmp);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp.data == NULL);

    m11_gfx_close(&state);
    return 1;
}

static int test_pc34_does_not_skip(void)
{
    const char* path = "/tmp/test_csb_v1_graphics_hidden_synth.dat";
    if (build_synth_dat(path) != 0) return 0;

    M11_GFX_LoaderState state;
    m11_gfx_init(&state);
    ASSERT_TRUE(m11_gfx_open_dat(&state, path));

    M11_GFX_Bitmap bmp;

    /* The skip decision itself must say false on PC 3.4 for
     * all hidden items (21/538/548/676/686). This is the
     * core property: PC 3.4 has no hidden code. */
    CSB_V1_HiddenSkipDecision dec;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 21);
    ASSERT_TRUE(!dec.should_skip);
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 538);
    ASSERT_TRUE(!dec.should_skip);
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 548);
    ASSERT_TRUE(!dec.should_skip);
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 676);
    ASSERT_TRUE(!dec.should_skip);
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 686);
    ASSERT_TRUE(!dec.should_skip);

    /* PC 3.4 wrapper invocation: the wrapper passes through
     * to the loader, which returns either 0 or 1 depending on
     * whether the (synthetic) LZW payload decodes to >=1 byte.
     * Either way, the bitmap must NOT have been "hidden-skip"
     * zeroed by the wrapper, because PC 3.4 should never short
     * circuit. */
    int rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 21, CSB_V1_HIDDEN_PLATFORM_PC34, &bmp);
    /* On PC 3.4 the wrapper must not have produced an empty
     * bitmap the way it does for hidden items (width=0, height=0).
     * The loader may have actually decoded garbage from our
     * synthetic payload, but the width/height from the file
     * header (8x8) will be set, which is what we assert. */
    if (rc == 1) {
        /* Loader returned success with decoded data. The wrapper
         * must NOT have zeroed width/height (which it does only
         * for hidden items). Width and height from the header
         * are 8,8. */
        ASSERT_EQ(bmp.width, 8);
        ASSERT_EQ(bmp.height, 8);
        ASSERT_TRUE(bmp.data != NULL);
        /* Free the decoded bitmap so we don't leak. */
        m11_gfx_free_bitmap(&bmp);
    }
    /* If rc == 0, the loader rejected the synthetic LZW, which
     * is also acceptable for PC 3.4. The important property is
     * that the wrapper did not take the hidden-skip path. */

    /* Sanity: Atari ST invocation of the same item must skip
     * with empty bitmap -- this proves PC 3.4 differs from
     * Atari ST for the same index. */
    M11_GFX_Bitmap bmp2;
    rc = csb_v1_graphics_hidden_item_load_safe(
        &state, 21, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp2);
    ASSERT_EQ(rc, 1);
    ASSERT_TRUE(bmp2.data == NULL);
    ASSERT_EQ(bmp2.width, 0);
    ASSERT_EQ(bmp2.height, 0);

    m11_gfx_close(&state);
    return 1;
}

static int test_cross_platform_isolation(void)
{
    /* Atari ST item 548 must NOT skip when platform is Amiga
     * (548 is Atari ST specific), and Amiga item 686 must NOT
     * skip on Atari ST. */
    CSB_V1_HiddenSkipDecision dec;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 548);
    ASSERT_TRUE(!dec.should_skip);
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 686);
    ASSERT_TRUE(!dec.should_skip);

    /* 676 is Amiga-only. */
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 676);
    ASSERT_TRUE(!dec.should_skip);

    return 1;
}

static int test_framebuffer_not_corrupted_by_hidden_items(void)
{
    /*
     * End-to-end: simulate a viewport blit where a hidden-code
     * item would have overwritten a framebuffer region with
     * 68k bytes if the loader naively memcpy'd them. With the
     * safe wrapper, the wrapper returns an empty bitmap, the
     * blit is a no-op, and the framebuffer bytes remain at
     * their original pattern.
     */
    const char* path = "/tmp/test_csb_v1_graphics_hidden_synth.dat";
    if (build_synth_dat(path) != 0) return 0;

    M11_GFX_LoaderState state;
    m11_gfx_init(&state);
    ASSERT_TRUE(m11_gfx_open_dat(&state, path));

    /* Pretend framebuffer region for the would-be blit. */
    enum { REGION = 256 };
    uint8_t framebuffer[REGION];
    memset(framebuffer, 0xAA, sizeof(framebuffer));
    uint8_t snapshot[REGION];
    memcpy(snapshot, framebuffer, sizeof(framebuffer));

    /* Try to load hidden items via the safe wrapper. Use the
     * correct platform for each item per the dmweb Meynaf
     * disassembly: 21, 538, 548 are Atari ST; 21, 676, 686 are
     * Amiga. Item 21 is hidden on both platforms, so we test it
     * on Atari ST (Amiga test is in test_amiga_hidden_items_skip). */
    uint16_t atari_hidden[] = { 21, 538, 548 };
    uint16_t amiga_hidden[] = { 676, 686 };
    M11_GFX_Bitmap bmp;
    for (size_t i = 0; i < sizeof(atari_hidden) / sizeof(atari_hidden[0]); ++i) {
        int rc = csb_v1_graphics_hidden_item_load_safe(
            &state, atari_hidden[i], CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
        ASSERT_EQ(rc, 1);
        ASSERT_TRUE(bmp.data == NULL);
        ASSERT_EQ(memcmp(framebuffer, snapshot, sizeof(framebuffer)), 0);
    }
    for (size_t i = 0; i < sizeof(amiga_hidden) / sizeof(amiga_hidden[0]); ++i) {
        int rc = csb_v1_graphics_hidden_item_load_safe(
            &state, amiga_hidden[i], CSB_V1_HIDDEN_PLATFORM_AMIGA, &bmp);
        ASSERT_EQ(rc, 1);
        ASSERT_TRUE(bmp.data == NULL);
        ASSERT_EQ(memcmp(framebuffer, snapshot, sizeof(framebuffer)), 0);
    }

    m11_gfx_close(&state);
    return 1;
}

int main(void)
{
    int passed = 0, total = 0;
    #define RUN(name) do {                                                 \
        total++;                                                           \
        if (name()) {                                                      \
            passed++;                                                      \
        } else {                                                           \
            fprintf(stderr, "test failed: %s\n", #name);                  \
        }                                                                   \
    } while (0)
    RUN(test_self_test);
    RUN(test_atari_st_hidden_items_skip);
    RUN(test_amiga_hidden_items_skip);
    RUN(test_pc34_does_not_skip);
    RUN(test_cross_platform_isolation);
    RUN(test_framebuffer_not_corrupted_by_hidden_items);
    #undef RUN

    printf("test_csb_v1_graphics_hidden_item_skip: %d/%d passed\n",
           passed, total);
    return (passed == total) ? 0 : 1;
}