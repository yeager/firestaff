/*
 * test_m11_qol_runtime_pc34_compat.c
 *
 * Data-free contract test for the M11 QoL runtime singleton
 * (src/engine/m11_qol_runtime.c + include/m11_qol_runtime.h).
 *
 * The QoL runtime is the single source of truth for the four user-facing
 * gameplay extras (Speed Control, Minimap, Auto-Map, Combat Log) that
 * live in the M11 game view and the M12 launcher config.  A wrong
 * clamp, a missed init-from-config field, or a stuck toggle would
 * silently corrupt gameplay settings for every Firestaff launch, so
 * this regression pins:
 *
 *   - The default state matches original DM1 behaviour (speed=100,
 *     minimap=off, autoMap=on, combatLog=off, sizes within bounds).
 *   - InitFromConfig picks up every M12_Config field used by the four
 *     extras and tolerates a NULL config without crashing or
 *     mutating state.
 *   - InitFromConfig honours the documented bounds:
 *       gameSpeedMultiplier: 50 / 100 / 150 / 200 (clamped)
 *       minimapSize:         64..256  (out-of-range falls back to 128)
 *       minimapCorner:       0..3     (out-of-range falls back to 0)
 *       combatLogMaxLines:   50..500  (out-of-range falls back to 200)
 *   - SetSpeedMultiplier clamps arbitrary input to the same 4-step
 *     grid (50 → 100, 51..100 → 100, 101..150 → 150, 151..200 → 200,
 *     201+ → 200, 0 → 50, negative → 50).
 *   - CycleSpeedMultiplier walks 50→100→150→200→50 in that order,
 *     matching the F6 hotkey contract documented in the header.
 *   - Toggle/Minimap/CombatLog return the post-toggle state and
 *     treat any non-zero as 1 (no leakage from weird bit patterns).
 *   - GetMinimapSize / GetMinimapCorner / GetAutoMapEnabled /
 *     GetCombatLogMaxLines are pure getters and reflect what was
 *     last stored via InitFromConfig.
 *
 * Source: include/m11_qol_runtime.h + include/config_m12.h + src/engine/m11_qol_runtime.c
 *   (no ReDMCSB equivalent; Firestaff gameplay/quality-of-life extras).
 */

#include "m11_qol_runtime.h"
#include "config_m12.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        ++g_failures;
    }
}

static void reset_to_original_defaults(void) {
    /* The runtime has no public reset; the cleanest way to drop back
     * to the original-DM1 baseline is to construct a config with the
     * documented defaults and feed it back in. */
    M12_Config defaults;
    memset(&defaults, 0, sizeof(defaults));
    defaults.gameSpeedMultiplier = 100;
    defaults.minimapEnabled      = 0;
    defaults.minimapSize         = 128;
    defaults.minimapCorner       = 0;
    defaults.autoMapEnabled      = 1;
    defaults.combatLogEnabled    = 0;
    defaults.combatLogMaxLines   = 200;
    M11_QolRuntime_InitFromConfig(&defaults);
}

static void test_defaults_match_original_dm1(void) {
    /* No init => still original. */
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "default speed multiplier is 100 (original DM1)");
    check(M11_QolRuntime_GetMinimapEnabled() == 0,
          "default minimap disabled");
    check(M11_QolRuntime_GetAutoMapEnabled() == 1,
          "default auto-map enabled");
    check(M11_QolRuntime_GetCombatLogEnabled() == 0,
          "default combat log disabled");
    check(M11_QolRuntime_GetCombatLogMaxLines() == 200,
          "default combat log cap is 200 lines");
    check(M11_QolRuntime_GetMinimapSize() == 128,
          "default minimap size is 128 px");
    check(M11_QolRuntime_GetMinimapCorner() == 0,
          "default minimap corner is 0 (TR)");
}

static void test_init_from_null_config_is_safe(void) {
    reset_to_original_defaults();
    M11_QolRuntime_InitFromConfig(NULL);
    /* init with NULL must NOT crash AND must NOT mutate prior state. */
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "InitFromConfig(NULL) leaves speed at 100");
    check(M11_QolRuntime_GetMinimapEnabled() == 0,
          "InitFromConfig(NULL) leaves minimap off");
    check(M11_QolRuntime_GetCombatLogMaxLines() == 200,
          "InitFromConfig(NULL) leaves cap at 200");
}

static void test_init_from_config_picks_every_field(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.gameSpeedMultiplier = 150;
    cfg.minimapEnabled      = 1;
    cfg.minimapSize         = 192;
    cfg.minimapCorner       = 2;
    cfg.autoMapEnabled      = 0;
    cfg.combatLogEnabled    = 1;
    cfg.combatLogMaxLines   = 350;
    M11_QolRuntime_InitFromConfig(&cfg);

    check(M11_QolRuntime_GetSpeedMultiplier() == 150,
          "speed multiplier round-trips through config");
    check(M11_QolRuntime_GetMinimapEnabled() == 1,
          "minimap enabled round-trips through config");
    check(M11_QolRuntime_GetMinimapSize() == 192,
          "minimap size round-trips through config");
    check(M11_QolRuntime_GetMinimapCorner() == 2,
          "minimap corner round-trips through config");
    check(M11_QolRuntime_GetAutoMapEnabled() == 0,
          "auto-map disable round-trips through config");
    check(M11_QolRuntime_GetCombatLogEnabled() == 1,
          "combat log enabled round-trips through config");
    check(M11_QolRuntime_GetCombatLogMaxLines() == 350,
          "combat log cap round-trips through config");
}

static void test_init_clamps_speed_multiplier(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.gameSpeedMultiplier = 50;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 50,
          "speed=50 clamps to 50");

    cfg.gameSpeedMultiplier = 200;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 200,
          "speed=200 clamps to 200");

    cfg.gameSpeedMultiplier = 0;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "speed=0 falls back to the 100 default (config path)");

    cfg.gameSpeedMultiplier = 75; /* falls into the <=100 branch */
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "speed=75 clamps to 100");

    cfg.gameSpeedMultiplier = 199; /* falls into the <=200 branch */
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 200,
          "speed=199 clamps to 200");

    cfg.gameSpeedMultiplier = -42; /* negative should not crash */
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "negative speed falls back to 100 (config path)");
}

static void test_init_clamps_minimap_size(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.minimapSize = 64;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapSize() == 64,
          "minimap size=64 stays 64");

    cfg.minimapSize = 256;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapSize() == 256,
          "minimap size=256 stays 256");

    cfg.minimapSize = 63;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapSize() == 128,
          "minimap size=63 falls back to 128");

    cfg.minimapSize = 257;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapSize() == 128,
          "minimap size=257 falls back to 128");

    cfg.minimapSize = 100; /* in-range */
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapSize() == 100,
          "minimap size=100 stays 100");
}

static void test_init_clamps_minimap_corner(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.minimapCorner = 0;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapCorner() == 0, "corner 0 stays 0");

    cfg.minimapCorner = 3;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapCorner() == 3, "corner 3 stays 3");

    cfg.minimapCorner = -1;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapCorner() == 0, "corner -1 falls back to 0");

    cfg.minimapCorner = 4;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetMinimapCorner() == 0, "corner 4 falls back to 0");
}

static void test_init_clamps_combat_log_max_lines(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.combatLogMaxLines = 50;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetCombatLogMaxLines() == 50,
          "cap=50 stays 50");

    cfg.combatLogMaxLines = 500;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetCombatLogMaxLines() == 500,
          "cap=500 stays 500");

    cfg.combatLogMaxLines = 49;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetCombatLogMaxLines() == 200,
          "cap=49 falls back to 200");

    cfg.combatLogMaxLines = 501;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetCombatLogMaxLines() == 200,
          "cap=501 falls back to 200");

    cfg.combatLogMaxLines = 100;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetCombatLogMaxLines() == 100,
          "cap=100 stays 100");
}

static void test_set_speed_multiplier_clamps(void) {
    /* Independent path: SetSpeedMultiplier must clamp on its own
     * without going through the config. */
    M11_QolRuntime_SetSpeedMultiplier(50);
    check(M11_QolRuntime_GetSpeedMultiplier() == 50, "set 50 clamps to 50");

    M11_QolRuntime_SetSpeedMultiplier(75);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100, "set 75 clamps to 100");

    M11_QolRuntime_SetSpeedMultiplier(125);
    check(M11_QolRuntime_GetSpeedMultiplier() == 150, "set 125 clamps to 150");

    M11_QolRuntime_SetSpeedMultiplier(200);
    check(M11_QolRuntime_GetSpeedMultiplier() == 200, "set 200 clamps to 200");

    M11_QolRuntime_SetSpeedMultiplier(999);
    check(M11_QolRuntime_GetSpeedMultiplier() == 200, "set 999 clamps to 200");

    M11_QolRuntime_SetSpeedMultiplier(0);
    check(M11_QolRuntime_GetSpeedMultiplier() == 50, "set 0 clamps to 50");

    M11_QolRuntime_SetSpeedMultiplier(-7);
    check(M11_QolRuntime_GetSpeedMultiplier() == 50, "set -7 clamps to 50");

    /* Boundary 1: anything 51..100 lands on 100 (not 50). */
    M11_QolRuntime_SetSpeedMultiplier(51);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "set 51 clamps to 100, not 50");
}

static void test_cycle_speed_multiplier_order(void) {
    M11_QolRuntime_SetSpeedMultiplier(50);
    check(M11_QolRuntime_CycleSpeedMultiplier() == 100,
          "cycle 50 -> 100");
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "cycle persists to global state");

    check(M11_QolRuntime_CycleSpeedMultiplier() == 150,
          "cycle 100 -> 150");
    check(M11_QolRuntime_CycleSpeedMultiplier() == 200,
          "cycle 150 -> 200");
    check(M11_QolRuntime_CycleSpeedMultiplier() == 50,
          "cycle 200 -> 50 wraps to 50");

    /* Out-of-grid values must snap back to a known step then continue. */
    M11_QolRuntime_SetSpeedMultiplier(73);
    check(M11_QolRuntime_GetSpeedMultiplier() == 100,
          "SetSpeedMultiplier(73) snaps to 100");

    M11_QolRuntime_SetSpeedMultiplier(201);
    check(M11_QolRuntime_GetSpeedMultiplier() == 200,
          "SetSpeedMultiplier(201) clamps to 200");

    M11_QolRuntime_SetSpeedMultiplier(100);
    check(M11_QolRuntime_CycleSpeedMultiplier() == 150,
          "cycle 100 -> 150 again");
}

static void test_toggle_minimap_round_trip(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.minimapEnabled = 0;
    M11_QolRuntime_InitFromConfig(&cfg);

    check(M11_QolRuntime_ToggleMinimap() == 1,
          "ToggleMinimap 0 -> 1 returns 1");
    check(M11_QolRuntime_GetMinimapEnabled() == 1,
          "ToggleMinimap 0 -> 1 persists");

    check(M11_QolRuntime_ToggleMinimap() == 0,
          "ToggleMinimap 1 -> 0 returns 0");
    check(M11_QolRuntime_GetMinimapEnabled() == 0,
          "ToggleMinimap 1 -> 0 persists");
}

static void test_toggle_minimap_truthiness(void) {
    /* Any nonzero treated as 1 (truthiness normalization), so
     * SetMinimapEnabled(42) flips the state on without leaking the
     * integer into the getter.  SetMinimapEnabled(0) collapses to 0. */
    M11_QolRuntime_SetMinimapEnabled(0);
    M11_QolRuntime_SetMinimapEnabled(42);
    check(M11_QolRuntime_GetMinimapEnabled() == 1,
          "SetMinimapEnabled(42) normalizes to 1");
    M11_QolRuntime_SetMinimapEnabled(0);
    check(M11_QolRuntime_GetMinimapEnabled() == 0,
          "SetMinimapEnabled(0) normalizes to 0");
    /* Confirms the runtime still treats negative as truthy: callers
     * should pass 0/1; the API does not clamp negatives. */
    M11_QolRuntime_SetMinimapEnabled(0);
    M11_QolRuntime_SetMinimapEnabled(-7);
    check(M11_QolRuntime_GetMinimapEnabled() == 1,
          "SetMinimapEnabled(-7) is treated as truthy (nonzero)");
}

static void test_toggle_combat_log_round_trip(void) {
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.combatLogEnabled = 0;
    M11_QolRuntime_InitFromConfig(&cfg);

    check(M11_QolRuntime_ToggleCombatLog() == 1,
          "ToggleCombatLog 0 -> 1 returns 1");
    check(M11_QolRuntime_GetCombatLogEnabled() == 1,
          "ToggleCombatLog 0 -> 1 persists");

    check(M11_QolRuntime_ToggleCombatLog() == 0,
          "ToggleCombatLog 1 -> 0 returns 0");
}

static void test_auto_map_is_getter_only(void) {
    /* Auto-map is a read-only gameplay signal — toggles must not
     * exist on the public API.  Confirm the getter round-trips a
     * disabled config and is unaffected by other toggles. */
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.autoMapEnabled = 0;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetAutoMapEnabled() == 0,
          "GetAutoMapEnabled reads disabled config");

    /* Toggle minimap a few times; auto-map must stay disabled. */
    (void)M11_QolRuntime_ToggleMinimap();
    (void)M11_QolRuntime_ToggleMinimap();
    (void)M11_QolRuntime_ToggleCombatLog();
    (void)M11_QolRuntime_ToggleCombatLog();
    check(M11_QolRuntime_GetAutoMapEnabled() == 0,
          "auto-map stays disabled across other toggles");

    cfg.autoMapEnabled = 1;
    M11_QolRuntime_InitFromConfig(&cfg);
    check(M11_QolRuntime_GetAutoMapEnabled() == 1,
          "GetAutoMapEnabled reads enabled config");
}

static void test_init_does_not_leak_between_calls(void) {
    /* A second InitFromConfig must overwrite ALL fields, not just
     * the ones that happen to be touched by the second call. */
    M12_Config first;
    M12_Config second;
    memset(&first,  0, sizeof(first));
    memset(&second, 0, sizeof(second));

    first.minimapSize      = 192;
    first.combatLogMaxLines = 400;
    first.minimapCorner    = 2;
    M11_QolRuntime_InitFromConfig(&first);
    check(M11_QolRuntime_GetMinimapSize() == 192,
          "first init sets minimap size 192");

    second.minimapSize      = 64;
    second.combatLogMaxLines = 50; /* valid in-range; would not force
                                       fallback if the second init
                                       forgot to copy it. */
    second.minimapCorner    = 1;
    M11_QolRuntime_InitFromConfig(&second);
    check(M11_QolRuntime_GetMinimapSize() == 64,
          "second init fully overwrites minimap size");
    check(M11_QolRuntime_GetMinimapCorner() == 1,
          "second init fully overwrites minimap corner");
    check(M11_QolRuntime_GetCombatLogMaxLines() == 50,
          "second init fully overwrites combat log cap");
}

int main(void) {
    test_defaults_match_original_dm1();
    test_init_from_null_config_is_safe();
    test_init_from_config_picks_every_field();
    test_init_clamps_speed_multiplier();
    test_init_clamps_minimap_size();
    test_init_clamps_minimap_corner();
    test_init_clamps_combat_log_max_lines();
    test_set_speed_multiplier_clamps();
    test_cycle_speed_multiplier_order();
    test_toggle_minimap_round_trip();
    test_toggle_minimap_truthiness();
    test_toggle_combat_log_round_trip();
    test_auto_map_is_getter_only();
    test_init_does_not_leak_between_calls();

    if (g_failures) {
        printf("test_m11_qol_runtime_pc34_compat: FAIL %d\n", g_failures);
        return 1;
    }
    puts("test_m11_qol_runtime_pc34_compat: PASS");
    return 0;
}
