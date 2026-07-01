/*
 * firestaff_theron_v2_hud_launch_mode_probe.c
 *
 * Theron V2 HUD launch-mode gate (presentation-only) headless probe.
 *
 * Verifies (no game data, no SDL):
 *   1. THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE exists in the phase-gate
 *      enum and is classified as V2-eligible (presentation-only).
 *   2. theron_v2_hud_launch_mode_reset() defaults to OFF (V1 chrome
 *      preserved) under every gate combination.
 *   3. Resolution table:
 *      - V1_FAITHFUL=1                -> OFF (always)
 *      - v2PresentationEnabled=0     -> OFF (always)
 *      - OVERLAY + V2-on             -> OVERLAY
 *      - TOUCH + V2-on + persistence -> TOUCH
 *      - TOUCH + V2-on + no-persist  -> OVERLAY (downgraded)
 *      - CONTROLLER + V2-on + persist + pack present -> CONTROLLER
 *      - CONTROLLER + V2-on + persist + pack missing -> OVERLAY
 *      - CONTROLLER + V2-on + no-persist              -> OVERLAY
 *   4. M11 int mapping (0..3) is stable.
 *   5. Touch hit-test:
 *      - hit when zone covers screenX/screenY
 *      - no-hit outside any zone
 *      - hit zones carry champion_index / action_index when applicable
 *      - hit-test is a no-op when mode is not TOUCH
 *   6. Controller glyph rail:
 *      - active flag round-trip per cardinal / per action
 *      - should_render is 0 unless CONTROLLER mode + visibility
 *   7. V1 source-locked contract:
 *      - launch-mode selection never reads/writes any V1 state
 *      - phase gate keeps V1-source-locked domains locked when V2-on
 *   8. Source evidence citations + null safety.
 *
 * Skip-safe by construction:
 *   - No file I/O, no real asset lookup.
 *   - No SDL, no real-game-launch.
 *   - All assertions run with SDL_VIDEODRIVER=dummy in CI.
 *
 * Source-lock:
 *   THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900
 *   HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)
 *   ReDMCSB COMMAND.C F0359 + PANEL.C F0354 + CLIKMENU.C F0365/F0366
 *   include/theron_v2_hud_launch_mode_pc34.h
 *   include/theron_v2_phase_gate_pc34.h
 *   include/theron_v2_presentation_mode_pc34.h
 *   include/theron_v2_hud_overlay_pc34.h
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include "theron_v2_hud_launch_mode_pc34.h"
#include "theron_v2_phase_gate_pc34.h"
#include "theron_v2_presentation_mode_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "theron_v2_hud_overlay_pc34.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char* name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

/* ── 1. Phase-gate enum presence ─────────────────────────────────────── */
static void p_phase_gate_enum(void)
{
    printf("\n[ Phase-gate: HUD_LAUNCH_MODE enum ]\n");
    check((int)THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE == 16,
          "THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE = 16");
    check((int)THERON_V2_PHASE_DOMAIN_COUNT >= 17,
          "THERON_V2_PHASE_DOMAIN_COUNT >= 17");

    THERON_V2_PhaseGateConfig cfg = {1, 1};
    THERON_V2_PhaseGateDecision d =
        theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE);
    check(d.v1SourceLocked == 0,
          "HUD_LAUNCH_MODE is V2-eligible (not V1-locked)");
    check(d.v2PresentationAllowed == 1,
          "HUD_LAUNCH_MODE allowed when v2PresentationEnabled=1 + v2ConfigPersistenceEnabled=1");

    /* When V2 is off, HUD_LAUNCH_MODE is blocked (Phase 0 contract). */
    theron_v2_phase_gate_defaults(&cfg);
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE);
    check(d.v2PresentationAllowed == 0,
          "HUD_LAUNCH_MODE blocked when v2PresentationEnabled=0 (V1-locked)");

    /* When V2-on, persistence-off, HUD_LAUNCH_MODE is still allowed
     * (the launch-mode selector itself internally gates TOUCH/
     * CONTROLLER on persistence; the gate domain only encodes the
     * "this is presentation-only" rule). */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE);
    check(d.v2PresentationAllowed == 1,
          "HUD_LAUNCH_MODE allowed when V2-on, persistence-off (gate domain)");
    check(d.v1SourceLocked == 0,
          "HUD_LAUNCH_MODE never V1-source-locked (presentation-only contract)");
}

/* ── 2. Reset defaults ──────────────────────────────────────────────── */
static void p_reset_defaults(void)
{
    printf("\n[ Reset defaults: V1-locked ]\n");
    theron_v2_hud_launch_mode_reset();
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "reset() -> OFF (V1 chrome preserved)");
    check(theron_v2_hud_launch_mode_is_off() == 1, "is_off()=1 after reset");
    check(theron_v2_hud_launch_mode_is_overlay() == 0, "is_overlay()=0 after reset");
    check(theron_v2_hud_launch_mode_is_touch() == 0, "is_touch()=0 after reset");
    check(theron_v2_hud_launch_mode_is_controller() == 0,
          "is_controller()=0 after reset");
    check(theron_v2_hud_launch_mode_allows_overlay() == 0,
          "allows_overlay()=0 after reset");
    check(theron_v2_hud_launch_mode_allows_touch() == 0,
          "allows_touch()=0 after reset");
    check(theron_v2_hud_launch_mode_allows_controller() == 0,
          "allows_controller()=0 after reset");

    const Theron_V2_HudLaunchModeState* s = theron_v2_hud_launch_mode_state();
    check(s->v1FaithfulActive == 1, "reset: v1FaithfulActive=1");
    check(s->v2PresentationEnabled == 0, "reset: v2PresentationEnabled=0");
    check(s->configPersistenceEnabled == 0, "reset: configPersistenceEnabled=0");
    check(s->modernPackAvailable == 0, "reset: modernPackAvailable=0");
}

/* ── 3. Resolution table ────────────────────────────────────────────── */
static void p_resolution_table(void)
{
    printf("\n[ Resolution table: OFF/OVERLAY/TOUCH/CONTROLLER ]\n");

    /* V1_FAITHFUL wins: every mode collapses to OFF. */
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 1, 1, 1, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V1_FAITHFUL=1 + OVERLAY -> OFF");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 1, 1, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V1_FAITHFUL=1 + TOUCH -> OFF");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 1, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V1_FAITHFUL=1 + CONTROLLER -> OFF");

    /* V2-off wins. */
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 0, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V2-off + OVERLAY -> OFF");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_TOUCH, 0, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V2-off + TOUCH -> OFF");

    /* V2-on + persistence-on: full resolution table. */
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OFF, 1, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V2-on + persist + OFF -> OFF (passthrough)");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 1, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "V2-on + persist + OVERLAY -> OVERLAY");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_TOUCH,
          "V2-on + persist + TOUCH -> TOUCH");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER,
          "V2-on + persist + pack present + CONTROLLER -> CONTROLLER");

    /* CONTROLLER without modern pack -> OVERLAY (skip-safe). */
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 0, 0)
              == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "V2-on + persist + pack missing + CONTROLLER -> OVERLAY");

    /* V2-on + persistence-off: TOUCH / CONTROLLER downgrade to OVERLAY. */
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 0, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "V2-on + persist off + TOUCH -> OVERLAY");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 0, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "V2-on + persist off + CONTROLLER -> OVERLAY");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 1, 0, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "V2-on + persist off + OVERLAY -> OVERLAY (passthrough)");
    check(theron_v2_hud_launch_mode_resolve(
              THERON_V2_HUD_LAUNCH_MODE_OFF, 1, 0, 0, 1)
              == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V2-on + persist off + OFF -> OFF (passthrough)");
}

/* ── 4. Live state transitions ─────────────────────────────────────── */
static void p_live_state(void)
{
    printf("\n[ Live state transitions ]\n");
    theron_v2_hud_launch_mode_reset();
    /* V2-on, persistence-on, pack present. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "live: V2-on + persist + pack + OVERLAY -> OVERLAY");
    check(theron_v2_hud_launch_mode_allows_overlay() == 1,
          "live: allows_overlay=1");
    check(theron_v2_hud_launch_mode_allows_touch() == 0,
          "live: allows_touch=0 (mode is OVERLAY)");

    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_TOUCH,
          "live: TOUCH -> TOUCH");
    check(theron_v2_hud_launch_mode_allows_touch() == 1,
          "live: allows_touch=1");

    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER,
          "live: CONTROLLER + pack -> CONTROLLER");
    check(theron_v2_hud_launch_mode_controller_should_render() == 1,
          "live: controller_should_render=1");

    /* Yanking persistence off mid-flight downgrades CONTROLLER -> OVERLAY. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 0);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "live: persist off -> CONTROLLER downgrades to OVERLAY");
    check(theron_v2_hud_launch_mode_controller_should_render() == 0,
          "live: persist off -> controller_should_render=0");

    /* Yanking modern-pack off mid-flight downgrades CONTROLLER -> OVERLAY. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    theron_v2_hud_launch_mode_set_modern_pack_available(0);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "live: pack missing -> CONTROLLER downgrades to OVERLAY");

    /* V1_FAITHFUL wins last: any mode -> OFF. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER,
          "live: CONTROLLER + pack + persist -> CONTROLLER");
    theron_v2_hud_launch_mode_set_v1_faithful(1);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "live: V1_FAITHFUL=1 collapses to OFF");
}

/* ── 5. M11 int mapping ────────────────────────────────────────────── */
static void p_m11_mapping(void)
{
    printf("\n[ M11 launch spec -> launch-mode mapping ]\n");
    check(theron_v2_hud_launch_mode_from_m11(0) == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "m11 0 -> OFF");
    check(theron_v2_hud_launch_mode_from_m11(1) == THERON_V2_HUD_LAUNCH_MODE_OVERLAY,
          "m11 1 -> OVERLAY");
    check(theron_v2_hud_launch_mode_from_m11(2) == THERON_V2_HUD_LAUNCH_MODE_TOUCH,
          "m11 2 -> TOUCH");
    check(theron_v2_hud_launch_mode_from_m11(3) == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER,
          "m11 3 -> CONTROLLER");
    check(theron_v2_hud_launch_mode_from_m11(99) == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "m11 99 (out of range) -> OFF (skip-safe)");
    check(theron_v2_hud_launch_mode_from_m11(-7) == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "m11 -7 (out of range) -> OFF (skip-safe)");
}

/* ── 6. Touch hit-test ─────────────────────────────────────────────── */
static void p_touch_hittest(void)
{
    Theron_V2_HudLaunchTouchResult r;
    printf("\n[ Touch hit-test (TOUCH mode) ]\n");

    /* Enable TOUCH mode. */
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_TOUCH,
          "hittest prep: TOUCH mode active");

    /* TOP_BAR zone (y=0..23, x=0..255). */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 1,
          "hittest (120,12) -> hit");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_TOP_BAR,
          "hittest (120,12) -> TOP_BAR");

    /* COMPASS (x=8..24, y=4..20). */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(12, 12, &r) == 1,
          "hittest (12,12) -> hit (compass)");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_COMPASS,
          "hittest (12,12) -> COMPASS");

    /* QUEST_ITEMS (x=64..104, y=0..23). */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(80, 12, &r) == 1,
          "hittest (80,12) -> hit (quest)");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_QUEST_ITEMS,
          "hittest (80,12) -> QUEST_ITEMS");

    /* CHAMPION_BAR_2 (x=128..188, y=184..191). */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(150, 188, &r) == 1,
          "hittest (150,188) -> hit (champion 2)");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_2,
          "hittest (150,188) -> CHAMPION_BAR_2");
    check(r.champion_index == 2,
          "hittest (150,188) -> champion_index=2");

    /* ACTION_CAST (x=48..76, y=208..221). */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(60, 215, &r) == 1,
          "hittest (60,215) -> hit (action cast)");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_CAST,
          "hittest (60,215) -> ACTION_CAST");
    check(r.action_index == 1,
          "hittest (60,215) -> action_index=1 (cast)");

    /* Outside any zone. */
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(120, 100, &r) == 0,
          "hittest (120,100) -> no hit");
    check(r.zone == THERON_V2_HUD_LAUNCH_ZONE_NONE,
          "hittest (120,100) -> NONE");

    /* Null result: skip-safe no-op. */
    check(theron_v2_hud_launch_mode_touch_hittest(120, 12, NULL) == 0,
          "hittest NULL result -> no-op, no crash");

    /* OFF mode: hit-test must return 0. */
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OFF);
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 0,
          "hittest under OFF -> no hit (touch not allowed)");

    /* OVERLAY mode: hit-test also returns 0. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    memset(&r, 0xAA, sizeof(r));
    check(theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 0,
          "hittest under OVERLAY -> no hit (touch not allowed)");
}

/* ── 7. Controller glyph rail ──────────────────────────────────────── */
static void p_controller_glyph(void)
{
    printf("\n[ Controller glyph rail ]\n");
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check(theron_v2_hud_launch_mode_controller_should_render() == 1,
          "controller_should_render=1 under CONTROLLER + pack + persist");

    theron_v2_hud_launch_mode_controller_set_active(
        THERON_V2_HUD_LAUNCH_GLYPH_NORTH, 1);
    theron_v2_hud_launch_mode_controller_set_action_active(
        THERON_V2_HUD_LAUNCH_GLYPH_ACTION_CAST, 1);
    const Theron_V2_HudLaunchModeState* s = theron_v2_hud_launch_mode_state();
    check(s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_NORTH] == 1,
          "cardinal NORTH active=1");
    check(s->controller_glyph_rail.action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_CAST] == 1,
          "action CAST active=1");
    check(s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_EAST] == 0,
          "cardinal EAST active=0");

    theron_v2_hud_launch_mode_controller_reset_active();
    s = theron_v2_hud_launch_mode_state();
    check(s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_NORTH] == 0,
          "reset clears cardinal flags");
    check(s->controller_glyph_rail.action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_CAST] == 0,
          "reset clears action flags");

    /* Out-of-range setters: skip-safe no-op. */
    theron_v2_hud_launch_mode_controller_set_active(
        (Theron_V2_HudLaunchCardinal)99, 1);
    theron_v2_hud_launch_mode_controller_set_active(
        (Theron_V2_HudLaunchCardinal)-1, 1);
    theron_v2_hud_launch_mode_controller_set_action_active(
        (Theron_V2_HudLaunchActionGlyph)42, 1);
    check(1, "controller out-of-range setters: no crash");

    /* Switch mode away from CONTROLLER: rail must hide. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check(theron_v2_hud_launch_mode_controller_should_render() == 0,
          "controller_should_render=0 under OVERLAY");
}

/* ── 8. V1 source-locked contract ──────────────────────────────────── */
static void p_v1_lock_contract(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;
    printf("\n[ V1 source-locked contract ]\n");
    /* Even when V2-on, V1-source-locked domains MUST stay locked. */
    theron_v2_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_TRACK02_BANK);
    check(d.v1SourceLocked == 1, "TRACK02_BANK stays V1-locked (V2-on)");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_BOOT_PROFILE);
    check(d.v1SourceLocked == 1, "BOOT_PROFILE stays V1-locked (V2-on)");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY);
    check(d.v1SourceLocked == 1, "CHAMPION_PARTY stays V1-locked (V2-on)");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_MECHANICS);
    check(d.v1SourceLocked == 1, "MECHANICS stays V1-locked (V2-on)");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_SAVE_LOAD);
    check(d.v1SourceLocked == 1, "SAVE_LOAD stays V1-locked (V2-on)");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_WORLD_STATE);
    check(d.v1SourceLocked == 1, "WORLD_STATE stays V1-locked (V2-on)");
}

/* ── 9. Source evidence + name + null safety ───────────────────────── */
static void p_evidence_and_null(void)
{
    const char* ev;
    printf("\n[ Source evidence + null safety ]\n");
    ev = theron_v2_hud_launch_mode_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "source_evidence non-trivial");
    check(strstr(ev, "THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE") != NULL,
          "ev mentions new phase-gate domain");
    check(strstr(ev, "ReDMCSB COMMAND.C F0359") != NULL,
          "ev cites ReDMCSB COMMAND.C F0359");
    check(strstr(ev, "THQUEST.ASM T600") != NULL, "ev cites THQUEST.ASM T600");
    check(strstr(ev, "HuC6260") != NULL, "ev cites HuC6260");
    check(strstr(ev, "theron_v2_hud_launch_mode_pc34") != NULL,
          "ev names the module");

    /* Names. */
    check(strcmp(theron_v2_hud_launch_mode_name(THERON_V2_HUD_LAUNCH_MODE_OFF), "OFF") == 0,
          "name(OFF)=OFF");
    check(strcmp(theron_v2_hud_launch_mode_name(THERON_V2_HUD_LAUNCH_MODE_OVERLAY), "OVERLAY") == 0,
          "name(OVERLAY)=OVERLAY");
    check(strcmp(theron_v2_hud_launch_mode_name(THERON_V2_HUD_LAUNCH_MODE_TOUCH), "TOUCH") == 0,
          "name(TOUCH)=TOUCH");
    check(strcmp(theron_v2_hud_launch_mode_name(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER), "CONTROLLER") == 0,
          "name(CONTROLLER)=CONTROLLER");
    check(strcmp(theron_v2_hud_launch_mode_name((Theron_V2_HudLaunchMode)42), "UNKNOWN") == 0,
          "name(out-of-range)=UNKNOWN");

    /* Track 02 hashes are pinned (sanity: the launch-mode gate is
     * independent of any Track 02 hash; this guards against
     * accidental coupling). */
    check(strlen(THERON_TRACK02_MD5_JP_REV1_ISO) == 32,
          "Track 02 JP Rev 1 hash pinned (32 hex chars)");
    check(strlen(THERON_TRACK02_MD5_US_ISO) == 32,
          "Track 02 US ISO hash pinned (32 hex chars)");

    /* Null safety. */
    theron_v2_hud_launch_mode_touch_hittest(0, 0, NULL);
    theron_v2_hud_launch_mode_controller_set_active(
        (Theron_V2_HudLaunchCardinal)0, 1);
    check(1, "null / out-of-range inputs: no crash");
}

/* ── 10. Interaction with presentation-mode selector ───────────────── */
static void p_interaction_with_pm(void)
{
    printf("\n[ Interaction with presentation-mode selector ]\n");
    /* When V1_FAITHFUL is active, launch-mode is OFF even under V22
     * mode selection; the V1 chrome is preserved. */
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    /* V22 is the active presentation mode here, but V1_FAITHFUL is
     * still 1 by reset() default -> launch-mode is OFF. */
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "V1_FAITHFUL reset default keeps launch-mode OFF even under V22");

    /* Once V1_FAITHFUL=0, the launch-mode selector accepts the
     * caller's requested mode (when V2-on + persist + pack). */
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER,
          "V1_FAITHFUL=0 + V22 + CONTROLLER -> CONTROLLER (allowed)");

    /* DM1 V2 / CSB V2 presentation modules stay independent. */
    (void)dm1_v2_presentation_mode_reset();
    (void)csb_v2_presentation_mode_reset();
    theron_v2_hud_launch_mode_reset();
    check(theron_v2_hud_launch_mode_get() == THERON_V2_HUD_LAUNCH_MODE_OFF,
          "Theron launch-mode reset after DM1/CSB PM reset -> OFF");
}

int main(void)
{
    printf("=== Theron V2 HUD launch-mode probe ===\n");
    p_phase_gate_enum();
    p_reset_defaults();
    p_resolution_table();
    p_live_state();
    p_m11_mapping();
    p_touch_hittest();
    p_controller_glyph();
    p_v1_lock_contract();
    p_evidence_and_null();
    p_interaction_with_pm();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
