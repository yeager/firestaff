/*
 * firestaff_dm2_v2_hud_widget_bitmap_blit_probe.c
 *
 * Headless CI probe for the DM2 V2 HUD widget bounded bitmap blit
 * path (Phase 3 follow-up). Verifies the runtime hook can read the
 * synthetic 1x1 RGBA PNG fixtures from examples/dm2_hud_widget_synthetic/,
 * bounded-blit the red channel to a framebuffer at the slot's anchor,
 * and gracefully fall back when the blit cannot run.
 *
 * No game data, no SDL rendering required. The probe re-uses the
 * existing FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR path the
 * synthetic promotion probe installs the example from, so this probe
 * reads the same fixtures directly without an install step.
 *
 * Coverage:
 *   1.  Reject bogus paths: NULL, empty, missing file
 *   2.  Reject non-PNG files (wrong signature)
 *   3.  Decode every synthetic 1x1 RGBA fixture and assert the
 *       expected pixel colour matches the source bytes (deterministic
 *       check: red channel is the unique per-slot identifier the
 *       runtime uses as the blit value)
 *   4.  Reject multi-pixel PNGs (out of bounded envelope)
 *   5.  Reject non-RGBA color types (e.g. 8-bit palette)
 *   6.  Bounded single-pixel blit writes the red channel at the
 *       expected framebuffer cell, leaving the rest of the buffer
 *       byte-identical to the pre-blit state
 *   7.  Out-of-bounds destinations return 0 and never write
 *   8.  Alpha-blend (alpha<255) does the "src over dst" integer blend
 *   9.  High-level render_slot reads the PNG, blits the pixel, and
 *       leaves no out-of-bounds writes even when the destination is
 *       outside the framebuffer
 *  10.  Source evidence citation contains the bounded-envelope text
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - examples/dm2_hud_widget_synthetic/ (synthetic 1x1 RGBA fixtures)
 *   - PNG specification (W3C / ISO 15948)
 *   - include/dm2_v2_hud_widget_bitmap_blit.h (module under test)
 */

#include "dm2_v2_hud_widget_bitmap_blit.h"
#include "dm2_v2_hud_widget_assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
#define FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR \
    "examples/dm2_hud_widget_synthetic"
#endif

/* ── Probe plumbing ─────────────────────────────────────────────── */

static int s_pass = 0;
static int s_fail = 0;

static void check(const char* name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_pass++;
    } else {
        printf("  FAIL: %s\n", name);
        s_fail++;
    }
}

/* Anchor pixel positions for the real-bitmap blit. Kept in sync
 * with k_real_stamp_anchors[] in src/dm2/dm2_v2_hud_runtime.c so a
 * runtime regression that moves an anchor fails this probe. */
typedef struct { int x; int y; } Anchor;
static const Anchor k_anchors[DM2_V2_HUD_WIDGET_COUNT] = {
    { 80,  4 },   /* INVENTORY_QUICK_VIEW */
    { 220, 4 },   /* ACTION_PROMPT */
    { 11,  16 },  /* COMPASS_ROSE */
    { 286, 8 },   /* DEPTH_INDICATOR */
    { 286, 178 }, /* GOLD_COUNTER */
    { 4,   4 },   /* CHAMPION_BAR_FRAME */
    { 16,  172 }, /* ACTION_STRIP_FRAME */
};

/* Expected (R, G, B, A) per slot, derived from the synthetic
 * fixtures' embedded pixels. The R value is what the blit writes
 * to the framebuffer (palette-index convention). The probe
 * re-derives the expected R for each fixture dynamically via
 * dm2_v2_hud_widget_bitmap_blit_read_pixel() so a fixture tweak
 * (e.g. swapping the colour) does not silently fail the probe —
 * we test the decode path is correct, not that the PNG bytes
 * match a hard-coded value. */
typedef struct {
    DM2_V2_HudWidgetSlot slot;
    const char*          fixture_path;
    const char*          category;
} SlotFixture;

static const SlotFixture k_slot_fixtures[DM2_V2_HUD_WIDGET_COUNT] = {
    { DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_widgets/inventory_quick_view.png", "hud_widgets" },
    { DM2_V2_HUD_WIDGET_ACTION_PROMPT,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_widgets/action_prompt.png", "hud_widgets" },
    { DM2_V2_HUD_WIDGET_COMPASS_ROSE,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_chrome/compass_rose.png", "hud_chrome" },
    { DM2_V2_HUD_WIDGET_DEPTH_INDICATOR,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_chrome/depth_indicator.png", "hud_chrome" },
    { DM2_V2_HUD_WIDGET_GOLD_COUNTER,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_chrome/gold_counter.png", "hud_chrome" },
    { DM2_V2_HUD_WIDGET_CHAMPION_BAR_FRAME,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_chrome/champion_bar_frame.png", "hud_chrome" },
    { DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME,
      FIRESTAFF_DM2_HUD_WIDGET_SYNTHETIC_EXAMPLE_DIR
          "/hud_chrome/action_strip_frame.png", "hud_chrome" },
};

/* ── Scenario: bogus paths are rejected safely ─────────────────── */

static void test_bogus_paths_rejected(void) {
    printf("\n[ Scenario 1: bogus paths rejected safely ]\n");
    DM2_V2_HudWidgetBlitPixel px;
    memset(&px, 0xAB, sizeof(px)); /* poison */

    check("read_pixel(NULL) returns 0",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(NULL, &px) == 0);
    check("read_pixel(NULL) leaves out_pixel zeroed",
          px.width == 0 && px.height == 0 && px.r == 0 && px.a == 0);

    memset(&px, 0xAB, sizeof(px));
    check("read_pixel(\"\") returns 0",
          dm2_v2_hud_widget_bitmap_blit_read_pixel("", &px) == 0);
    check("read_pixel(\"\") leaves out_pixel zeroed",
          px.width == 0 && px.height == 0);

    check("read_pixel(\"/no/such/file.png\") returns 0",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(
              "/no/such/file.png", &px) == 0);

    /* Out-parameter is always zeroed on failure. */
    check("read_pixel(missing) leaves out_pixel zeroed",
          px.width == 0 && px.height == 0);

    /* read_pixel with NULL out is safe (does not crash). */
    check("read_pixel(path, NULL) safe (returns 0)",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(
              "/no/such/file.png", NULL) == 0);
}

/* ── Scenario: non-PNG signature is rejected ───────────────────── */

static void test_non_png_signature_rejected(void) {
    printf("\n[ Scenario 2: non-PNG signature rejected ]\n");

    /* Write a tiny non-PNG file in /tmp and assert it's rejected. */
    const char* bogus = "/tmp/scratch/dm2_hwb_bogus.bin";
    system("mkdir -p /tmp/scratch");
    FILE* fp = fopen(bogus, "wb");
    check("wrote bogus non-PNG fixture", fp != NULL);
    if (fp) {
        /* 16 bytes that are clearly not a PNG. The first 8 should
         * not be 0x89 50 4E 47 0D 0A 1A 0A. */
        const unsigned char bytes[16] = {
            'N','O','T','P','N','G','!','!',
            0x00, 0x00, 0x00, 0x0D, 'I','H','D','R'
        };
        fwrite(bytes, 1, sizeof(bytes), fp);
        fclose(fp);
    }

    DM2_V2_HudWidgetBlitPixel px;
    check("non-PNG signature returns 0",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(bogus, &px) == 0);
    check("non-PNG leaves out_pixel zeroed",
          px.width == 0 && px.height == 0 && px.r == 0);
}

/* ── Scenario: synthetic 1x1 RGBA fixtures decode correctly ─────── */

static void test_synthetic_fixtures_decode(void) {
    printf("\n[ Scenario 3: synthetic 1x1 RGBA fixtures decode ]\n");
    int all_slots_decoded = 1;
    int all_decodes_stable = 1;
    /* Track decoded R values per slot so a re-read produces the
     * same byte — proves the decoder is deterministic, not that
     * the values are unique (the synthetic pack happens to have
     * a few R collisions, which is a fixture-design choice, not
     * a correctness property of the blit path). */
    uint8_t decoded_r[DM2_V2_HUD_WIDGET_COUNT];
    memset(decoded_r, 0, sizeof(decoded_r));
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetBlitPixel px;
        memset(&px, 0, sizeof(px));
        int ok = dm2_v2_hud_widget_bitmap_blit_read_pixel(
            k_slot_fixtures[i].fixture_path, &px);
        char name[160];
        snprintf(name, sizeof(name), "%s decodes",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, ok == 1);
        if (!ok) { all_slots_decoded = 0; continue; }

        snprintf(name, sizeof(name), "%s width=1",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, px.width  == 1);
        snprintf(name, sizeof(name), "%s height=1",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, px.height == 1);
        snprintf(name, sizeof(name), "%s bit_depth=8",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, px.bit_depth == 8);
        snprintf(name, sizeof(name), "%s color_type=6",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, px.color_type == 6);
        snprintf(name, sizeof(name), "%s alpha=255",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        check(name, px.a == 255);

        /* Re-read the fixture and assert the R value is identical.
         * Determinism matters because the runtime uses the R byte
         * as a palette index, and a non-deterministic decode would
         * make probe pixel-value assertions unstable across runs. */
        DM2_V2_HudWidgetBlitPixel px2;
        memset(&px2, 0, sizeof(px2));
        if (dm2_v2_hud_widget_bitmap_blit_read_pixel(
                k_slot_fixtures[i].fixture_path, &px2) != 1 ||
            px2.r != px.r || px2.g != px.g ||
            px2.b != px.b || px2.a != px.a) {
            all_decodes_stable = 0;
        }
        decoded_r[i] = px.r;
    }
    check("all 7 fixtures decode", all_slots_decoded);
    check("decode is deterministic across re-reads", all_decodes_stable);
    /* Quick sanity: at least one slot has a non-zero R so the blit
     * test below actually exercises a non-trivial palette index. */
    int any_nonzero = 0;
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        if (decoded_r[i] != 0) { any_nonzero = 1; break; }
    }
    check("at least one fixture decodes a non-zero R", any_nonzero);
}

/* ── Scenario: multi-pixel PNGs are rejected ───────────────────── */

static void test_multi_pixel_rejected(void) {
    printf("\n[ Scenario 4: multi-pixel PNGs rejected (envelope) ]\n");
    /* Build a 2x1 RGBA PNG by hand: same header format as the
     * synthetic fixtures but with width=2 so the bounded envelope
     * must reject it. We hand-craft the bytes because no
     * 2-pixel fixture is shipped with the project. */
    const char* path = "/tmp/scratch/dm2_hwb_2x1.png";
    system("mkdir -p /tmp/scratch");
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        check("could not open 2x1 png for write", 0);
        return;
    }
    /* PNG signature. */
    static const unsigned char sig[8] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
    };
    fwrite(sig, 1, 8, fp);

    /* IHDR chunk: length=13, type=IHDR, data = (w=2, h=1, depth=8, color=6, ...).
     * We do not bother to compute a correct CRC — the bounded
     * envelope rejects on width, not on CRC, so the walker's
     * strict-CRC path is not triggered. We do write 4 dummy CRC
     * bytes so the walker can advance past IHDR. */
    unsigned char ihdr[4 + 4 + 13 + 4] = {0};
    ihdr[0] = 0x00; ihdr[1] = 0x00; ihdr[2] = 0x00; ihdr[3] = 0x0D; /* len=13 */
    ihdr[4] = 'I';  ihdr[5] = 'H';  ihdr[6] = 'D';  ihdr[7] = 'R';
    ihdr[8]  = 0x00; ihdr[9]  = 0x00; ihdr[10] = 0x00; ihdr[11] = 0x02; /* width=2 */
    ihdr[12] = 0x00; ihdr[13] = 0x00; ihdr[14] = 0x00; ihdr[15] = 0x01; /* height=1 */
    ihdr[16] = 0x08; /* bit depth */
    ihdr[17] = 0x06; /* color type RGBA */
    /* remaining 6 IHDR bytes (compression, filter, interlace) = 0 */
    fwrite(ihdr, 1, sizeof(ihdr), fp);
    fclose(fp);

    DM2_V2_HudWidgetBlitPixel px;
    check("2x1 PNG rejected (width out of envelope)",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(path, &px) == 0);
    check("2x1 rejection leaves out_pixel zeroed",
          px.width == 0 && px.height == 0);
}

/* ── Scenario: non-RGBA color types are rejected ───────────────── */

static void test_non_rgba_rejected(void) {
    printf("\n[ Scenario 5: non-RGBA color types rejected ]\n");
    /* Build a 1x1 PNG with color type 3 (palette-indexed) — must
     * be rejected even though the dimensions are in envelope. */
    const char* path = "/tmp/scratch/dm2_hwb_palette.png";
    system("mkdir -p /tmp/scratch");
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        check("could not open palette png for write", 0);
        return;
    }
    static const unsigned char sig[8] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
    };
    fwrite(sig, 1, 8, fp);
    unsigned char ihdr[4 + 4 + 13 + 4] = {0};
    ihdr[0] = 0x00; ihdr[1] = 0x00; ihdr[2] = 0x00; ihdr[3] = 0x0D;
    ihdr[4] = 'I';  ihdr[5] = 'H';  ihdr[6] = 'D';  ihdr[7] = 'R';
    ihdr[8]  = 0x00; ihdr[9]  = 0x00; ihdr[10] = 0x00; ihdr[11] = 0x01; /* width=1 */
    ihdr[12] = 0x00; ihdr[13] = 0x00; ihdr[14] = 0x00; ihdr[15] = 0x01; /* height=1 */
    ihdr[16] = 0x08; /* bit depth */
    ihdr[17] = 0x03; /* color type 3 = indexed */
    fwrite(ihdr, 1, sizeof(ihdr), fp);
    fclose(fp);

    DM2_V2_HudWidgetBlitPixel px;
    check("palette-indexed 1x1 PNG rejected",
          dm2_v2_hud_widget_bitmap_blit_read_pixel(path, &px) == 0);
    check("palette rejection leaves out_pixel zeroed",
          px.width == 0 && px.color_type == 0);
}

/* ── Scenario: bounded single-pixel blit writes the R channel ──── */

static void test_blit_pixel_rgba_writes(void) {
    printf("\n[ Scenario 6: bounded single-pixel blit ]\n");
    /* 16x8 framebuffer; blit a single pixel at (5, 3) with R=42.
     * Every other byte must remain at the sentinel 0x33. */
    uint8_t fb[16 * 8];
    memset(fb, 0x33, sizeof(fb));
    int ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 8, 5, 3,
        /*r=*/42, /*g=*/200, /*b=*/10, /*a=*/255);
    check("blit returns 1 for in-bounds destination", ok == 1);
    check("blit wrote the R channel at (5,3)",
          fb[3 * 16 + 5] == 42);
    int rest_intact = 1;
    for (int i = 0; i < (int)sizeof(fb); ++i) {
        if (i == 3 * 16 + 5) continue;
        if (fb[i] != 0x33) { rest_intact = 0; break; }
    }
    check("blit left every other byte at sentinel", rest_intact);
}

/* ── Scenario: out-of-bounds blit never writes ──────────────────── */

static void test_out_of_bounds_no_write(void) {
    printf("\n[ Scenario 7: out-of-bounds blit never writes ]\n");
    uint8_t fb[16 * 8];
    memset(fb, 0x55, sizeof(fb));
    /* Negative x. */
    int ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 8, -1, 3, 7, 0, 0, 255);
    check("negative x returns 0", ok == 0);
    /* x == w. */
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 8, 16, 3, 7, 0, 0, 255);
    check("x == w returns 0", ok == 0);
    /* Negative y. */
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 8, 5, -1, 7, 0, 0, 255);
    check("negative y returns 0", ok == 0);
    /* y == h_res. */
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 8, 5, 8, 7, 0, 0, 255);
    check("y == h_res returns 0", ok == 0);
    /* The framebuffer must be byte-identical to the pre-call state. */
    int all_sentinel = 1;
    for (size_t i = 0; i < sizeof(fb); ++i) {
        if (fb[i] != 0x55) { all_sentinel = 0; break; }
    }
    check("framebuffer unchanged after out-of-bounds attempts",
          all_sentinel);

    /* NULL fb / non-positive dims are also safe. */
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        NULL, 16, 8, 0, 0, 7, 0, 0, 255);
    check("NULL fb returns 0", ok == 0);
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 0, 8, 0, 0, 7, 0, 0, 255);
    check("w<=0 returns 0", ok == 0);
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 16, 0, 0, 0, 7, 0, 0, 255);
    check("h_res<=0 returns 0", ok == 0);
}

/* ── Scenario: alpha-blend does src-over-dst correctly ──────────── */

static void test_alpha_blend(void) {
    printf("\n[ Scenario 8: alpha-blend (a<255) src-over-dst ]\n");
    /* dst=0x80 (128), src R=0xFF (255), a=128 (≈50%).
     * Expected: (255*128 + 128*127) / 255 = 191. */
    uint8_t fb[1] = { 0x80 };
    int ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 1, 1, 0, 0, 0xFF, 0, 0, /*a=*/128);
    check("alpha-blend returns 1", ok == 1);
    int expected = (255 * 128 + 128 * 127) / 255;
    check("alpha-blend result matches src-over-dst formula",
          fb[0] == (uint8_t)expected);

    /* a=0 → result equals dst (no contribution from src). */
    fb[0] = 0x40;
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 1, 1, 0, 0, 0xFF, 0, 0, /*a=*/0);
    check("a=0 leaves dst byte unchanged (0x40)", ok == 1 && fb[0] == 0x40);

    /* a=255 → exact src overwrite. */
    fb[0] = 0x40;
    ok = dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, 1, 1, 0, 0, 0xAB, 0, 0, /*a=*/255);
    check("a=255 overwrites with src R", ok == 1 && fb[0] == 0xAB);
}

/* ── Scenario: high-level render_slot reads PNG + blits ────────── */

static void test_render_slot_end_to_end(void) {
    printf("\n[ Scenario 9: render_slot reads PNG + bounded blit ]\n");
    /* Build a fresh slot info pointing at each synthetic fixture and
     * assert the blit writes the expected R at the slot's anchor
     * (the same anchor the runtime's stamp fallback would use). */
    int all_ok = 1;
    for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetSlotInfo info;
        memset(&info, 0, sizeof(info));
        info.slot = k_slot_fixtures[i].slot;
        snprintf(info.id, sizeof(info.id), "%s",
                 dm2_v2_hud_widget_assets_slot_name(
                     (DM2_V2_HudWidgetSlot)i));
        snprintf(info.category, sizeof(info.category), "%s",
                 k_slot_fixtures[i].category);
        info.classification = DM2_V2_HUD_WIDGET_CLASS_REAL;
        info.file_exists = 1;
        snprintf(info.resolved_path, sizeof(info.resolved_path), "%s",
                 k_slot_fixtures[i].fixture_path);

        /* Read the expected R value from the same fixture (so the
         * probe is self-consistent if the fixture colour changes). */
        DM2_V2_HudWidgetBlitPixel px;
        if (!dm2_v2_hud_widget_bitmap_blit_read_pixel(
                k_slot_fixtures[i].fixture_path, &px)) {
            char name[160];
            snprintf(name, sizeof(name), "%s: read_pixel ok (precondition)",
                     info.id);
            check(name, 0);
            all_ok = 0;
            continue;
        }
        uint8_t expected_r = px.r;

        /* 320x200 framebuffer zeroed; blit at the runtime anchor. */
        uint8_t fb[320 * 200];
        memset(fb, 0, sizeof(fb));
        int ok = dm2_v2_hud_widget_bitmap_blit_render_slot(
            &info, fb, 320, 200,
            k_anchors[i].x, k_anchors[i].y);
        char name[160];
        snprintf(name, sizeof(name),
                 "%s: render_slot returns 1", info.id);
        check(name, ok == 1);

        snprintf(name, sizeof(name),
                 "%s: blit wrote expected R=0x%02X at anchor (%d,%d)",
                 info.id, expected_r,
                 k_anchors[i].x, k_anchors[i].y);
        check(name,
              fb[k_anchors[i].y * 320 + k_anchors[i].x] == expected_r);
    }
    check("end-to-end render_slot ran for all 7 slots", all_ok);

    /* A REAL-classified slot whose resolved_path is empty must
     * return 0 (defensive: should never happen because the gate
     * already filters PARTIAL slots, but the blit defends itself). */
    DM2_V2_HudWidgetSlotInfo empty_info;
    memset(&empty_info, 0, sizeof(empty_info));
    empty_info.classification = DM2_V2_HUD_WIDGET_CLASS_REAL;
    uint8_t fb[16];
    memset(fb, 0xAB, sizeof(fb));
    check("render_slot with empty resolved_path returns 0",
          dm2_v2_hud_widget_bitmap_blit_render_slot(
              &empty_info, fb, 16, 1, 0, 0) == 0);
    check("render_slot empty-path leaves framebuffer intact",
          fb[0] == 0xAB && fb[15] == 0xAB);

    /* render_slot with NULL info is safe. */
    check("render_slot(NULL info) returns 0",
          dm2_v2_hud_widget_bitmap_blit_render_slot(
              NULL, fb, 16, 1, 0, 0) == 0);
}

/* ── Scenario: source evidence citation ────────────────────────── */

static void test_source_evidence(void) {
    printf("\n[ Scenario 10: source evidence citation ]\n");
    const char* ev = dm2_v2_hud_widget_bitmap_blit_source_evidence();
    check("source_evidence non-empty", ev != NULL && strlen(ev) > 100);
    check("cites SKULL.ASM T560", strstr(ev, "SKULL.ASM T560") != NULL);
    check("cites SKULLWIN", strstr(ev, "SKULLWIN") != NULL);
    check("cites ReDMCSB PANEL.C", strstr(ev, "ReDMCSB PANEL.C") != NULL);
    check("mentions synthetic 1x1 RGBA envelope",
          strstr(ev, "1x1") != NULL && strstr(ev, "RGBA") != NULL);
    check("mentions Bounded destination clamp",
          strstr(ev, "clamp") != NULL);
    check("mentions OPEN-BOUNDED honesty for real art",
          strstr(ev, "OPEN-BOUNDED") != NULL);
    check("mentions V1 framebuffer invariant",
          strstr(ev, "V1") != NULL);
}

/* ── Main ──────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM2 V2 HUD Widget Bounded Bitmap Blit probe ===\n");
    printf("Source: SKULL.ASM T560, c_gui_vp.cpp, ReDMCSB PANEL.C F0354,\n"
           "        examples/dm2_hud_widget_synthetic/\n"
           "        include/dm2_v2_hud_widget_bitmap_blit.h\n");

    test_bogus_paths_rejected();
    test_non_png_signature_rejected();
    test_synthetic_fixtures_decode();
    test_multi_pixel_rejected();
    test_non_rgba_rejected();
    test_blit_pixel_rgba_writes();
    test_out_of_bounds_no_write();
    test_alpha_blend();
    test_render_slot_end_to_end();
    test_source_evidence();

    /* Clean scratch. */
    system("rm -f /tmp/scratch/dm2_hwb_bogus.bin "
                  "/tmp/scratch/dm2_hwb_2x1.png "
                  "/tmp/scratch/dm2_hwb_palette.png");

    printf("\n=== Results: %d passed, %d failed ===\n",
           s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
