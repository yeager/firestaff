#ifndef FIRESTAFF_THERON_V2_HUD_LAUNCH_MODE_PC34_H
#define FIRESTAFF_THERON_V2_HUD_LAUNCH_MODE_PC34_H

/*
 * theron_v2_hud_launch_mode_pc34.h
 *
 * Theron V2 HUD launch-mode gate (presentation-only).
 *
 * What this is:
 *   A bounded M11 launch-intent -> Theron V2 HUD overlay launch-mode
 *   selector. It exists so the launcher can pick a presentation-only
 *   HUD overlay behaviour at launch time WITHOUT changing any V1
 *   game-logic, input, champion, world, save, or Track 02 bank state.
 *
 * Why it exists:
 *   The existing Theron V2 HUD overlay (theron_v2_hud_overlay_pc34)
 *   is presentation-only and its M11 wire-up is gated on the
 *   presentation-mode selector (V1_FAITHFUL skips it, V20/V21/V22
 *   enable it). However the launcher may want a finer-grained
 *   *launch-mode* knob that further controls how the HUD behaves
 *   relative to input devices:
 *
 *     OFF         - no HUD overlay at all, even under V20/V21/V22
 *                   (V1 chrome preserved). Mirrors the historical
 *                   PC Engine "no-overlay" presentation.
 *     OVERLAY     - the existing V2.0/V2.1 HUD overlay (compass,
 *                   quest, dungeon progress, relic counter, rune
 *                   indicator, champion bars, action strip).
 *     TOUCH       - OVERLAY + touch-zone hit-test reporting. The
 *                   HUD overlay surface is widened with the
 *                   sibling TOUCHCLICK_Compat_* zone matrix so a
 *                   touch device can address top-bar / champion-bar
 *                   / action-strip regions. Hit-test results are
 *                   returned to the caller; this module never
 *                   dispatches V1 click routes.
 *     CONTROLLER  - OVERLAY + a controller-glyph hint rail painted
 *                   into the top-bar zone (4 cardinal glyph slots
 *                   + 5 action glyph slots). Pure presentation; no
 *                   controller input is read or routed.
 *
 *   All four modes are presentation-only. None of them mutate V1
 *   state. None of them claim a finished bitmap/PBR art surface;
 *   the existing placeholder/rectangle chrome from
 *   theron_v2_hud_overlay_pc34 is the documented baseline.
 *
 * Phase gate integration:
 *   THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE is a new V2-eligible
 *   domain in theron_v2_phase_gate_pc34. The launcher launch-mode
 *   selector consults the phase gate so:
 *     - V1-only default (v2PresentationEnabled=0) -> mode resolves
 *       to OFF (HUD launch-mode is locked to V1 chrome).
 *     - V2-on, config-persistence-on (v2PresentationEnabled=1 &&
 *       v2ConfigPersistenceEnabled=1) -> mode can be any of
 *       OFF / OVERLAY / TOUCH / CONTROLLER.
 *     - V2-on, config-persistence-off -> mode is locked to OFF.
 *       Rationale: TOUCH/CONTROLLER persist per-zone hit-test
 *       state into the M12 settings store, so they require the
 *       config-persistence toggle. OVERLAY does not persist, so
 *       it's allowed with V2 alone (mirrors the FILTER_CONFIG
 *       domain rule but inverts the meaning: FILTER_CONFIG requires
 *       both; HUD_LAUNCH_MODE requires V2 alone for OFF/OVERLAY,
 *       both for TOUCH/CONTROLLER).
 *
 * Skip-safe defaults:
 *   The module is skip-safe by default:
 *     - HUD launch mode defaults to OFF (matches V1-only behavior).
 *     - Per-zone hit-test reporting is OFF by default.
 *     - Controller glyph rail is OFF by default.
 *     - When V1 chrome is preserved (V1_FAITHFUL active), the
 *       module becomes a no-op regardless of launch-mode selection.
 *     - When the modern asset pack is absent, the controller glyph
 *       rail uses the placeholder rectangle baseline (no real art).
 *
 * Source-lock anchors:
 *   THQUEST.ASM T080  between-dungeon save/load
 *   THQUEST.ASM T400  dungeon bank loading
 *   THQUEST.ASM T520  party placement / start position
 *   THQUEST.ASM T560  dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  UI overlay zones (top-bar / right / bottom)
 *   THQUEST.ASM T700  timers / world tick
 *   THQUEST.ASM T800  champion persistence + inventory reset
 *   THQUEST.ASM T900  object database / thing list / rune magic
 *   HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)
 *   HuC6280 CPU datasheet
 *   ReDMCSB COMMAND.C F0359 LoadGameSettings (M12 menu selection)
 *   ReDMCSB PANEL.C F0354 (champion status-box drawing, sibling)
 *   ReDMCSB CLIKMENU.C F0365/F0366 (turn/move, sibling pattern)
 *   ReDMCSB MOVESENS.C:475-538 (move/turn sense table, sibling)
 *   include/touch_click_zone_matrix_pc34_compat.h (sibling matrix)
 *   include/dm1_v2_hud_interaction_pc34.h (DM1 V2 sibling)
 *   dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *
 * Module:
 *   src/theron/theron_v2_hud_launch_mode_pc34.c
 * Test:
 *   tests/test_theron_v2_hud_launch_mode_pc34.c
 * Probe:
 *   probes/firestaff_theron_v2_hud_launch_mode_probe.c
 *
 * Out of scope for this initial seed:
 *   - Real touch input dispatch (the launcher does not yet route
 *     touch events into M11; that is a separate launcher-side
 *     contract tracked in TODO.md).
 *   - Real controller input dispatch (no SDL gamepad subscription
 *     is opened by the launch-mode module; the controller glyph
 *     rail is a presentation-only hint).
 *   - Per-zone hit-test result persistence into M12 settings.
 *   - Real-art bitmap / PBR glyph assets under
 *     ~/.firestaff/assets/theron/hud/controller_glyphs/.
 *   - README-eligible screenshot proof (this seed is data-free).
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Launch-mode enum ──────────────────────────────────────────────── */

typedef enum {
    /* V1-faithful: HUD launch mode is OFF (no overlay). Default. */
    THERON_V2_HUD_LAUNCH_MODE_OFF = 0,
    /* OVERLAY: presentation-only HUD overlay enabled (compass +
     * quest + dungeon + relic + rune + champion bars + action
     * strip). Mirrors the existing M11 wire-up. */
    THERON_V2_HUD_LAUNCH_MODE_OVERLAY = 1,
    /* TOUCH: OVERLAY + per-zone hit-test reporting via the
     * sibling TOUCHCLICK_Compat_* matrix. The module returns a
     * hit-test result when asked; it never dispatches the V1
     * click route. */
    THERON_V2_HUD_LAUNCH_MODE_TOUCH = 2,
    /* CONTROLLER: OVERLAY + controller-glyph hint rail in the
     * top-bar zone (4 cardinal slots + 5 action slots). Pure
     * presentation. */
    THERON_V2_HUD_LAUNCH_MODE_CONTROLLER = 3
} Theron_V2_HudLaunchMode;

/* ── Per-zone hit-test reporting (TOUCH mode) ─────────────────────── */

/* Logical HUD zones the hit-test can report. The matrix adapter
 * uses the sibling TOUCHCLICK_Compat_* ordinals (see
 * touch_click_zone_matrix_pc34_compat.h) under the hood; the
 * Theron-specific wrapper maps back to this enum so M11 callers
 * do not have to know the cross-game ordinal table. */
typedef enum {
    THERON_V2_HUD_LAUNCH_ZONE_NONE = 0,
    THERON_V2_HUD_LAUNCH_ZONE_TOP_BAR = 1,
    THERON_V2_HUD_LAUNCH_ZONE_COMPASS = 2,
    THERON_V2_HUD_LAUNCH_ZONE_QUEST_ITEMS = 3,
    THERON_V2_HUD_LAUNCH_ZONE_DUNGEON_PROGRESS = 4,
    THERON_V2_HUD_LAUNCH_ZONE_RELIC_COUNTER = 5,
    THERON_V2_HUD_LAUNCH_ZONE_RUNE_INDICATOR = 6,
    THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_0 = 7,
    THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_1 = 8,
    THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_2 = 9,
    THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_3 = 10,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_STRIP = 11,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_ATTACK = 12,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_CAST = 13,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_USE = 14,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_DROP = 15,
    THERON_V2_HUD_LAUNCH_ZONE_ACTION_MOVE = 16,
    THERON_V2_HUD_LAUNCH_ZONE_CONTROLLER_GLYPH = 17
} Theron_V2_HudLaunchZone;

/* Result of a touch hit-test against the Theron V2 HUD overlay
 * surface. Coordinates are in the 256x224 indexed framebuffer
 * coordinate space (PC Engine native). The launcher can use this
 * to drive its own touch-input UI, but the module itself does NOT
 * dispatch V1 click routes. */
typedef struct {
    int hit;                              /* 1 if a zone was hit, else 0 */
    Theron_V2_HudLaunchZone zone;         /* hit zone (NONE if !hit) */
    int framebufferX;                     /* x in 256x224 indexed fb space */
    int framebufferY;                     /* y in 256x224 indexed fb space */
    int champion_index;                   /* -1 unless a champion bar was hit */
    int action_index;                     /* -1 unless an action icon was hit */
    int ordinal;                          /* ordinal in the sibling matrix */
    const char* zone_name;                /* human-readable zone label */
} Theron_V2_HudLaunchTouchResult;

/* ── Controller-glyph rail (CONTROLLER mode) ───────────────────────── */

/* 4 cardinal directions (compass) + 5 action icons (attack / cast /
 * use / drop / move). The glyph rail paints a fixed-size rectangle
 * baseline at the documented anchor positions in the top-bar zone;
 * the rail does not read input, it is a presentation hint. */
typedef enum {
    THERON_V2_HUD_LAUNCH_GLYPH_NORTH = 0,
    THERON_V2_HUD_LAUNCH_GLYPH_EAST = 1,
    THERON_V2_HUD_LAUNCH_GLYPH_SOUTH = 2,
    THERON_V2_HUD_LAUNCH_GLYPH_WEST = 3,
    THERON_V2_HUD_LAUNCH_GLYPH_COUNT = 4
} Theron_V2_HudLaunchCardinal;

typedef enum {
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_ATTACK = 0,
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_CAST = 1,
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_USE = 2,
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_DROP = 3,
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_MOVE = 4,
    THERON_V2_HUD_LAUNCH_GLYPH_ACTION_COUNT = 5
} Theron_V2_HudLaunchActionGlyph;

typedef struct {
    int cardinal_active[THERON_V2_HUD_LAUNCH_GLYPH_COUNT];
    int action_active[THERON_V2_HUD_LAUNCH_GLYPH_ACTION_COUNT];
    int visible; /* 1 when CONTROLLER mode is active AND V2 chrome is live */
    int x;
    int y;
    int w;
    int h;
} Theron_V2_HudLaunchControllerGlyphRail;

/* ── Live state ────────────────────────────────────────────────────── */

typedef struct {
    Theron_V2_HudLaunchMode mode;
    Theron_V2_HudLaunchMode requested;
    Theron_V2_HudLaunchMode resolved;
    int v1FaithfulActive;       /* 1 when presentation-mode is V1_FAITHFUL */
    int v2PresentationEnabled;  /* phase-gate config.v2PresentationEnabled */
    int configPersistenceEnabled; /* phase-gate config.v2ConfigPersistenceEnabled */
    int overlayAllowed;         /* 1 when HUD overlay should render */
    int touchAllowed;           /* 1 when touch hit-test is enabled */
    int controllerAllowed;      /* 1 when controller glyph rail is enabled */
    int modernPackAvailable;    /* 1 when V22 modern assets are present */
    Theron_V2_HudLaunchControllerGlyphRail controller_glyph_rail;
    uint32_t setCount;          /* monotonically increasing on each set */
} Theron_V2_HudLaunchModeState;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

void theron_v2_hud_launch_mode_reset(void);

/* ── Setters ───────────────────────────────────────────────────────── */

/* Set the launch mode directly. Resolves through the phase gate and
 * V22 modern-pack availability (CONTROLLER + missing pack -> OVERLAY). */
void theron_v2_hud_launch_mode_set(Theron_V2_HudLaunchMode mode);

/* Set the phase-gate config that the launch-mode selector consults.
 * Re-resolves the active mode so toggling the gate is observable
 * immediately. */
void theron_v2_hud_launch_mode_set_phase_gate(int v2PresentationEnabled,
                                              int configPersistenceEnabled);

/* Set the V22 modern-pack availability hint (used to downgrade
 * CONTROLLER -> OVERLAY when no real glyph art is available). */
void theron_v2_hud_launch_mode_set_modern_pack_available(int available);

/* Set the V1-faithful flag (1 when presentation-mode is V1_FAITHFUL).
 * When V1_FAITHFUL is active the launcher-mode is locked to OFF. */
void theron_v2_hud_launch_mode_set_v1_faithful(int v1FaithfulActive);

/* ── Getters ───────────────────────────────────────────────────────── */

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get(void);
Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get_requested(void);
Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get_resolved(void);
const Theron_V2_HudLaunchModeState*
    theron_v2_hud_launch_mode_state(void);

/* ── Per-mode predicates ───────────────────────────────────────────── */

int theron_v2_hud_launch_mode_is_off(void);
int theron_v2_hud_launch_mode_is_overlay(void);
int theron_v2_hud_launch_mode_is_touch(void);
int theron_v2_hud_launch_mode_is_controller(void);
int theron_v2_hud_launch_mode_allows_overlay(void);
int theron_v2_hud_launch_mode_allows_touch(void);
int theron_v2_hud_launch_mode_allows_controller(void);

/* ── Resolution helper (test-only) ─────────────────────────────────── */

/* Resolve a requested mode against the phase gate + V1-faithful flag
 * + modern-pack availability. Pure function; no global state. */
Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_resolve(
    Theron_V2_HudLaunchMode requested,
    int v2PresentationEnabled,
    int configPersistenceEnabled,
    int v1FaithfulActive,
    int modernPackAvailable);

/* Map the M11 launch spec hudLaunchMode int (M12-launched) onto the
 * Theron_V2_HudLaunchMode enum. Returns OFF for out-of-range values. */
Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_from_m11(int m11HudLaunchMode);

/* ── Touch hit-test (TOUCH mode) ───────────────────────────────────── */

/* Run a hit-test against the Theron V2 HUD overlay surface. Returns
 * 1 when a zone was hit (and fills outResult), else 0. Always
 * non-mutating. When the resolved mode does not allow touch the
 * function returns 0 with zone=NONE. The screenX/screenY inputs are
 * interpreted in the 256x224 indexed framebuffer coordinate space
 * (the same coordinate space the HUD overlay paints into). */
int theron_v2_hud_launch_mode_touch_hittest(int screenX, int screenY,
                                            Theron_V2_HudLaunchTouchResult* outResult);

/* ── Controller glyph rail (CONTROLLER mode) ───────────────────────── */

void theron_v2_hud_launch_mode_controller_set_active(
    Theron_V2_HudLaunchCardinal cardinal,
    int active);
void theron_v2_hud_launch_mode_controller_set_action_active(
    Theron_V2_HudLaunchActionGlyph action,
    int active);
void theron_v2_hud_launch_mode_controller_reset_active(void);
int theron_v2_hud_launch_mode_controller_should_render(void);

/* ── Source evidence ───────────────────────────────────────────────── */

const char* theron_v2_hud_launch_mode_name(Theron_V2_HudLaunchMode mode);
const char* theron_v2_hud_launch_mode_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_HUD_LAUNCH_MODE_PC34_H */
