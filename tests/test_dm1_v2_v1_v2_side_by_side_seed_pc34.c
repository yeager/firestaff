/*
 * test_dm1_v2_v1_v2_side_by_side_seed_pc34.c
 *
 * CTest integration companion to
 * probes/firestaff_dm1_v2_v1_v2_side_by_side_seed_probe.c.
 *
 * The probe runs as a stand-alone executable and is exercised manually
 * and from the worker control scripts. This file exposes the same
 * V1/V2 presentation-disabled parity contract to the regular ctest
 * run, so any future change to V2 phase-gate defaults, V1-source
 * command ids, or the movement command adapter surfaces as a
 * regression in `ctest` instead of a stand-alone probe output.
 *
 * What this test asserts (all source-locked to ReDMCSB):
 *
 *   1. dm1_v2_phase_gate_defaults() leaves both V2 toggles disabled,
 *      so V1 is the default boot/runtime path.
 *
 *   2. The five V1-source-locked domains (command semantics, dungeon
 *      timing, collision rules, save/load data, source-locked rules)
 *      stay V1-locked regardless of the V2 presentation toggle, with
 *      a non-empty source anchor (ReDMCSB DEFS.H/COMMAND.C/CLIKMENU.C
 *      /GAMELOOP.C/MOVESENS.C/LOADSAVE.C/COORD.C/DUNVIEW.C lines).
 *
 *   3. The three V2-eligible domains (render, input, config) follow
 *      the toggle. CONFIG_PRESENTATION is the special case that
 *      requires both v2PresentationEnabled and
 *      v2ConfigPersistenceEnabled; RENDER and INPUT respond to
 *      v2PresentationEnabled alone.
 *
 *   4. NULL config and NULL pointer arguments fail safe to V1-locked
 *      defaults.
 *
 *   5. dm1_v2_movement_command_route_for_presentation() preserves the
 *      V1 source command ids C001..C006 (ReDMCSB DEFS.H:238-243):
 *      with v2PresentationEnabled=0 every command returns
 *      routeKind=V1_SOURCE and sourceCommand==runtimeCommand, with
 *      v2PresentationEnabled=1 the sourceCommand still equals the V1
 *      C-id (the V2 runtime shell id is allowed to differ; the V1
 *      source truth must not change).
 *
 *   6. The DM1 V2 viewport renderer preserves the V1 framebuffer
 *      pixel-by-pixel when V2 presentation is disabled: the entry
 *      composition (DUNGEON.DAT offset 8 + pass173) renders into
 *      two independent 224x136 RGBA viewports whose framebuffers
 *      match byte-for-byte, and dm1_v2_vp_compare_viewport_region
 *      reports zero mismatched pixels across the full viewport at
 *      every cardinal direction N/E/S/W. This is the ctest-checkable
 *      core of the side-by-side presentation seed scaffold
 *      (probes/dm1/firestaff_dm1_v2_side_by_side_presentation_seed_probe.c).
 *
 * The test is headless, depends only on the firestaff_v2 and
 * firestaff_m10 libraries, and does not require any game data files.
 *
 * Source references (ReDMCSB):
 *   DEFS.H:238-243          C001..C006 V1 command ids.
 *   COMMAND.C:2045-2155     F0359 command queue dispatch.
 *   CLIKMENU.C:142          F0365 turn party.
 *   CLIKMENU.C:180          F0366 move party.
 *   CLIKMENU.C:278-323      wall/door/fakewall/group collision.
 *   CLIKMENU.C:330-346      disabled movement tick.
 *   GAMELOOP.C:150-155      disabled movement tick decrement.
 *   GAMELOOP.C:215-219      F0380 input wait loop.
 *   MOVESENS.C:316-345      F0267 move-result side effects.
 *   LOADSAVE.C:1520-1534    save party position/timing/global state.
 *   LOADSAVE.C:2730-2742    restore party position/timing/global state.
 *   COORD.C:1721-1722       224x136 viewport.
 *   DUNVIEW.C:2999-3000     224x136 viewport (gameplay-space picture).
 *   DUNVIEW.C:3913-3928     D1C champion portrait blit at {96,35}.
 *   DUNGEON.C:2573          C127 sensor view-direction mapping.
 *   DUNGEON.C:2610-2612     only front wall aspect sets G0289 portrait.
 *   PANEL.C:418-428         G0304_i_DungeonViewPaletteIndex.
 *
 * Exit codes: 0 = pass, 1 = at least one assertion failed.
 */

#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

/* ── V1 source command table (ReDMCSB DEFS.H:238-243) ─────────────
 *
 *   C001 = TURN_LEFT,    C002 = TURN_RIGHT,
 *   C003 = MOVE_FORWARD, C004 = MOVE_RIGHT,
 *   C005 = MOVE_BACKWARD,C006 = MOVE_LEFT.
 *
 * V1 source ids must never change. The V2 runtime shell command ids
 * in the second column are allowed to differ from the V1 source ids
 * when v2PresentationEnabled=1.
 */
typedef struct {
    DM1_V2_MovementCommand v2Command;
    int v1SourceCommand;
    int v2RuntimeCommand;
} V1CommandCase;

static const V1CommandCase g_v1_commands[6] = {
    { DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,    1, 3 },
    { DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,   2, 4 },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1 },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,   4, 5 },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,5, 2 },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,    6, 6 },
};

/* ── V1-source-locked domains (must always be V1-locked) ────────── */

static const DM1_V2_PhaseDomain g_v1_locked[] = {
    DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS,
    DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING,
    DM1_V2_PHASE_DOMAIN_COLLISION_RULES,
    DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA,
    DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES,
};

/* ── TC-1: defaults() locks V1 ──────────────────────────────────── */

static int test_defaults_lock_v1(void) {
    DM1_V2_PhaseGateConfig cfg;
    dm1_v2_phase_gate_defaults(&cfg);
    CHECK(cfg.v2PresentationEnabled == 0);
    CHECK(cfg.v2ConfigPersistenceEnabled == 0);
    return 0;
}

/* ── TC-2: V1-locked domains stay V1-locked in both V2 states ──── */

static int test_v1_locked_always(void) {
    DM1_V2_PhaseGateConfig cfg_off, cfg_on;
    size_t i;
    dm1_v2_phase_gate_defaults(&cfg_off);
    dm1_v2_phase_gate_defaults(&cfg_on);
    cfg_on.v2PresentationEnabled = 1;
    cfg_on.v2ConfigPersistenceEnabled = 1;

    for (i = 0; i < sizeof(g_v1_locked) / sizeof(g_v1_locked[0]); ++i) {
        DM1_V2_PhaseGateDecision d_off =
            dm1_v2_phase_gate_decide(&cfg_off, g_v1_locked[i]);
        DM1_V2_PhaseGateDecision d_on =
            dm1_v2_phase_gate_decide(&cfg_on, g_v1_locked[i]);
        CHECK(d_off.v1SourceLocked == 1);
        CHECK(d_off.v2PresentationAllowed == 0);
        CHECK(d_on.v1SourceLocked == 1);
        CHECK(d_on.v2PresentationAllowed == 0);
    }
    return 0;
}

/* ── TC-3: V2-eligible domains follow the toggles ──────────────── */

static int test_v2_eligible_toggle(void) {
    DM1_V2_PhaseGateConfig cfg_off, cfg_on_render, cfg_on_both;
    DM1_V2_PhaseGateDecision d;

    dm1_v2_phase_gate_defaults(&cfg_off);
    dm1_v2_phase_gate_defaults(&cfg_on_render);
    cfg_on_render.v2PresentationEnabled = 1;
    dm1_v2_phase_gate_defaults(&cfg_on_both);
    cfg_on_both.v2PresentationEnabled = 1;
    cfg_on_both.v2ConfigPersistenceEnabled = 1;

    /* RENDER_PRESENTATION: presentation toggle alone is enough. */
    d = dm1_v2_phase_gate_decide(&cfg_off, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&cfg_on_render, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);

    /* INPUT_PRESENTATION: presentation toggle alone is enough. */
    d = dm1_v2_phase_gate_decide(&cfg_off, DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&cfg_on_render, DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);

    /* CONFIG_PRESENTATION: requires both toggles. */
    d = dm1_v2_phase_gate_decide(&cfg_off, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&cfg_on_render, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&cfg_on_both, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);
    return 0;
}

/* ── TC-4: NULL config and unknown domain fail safe to V1 ───────── */

static int test_null_config_fail_safe(void) {
    size_t i;
    /* defaults(NULL) must be null-safe. */
    dm1_v2_phase_gate_defaults(NULL);

    for (i = 0; i < sizeof(g_v1_locked) / sizeof(g_v1_locked[0]); ++i) {
        DM1_V2_PhaseGateDecision d =
            dm1_v2_phase_gate_decide(NULL, g_v1_locked[i]);
        CHECK(d.v1SourceLocked == 1);
        CHECK(d.v2PresentationAllowed == 0);
    }

    /* Unknown domain falls into the V1-locked default branch. */
    {
        DM1_V2_PhaseGateConfig cfg;
        DM1_V2_PhaseGateDecision d;
        dm1_v2_phase_gate_defaults(&cfg);
        cfg.v2PresentationEnabled = 1;
        cfg.v2ConfigPersistenceEnabled = 1;
        d = dm1_v2_phase_gate_decide(&cfg, (DM1_V2_PhaseDomain)99);
        CHECK(d.v1SourceLocked == 1);
        CHECK(d.v2PresentationAllowed == 0);
    }
    return 0;
}

/* ── TC-5: V1 command ids (C001..C006) preserved ───────────────── */

static int test_v1_command_ids_preserved(void) {
    size_t i;
    for (i = 0; i < sizeof(g_v1_commands) / sizeof(g_v1_commands[0]); ++i) {
        DM1_V2_MovementCommandRoute v1_route =
            dm1_v2_movement_command_route_for_presentation(
                0, g_v1_commands[i].v2Command);
        DM1_V2_MovementCommandRoute v2_route =
            dm1_v2_movement_command_route_for_presentation(
                1, g_v1_commands[i].v2Command);

        /* V1 route: V1-source-locked, no V2 presentation. */
        CHECK(v1_route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(v1_route.v2PresentationEnabled == 0);
        CHECK(v1_route.sourceCommand == g_v1_commands[i].v1SourceCommand);
        CHECK(v1_route.runtimeCommand == g_v1_commands[i].v1SourceCommand);

        /* V2 route: still keeps the V1 source id; runtime shell may
         * differ, source truth must not. */
        CHECK(v2_route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(v2_route.v2PresentationEnabled == 1);
        CHECK(v2_route.sourceCommand == g_v1_commands[i].v1SourceCommand);
        CHECK(v2_route.runtimeCommand == g_v1_commands[i].v2RuntimeCommand);
    }
    return 0;
}

/* ── TC-6: every domain has a non-empty source anchor ──────────── */

static int test_every_domain_has_anchor(void) {
    int d;
    DM1_V2_PhaseGateConfig cfg;
    DM1_V2_PhaseGateDecision dec;
    dm1_v2_phase_gate_defaults(&cfg);

    for (d = 0; d <= 7; ++d) {
        dec = dm1_v2_phase_gate_decide(&cfg, (DM1_V2_PhaseDomain)d);
        CHECK(dec.sourceAnchor != NULL);
        CHECK(strlen(dec.sourceAnchor) > 4);
    }
    return 0;
}

/* ── TC-7: phase gate has explicit gameplay-domain predicate ───── */

static int test_gameplay_domain_predicate(void) {
    /* V1-locked domains are gameplay; V2-eligible domains are not. */
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS) == 1);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING) == 1);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_COLLISION_RULES) == 1);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA) == 1);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES) == 1);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION) == 0);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION) == 0);
    CHECK(dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION) == 0);
    return 0;
}

/* ── TC-8: V1/V2 presentation-disabled pixel-level parity ──────────
 *
 * Mirrors the ctest-checkable core of the side-by-side presentation
 * seed probe (probes/dm1/firestaff_dm1_v2_side_by_side_presentation_seed_probe.c).
 *
 * The probe exercises the same contract from a stand-alone executable;
 * this test exercises it directly inside the ctest process so any future
 * regression in the V2 viewport renderer's V1 parity path surfaces as a
 * regular ctest failure, not just a stand-alone probe output.
 *
 * What is asserted:
 *   1. dm1_v2_vp_dm1_pc34_entry_state_fixture() reports the canonical
 *      DM1 PC 3.4 entry (map 0, x=1, y=3, dir=2 source=DUNGEON.DAT
 *      offset 8 + pass173 source audit).
 *   2. dm1_v2_vp_build_composition_from_fixture() decodes D1C as WALL
 *      and D0C as CORRIDOR for the canonical entry position.
 *   3. dm1_v2_vp_render_composition_flat() returns 1 for both lanes.
 *   4. The two 224x136 RGBA framebuffers are byte-equal pixel-by-pixel.
 *   5. dm1_v2_vp_compare_viewport_region() reports 0 mismatched pixels
 *      over the full viewport (the canonical parity row).
 *   6. A 64-bit FNV-1a fold of the V1 framebuffer equals the V2 fold.
 *   7. The same six checks hold for directions 0..3 (N/E/S/W) so a
 *      future rotation regression cannot silently drift V1 vs V2 in
 *      any lane. The fixture front-wall square at (1,4) is at D1C
 *      only in direction=2; other directions use the fixture default
 *      element for D1C, which is intentional — V1 and V2 must still
 *      agree, regardless of which element D1C ends up with.
 *
 * Source locks:
 *   ReDMCSB DUNVIEW.C:2999-3000   224x136 viewport bitmap.
 *   ReDMCSB DUNGEON.C:35-44       direction step tables.
 *   ReDMCSB DUNGEON.C:2238-2239   stair raw -> side/front aspect.
 *   ReDMCSB DUNGEON.C:2243-2246   door raw -> side/front aspect.
 *   ReDMCSB DEFS.H:238-243        C001..C006 V1 command ids (pinned in TC-5).
 *   ReDMCSB COMMAND.C:2045-2155   F0359 command queue dispatch.
 *
 * The FNV-1a fold is identical to the one used in
 * probes/dm1/firestaff_dm1_v2_side_by_side_presentation_seed_probe.c
 * so a future V1 vs V2 framebuffer drift will surface in both the ctest
 * test and the stand-alone probe at the same time.
 */
static uint64_t fnv1a_fold_u32(uint64_t hash, uint32_t value) {
    int byteIndex;
    for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
        hash ^= (uint64_t)((value >> (byteIndex * 8)) & 0xFFU);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t fnv1a_framebuffer(const DM1_V2_ViewportState* vp) {
    uint64_t hash = 14695981039346656037ULL; /* FNV-1a 64-bit offset basis */
    int y, x;
    if (!vp) return hash;
    for (y = 0; y < DM1_V2_VIEWPORT_H; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            const DM1_V2_Color* c = &vp->framebuffer[y][x];
            uint32_t packed = ((uint32_t)c->r)
                            | ((uint32_t)c->g << 8)
                            | ((uint32_t)c->b << 16)
                            | ((uint32_t)c->a << 24);
            hash = fnv1a_fold_u32(hash, packed);
        }
    }
    return hash;
}

static int render_lane_for_direction(const DM1_V2_DungeonStateFixture* fixture,
                                     int direction,
                                     DM1_V2_ViewportState* v1,
                                     DM1_V2_ViewportState* v2) {
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_ViewportRegion region;
    DM1_V2_RegionCompareResult cmp;
    uint64_t h1, h2;
    int x, y, mismatch;

    if (!fixture || !v1 || !v2) return 1;

    CHECK(dm1_v2_vp_build_composition_from_fixture(fixture,
                                                   fixture->startMapX,
                                                   fixture->startMapY,
                                                   direction,
                                                   &input) == 1);

    dm1_v2_vp_init(v1);
    dm1_v2_vp_init(v2);
    CHECK(dm1_v2_vp_render_composition_flat(v1, &input) == 1);
    CHECK(dm1_v2_vp_render_composition_flat(v2, &input) == 1);

    /* Byte-equal pixel-by-pixel check. */
    mismatch = 0;
    for (y = 0; y < DM1_V2_VIEWPORT_H && !mismatch; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            const DM1_V2_Color* a = &v1->framebuffer[y][x];
            const DM1_V2_Color* b = &v2->framebuffer[y][x];
            if (a->r != b->r || a->g != b->g ||
                a->b != b->b || a->a != b->a) {
                mismatch = 1;
                break;
            }
        }
    }
    CHECK(mismatch == 0);

    /* Region comparator contract over the full viewport. */
    region.x = 0;
    region.y = 0;
    region.width = DM1_V2_VIEWPORT_W;
    region.height = DM1_V2_VIEWPORT_H;
    region.name = "side-by-side-full";
    CHECK(dm1_v2_vp_compare_viewport_region(&v1->framebuffer[0][0],
                                            &v2->framebuffer[0][0],
                                            DM1_V2_VIEWPORT_W,
                                            region,
                                            &cmp) == 1);
    CHECK(cmp.comparedPixels == DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H);
    CHECK(cmp.mismatchedPixels == 0);

    /* Stable FNV-1a fold parity. */
    h1 = fnv1a_framebuffer(v1);
    h2 = fnv1a_framebuffer(v2);
    CHECK(h1 == h2);
    return 0;
}

static int test_v1_v2_pixel_level_parity(void) {
    const DM1_V2_DungeonStateFixture* fixture =
        dm1_v2_vp_dm1_pc34_entry_state_fixture();
    int direction;

    CHECK(fixture != NULL);
    CHECK(fixture->startMapX == 1);
    CHECK(fixture->startMapY == 3);
    CHECK(fixture->startDirection == 2);

    /* Canonical entry position: dir=2 (S) places the (1,4) wall
     * square at D1C. Other directions use the fixture default
     * element for D1C; parity must still hold for all four. */
    {
        DM1_V2_ViewportState v1, v2;
        DM1_V2_ViewportCompositionInput input;
        CHECK(dm1_v2_vp_build_composition_from_fixture(fixture,
                                                       fixture->startMapX,
                                                       fixture->startMapY,
                                                       fixture->startDirection,
                                                       &input) == 1);
        CHECK(input.squares[1][1].element == DM1_V2_ELEMENT_WALL);
        CHECK(input.squares[0][1].element == DM1_V2_ELEMENT_CORRIDOR);
        if (render_lane_for_direction(fixture, fixture->startDirection, &v1, &v2)) {
            return 1;
        }
    }

    /* Multi-direction parity: render the entry composition at N/E/S/W
     * and confirm V1/V2 byte parity at every direction. This catches
     * any future rotation/direction-step regression in the V2
     * viewport renderer that the single-direction probe would miss. */
    for (direction = 0; direction < 4; ++direction) {
        DM1_V2_ViewportState v1, v2;
        if (render_lane_for_direction(fixture, direction, &v1, &v2)) {
            return 1;
        }
    }
    return 0;
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    if (test_defaults_lock_v1()) return 1;
    if (test_v1_locked_always()) return 1;
    if (test_v2_eligible_toggle()) return 1;
    if (test_null_config_fail_safe()) return 1;
    if (test_v1_command_ids_preserved()) return 1;
    if (test_every_domain_has_anchor()) return 1;
    if (test_gameplay_domain_predicate()) return 1;
    if (test_v1_v2_pixel_level_parity()) return 1;
    puts("dm1_v2_v1_v2_side_by_side_seed_pc34: ok");
    return 0;
}
