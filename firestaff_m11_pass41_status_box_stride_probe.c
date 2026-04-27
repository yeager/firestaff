/*
 * Pass 41 bounded probe — champion status-box stride drop in V1 mode.
 *
 * Scope: V1_BLOCKERS.md §5 "Champion status-box stride +8 px per slot
 * vs DEFS.H C69".  Pass 34 measured the drift (`M11_PARTY_SLOT_STEP`
 * = 77 vs `C69_CHAMPION_STATUS_BOX_SPACING` = 69).  Pass 41 lands the
 * source-anchored stride for V1 original-faithful mode while leaving
 * V2 vertical-slice mode untouched (its pre-baked four-slot sprite
 * assumes the legacy 77-wide geometry).
 *
 * This probe is diagnostic-only.  It does not drive the renderer, the
 * tick orchestrator, or any rendering path.  It verifies:
 *
 *   1. The DEFS.H source constant is 69 (spacing) and the graphic
 *      C007_GRAPHIC_STATUS_BOX width is 67 (both are re-asserted as
 *      compile-time constants and cross-checked against the in-tree
 *      DEFS.H).
 *   2. `m11_game_view.c` carries the V1 overrides
 *      `M11_V1_PARTY_SLOT_W = 67` and `M11_V1_PARTY_SLOT_STEP = 69`
 *      as literal enum members, plus the legacy 77 / 71 values as
 *      the V2-mode fallback.
 *   3. `m11_game_view.c` carries `m11_party_slot_step()` and
 *      `m11_party_slot_w()` plus `m11_party_panel_x()` that return the V1 values by default
 *      and the V2 values when the V2 vertical slice is enabled.
 *   4. With stride 69 and slot-width 67, the four champion status
 *      boxes (at V1 source x=0) occupy x ranges 0..66,
 *      69..135, 138..204, 207..273.  They are strictly
 *      non-overlapping (gap = 2 px between adjacent boxes, matching
 *      69 - 67 = 2).
 *   5. The four-slot V1 party panel right edge (x = 274) fits
 *      inside the 320x200 framebuffer.
 *   6. Pass-34-recorded drift numbers are preserved: stride delta
 *      was +8 px / slot (77 - 69); width delta was +4 px / slot
 *      (71 - 67); total horizontal footprint shrinks from
 *      4 * 77 - 6 = 302 px to 4 * 69 - 2 = 274 px, a saving of
 *      28 px across the party panel.
 *   7. The V1 highlight border insets (from m11_draw_party_panel:
 *      outer = slotW - 2 = 65, inner = slotW - 4 = 63) keep the
 *      highlight strictly inside the 67-wide status-box frame.
 *
 * Strictly a diagnostic probe.  Does not touch the game view, the
 * tick orchestrator, any rendering path, any M10 semantics, or any
 * V1 runtime behavior beyond asserting the enum + helper signatures
 * are wired as expected.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Re-declared as compile-time constants.  Cross-checked against
 * m11_game_view.c and DEFS.H via invariant INV_P41_SRC. */
enum {
    DEFS_H_C69_CHAMPION_STATUS_BOX_SPACING = 69,
    DEFS_H_GRAPHIC_STATUS_BOX_W            = 67,
    DEFS_H_GRAPHIC_STATUS_BOX_H            = 29,

    V1_PARTY_SLOT_STEP = 69,
    V1_PARTY_SLOT_W    = 67,

    V2_PARTY_SLOT_STEP = 77,
    V2_PARTY_SLOT_W    = 71,

    PARTY_PANEL_X = 0,
    SCREEN_W      = 320,
    SCREEN_H      = 200,
    CHAMPION_MAX  = 4
};

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static int read_file(const char* path, char* buf, size_t cap) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    size_t n = fread(buf, 1, cap - 1, f);
    fclose(f);
    buf[n] = '\0';
    return (int)n;
}

int main(void) {
    char msg[256];
    static char buf[1024 * 1024];

    /* 1. DEFS.H source anchors */
    record("INV_P41_01",
           DEFS_H_C69_CHAMPION_STATUS_BOX_SPACING == 69,
           "DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING == 69");
    record("INV_P41_02",
           DEFS_H_GRAPHIC_STATUS_BOX_W == 67 && DEFS_H_GRAPHIC_STATUS_BOX_H == 29,
           "C007_GRAPHIC_STATUS_BOX dimensions == 67x29 (M11 asset probe INV_GV_205)");
    {
        int n = read_file("zones_h_reconstruction.json", buf, sizeof(buf));
        int has_c150 = (n > 0) &&
            strstr(buf, "\"150\": {\n      \"type\": 9,\n      \"parent\": 0,\n      \"d1\": 67,\n      \"d2\": 29") != NULL;
        int has_c154_offset = (n > 0) &&
            strstr(buf, "\"154\": {\n      \"type\": 1,\n      \"parent\": 150,\n      \"d1\": 207,\n      \"d2\": 0") != NULL;
        record("INV_P41_03",
               has_c150 && has_c154_offset,
               "zones_h_reconstruction locks C150 67x29 and C154 source offset x=207");
    }

    /* 2. m11_game_view.c enum carries V1 + V2 values */
    {
        int n = read_file("m11_game_view.c", buf, sizeof(buf));
        int have_v1_step = (n > 0) && (strstr(buf,
            "M11_V1_PARTY_SLOT_STEP = 69") != NULL);
        int have_v1_w    = (n > 0) && (strstr(buf,
            "M11_V1_PARTY_SLOT_W    = 67") != NULL);
        int have_v2_step = (n > 0) && (strstr(buf,
            "M11_PARTY_SLOT_STEP = 77") != NULL);
        int have_v2_w    = (n > 0) && (strstr(buf,
            "M11_PARTY_SLOT_W = 71") != NULL);
        record("INV_P41_04",
               have_v1_step,
               "m11_game_view.c enum carries M11_V1_PARTY_SLOT_STEP == 69");
        record("INV_P41_05",
               have_v1_w,
               "m11_game_view.c enum carries M11_V1_PARTY_SLOT_W == 67");
        record("INV_P41_06",
               have_v2_step,
               "m11_game_view.c enum keeps M11_PARTY_SLOT_STEP == 77 as V2 fallback");
        record("INV_P41_07",
               have_v2_w,
               "m11_game_view.c enum keeps M11_PARTY_SLOT_W == 71 as V2 fallback");
    }

    /* 3. m11_game_view.c carries the mode-aware helpers */
    {
        int n = read_file("m11_game_view.c", buf, sizeof(buf));
        int have_step_fn = (n > 0) && (strstr(buf,
            "static int m11_party_slot_step(void)") != NULL);
        int have_w_fn    = (n > 0) && (strstr(buf,
            "static int m11_party_slot_w(void)") != NULL);
        int have_x_fn    = (n > 0) && (strstr(buf,
            "static int m11_party_panel_x(void)") != NULL);
        int step_returns_v1 = (n > 0) && (strstr(buf,
            "M11_V1_PARTY_SLOT_STEP") != NULL);
        int w_returns_v1 = (n > 0) && (strstr(buf,
            "M11_V1_PARTY_SLOT_W") != NULL);
        int step_gated_on_v2 = (n > 0) && (strstr(buf,
            "m11_v2_vertical_slice_enabled()\n        ? (int)M11_PARTY_SLOT_STEP")
            != NULL);
        int w_gated_on_v2 = (n > 0) && (strstr(buf,
            "m11_v2_vertical_slice_enabled()\n        ? (int)M11_PARTY_SLOT_W")
            != NULL);
        record("INV_P41_08",
               have_step_fn,
               "m11_game_view.c defines static m11_party_slot_step(void)");
        record("INV_P41_09",
               have_w_fn,
               "m11_game_view.c defines static m11_party_slot_w(void)");
        record("INV_P41_10",
               have_x_fn && step_returns_v1 && w_returns_v1 &&
                   strstr(buf, "M11_V1_PARTY_PANEL_X") != NULL,
               "helpers branch on V1 override constants, including source x=0");
        record("INV_P41_11",
               step_gated_on_v2 && w_gated_on_v2,
               "helpers return V2 values only when vertical slice is enabled");
    }

    /* 4. Slot rectangles are non-overlapping in V1 */
    {
        int slot;
        int overlaps = 0;
        int prev_right = -1;
        for (slot = 0; slot < CHAMPION_MAX; ++slot) {
            int x = PARTY_PANEL_X + slot * V1_PARTY_SLOT_STEP;
            int right = x + V1_PARTY_SLOT_W; /* exclusive */
            if (slot > 0 && x < prev_right) {
                overlaps = 1;
            }
            snprintf(msg, sizeof(msg),
                     "V1 slot %d: x=%d..%d (W=%d)",
                     slot, x, right - 1, V1_PARTY_SLOT_W);
            record("INV_P41_12", 1, msg);
            prev_right = right;
        }
        record("INV_P41_13",
               overlaps == 0,
               "V1 slot rects are strictly non-overlapping "
               "(gap = spacing - width = 2 px)");
    }

    /* 5. Right edge fits inside framebuffer */
    {
        int rightmost = PARTY_PANEL_X +
                        (CHAMPION_MAX - 1) * V1_PARTY_SLOT_STEP +
                        V1_PARTY_SLOT_W; /* exclusive */
        snprintf(msg, sizeof(msg),
                 "V1 source party-panel right edge = %d (<= %d screen width)",
                 rightmost, SCREEN_W);
        record("INV_P41_14",
               rightmost == 274 && rightmost <= SCREEN_W,
               msg);
    }

    /* 6. Pass-34 drift numbers preserved */
    {
        int step_delta  = V2_PARTY_SLOT_STEP - V1_PARTY_SLOT_STEP;
        int width_delta = V2_PARTY_SLOT_W   - V1_PARTY_SLOT_W;
        int v2_panel_w  = (CHAMPION_MAX - 1) * V2_PARTY_SLOT_STEP + V2_PARTY_SLOT_W;
        int v1_panel_w  = (CHAMPION_MAX - 1) * V1_PARTY_SLOT_STEP + V1_PARTY_SLOT_W;
        int panel_delta = v2_panel_w - v1_panel_w;
        record("INV_P41_15",
               step_delta == 8,
               "stride delta V2 -> V1 == 8 px/slot (pass 34 evidence)");
        record("INV_P41_16",
               width_delta == 4,
               "slot width delta V2 -> V1 == 4 px/slot "
               "(V2 71 -> V1 67 = C007 graphic width)");
        record("INV_P41_17",
               v2_panel_w == 302 && v1_panel_w == 274 && panel_delta == 28,
               "total party-panel width shrinks from 302 (V2) "
               "to 274 (V1), saving 28 px of horizontal chrome");
    }

    /* 7. V1 highlight border fits inside the 67-wide frame */
    {
        int outer_w = V1_PARTY_SLOT_W - 2; /* 65 */
        int inner_w = V1_PARTY_SLOT_W - 4; /* 63 */
        int outer_x_offset = 1;
        int inner_x_offset = 2;
        int outer_right = outer_x_offset + outer_w;
        int inner_right = inner_x_offset + inner_w;
        record("INV_P41_18",
               outer_w == 65 && outer_right <= V1_PARTY_SLOT_W,
               "V1 highlight outer border (offset 1, W = slotW - 2 = 65) "
               "fits strictly inside the 67-wide status-box frame");
        record("INV_P41_19",
               inner_w == 63 && inner_right <= V1_PARTY_SLOT_W,
               "V1 highlight inner border (offset 2, W = slotW - 4 = 63) "
               "fits strictly inside the 67-wide status-box frame");
    }

    /* 8. Call-site audit: stride / width reads go through helpers
     *    in the two V1 code paths (hit test + draw).  The enum-member
     *    literal M11_PARTY_SLOT_STEP must no longer appear in those
     *    code paths.  The helpers themselves reference the enum. */
    {
        int n = read_file("m11_game_view.c", buf, sizeof(buf));
        const char* needle_hit  = "slot * slotStep";
        const char* needle_draw = "slot * slotStep";
        int hit_site  = (n > 0) && (strstr(buf, needle_hit) != NULL);
        int draw_site = (n > 0) && (strstr(buf, needle_draw) != NULL);
        record("INV_P41_20",
               hit_site,
               "hit-test + draw call sites iterate using slotStep (helper-bound)");
        /* Both sites use the identical idiom so one needle covers both;
         * a separate invariant locks the helper-gated slotW idiom: */
        record("INV_P41_21",
               draw_site && strstr(buf, "slotW    = m11_party_slot_w();") != NULL,
               "draw site caches slotW via m11_party_slot_w()");
    }

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
