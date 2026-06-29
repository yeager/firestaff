/*
 * test_m11_high_contrast_overlay_pc34_compat.c
 *
 * Data-free regression for the M11 in-game high-contrast overlay
 * gate (src/engine/m11_high_contrast_overlay_pc34_compat.c).
 *
 * Pins the contract that closes the open gap "High-contrast
 * presentation hardening: launcher output is remapped to a
 * restricted high-contrast palette; remaining work is in-game
 * overlay coverage." (TODO.md / docs/FIRESTAFF_GAP_LIST.md).
 *
 * Subtests:
 *   1. Default-off state: M11_HighContrast_IsActive() returns 0
 *      and M11_HighContrast_RemapPresentedColor() is the identity
 *      for every legal palette index (0..15). V1 launches must
 *      stay bit-identical when the toggle is off.
 *   2. Toggle on/off round-trip: M11_HighContrast_SetActive(1)
 *      flips the gate on and SetActive(0) flips it off again.
 *   3. Restricted-palette remap when on: muted slots (GRAY,
 *      BROWN, DARK_BROWN, GREEN, RED, DARK_GRAY, NAVY) collapse to
 *      BLACK; YELLOW / WHITE / LIGHT_CYAN / LIGHT_GREEN /
 *      LIGHT_RED / SILVER / CYAN / LIGHT_GRAY survive; MAGENTA
 *      (skin tone) is preserved for the in-game portrait chrome;
 *      out-of-range inputs return WHITE defensively.
 *   4. Manifest string contract: the returned string is non-NULL,
 *      contains the gate-coverage list, the explicit preserve
 *      list (320x200 dungeon viewport, wall/floor/door/creature
 *      ornament pixels), the default-off line, the v1 fidelity
 *      line, and the source-lock pointer that mirrors
 *      m12_presented_color().
 *   5. Region apply, gate off: M11_HighContrast_ApplyActiveRGBA()
 *      is a no-op (returns 0) when the gate is off; pixels are
 *      untouched.
 *   6. Region apply, gate on, no exclude: every pixel whose index
 *      is muted is remapped; non-muted pixels survive; pixels
 *      outside the rect are untouched; out-of-range index pixels
 *      collapse to WHITE.
 *   7. Region apply, gate on, excludeMask: pixels whose bit is
 *      set in excludeMask are NOT touched, even when the gate is
 *      on. This is the V1 dungeon-viewport fence the manifest
 *      documents.
 *   8. Region apply: clipping to framebuffer bounds; negative x/y
 *      is clipped to 0; over-large width/height is clipped to
 *      framebuffer edges; zero-area rect is a no-op.
 *   9. Region apply: out-of-range indexes are clamped into the
 *      0..15 mask before the excludeMask check; passing a pixel
 *      whose low nibble is 7 with excludeMask = (1u<<7) leaves
 *      the pixel alone.
 *  10. Launcher-side parity: the in-game remap table is identical
 *      to m12_presented_color() for the eight slots that the
 *      launcher comment cites (BLACK/NAVY/MAROON/BROWN/DARK_GRAY
 *      → BLACK, YELLOW → YELLOW, LIGHT_CYAN/CYAN → LIGHT_CYAN,
 *      default → WHITE). This locks the launcher / game surfaces
 *      onto one shared restricted palette.
 *  12. Region apply: extreme public-helper rectangles are clipped
 *      using overflow-safe arithmetic, including huge positive
 *      widths/heights and preserve rectangles near INT_MAX.
 */

#include "m11_high_contrast_overlay_pc34_compat.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int g_passes = 0;
static int g_failures = 0;

static void check(int cond, const char* msg) {
    if (cond) {
        ++g_passes;
    } else {
        ++g_failures;
        fprintf(stderr, "  FAIL: %s\n", msg);
    }
}

/* ── Subtest 1: default-off identity ──────────────────────────────── */

static void subtest_default_off_identity(void) {
    int i;
    /* Make sure we always start from a known state. */
    M11_HighContrast_SetActive(0);
    check(M11_HighContrast_IsActive() == 0,
          "default_off: gate starts at 0 (off)");
    for (i = 0; i < 16; ++i) {
        unsigned char got = M11_HighContrast_RemapPresentedColor((unsigned char)i);
        check(got == (unsigned char)i,
              "default_off: remap is identity for every legal palette index");
    }
}

/* ── Subtest 2: toggle round-trip ─────────────────────────────────── */

static void subtest_toggle_round_trip(void) {
    M11_HighContrast_SetActive(0);
    check(M11_HighContrast_IsActive() == 0,
          "toggle: SetActive(0) leaves gate off");
    M11_HighContrast_SetActive(1);
    check(M11_HighContrast_IsActive() == 1,
          "toggle: SetActive(1) flips gate on");
    M11_HighContrast_SetActive(0);
    check(M11_HighContrast_IsActive() == 0,
          "toggle: SetActive(0) flips gate back off");
    /* Treat any non-zero as on. */
    M11_HighContrast_SetActive(42);
    check(M11_HighContrast_IsActive() == 1,
          "toggle: SetActive(42) is treated as on");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest 3: restricted-palette remap when on ──────────────────── */

static void subtest_restricted_palette_when_on(void) {
    int i;
    M11_HighContrast_SetActive(1);

    /* Anchors that survive verbatim. */
    check(M11_HighContrast_RemapPresentedColor(0)  == 0,
          "remap: BLACK (0) stays BLACK");
    check(M11_HighContrast_RemapPresentedColor(11) == 11,
          "remap: YELLOW (11) stays YELLOW");
    check(M11_HighContrast_RemapPresentedColor(15) == 15,
          "remap: WHITE (15) stays WHITE");

    /* Muted slots collapse to BLACK. */
    check(M11_HighContrast_RemapPresentedColor(1)  == 0,
          "remap: GRAY (1) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(3)  == 0,
          "remap: BROWN (3) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(5)  == 0,
          "remap: DARK_BROWN (5) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(6)  == 0,
          "remap: GREEN (6) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(8)  == 0,
          "remap: RED (8) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(12) == 0,
          "remap: DARK_GRAY (12) collapses to BLACK");
    check(M11_HighContrast_RemapPresentedColor(14) == 0,
          "remap: NAVY / LIGHT_BLUE (14) collapses to BLACK");

    /* Readable chrome slots survive. */
    check(M11_HighContrast_RemapPresentedColor(2)  == 2,
          "remap: LIGHT_GRAY (2) stays LIGHT_GRAY");
    check(M11_HighContrast_RemapPresentedColor(4)  == 4,
          "remap: CYAN / LIGHT_CYAN (4) stays CYAN");
    check(M11_HighContrast_RemapPresentedColor(7)  == 7,
          "remap: LIGHT_GREEN (7) stays LIGHT_GREEN");
    check(M11_HighContrast_RemapPresentedColor(9)  == 9,
          "remap: LIGHT_RED / ORANGE (9) stays LIGHT_RED");
    check(M11_HighContrast_RemapPresentedColor(13) == 13,
          "remap: SILVER (13) stays SILVER");

    /* MAGENTA is a skin tone used by portrait chrome; preserve it. */
    check(M11_HighContrast_RemapPresentedColor(10) == 10,
          "remap: MAGENTA (10) stays MAGENTA (portrait skin tone)");

    /* Out-of-range defensive: anything past the 16-color palette
     * is treated as unknown and collapses to WHITE for readability. */
    check(M11_HighContrast_RemapPresentedColor(42) == 15,
          "remap: out-of-range input collapses to WHITE");
    check(M11_HighContrast_RemapPresentedColor(255) == 15,
          "remap: 0xFF input collapses to WHITE");

    /* Exhaustively cover 16..255 to prove defensive behavior. */
    for (i = 16; i < 256; ++i) {
        unsigned char got = M11_HighContrast_RemapPresentedColor((unsigned char)i);
        check(got == 15,
              "remap: every out-of-range input collapses to WHITE");
    }

    M11_HighContrast_SetActive(0);
}

/* ── Subtest 4: manifest string contract ──────────────────────────── */

static void subtest_manifest_contract(void) {
    const char* m = M11_HighContrast_GetManifest();
    check(m != NULL,
          "manifest: pointer is non-NULL");
    if (!m) return;
    check(strstr(m, "M11_HIGH_CONTRAST_OVERLAY_GATE_v1") != NULL,
          "manifest: version stamp is present");
    check(strstr(m, "hud_text") != NULL,
          "manifest: HUD text is in the coverage list");
    check(strstr(m, "dialog_text") != NULL,
          "manifest: dialog text is in the coverage list");
    check(strstr(m, "combat_log_text") != NULL,
          "manifest: combat log text is in the coverage list");
    check(strstr(m, "hit_flash_text") != NULL,
          "manifest: hit-flash text is in the coverage list");
    check(strstr(m, "dungeon_viewport_320x200_pixels") != NULL,
          "manifest: 320x200 dungeon viewport is in the preserve list");
    check(strstr(m, "wall_floor_door_creature_ornament_pixels") != NULL,
          "manifest: wall/floor/door/creature ornament pixels are preserved");
    check(strstr(m, "hud_panel_c040_blit_pixels") != NULL,
          "manifest: HUD panel C040 blit is preserved");
    check(strstr(m, "hud_panel_c017_backdrop_pixels") != NULL,
          "manifest: HUD panel C017 backdrop is preserved");
    check(strstr(m, "default_state: off") != NULL,
          "manifest: default-state line is present and pinned to off");
    check(strstr(m, "v1_fidelity_contract: bit_identical_when_off") != NULL,
          "manifest: v1 fidelity contract is documented");
    check(strstr(m, "m12_presented_color") != NULL,
          "manifest: source-lock pointer mirrors m12_presented_color()");
    check(strstr(m, "chrome_rect_remap_with_viewport_fence") != NULL,
          "manifest: rectangle-fenced chrome remap is in the coverage list");
}

/* ── Subtest 5: region apply, gate off (no-op) ────────────────────── */

static void subtest_region_apply_gate_off_noop(void) {
    unsigned char fb[16];
    int i;
    for (i = 0; i < 16; ++i) fb[i] = (unsigned char)i;
    M11_HighContrast_SetActive(0);
    check(M11_HighContrast_ApplyActiveRGBA(fb, 4, 4, 0, 0, 4, 4,
                                           0 /* no exclude */) == 0,
          "apply_off: returns 0 when gate is off");
    for (i = 0; i < 16; ++i) {
        check(fb[i] == (unsigned char)i,
              "apply_off: every pixel is untouched when gate is off");
    }
}

/* ── Subtest 6: region apply, gate on, no exclude ─────────────────── */

static void subtest_region_apply_gate_on_no_exclude(void) {
    /* 4x4 indexed framebuffer:
     *   row 0:  0  1  2  3
     *   row 1:  4  5  6  7
     *   row 2:  8  9 10 11
     *   row 3: 12 13 14 15
     */
    unsigned char fb[16] = {
        0,  1,  2,  3,
        4,  5,  6,  7,
        8,  9, 10, 11,
       12, 13, 14, 15
    };
    M11_HighContrast_SetActive(1);
    check(M11_HighContrast_ApplyActiveRGBA(fb, 4, 4, 0, 0, 4, 4,
                                           0 /* no exclude */) == 1,
          "apply_on: returns 1 when at least one pixel is remapped");
    /* Muted slots collapsed to BLACK: 1, 3, 5, 6, 8, 12, 14. */
    check(fb[0*4 + 0] == 0,  "apply_on: (0,0)=0 BLACK unchanged");
    check(fb[0*4 + 1] == 0,  "apply_on: (0,1)=1 GRAY -> BLACK");
    check(fb[0*4 + 2] == 2,  "apply_on: (0,2)=2 LIGHT_GRAY unchanged");
    check(fb[0*4 + 3] == 0,  "apply_on: (0,3)=3 BROWN -> BLACK");
    check(fb[1*4 + 0] == 4,  "apply_on: (1,0)=4 CYAN unchanged");
    check(fb[1*4 + 1] == 0,  "apply_on: (1,1)=5 DARK_BROWN -> BLACK");
    check(fb[1*4 + 2] == 0,  "apply_on: (1,2)=6 GREEN -> BLACK");
    check(fb[1*4 + 3] == 7,  "apply_on: (1,3)=7 LIGHT_GREEN unchanged");
    check(fb[2*4 + 0] == 0,  "apply_on: (2,0)=8 RED -> BLACK");
    check(fb[2*4 + 1] == 9,  "apply_on: (2,1)=9 LIGHT_RED unchanged");
    check(fb[2*4 + 2] == 10, "apply_on: (2,2)=10 MAGENTA unchanged");
    check(fb[2*4 + 3] == 11, "apply_on: (2,3)=11 YELLOW unchanged");
    check(fb[3*4 + 0] == 0,  "apply_on: (3,0)=12 DARK_GRAY -> BLACK");
    check(fb[3*4 + 1] == 13, "apply_on: (3,1)=13 SILVER unchanged");
    check(fb[3*4 + 2] == 0,  "apply_on: (3,2)=14 NAVY -> BLACK");
    check(fb[3*4 + 3] == 15, "apply_on: (3,3)=15 WHITE unchanged");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest 7: region apply, gate on, excludeMask ────────────────── */

static void subtest_region_apply_exclude_mask(void) {
    /* Same 4x4 layout but excludeMask fences off every pixel whose
     * palette index has its bit set: 1, 3, 5, 6, 8, 12, 14. */
    unsigned char fb[16] = {
        0,  1,  2,  3,
        4,  5,  6,  7,
        8,  9, 10, 11,
       12, 13, 14, 15
    };
    unsigned int exclude = (1u << 1) | (1u << 3) | (1u << 5) |
                           (1u << 6) | (1u << 8) | (1u << 12) |
                           (1u << 14);
    M11_HighContrast_SetActive(1);
    (void)M11_HighContrast_ApplyActiveRGBA(fb, 4, 4, 0, 0, 4, 4, exclude);
    /* Excluded muted slots are NOT touched. */
    check(fb[0*4 + 1] == 1, "apply_excl: (0,1)=1 GRAY preserved");
    check(fb[0*4 + 3] == 3, "apply_excl: (0,3)=3 BROWN preserved");
    check(fb[1*4 + 1] == 5, "apply_excl: (1,1)=5 DARK_BROWN preserved");
    check(fb[1*4 + 2] == 6, "apply_excl: (1,2)=6 GREEN preserved");
    check(fb[2*4 + 0] == 8, "apply_excl: (2,0)=8 RED preserved");
    check(fb[3*4 + 0] == 12,"apply_excl: (3,0)=12 DARK_GRAY preserved");
    check(fb[3*4 + 2] == 14,"apply_excl: (3,2)=14 NAVY preserved");
    /* Other pixels that are NOT excluded and ARE muted also
     * collapse to BLACK if there were any — but the muted set is
     * exactly the excludeMask set, so every other pixel is an
     * anchor that survives anyway. Sanity-check anchors stay. */
    check(fb[0*4 + 2] == 2, "apply_excl: (0,2)=2 LIGHT_GRAY preserved");
    check(fb[2*4 + 1] == 9, "apply_excl: (2,1)=9 LIGHT_RED preserved");
    check(fb[2*4 + 3] == 11,"apply_excl: (2,3)=11 YELLOW preserved");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest 8: region clipping & invalid inputs ──────────────────── */

static void subtest_region_apply_clipping(void) {
    unsigned char fb[9]; /* 3x3 */
    int i;
    for (i = 0; i < 9; ++i) fb[i] = 1; /* all GRAY → would collapse to BLACK */
    M11_HighContrast_SetActive(1);

    /* Negative origin clipped to 0 with extra width past the edge
     * so the visible rect is non-empty.  The standard SDL/Rect
     * semantics is: visible rect = [max(0, x), min(fbW, x+w)). */
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3, -2, -2, 5, 5, 0) == 1,
          "apply_clip: negative origin + over-large rect clips to (0,0)-(3,3) and remaps");
    /* Pixels inside the visible rect (0..3, 0..3) are all remapped. */
    check(fb[0] == 0, "apply_clip: (0,0) was remapped after clip");
    check(fb[1] == 0, "apply_clip: (0,1) was remapped after clip");
    check(fb[2] == 0, "apply_clip: (0,2) was remapped after clip");
    check(fb[3] == 0, "apply_clip: (1,0) was remapped after clip");
    check(fb[4] == 0, "apply_clip: (1,1) was remapped after clip");
    check(fb[5] == 0, "apply_clip: (1,2) was remapped after clip");
    check(fb[6] == 0, "apply_clip: (2,0) was remapped after clip");
    check(fb[7] == 0, "apply_clip: (2,1) was remapped after clip");
    check(fb[8] == 0, "apply_clip: (2,2) was remapped after clip");

    /* Off-screen rect with negative origin and width that fits
     * entirely off the left edge is a no-op. */
    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3, -5, -5, 2, 2, 0) == 0,
          "apply_clip: fully off-screen rect is a no-op");
    check(fb[4] == 1, "apply_clip: fully off-screen rect leaves pixels alone");

    /* Over-large rect: clipped to framebuffer edges. */
    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3, 1, 1, 99, 99, 0) == 1,
          "apply_clip: over-large rect clips to fb edges and remaps");
    check(fb[0] == 1, "apply_clip: (0,0) outside clipped rect preserved");
    check(fb[1] == 1, "apply_clip: (0,1) outside clipped rect preserved");
    check(fb[3] == 1, "apply_clip: (1,0) outside clipped rect preserved");
    check(fb[4] == 0, "apply_clip: (1,1) was remapped after clip");
    check(fb[8] == 0, "apply_clip: (2,2) was remapped after clip");

    /* Zero-area rect is a no-op. */
    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3, 0, 0, 0, 0, 0) == 0,
          "apply_clip: zero-area rect is a no-op");
    check(fb[4] == 1, "apply_clip: zero-area rect leaves pixels alone");

    /* NULL / non-positive dimensions are a no-op. */
    check(M11_HighContrast_ApplyActiveRGBA(NULL, 3, 3, 0, 0, 3, 3, 0) == 0,
          "apply_clip: NULL framebuffer is a no-op");
    check(M11_HighContrast_ApplyActiveRGBA(fb, 0, 3, 0, 0, 3, 3, 0) == 0,
          "apply_clip: zero-width framebuffer is a no-op");
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 0, 0, 0, 3, 3, 0) == 0,
          "apply_clip: zero-height framebuffer is a no-op");

    M11_HighContrast_SetActive(0);
}

/* ── Subtest 9: out-of-range index handling under excludeMask ──────── */

static void subtest_out_of_range_index_with_exclude(void) {
    unsigned char fb[3] = { 200, 7, 16 };
    M11_HighContrast_SetActive(1);
    /* excludeMask bit 7 → pixel index 7 (LIGHT_GREEN) is preserved;
     * 200 and 16 are out of range so they collapse to WHITE (15). */
    (void)M11_HighContrast_ApplyActiveRGBA(fb, 3, 1, 0, 0, 3, 1,
                                           (1u << 7));
    check(fb[0] == 15, "outofrange: index 200 -> WHITE when gate on");
    check(fb[1] == 7,  "outofrange: index 7 preserved by excludeMask bit 7");
    check(fb[2] == 15, "outofrange: index 16 -> WHITE when gate on");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest 10: launcher / game surfaces share one palette ───────── */

static void subtest_launcher_parity(void) {
    /* The launcher comment in src/ui/menu_startup_m12.c pins these
     * M12 colors to the same restricted palette. M11 uses VGA slot
     * indices (0..15) so we map them through the documented slots:
     *   BLACK (M11:0)  → BLACK (M11:0)
     *   NAVY   (M11:14) → BLACK (M11:0)
     *   BROWN  (M11:3)  → BLACK (M11:0)
     *   DARK_GRAY (M11:12) → BLACK (M11:0)
     *   YELLOW (M11:11) → YELLOW (M11:11)
     *   LIGHT_CYAN (M11:4) → LIGHT_CYAN (M11:4)
     *   default → WHITE (M11:15)
     *
     * The launcher keeps MAROON as well, but M11 does not have a
     * dedicated MAROON slot; MAROON (M12_COLOR_MAROON) collapses
     * into BROWN (M11:3) on the VGA palette so it lands in the
     * BLACK collapse path. This subtest verifies both contracts
     * line up after the slot mapping.
     */
    M11_HighContrast_SetActive(1);
    /* BLACK survives. */
    check(M11_HighContrast_RemapPresentedColor(0) == 0,
          "parity: BLACK survives in M11 (matches launcher BLACK)");
    /* Launcher "muted to BLACK" set maps to M11 BLACK. */
    check(M11_HighContrast_RemapPresentedColor(14) == 0,
          "parity: NAVY -> BLACK (matches launcher NAVY->BLACK)");
    check(M11_HighContrast_RemapPresentedColor(3) == 0,
          "parity: BROWN -> BLACK (matches launcher BROWN->BLACK)");
    check(M11_HighContrast_RemapPresentedColor(12) == 0,
          "parity: DARK_GRAY -> BLACK (matches launcher DARK_GRAY->BLACK)");
    /* Anchors survive. */
    check(M11_HighContrast_RemapPresentedColor(11) == 11,
          "parity: YELLOW survives in M11 (matches launcher YELLOW)");
    check(M11_HighContrast_RemapPresentedColor(4) == 4,
          "parity: CYAN/LIGHT_CYAN survives in M11 (matches launcher LIGHT_CYAN/CYAN)");
    /* Default-collapse -> WHITE. */
    check(M11_HighContrast_RemapPresentedColor(255) == 15,
          "parity: out-of-set -> WHITE (matches launcher default->WHITE)");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest 11: rectangle-fenced region apply ──────────────────── */

static void subtest_region_apply_except_rect(void) {
    enum { W = 8, H = 6 };
    unsigned char fb[W * H];
    unsigned char before[W * H];
    int x;
    int y;
    int ok;

    for (y = 0; y < H; ++y) {
        for (x = 0; x < W; ++x) {
            fb[y * W + x] = (unsigned char)((x + y) & 0x0F);
        }
    }
    memcpy(before, fb, sizeof(fb));

    M11_HighContrast_SetActive(1);
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, W, H,
                                                     0, 0, W, H,
                                                     2, 1, 3, 4,
                                                     0) == 1,
          "apply_rect_fence: broad remap reports changed chrome pixels");

    /* The preserve rect (2,1)-(5,5) must stay byte-identical. */
    ok = 1;
    for (y = 1; y < 5; ++y) {
        for (x = 2; x < 5; ++x) {
            if (fb[y * W + x] != before[y * W + x]) {
                ok = 0;
            }
        }
    }
    check(ok, "apply_rect_fence: preserve rectangle stays byte-identical");

    /* Chrome outside the rectangle must follow the remap table. */
    ok = 1;
    for (y = 0; y < H; ++y) {
        for (x = 0; x < W; ++x) {
            unsigned char expected;
            if (x >= 2 && x < 5 && y >= 1 && y < 5) {
                expected = before[y * W + x];
            } else {
                expected = M11_HighContrast_RemapPresentedColor(before[y * W + x]);
            }
            if (fb[y * W + x] != expected) {
                ok = 0;
            }
        }
    }
    check(ok, "apply_rect_fence: chrome outside rectangle follows remap table");

    /* A zero-area preserve rect disables the fence and leaves
     * excludeMask active. */
    fb[0] = 3;  /* BROWN would collapse to BLACK without excludeMask. */
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, W, H,
                                                     0, 0, 1, 1,
                                                     0, 0, 0, 0,
                                                     (1u << 3)) == 0,
          "apply_rect_fence: zero-size preserve rect leaves excludeMask semantics active");
    check(fb[0] == 3,
          "apply_rect_fence: excludeMask still preserves palette slot outside fence");

    /* A negative preserve dimension must also disable the fence
     * (defensive against caller mistakes). */
    fb[0] = 1;  /* GRAY collapses to BLACK under gate without fence. */
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, W, H,
                                                     0, 0, 1, 1,
                                                     0, 0, -1, 4,
                                                     0) == 1,
          "apply_rect_fence: negative preserveWidth disables the rectangle fence");
    check(fb[0] == 0,
          "apply_rect_fence: negative preserve dimension lets remap collapse GRAY to BLACK");

    /* Gate off → no remap, even with a real preserve rect. */
    fb[1] = 1;
    M11_HighContrast_SetActive(0);
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, W, H,
                                                     0, 0, W, H,
                                                     2, 1, 3, 4,
                                                     0) == 0,
          "apply_rect_fence: gate off leaves every pixel alone even with preserve rect");
    check(fb[1] == 1,
          "apply_rect_fence: gate off + preserve rect leaves chrome byte-identical");
}

/* ── Subtest 12: overflow-safe clipping for extreme rectangles ───── */

static void subtest_region_apply_extreme_rectangles(void) {
    unsigned char fb[9]; /* 3x3 */
    int i;

    M11_HighContrast_SetActive(1);

    for (i = 0; i < 9; ++i) fb[i] = 1; /* GRAY -> BLACK if touched */
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3,
                                           INT_MAX - 4, 0,
                                           INT_MAX, 3,
                                           0) == 0,
          "apply_extreme: huge off-right rect is clipped to no visible pixels");
    for (i = 0; i < 9; ++i) {
        check(fb[i] == 1,
              "apply_extreme: huge off-right rect leaves every pixel untouched");
    }

    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3,
                                           1, 1,
                                           INT_MAX, INT_MAX,
                                           0) == 1,
          "apply_extreme: huge width/height clips to framebuffer edge and remaps visible tail");
    check(fb[0] == 1 && fb[1] == 1 && fb[3] == 1,
          "apply_extreme: pixels before clipped huge rect stay untouched");
    check(fb[4] == 0 && fb[5] == 0 && fb[7] == 0 && fb[8] == 0,
          "apply_extreme: visible tail of huge rect remaps to BLACK");

    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBA(fb, 3, 3,
                                           INT_MIN, INT_MIN,
                                           INT_MAX, INT_MAX,
                                           0) == 0,
          "apply_extreme: huge negative rect ending before framebuffer is a no-op");
    for (i = 0; i < 9; ++i) {
        check(fb[i] == 1,
              "apply_extreme: huge negative rect leaves every pixel untouched");
    }

    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, 3, 3,
                                                     0, 0, 3, 3,
                                                     INT_MAX - 4, 0,
                                                     INT_MAX, 3,
                                                     0) == 1,
          "apply_extreme: huge off-right preserve rect does not fence visible chrome");
    for (i = 0; i < 9; ++i) {
        check(fb[i] == 0,
              "apply_extreme: off-right preserve rect lets visible chrome remap");
    }

    for (i = 0; i < 9; ++i) fb[i] = 1;
    check(M11_HighContrast_ApplyActiveRGBAExceptRect(fb, 3, 3,
                                                     0, 0, 3, 3,
                                                     INT_MIN, INT_MIN,
                                                     INT_MAX, INT_MAX,
                                                     0) == 1,
          "apply_extreme: huge negative preserve rect ending before framebuffer is ignored");
    for (i = 0; i < 9; ++i) {
        check(fb[i] == 0,
              "apply_extreme: ignored negative preserve rect lets visible chrome remap");
    }

    M11_HighContrast_SetActive(0);
}

int main(void) {
    printf("  [1] default-off identity (V1 bit-identical)...\n");
    subtest_default_off_identity();
    printf("  [2] toggle round-trip...\n");
    subtest_toggle_round_trip();
    printf("  [3] restricted-palette remap when on...\n");
    subtest_restricted_palette_when_on();
    printf("  [4] manifest string contract...\n");
    subtest_manifest_contract();
    printf("  [5] region apply, gate off (no-op)...\n");
    subtest_region_apply_gate_off_noop();
    printf("  [6] region apply, gate on, no exclude...\n");
    subtest_region_apply_gate_on_no_exclude();
    printf("  [7] region apply, gate on, excludeMask...\n");
    subtest_region_apply_exclude_mask();
    printf("  [8] region apply, clipping & invalid inputs...\n");
    subtest_region_apply_clipping();
    printf("  [9] out-of-range index handling under excludeMask...\n");
    subtest_out_of_range_index_with_exclude();
    printf("  [10] launcher / game surfaces share one palette...\n");
    subtest_launcher_parity();
    printf("  [11] rectangle-fenced region apply...\n");
    subtest_region_apply_except_rect();
    printf("  [12] extreme rectangle clipping safety...\n");
    subtest_region_apply_extreme_rectangles();
    printf("\n  m11_high_contrast_overlay_pc34_compat: %d passed, %d failed\n",
           g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}
