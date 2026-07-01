/*
 * test_theron_v2_hud_launch_mode_pc34.c
 *
 * Unit test for Theron V2 HUD launch-mode gate (presentation-only).
 *
 * Source-lock anchors:
 *   include/theron_v2_hud_launch_mode_pc34.h
 *   include/theron_v2_phase_gate_pc34.h
 *   src/theron/theron_v2_hud_launch_mode_pc34.c
 *   src/theron/theron_v2_phase_gate_pc34.c
 *   THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900
 *   HuC6260/HuC6270 (PC Engine VDC + VCE)
 *   ReDMCSB COMMAND.C F0359, PANEL.C F0354, CLIKMENU.C F0365/F0366
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *
 * Mirrors the test_theron_v2_hud_overlay_pc34 + test_theron_v2_phase_gate_pc34
 * layout: pin-the-API surface + every resolution path + null safety.
 */

#include "theron_v2_hud_launch_mode_pc34.h"
#include "theron_v2_phase_gate_pc34.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_passed = 0;

static void check_true(const char* id, int cond)
{
    ++g_assertions;
    if (cond) {
        ++g_passed;
    } else {
        printf("FAIL %s\n", id);
    }
}

static void check_int(const char* id, int got, int want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    }
}

int main(void)
{
    /* ── 1. Reset defaults ──────────────────────────────────────────── */
    theron_v2_hud_launch_mode_reset();
    check_int("reset.mode", theron_v2_hud_launch_mode_get(),
              THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_true("reset.is_off",
               theron_v2_hud_launch_mode_is_off() == 1);
    check_true("reset.v1Faithful=1",
               theron_v2_hud_launch_mode_state()->v1FaithfulActive == 1);
    check_true("reset.v2PresentationEnabled=0",
               theron_v2_hud_launch_mode_state()->v2PresentationEnabled == 0);
    check_true("reset.configPersistenceEnabled=0",
               theron_v2_hud_launch_mode_state()->configPersistenceEnabled == 0);
    check_true("reset.modernPackAvailable=0",
               theron_v2_hud_launch_mode_state()->modernPackAvailable == 0);

    /* ── 2. Phase-gate domain presence + classification ───────────── */
    check_true("THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE=16",
               (int)THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE == 16);
    check_true("THERON_V2_PHASE_DOMAIN_COUNT>=17",
               (int)THERON_V2_PHASE_DOMAIN_COUNT >= 17);
    {
        THERON_V2_PhaseGateConfig cfg = {1, 1};
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE);
        check_true("HUD_LAUNCH_MODE.v1SourceLocked=0",
                   d.v1SourceLocked == 0);
        check_true("HUD_LAUNCH_MODE.v2PresentationAllowed=1",
                   d.v2PresentationAllowed == 1);
        theron_v2_phase_gate_defaults(&cfg);
        d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE);
        check_true("HUD_LAUNCH_MODE.v2PresentationAllowed=0 (V1-locked default)",
                   d.v2PresentationAllowed == 0);
    }

    /* ── 3. Resolution table ───────────────────────────────────────── */
    check_int("resolve.V1_FAITHFUL=1 OVERLAY -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 1, 1, 1, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("resolve.V1_FAITHFUL=1 TOUCH -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 1, 1, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("resolve.V1_FAITHFUL=1 CONTROLLER -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 1, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);

    check_int("resolve.V2-off OVERLAY -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 0, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("resolve.V2-off TOUCH -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_TOUCH, 0, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("resolve.V2-off CONTROLLER -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 0, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);

    check_int("resolve.OVERLAY V2-on persist -> OVERLAY",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_OVERLAY, 1, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_int("resolve.TOUCH V2-on persist -> TOUCH",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check_int("resolve.CONTROLLER V2-on persist pack present -> CONTROLLER",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);

    check_int("resolve.TOUCH V2-on persist off -> OVERLAY",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_TOUCH, 1, 0, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_int("resolve.CONTROLLER V2-on persist off -> OVERLAY",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 0, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_int("resolve.CONTROLLER V2-on persist pack missing -> OVERLAY",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_CONTROLLER, 1, 1, 0, 0),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);

    check_int("resolve.OFF V2-on persist off -> OFF",
              (int)theron_v2_hud_launch_mode_resolve(
                  THERON_V2_HUD_LAUNCH_MODE_OFF, 1, 0, 0, 1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);

    /* ── 4. Live state transitions ────────────────────────────────── */
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_int("live.OVERLAY",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_true("live.OVERLAY allows_overlay=1",
               theron_v2_hud_launch_mode_allows_overlay() == 1);
    check_true("live.OVERLAY allows_touch=0",
               theron_v2_hud_launch_mode_allows_touch() == 0);
    check_true("live.OVERLAY allows_controller=0",
               theron_v2_hud_launch_mode_allows_controller() == 0);

    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check_int("live.TOUCH",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check_true("live.TOUCH allows_touch=1",
               theron_v2_hud_launch_mode_allows_touch() == 1);

    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_int("live.CONTROLLER + pack present",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_true("live.CONTROLLER controller_should_render=1",
               theron_v2_hud_launch_mode_controller_should_render() == 1);

    /* Yank persist mid-flight. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 0);
    check_int("live.persist off -> CONTROLLER downgrades to OVERLAY",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_true("live.persist off controller_should_render=0",
               theron_v2_hud_launch_mode_controller_should_render() == 0);

    /* Yank modern pack. */
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_int("live.CONTROLLER restored",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    theron_v2_hud_launch_mode_set_modern_pack_available(0);
    check_int("live.pack missing -> CONTROLLER downgrades to OVERLAY",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);

    /* V1_FAITHFUL wins last. */
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_int("live.CONTROLLER + pack + persist",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    theron_v2_hud_launch_mode_set_v1_faithful(1);
    check_int("live.V1_FAITHFUL=1 collapses to OFF",
              (int)theron_v2_hud_launch_mode_get(),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);

    /* ── 5. M11 int mapping ───────────────────────────────────────── */
    check_int("m11.0 -> OFF",
              (int)theron_v2_hud_launch_mode_from_m11(0),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("m11.1 -> OVERLAY",
              (int)theron_v2_hud_launch_mode_from_m11(1),
              (int)THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_int("m11.2 -> TOUCH",
              (int)theron_v2_hud_launch_mode_from_m11(2),
              (int)THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check_int("m11.3 -> CONTROLLER",
              (int)theron_v2_hud_launch_mode_from_m11(3),
              (int)THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_int("m11.99 -> OFF (out of range)",
              (int)theron_v2_hud_launch_mode_from_m11(99),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_int("m11.-7 -> OFF (out of range)",
              (int)theron_v2_hud_launch_mode_from_m11(-7),
              (int)THERON_V2_HUD_LAUNCH_MODE_OFF);

    /* ── 6. Touch hit-test (geometry + gating) ────────────────────── */
    Theron_V2_HudLaunchTouchResult r;
    theron_v2_hud_launch_mode_reset();
    theron_v2_hud_launch_mode_set_phase_gate(1, 1);
    theron_v2_hud_launch_mode_set_v1_faithful(0);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_TOUCH);

    /* Top-bar zone (y=0..23, x=0..255). */
    check_true("hittest (120,12) -> TOP_BAR hit",
               theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_TOP_BAR);
    check_true("hittest (12,12) -> COMPASS",
               theron_v2_hud_launch_mode_touch_hittest(12, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_COMPASS);
    check_true("hittest (80,12) -> QUEST_ITEMS",
               theron_v2_hud_launch_mode_touch_hittest(80, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_QUEST_ITEMS);
    check_true("hittest (160,12) -> DUNGEON_PROGRESS",
               theron_v2_hud_launch_mode_touch_hittest(160, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_DUNGEON_PROGRESS);
    check_true("hittest (220,12) -> RELIC_COUNTER",
               theron_v2_hud_launch_mode_touch_hittest(220, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_RELIC_COUNTER);
    check_true("hittest (44,12) -> RUNE_INDICATOR",
               theron_v2_hud_launch_mode_touch_hittest(44, 12, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_RUNE_INDICATOR);

    /* Champion bars (y=184..191). */
    check_true("hittest (30,188) -> CHAMPION_BAR_0",
               theron_v2_hud_launch_mode_touch_hittest(30, 188, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_0 &&
               r.champion_index == 0);
    check_true("hittest (90,188) -> CHAMPION_BAR_1",
               theron_v2_hud_launch_mode_touch_hittest(90, 188, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_1 &&
               r.champion_index == 1);
    check_true("hittest (150,188) -> CHAMPION_BAR_2",
               theron_v2_hud_launch_mode_touch_hittest(150, 188, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_2 &&
               r.champion_index == 2);
    check_true("hittest (210,188) -> CHAMPION_BAR_3",
               theron_v2_hud_launch_mode_touch_hittest(210, 188, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_3 &&
               r.champion_index == 3);

    /* Action strip. */
    check_true("hittest (24,215) -> ACTION_ATTACK",
               theron_v2_hud_launch_mode_touch_hittest(24, 215, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_ATTACK &&
               r.action_index == 0);
    check_true("hittest (60,215) -> ACTION_CAST",
               theron_v2_hud_launch_mode_touch_hittest(60, 215, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_CAST &&
               r.action_index == 1);
    check_true("hittest (92,215) -> ACTION_USE",
               theron_v2_hud_launch_mode_touch_hittest(92, 215, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_USE &&
               r.action_index == 2);
    check_true("hittest (124,215) -> ACTION_DROP",
               theron_v2_hud_launch_mode_touch_hittest(124, 215, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_DROP &&
               r.action_index == 3);
    check_true("hittest (156,215) -> ACTION_MOVE",
               theron_v2_hud_launch_mode_touch_hittest(156, 215, &r) == 1 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_ACTION_MOVE &&
               r.action_index == 4);

    /* Outside any zone. */
    check_true("hittest (120,100) -> no hit",
               theron_v2_hud_launch_mode_touch_hittest(120, 100, &r) == 0 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_NONE);
    check_true("hittest (-5,-5) -> no hit (off-frame)",
               theron_v2_hud_launch_mode_touch_hittest(-5, -5, &r) == 0 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_NONE);
    check_true("hittest (300,300) -> no hit (off-frame)",
               theron_v2_hud_launch_mode_touch_hittest(300, 300, &r) == 0 &&
               r.zone == THERON_V2_HUD_LAUNCH_ZONE_NONE);

    /* OFF mode gates touch off. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OFF);
    check_true("hittest under OFF -> no hit",
               theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 0);
    /* OVERLAY mode gates touch off. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_true("hittest under OVERLAY -> no hit",
               theron_v2_hud_launch_mode_touch_hittest(120, 12, &r) == 0);
    /* Null result: skip-safe. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_TOUCH);
    check_true("hittest NULL result -> no crash, no hit",
               theron_v2_hud_launch_mode_touch_hittest(120, 12, NULL) == 0);

    /* ── 7. Controller glyph rail ─────────────────────────────────── */
    theron_v2_hud_launch_mode_set_modern_pack_available(1);
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_CONTROLLER);
    check_true("controller_should_render=1",
               theron_v2_hud_launch_mode_controller_should_render() == 1);
    theron_v2_hud_launch_mode_controller_set_active(
        THERON_V2_HUD_LAUNCH_GLYPH_NORTH, 1);
    theron_v2_hud_launch_mode_controller_set_active(
        THERON_V2_HUD_LAUNCH_GLYPH_WEST, 1);
    theron_v2_hud_launch_mode_controller_set_action_active(
        THERON_V2_HUD_LAUNCH_GLYPH_ACTION_ATTACK, 1);
    {
        const Theron_V2_HudLaunchModeState* s = theron_v2_hud_launch_mode_state();
        check_true("cardinal NORTH active=1",
                   s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_NORTH] == 1);
        check_true("cardinal WEST active=1",
                   s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_WEST] == 1);
        check_true("cardinal EAST active=0",
                   s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_EAST] == 0);
        check_true("action ATTACK active=1",
                   s->controller_glyph_rail.action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_ATTACK] == 1);
        check_true("action CAST active=0",
                   s->controller_glyph_rail.action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_CAST] == 0);
        check_true("rail.visible=1 under CONTROLLER",
                   s->controller_glyph_rail.visible == 1);
    }

    /* Out-of-range setters: skip-safe. */
    theron_v2_hud_launch_mode_controller_set_active(
        (Theron_V2_HudLaunchCardinal)99, 1);
    theron_v2_hud_launch_mode_controller_set_action_active(
        (Theron_V2_HudLaunchActionGlyph)-1, 1);
    check_true("out-of-range controller setters: no crash", 1);

    theron_v2_hud_launch_mode_controller_reset_active();
    {
        const Theron_V2_HudLaunchModeState* s = theron_v2_hud_launch_mode_state();
        check_true("reset clears cardinal flags",
                   s->controller_glyph_rail.cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_NORTH] == 0);
        check_true("reset clears action flags",
                   s->controller_glyph_rail.action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_ATTACK] == 0);
    }

    /* Switch mode away: rail must hide. */
    theron_v2_hud_launch_mode_set(THERON_V2_HUD_LAUNCH_MODE_OVERLAY);
    check_true("controller_should_render=0 under OVERLAY",
               theron_v2_hud_launch_mode_controller_should_render() == 0);

    /* ── 8. Name + source evidence ────────────────────────────────── */
    check_true("name.OFF", strcmp(theron_v2_hud_launch_mode_name(
                                 THERON_V2_HUD_LAUNCH_MODE_OFF), "OFF") == 0);
    check_true("name.OVERLAY", strcmp(theron_v2_hud_launch_mode_name(
                                    THERON_V2_HUD_LAUNCH_MODE_OVERLAY), "OVERLAY") == 0);
    check_true("name.TOUCH", strcmp(theron_v2_hud_launch_mode_name(
                                  THERON_V2_HUD_LAUNCH_MODE_TOUCH), "TOUCH") == 0);
    check_true("name.CONTROLLER", strcmp(theron_v2_hud_launch_mode_name(
                                        THERON_V2_HUD_LAUNCH_MODE_CONTROLLER), "CONTROLLER") == 0);
    check_true("name.UNKNOWN (out of range)",
               strcmp(theron_v2_hud_launch_mode_name(
                          (Theron_V2_HudLaunchMode)42), "UNKNOWN") == 0);

    {
        const char* ev = theron_v2_hud_launch_mode_source_evidence();
        check_true("ev non-trivial", ev && strlen(ev) > 50);
        check_true("ev cites new phase-gate domain",
                   strstr(ev, "THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE") != NULL);
        check_true("ev cites ReDMCSB COMMAND.C F0359",
                   strstr(ev, "ReDMCSB COMMAND.C F0359") != NULL);
        check_true("ev cites THQUEST.ASM T600",
                   strstr(ev, "THQUEST.ASM T600") != NULL);
        check_true("ev cites HuC6260", strstr(ev, "HuC6260") != NULL);
    }

    /* ── 9. Null safety ───────────────────────────────────────────── */
    theron_v2_hud_launch_mode_touch_hittest(0, 0, NULL);
    check_true("null hittest result: no crash", 1);

    printf("=== Theron V2 HUD launch-mode: %d / %d assertions passed ===\n",
           g_passed, g_assertions);
    return (g_passed == g_assertions) ? 0 : 1;
}
