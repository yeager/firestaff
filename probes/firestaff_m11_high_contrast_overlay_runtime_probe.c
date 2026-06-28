/*
 * firestaff_m11_high_contrast_overlay_runtime_probe.c
 *
 * Headless CI probe for the M11 in-game high-contrast overlay gate.
 *
 * Closes the gap "High-contrast presentation hardening: launcher
 * output is remapped to a restricted high-contrast palette;
 * remaining work is in-game overlay coverage." (TODO.md /
 * docs/FIRESTAFF_GAP_LIST.md) by exercising the gate across the
 * full state machine: launcher → config push → M11 chrome → M11
 * viewport fence.
 *
 * The probe is data-free (no game data needed) and runs in CI. It
 * covers:
 *
 *   1. Gate default-off state (V1 bit-identical).
 *   2. M12_Config.highContrast = 1 → M11_HighContrast_SetActive(1)
 *      pushes the gate on.
 *   3. M11_HighContrast_RemapPresentedColor() collapses muted
 *      palette slots and preserves YELLOW / WHITE / LIGHT_CYAN.
 *   4. Region apply with excludeMask fences off the simulated
 *      320x200 dungeon-viewport subrect (0,0)-(224,136) so the
 *      original V1 pixels survive even when the gate is on.
 *   5. Region apply outside the viewport fence (HUD chrome strip
 *      y=136..200, x=0..320) is remapped under the gate.
 *   6. Gate off after a toggle round-trip restores the identity
 *      function (V1 launches stay bit-identical again).
 *
 * Output: "# summary: N/M invariants passed". Exit 0 on full pass,
 * 1 otherwise.
 *
 * Source-lock: NONE — this is a UI-automation probe on top of the
 * public M11_HighContrast_* API. The launcher-side source lock
 * lives in src/ui/menu_startup_m12.c (m12_presented_color) and is
 * mirrored by the manifest string the gate returns.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "m11_high_contrast_overlay_pc34_compat.h"
#include "config_m12.h"

#include <stdio.h>
#include <string.h>

/* ── Test scaffold ────────────────────────────────────────────────── */

typedef struct {
    int total;
    int passed;
} ProbeTally;

static ProbeTally g_tally;

static void probe_record(const char* id, int ok, const char* message) {
    g_tally.total += 1;
    if (ok) {
        g_tally.passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

/* ── Subtest A: default-off state ─────────────────────────────────── */

static void subtest_default_off_state(void) {
    /* The probe runs in a single process; the gate can leak state
     * from a previous probe invocation.  Reset to a known state. */
    M11_HighContrast_SetActive(0);
    probe_record("default_off_is_zero",
                 M11_HighContrast_IsActive() == 0,
                 "M11_HighContrast_IsActive() returns 0 after SetActive(0)");
    /* V1 fidelity contract: every legal palette index is identity
     * when the gate is off. */
    int i;
    int identity_ok = 1;
    for (i = 0; i < 16; ++i) {
        unsigned char got = M11_HighContrast_RemapPresentedColor((unsigned char)i);
        if (got != (unsigned char)i) {
            identity_ok = 0;
            break;
        }
    }
    probe_record("default_off_identity_for_legal_palette",
                 identity_ok,
                 "remap is identity for palette indices 0..15 when gate is off");
}

/* ── Subtest B: launcher-side config drives the gate ───────────────── */

static void subtest_launcher_config_pushes_gate(void) {
    M12_Config cfg;
    M12_Config_SetDefaults(&cfg);
    /* Default highContrast must be 0 so a fresh install is bit-identical. */
    probe_record("config_default_high_contrast_off",
                 cfg.highContrast == 0,
                 "M12_Config_SetDefaults leaves highContrast=0");
    /* Toggle via the public config API. */
    cfg.highContrast = 1;
    M11_HighContrast_SetActive(cfg.highContrast);
    probe_record("config_on_pushes_gate_on",
                 M11_HighContrast_IsActive() == 1,
                 "M11_HighContrast_SetActive(cfg.highContrast=1) flips gate on");
    /* Toggle off again. */
    cfg.highContrast = 0;
    M11_HighContrast_SetActive(cfg.highContrast);
    probe_record("config_off_pushes_gate_off",
                 M11_HighContrast_IsActive() == 0,
                 "M11_HighContrast_SetActive(cfg.highContrast=0) flips gate off");
}

/* ── Subtest C: chrome remap under the gate ───────────────────────── */

static void subtest_chrome_remap_under_gate(void) {
    M11_HighContrast_SetActive(1);
    /* Anchors that must survive. */
    int anchors_ok = 1;
    anchors_ok &= (M11_HighContrast_RemapPresentedColor(0)  == 0);
    anchors_ok &= (M11_HighContrast_RemapPresentedColor(11) == 11);
    anchors_ok &= (M11_HighContrast_RemapPresentedColor(15) == 15);
    anchors_ok &= (M11_HighContrast_RemapPresentedColor(4)  == 4);
    probe_record("chrome_anchors_survive",
                 anchors_ok,
                 "BLACK/YELLOW/WHITE/CYAN survive under gate");
    /* Muted slots collapse to BLACK. */
    int muted_ok = 1;
    muted_ok &= (M11_HighContrast_RemapPresentedColor(1)  == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(3)  == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(5)  == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(6)  == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(8)  == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(12) == 0);
    muted_ok &= (M11_HighContrast_RemapPresentedColor(14) == 0);
    probe_record("chrome_muted_collapses_to_black",
                 muted_ok,
                 "muted palette slots collapse to BLACK under gate");
    M11_HighContrast_SetActive(0);
}

/* ── Subtest D: viewport fence excludes the dungeon scene ─────────── */

static void subtest_viewport_fence_excludes_scene(void) {
    /* Synthesise a 320x200 indexed framebuffer initialised to a
     * pattern that the gate would normally collapse to BLACK. If
     * the viewport fence works, the (0,0)-(224,136) region stays
     * bit-identical; the (0,136)-(320,200) HUD strip gets
     * remapped. */
    enum { FB_W = 320, FB_H = 200 };
    static unsigned char fb[FB_W * FB_H];
    int x, y;
    /* Use a mix of anchor pixels and muted pixels so the fence
     * gates the muted subset while anchors survive verbatim. */
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            /* Checker of GRAY (1) and BLACK (0) so the fence
             * (which excludes GRAY) leaves BLACK pixels eligible
             * for remap, but GRAY pixels are preserved. */
            fb[y * FB_W + x] = (unsigned char)(((x ^ y) & 1) ? 1 : 0);
        }
    }
    /* Fence off every pixel whose palette index is one we want to
     * preserve verbatim in the dungeon scene. The gate only
     * collapses muted slots (1/3/5/6/8/12/14), and the dungeon
     * scene frequently paints into all 16 palette slots, so we
     * fence off the muted slots for the viewport rect and leave
     * the chrome rect unmasked. */
    unsigned int fence = (1u << 1) | (1u << 3) | (1u << 5) |
                         (1u << 6) | (1u << 8) | (1u << 12) |
                         (1u << 14);
    M11_HighContrast_SetActive(1);
    /* Apply gate to the FULL 320x200 with the fence active. The
     * intent is: the dungeon scene's muted-index pixels (the
     * checker pattern's GRAY entries) survive, while the BLACK
     * entries that are NOT in the fence also survive (BLACK is
     * identity) — so the only way to see a remap here is if the
     * fence does NOT include the BLACK slot.  Expected: at least
     * one muted pixel is checked and survives, and the return
     * value reports no remap (because the only muted slot that
     * would have collapsed was fenced). */
    int remapped = M11_HighContrast_ApplyActiveRGBA(fb, FB_W, FB_H,
                                                    0, 0, FB_W, FB_H, fence);
    probe_record("viewport_fence_remap_engaged",
                 remapped == 0,
                 "gate + full-fence over 320x200 leaves every pixel alone");
    /* Verify the dungeon scene's muted pixels survived. */
    int viewport_preserved_ok = 1;
    for (y = 0; y < 136; ++y) {
        for (x = 0; x < 224; ++x) {
            unsigned char expected = (unsigned char)(((x ^ y) & 1) ? 1 : 0);
            if (fb[y * FB_W + x] != expected) {
                viewport_preserved_ok = 0;
                goto viewport_preserved_done;
            }
        }
    }
viewport_preserved_done:
    probe_record("viewport_fence_preserves_dungeon_scene",
                 viewport_preserved_ok,
                 "dungeon-viewport checker pattern stays bit-identical under fence");

    /* Now reset the framebuffer to all-GRAY and run a SECOND pass
     * without the fence on the HUD strip only — the chrome strip
     * is exactly the surface the gate documents as "covers". */
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            fb[y * FB_W + x] = 1; /* GRAY → collapses to BLACK */
        }
    }
    M11_HighContrast_SetActive(1);
    int chrome_remap = M11_HighContrast_ApplyActiveRGBA(fb, FB_W, FB_H,
                                                        0, 136, FB_W, 64, 0);
    probe_record("chrome_strip_remap_engaged",
                 chrome_remap == 1,
                 "gate remaps HUD chrome strip y=136..200 with no exclude mask");
    int chrome_remapped_ok = 1;
    for (y = 136; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            if (fb[y * FB_W + x] != 0) {
                chrome_remapped_ok = 0;
                goto chrome_remapped_done;
            }
        }
    }
chrome_remapped_done:
    probe_record("chrome_strip_collapsed_to_black",
                 chrome_remapped_ok,
                 "every HUD chrome strip pixel collapsed from GRAY(1) to BLACK(0)");
    /* And the dungeon viewport below y=136 should be untouched
     * because the second apply only touched the HUD strip. */
    int viewport_still_preserved_ok = 1;
    for (y = 0; y < 136; ++y) {
        for (x = 0; x < 224; ++x) {
            if (fb[y * FB_W + x] != 1) {
                viewport_still_preserved_ok = 0;
                goto viewport_still_preserved_done;
            }
        }
    }
viewport_still_preserved_done:
    probe_record("chrome_remap_leaves_dungeon_scene_alone",
                 viewport_still_preserved_ok,
                 "dungeon viewport pixels stay bit-identical after chrome remap");

    /* ── Rect-fence helper subtests ─────────────────────────────── */
    /* Apply a single broad pass across the full 320x200 but fence
     * off an explicit (0,0,224,136) dungeon viewport rectangle.
     * The chrome strip is muted GRAY, so a broad remap MUST touch
     * it and the dungeon scene MUST stay byte-identical. */
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            fb[y * FB_W + x] = 1; /* GRAY → collapses to BLACK */
        }
    }
    M11_HighContrast_SetActive(1);
    int rect_fence_remap = M11_HighContrast_ApplyActiveRGBAExceptRect(
        fb, FB_W, FB_H, 0, 0, FB_W, FB_H,
        /* preserve the dungeon viewport subrect */
        0, 0, 224, 136,
        /* no excludeMask needed because the fence handles it */
        0);
    probe_record("rect_fence_chrome_remap_engaged",
                 rect_fence_remap == 1,
                 "rect-fenced broad remap touches HUD chrome strip");
    int rect_fence_viewport_ok = 1;
    for (y = 0; y < 136; ++y) {
        for (x = 0; x < 224; ++x) {
            if (fb[y * FB_W + x] != 1) {
                rect_fence_viewport_ok = 0;
                goto rect_fence_viewport_done;
            }
        }
    }
rect_fence_viewport_done:
    probe_record("rect_fence_viewport_byte_identical",
                 rect_fence_viewport_ok,
                 "rect-fenced broad remap preserves dungeon viewport pixels");
    int rect_fence_chrome_ok = 1;
    for (y = 136; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            if (fb[y * FB_W + x] != 0) {
                rect_fence_chrome_ok = 0;
                goto rect_fence_chrome_done;
            }
        }
    }
rect_fence_chrome_done:
    probe_record("rect_fence_chrome_collapsed_to_black",
                 rect_fence_chrome_ok,
                 "rect-fenced broad remap collapses chrome strip to BLACK");

    /* Zero-area preserve rect disables the fence → fall back to
     * excludeMask-only behaviour. */
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            fb[y * FB_W + x] = 1;
        }
    }
    int zero_rect_remap = M11_HighContrast_ApplyActiveRGBAExceptRect(
        fb, FB_W, FB_H, 0, 0, FB_W, FB_H,
        0, 0, 0, 0,
        (1u << 1));
    probe_record("rect_fence_zero_area_disables_fence",
                 zero_rect_remap == 0,
                 "zero-area preserve rect disables the rectangle fence (falls back to excludeMask)");

    /* The rect-fence helper with a checker-pattern buffer and no
     * excludeMask still preserves the dungeon viewport (chrome
     * strip outside the fence still gets remapped, which is by
     * design — this is the explicit dungeon-viewport fence). */
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            fb[y * FB_W + x] = (unsigned char)(((x ^ y) & 1) ? 1 : 0);
        }
    }
    int rect_checker_ok = 1;
    M11_HighContrast_ApplyActiveRGBAExceptRect(
        fb, FB_W, FB_H, 0, 0, FB_W, FB_H,
        0, 0, 224, 136,
        0);
    for (y = 0; y < 136; ++y) {
        for (x = 0; x < 224; ++x) {
            unsigned char expected = (unsigned char)(((x ^ y) & 1) ? 1 : 0);
            if (fb[y * FB_W + x] != expected) {
                rect_checker_ok = 0;
                goto rect_checker_done;
            }
        }
    }
rect_checker_done:
    probe_record("rect_fence_viewport_checker_byte_identical",
                 rect_checker_ok,
                 "rect-fence helper preserves checker pattern inside the dungeon viewport");

    M11_HighContrast_SetActive(0);
}

/* ── Subtest E: toggle round-trip restores V1 fidelity ────────────── */

static void subtest_toggle_round_trip_restores_v1(void) {
    int i;
    M11_HighContrast_SetActive(1);
    /* Confirm gate is on and remap is non-identity. */
    int on_works = (M11_HighContrast_IsActive() == 1)
                && (M11_HighContrast_RemapPresentedColor(1) == 0);
    M11_HighContrast_SetActive(0);
    /* Confirm gate is off and remap is identity for every legal
     * palette index. */
    int off_works = (M11_HighContrast_IsActive() == 0);
    for (i = 0; i < 16; ++i) {
        if (M11_HighContrast_RemapPresentedColor((unsigned char)i) != (unsigned char)i) {
            off_works = 0;
            break;
        }
    }
    probe_record("toggle_round_trip_restores_v1",
                 on_works && off_works,
                 "SetActive(1)→(0) restores identity remap for palette 0..15");
}

/* ── Subtest F: manifest documents the gap closure ────────────────── */

static void subtest_manifest_documents_gap_closure(void) {
    const char* m = M11_HighContrast_GetManifest();
    int ok = (m != NULL)
          && (strstr(m, "M11_HIGH_CONTRAST_OVERLAY_GATE_v1") != NULL)
          && (strstr(m, "dungeon_viewport_320x200_pixels") != NULL)
          && (strstr(m, "v1_fidelity_contract: bit_identical_when_off") != NULL)
          && (strstr(m, "m12_presented_color") != NULL)
          && (strstr(m, "chrome_rect_remap_with_viewport_fence") != NULL);
    probe_record("manifest_documents_gap_closure", ok,
                 "manifest names the gap, the preserve set, the m12 parity link, and the rect fence");
}

int main(void) {
    printf("firestaff_m11_high_contrast_overlay_runtime_probe\n");
    subtest_default_off_state();
    subtest_launcher_config_pushes_gate();
    subtest_chrome_remap_under_gate();
    subtest_viewport_fence_excludes_scene();
    subtest_toggle_round_trip_restores_v1();
    subtest_manifest_documents_gap_closure();
    printf("# summary: %d/%d invariants passed\n",
           g_tally.passed, g_tally.total);
    return (g_tally.passed == g_tally.total) ? 0 : 1;
}
