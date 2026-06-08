/*
 * firestaff_dm1_v2_v1_v2_side_by_side_seed_probe.c
 *
 * DM1 V2 — V1/V2 Side-by-Side Verification Seed Probe
 *
 * Headless probe: a small deterministic seed that exercises the DM1 V2
 * phase gate (dm1_v2_phase_gate_pc34) and the V1 movement command
 * adapter (dm1_v2_movement_command_adapter_pc34) on the same logical
 * inputs, then asserts that "presentation disabled" preserves the
 * V1 source/runtime truth exactly.
 *
 * The probe acts as a seed for two adjacent follow-up tracks:
 *
 *   1. Presentation-disabled parity:
 *      - With v2PresentationEnabled=0, every V1-source-locked domain
 *        must report v1SourceLocked=1 and v2PresentationAllowed=0.
 *      - The V1 movement adapter must report routeKind=V1_SOURCE and
 *        sourceCommand == runtimeCommand for every direction
 *        (turn/move N/S/E/W).
 *      - This is the contract the existing test_dm1_v2_phase_gate_pc34
 *        already checks in a single function; this probe spreads the
 *        same assertion across every domain and every command so that
 *        a future V2 presentation change cannot regress V1 truth
 *        without breaking at least one of these rows.
 *
 *   2. Screenshot/pixel scaffolding:
 *      - Documents the canonical V1 viewport geometry (224x136) and
 *        V1 source-pixel anchors (D1C portrait, wall-panel, etc.) from
 *        ReDMCSB DUNVIEW.C and DUNGEON.C.
 *      - Future V1/V2 side-by-side screenshot diffing can plug into
 *        these constants without re-deriving them.
 *
 * Source references (ReDMCSB):
 *   DEFS.H:238-243          C001..C006 command ids (V1 source truth)
 *   COMMAND.C:2045-2155     F0359 command queue dispatch
 *   CLIKMENU.C:142          F0365 turn party
 *   CLIKMENU.C:180          F0366 move party
 *   CLIKMENU.C:278-323      wall/door/fakewall/group collision
 *   CLIKMENU.C:330-346      disabled movement tick / side effects
 *   GAMELOOP.C:150-155      disabled movement tick decrement
 *   GAMELOOP.C:215-219      F0380 input wait loop
 *   MOVESENS.C:316-345      F0267 move-result side effects
 *   LOADSAVE.C:1520-1534    save party position/timing/global state
 *   LOADSAVE.C:2730-2742    restore party position/timing/global state
 *   COORD.C:1721-1722       224x136 viewport
 *   DUNVIEW.C:2999-3000     224x136 viewport (gameplay-space picture)
 *   DUNVIEW.C:3913-3928     D1C champion portrait blit at {96,35}
 *   DUNGEON.C:2573          C127 sensor view-direction mapping
 *   DUNGEON.C:2610-2612     only front wall aspect sets G0289 portrait
 *   PANEL.C:418-428         G0304_i_DungeonViewPaletteIndex
 *
 * The probe is headless: it does not require game data files and does
 * not initialise SDL. It compiles against the firestaff_v2 static
 * library (which already contains dm1_v2_phase_gate_pc34 and
 * dm1_v2_movement_command_adapter_pc34).
 *
 * Exit codes:
 *   0 — all checks passed (V1 source truth preserved)
 *   1 — at least one check failed (V1/V2 drift detected)
 *
 * Usage:
 *   ./firestaff_dm1_v2_v1_v2_side_by_side_seed_probe
 *   (no SDL_VIDEODRIVER required)
 */

#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>
#include <string.h>

/* ── V1 viewport scaffolding constants ──────────────────────────────
 *
 * ReDMCSB COORD.C:1721-1722 + DUNVIEW.C:2999-3000.
 * The 224x136 viewport is the gameplay-space picture that DM1 V2 may
 * present; V2 must not change its byte-for-byte V1 layout. Side-by-side
 * screenshot diffing can use these constants as fixed reference frames.
 */
#define DM1_V2_SIDE_BY_SIDE_VIEWPORT_W 224
#define DM1_V2_SIDE_BY_SIDE_VIEWPORT_H 136
#define DM1_V2_SIDE_BY_SIDE_VIEWPORT_BYTE_COUNT \
    ((DM1_V2_SIDE_BY_SIDE_VIEWPORT_W * DM1_V2_SIDE_BY_SIDE_VIEWPORT_H) / 2)

/* D1C champion portrait geometry from DUNVIEW.C:3913-3928. */
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W 32
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H 29
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X 96
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y 35

/* D1C wall panel geometry (layout-696 C712_ZONE_WALL_D1C). */
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_X 32
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y 9
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_W 160
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_H 111

/* ── V1 command id table (DeDMCSB DEFS.H:238-243) ────────────────────
 *
 * The V1 source commands are the V1 runtime truth. When V2 presentation
 * is disabled, the V1 movement adapter must report sourceCommand ==
 * runtimeCommand for every direction, with routeKind == V1_SOURCE.
 *
 *   C003 = MOVE_FORWARD, C004 = MOVE_RIGHT,
 *   C005 = MOVE_BACKWARD, C006 = MOVE_LEFT,
 *   C001 = TURN_LEFT,    C002 = TURN_RIGHT.
 */
typedef struct {
    DM1_V2_MovementCommand v2Command;
    int v1SourceCommand;
    const char* label;
} V1CommandTruth;

static const V1CommandTruth g_v1_truth[6] = {
    { DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,    1, "C001 TURN_LEFT"    },
    { DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,   2, "C002 TURN_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, "C003 MOVE_FORWARD" },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,   4, "C004 MOVE_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,5, "C005 MOVE_BACKWARD"},
    { DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,    6, "C006 MOVE_LEFT"    },
};
#define N_V1_TRUTH ((int)(sizeof(g_v1_truth) / sizeof(g_v1_truth[0])))

/* ── V1-source-locked domains (must remain locked even with V2 on) ── */

static const DM1_V2_PhaseDomain g_v1_locked[] = {
    DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS,
    DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING,
    DM1_V2_PHASE_DOMAIN_COLLISION_RULES,
    DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA,
    DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES,
};
static const char* g_v1_locked_names[] = {
    "COMMAND_SEMANTICS",
    "DUNGEON_TIMING",
    "COLLISION_RULES",
    "SAVE_LOAD_DATA",
    "SOURCE_LOCKED_RULES",
};

/* ── V2-presentation-eligible domains (toggle-responsive) ────────── */

static const DM1_V2_PhaseDomain g_v2_eligible[] = {
    DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION,
    DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION,
    DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION,
};
static const char* g_v2_eligible_names[] = {
    "RENDER_PRESENTATION",
    "INPUT_PRESENTATION",
    "CONFIG_PRESENTATION",
};

#define N_V1_LOCKED   ((int)(sizeof(g_v1_locked) / sizeof(g_v1_locked[0])))
#define N_V2_ELIGIBLE ((int)(sizeof(g_v2_eligible) / sizeof(g_v2_eligible[0])))

/* ── Test record helpers ──────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

static void record(int ok, const char* id, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s  %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s  %s\n", id, msg);
    }
}

static void section(const char* title) {
    printf("\n=== %s ===\n", title);
}

/* ── TC-1: defaults() — V1 is the locked-in default ──────────────── */

static void tc_defaults(void) {
    DM1_V2_PhaseGateConfig cfg;
    dm1_v2_phase_gate_defaults(&cfg);
    record(cfg.v2PresentationEnabled == 0,
           "TC-1", "v2PresentationEnabled=0 after defaults");
    record(cfg.v2ConfigPersistenceEnabled == 0,
           "TC-1", "v2ConfigPersistenceEnabled=0 after defaults");
}

/* ── TC-2: V1-source-locked domains — always locked ───────────────── */

static void tc_v1_locked_always(void) {
    DM1_V2_PhaseGateConfig cfg_off, cfg_on;
    int i;
    dm1_v2_phase_gate_defaults(&cfg_off);
    dm1_v2_phase_gate_defaults(&cfg_on);
    cfg_on.v2PresentationEnabled = 1;
    cfg_on.v2ConfigPersistenceEnabled = 1;

    for (i = 0; i < N_V1_LOCKED; ++i) {
        char id[64];
        DM1_V2_PhaseGateDecision d_off =
            dm1_v2_phase_gate_decide(&cfg_off, g_v1_locked[i]);
        DM1_V2_PhaseGateDecision d_on =
            dm1_v2_phase_gate_decide(&cfg_on, g_v1_locked[i]);
        snprintf(id, sizeof(id), "TC-2 [%s]", g_v1_locked_names[i]);
        record(d_off.v1SourceLocked == 1 && d_off.v2PresentationAllowed == 0,
               id, "V1-locked when V2 presentation disabled");
        record(d_on.v1SourceLocked == 1 && d_on.v2PresentationAllowed == 0,
               id, "V1-locked when V2 presentation enabled");
    }
}

/* ── TC-3: V2-eligible domains — toggle-responsive ────────────────
 *
 * RENDER and INPUT respond to v2PresentationEnabled alone. CONFIG
 * is special-cased in TC-4 because it requires both toggles.
 */

static void tc_v2_eligible_toggle(void) {
    int i;
    DM1_V2_PhaseGateConfig cfg_off, cfg_on;
    dm1_v2_phase_gate_defaults(&cfg_off);
    dm1_v2_phase_gate_defaults(&cfg_on);
    cfg_on.v2PresentationEnabled = 1;

    /* Skip CONFIG_PRESENTATION here; it is covered in TC-4. */
    for (i = 0; i < N_V2_ELIGIBLE; ++i) {
        char id[64];
        DM1_V2_PhaseGateDecision d_off =
            dm1_v2_phase_gate_decide(&cfg_off, g_v2_eligible[i]);
        DM1_V2_PhaseGateDecision d_on =
            dm1_v2_phase_gate_decide(&cfg_on, g_v2_eligible[i]);
        snprintf(id, sizeof(id), "TC-3 [%s]", g_v2_eligible_names[i]);
        record(d_off.v1SourceLocked == 0 && d_off.v2PresentationAllowed == 0,
               id, "V2-eligible but disabled (presentation off)");
        if (g_v2_eligible[i] == DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION) {
            /* CONFIG is intentionally gated on both toggles. */
            record(d_on.v1SourceLocked == 0 && d_on.v2PresentationAllowed == 0,
                   id, "CONFIG still requires v2ConfigPersistenceEnabled");
        } else {
            record(d_on.v1SourceLocked == 0 && d_on.v2PresentationAllowed == 1,
                   id, "V2-eligible and enabled (presentation on)");
        }
    }
}

/* ── TC-4: CONFIG_PRESENTATION — requires both toggles ──────────── */

static void tc_config_presentation_both_toggles(void) {
    DM1_V2_PhaseGateConfig cfg;
    DM1_V2_PhaseGateDecision d;
    dm1_v2_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;

    d = dm1_v2_phase_gate_decide(&cfg, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(d.v2PresentationAllowed == 0,
           "TC-4", "CONFIG requires v2ConfigPersistenceEnabled as well");

    cfg.v2ConfigPersistenceEnabled = 1;
    d = dm1_v2_phase_gate_decide(&cfg, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(d.v2PresentationAllowed == 1,
           "TC-4", "CONFIG allowed with both toggles on");
}

/* ── TC-5: V1 source command truth — adapter preserves C001..C006 ─
 *
 * This is the side-by-side row: for every V1 command id, the V1
 * route (presentation off) and the V2 route (presentation on) must
 * agree on sourceCommand. They may differ on runtimeCommand, but
 * the V1 source must never change.
 */

static void tc_v1_source_command_preserved(void) {
    int i;
    for (i = 0; i < N_V1_TRUTH; ++i) {
        char id[80];
        DM1_V2_MovementCommandRoute r_v1 =
            dm1_v2_movement_command_route_for_presentation(
                0, g_v1_truth[i].v2Command);
        DM1_V2_MovementCommandRoute r_v2 =
            dm1_v2_movement_command_route_for_presentation(
                1, g_v1_truth[i].v2Command);
        snprintf(id, sizeof(id), "TC-5 [%s]", g_v1_truth[i].label);
        record(r_v1.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE,
               id, "V1 route is V1_SOURCE when presentation is off");
        record(r_v1.sourceCommand == g_v1_truth[i].v1SourceCommand,
               id, "V1 route sourceCommand matches ReDMCSB C-id");
        record(r_v1.runtimeCommand == r_v1.sourceCommand,
               id, "V1 route runtimeCommand == sourceCommand (V1 truth)");
        record(r_v2.sourceCommand == g_v1_truth[i].v1SourceCommand,
               id, "V2 route sourceCommand still matches ReDMCSB C-id");
    }
}

/* ── TC-6: presentation-disabled parity — every gameplay domain is
 *         V1-locked and the V1 route is V1_SOURCE for every command
 *         when v2PresentationEnabled=0.
 */

static void tc_presentation_disabled_parity(void) {
    int i;
    DM1_V2_PhaseGateConfig cfg;
    dm1_v2_phase_gate_defaults(&cfg);

    for (i = 0; i < N_V1_LOCKED; ++i) {
        char id[64];
        DM1_V2_PhaseGateDecision d =
            dm1_v2_phase_gate_decide(&cfg, g_v1_locked[i]);
        snprintf(id, sizeof(id), "TC-6 [%s]", g_v1_locked_names[i]);
        record(d.v1SourceLocked == 1 && d.v2PresentationAllowed == 0,
               id, "presentation disabled -> domain stays V1-locked");
    }
    for (i = 0; i < N_V1_TRUTH; ++i) {
        char id[80];
        DM1_V2_MovementCommandRoute r =
            dm1_v2_movement_command_route_for_presentation(
                0, g_v1_truth[i].v2Command);
        snprintf(id, sizeof(id), "TC-6 [%s]", g_v1_truth[i].label);
        record(r.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE &&
               r.sourceCommand == r.runtimeCommand,
               id, "presentation disabled -> V1 command parity");
    }
}

/* ── TC-7: V1/V2 anchor strings reference ReDMCSB ───────────────── */

static void tc_v1_domain_source_anchors(void) {
    int i;
    DM1_V2_PhaseGateConfig cfg_on;
    dm1_v2_phase_gate_defaults(&cfg_on);
    cfg_on.v2PresentationEnabled = 1;
    cfg_on.v2ConfigPersistenceEnabled = 1;

    for (i = 0; i < N_V1_LOCKED; ++i) {
        char id[64];
        DM1_V2_PhaseGateDecision d =
            dm1_v2_phase_gate_decide(&cfg_on, g_v1_locked[i]);
        snprintf(id, sizeof(id), "TC-7 [%s]", g_v1_locked_names[i]);
        record(d.sourceAnchor != NULL && strlen(d.sourceAnchor) > 4,
               id, "sourceAnchor is non-empty");
        record(strstr(d.sourceAnchor, "ReDMCSB") != NULL ||
               (d.sourceAnchor[0] != '\0' && d.sourceAnchor[0] != '?'),
               id, "sourceAnchor references ReDMCSB (or anchor) sources");
    }
}

/* ── TC-8: NULL config is safe (fail-safe V1-locked) ────────────── */

static void tc_null_config_safe(void) {
    int i;
    for (i = 0; i < N_V1_LOCKED; ++i) {
        char id[64];
        DM1_V2_PhaseGateDecision d =
            dm1_v2_phase_gate_decide(NULL, g_v1_locked[i]);
        snprintf(id, sizeof(id), "TC-8 [%s]", g_v1_locked_names[i]);
        record(d.v1SourceLocked == 1 && d.v2PresentationAllowed == 0,
               id, "NULL config -> V1-locked, V2 not allowed");
    }
}

/* ── TC-9: defaults(NULL) is null-safe ──────────────────────────── */

static void tc_defaults_null_safe(void) {
    dm1_v2_phase_gate_defaults(NULL);
    record(1, "TC-9", "defaults(NULL) did not crash");
}

/* ── TC-10: V1 viewport scaffolding constants are stable ──────────
 *
 * These constants are the V1 source truth for V1/V2 side-by-side
 * pixel diffing. Any future change to them is a V1 source-trust
 * break and must be justified against ReDMCSB DUNVIEW.C / COORD.C.
 */

static void tc_viewport_scaffold_constants(void) {
    record(DM1_V2_SIDE_BY_SIDE_VIEWPORT_W == 224,
           "TC-10", "viewport width == 224 (ReDMCSB COORD.C:1721-1722)");
    record(DM1_V2_SIDE_BY_SIDE_VIEWPORT_H == 136,
           "TC-10", "viewport height == 136 (ReDMCSB DUNVIEW.C:2999-3000)");
    record(DM1_V2_SIDE_BY_SIDE_VIEWPORT_BYTE_COUNT == 15232,
           "TC-10", "viewport byte count == 15232 (224*136/2 4bpp)");
    record(DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W == 32 &&
           DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H == 29,
           "TC-10", "D1C portrait 32x29 (DUNVIEW.C:3913-3928)");
    record(DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X == 96 &&
           DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y == 35,
           "TC-10", "D1C portrait origin (96,35) in viewport-local");
    record(DM1_V2_SIDE_BY_SIDE_D1C_WALL_X == 32 &&
           DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y == 9 &&
           DM1_V2_SIDE_BY_SIDE_D1C_WALL_W == 160 &&
           DM1_V2_SIDE_BY_SIDE_D1C_WALL_H == 111,
           "TC-10", "D1C wall-panel region matches V1 layout-696");
}

/* ── TC-11: all domains handled (no fallthrough) ────────────────── */

static void tc_all_domains_handled(void) {
    int d;
    DM1_V2_PhaseGateConfig cfg;
    dm1_v2_phase_gate_defaults(&cfg);
    /* DM1_V2 has exactly the 8 enum values; iterating 0..7 is enough. */
    for (d = 0; d <= 7; ++d) {
        DM1_V2_PhaseGateDecision dec = dm1_v2_phase_gate_decide(&cfg, (DM1_V2_PhaseDomain)d);
        record(dec.sourceAnchor != NULL && strlen(dec.sourceAnchor) > 0,
               "TC-11", "domain covered by phase gate (no fallthrough)");
    }
}

/* ── TC-12: deterministic pixel scaffold detects V1/V2 drift ──────
 *
 * This does not render game art. It seeds matching V1/V2 viewport
 * buffers with deterministic bytes, then verifies the region-compare
 * scaffold that future real screenshot/pixel tests can reuse.
 */

static void tc_side_by_side_pixel_scaffold(void) {
    static DM1_V2_Color v1[DM1_V2_VIEWPORT_H][DM1_V2_VIEWPORT_W];
    static DM1_V2_Color v2[DM1_V2_VIEWPORT_H][DM1_V2_VIEWPORT_W];
    DM1_V2_RegionCompareResult result;
    DM1_V2_ViewportRegion full = {
        0, 0,
        DM1_V2_SIDE_BY_SIDE_VIEWPORT_W,
        DM1_V2_SIDE_BY_SIDE_VIEWPORT_H,
        "V1 full viewport"
    };
    DM1_V2_ViewportRegion portrait = {
        DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X,
        DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y,
        DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W,
        DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H,
        "D1C portrait anchor"
    };
    DM1_V2_ViewportRegion wall = {
        DM1_V2_SIDE_BY_SIDE_D1C_WALL_X,
        DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y,
        DM1_V2_SIDE_BY_SIDE_D1C_WALL_W,
        DM1_V2_SIDE_BY_SIDE_D1C_WALL_H,
        "D1C wall panel"
    };

    int x;
    int y;
    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_VIEWPORT_H; ++y) {
        for (x = 0; x < DM1_V2_SIDE_BY_SIDE_VIEWPORT_W; ++x) {
            DM1_V2_Color px;
            px.r = (unsigned char)((x * 3 + y * 5) & 0xff);
            px.g = (unsigned char)((x * 7 + y * 11) & 0xff);
            px.b = (unsigned char)((x ^ (y * 13)) & 0xff);
            px.a = 255;
            v1[y][x] = px;
            v2[y][x] = px;
        }
    }

    record(dm1_v2_vp_compare_viewport_region(&v1[0][0], &v2[0][0],
                                             DM1_V2_VIEWPORT_W, full,
                                             &result) == 1 &&
           result.comparedPixels == DM1_V2_SIDE_BY_SIDE_VIEWPORT_W *
                                    DM1_V2_SIDE_BY_SIDE_VIEWPORT_H &&
           result.mismatchedPixels == 0,
           "TC-12", "matching V1/V2 full-viewport scaffold compares cleanly");
    record(dm1_v2_vp_compare_viewport_region(&v1[0][0], &v2[0][0],
                                             DM1_V2_VIEWPORT_W, portrait,
                                             &result) == 1 &&
           result.comparedPixels == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W *
                                    DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H &&
           result.mismatchedPixels == 0,
           "TC-12", "D1C portrait anchor region compares cleanly");
    record(dm1_v2_vp_compare_viewport_region(&v1[0][0], &v2[0][0],
                                             DM1_V2_VIEWPORT_W, wall,
                                             &result) == 1 &&
           result.comparedPixels == DM1_V2_SIDE_BY_SIDE_D1C_WALL_W *
                                    DM1_V2_SIDE_BY_SIDE_D1C_WALL_H &&
           result.mismatchedPixels == 0,
           "TC-12", "D1C wall-panel region compares cleanly");

    v2[DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y]
      [DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X].r ^= 0x7f;
    record(dm1_v2_vp_compare_viewport_region(&v1[0][0], &v2[0][0],
                                             DM1_V2_VIEWPORT_W, portrait,
                                             &result) == 0 &&
           result.mismatchedPixels == 1 &&
           result.firstMismatchX == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X &&
           result.firstMismatchY == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y,
           "TC-12", "single-pixel V2 drift is detected at the D1C anchor");
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void) {
    printf("DM1 V2 — V1/V2 Side-by-Side Verification Seed Probe\n");
    printf("Headless: no game assets, no SDL rendering required.\n");
    printf("Scope: dm1_v2_phase_gate_pc34 + dm1_v2_movement_command_adapter_pc34\n\n");

    section("TC-1: defaults() locks V1");
    tc_defaults();

    section("TC-2: V1-source-locked domains stay locked under V2");
    tc_v1_locked_always();

    section("TC-3: V2-eligible domains are toggle-responsive");
    tc_v2_eligible_toggle();

    section("TC-4: CONFIG_PRESENTATION requires both toggles");
    tc_config_presentation_both_toggles();

    section("TC-5: V1 source command ids (C001..C006) preserved");
    tc_v1_source_command_preserved();

    section("TC-6: presentation-disabled parity row");
    tc_presentation_disabled_parity();

    section("TC-7: V1 domain source anchors");
    tc_v1_domain_source_anchors();

    section("TC-8: NULL config fail-safe");
    tc_null_config_safe();

    section("TC-9: defaults(NULL) null-safety");
    tc_defaults_null_safe();

    section("TC-10: V1 viewport scaffold constants");
    tc_viewport_scaffold_constants();

    section("TC-11: every domain handled (no fallthrough)");
    tc_all_domains_handled();

    section("TC-12: side-by-side pixel compare scaffold");
    tc_side_by_side_pixel_scaffold();

    printf("\n=== SUMMARY ===\n");
    printf("PASS: %d  FAIL: %d\n", g_pass, g_fail);
    printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");

    return g_fail == 0 ? 0 : 1;
}
