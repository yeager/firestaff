/*
 * theron_v2_hud_launch_mode_pc34.c
 *
 * Theron V2 HUD launch-mode gate (presentation-only).
 *
 * Source-lock anchors:
 *   THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900
 *   HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)
 *   ReDMCSB COMMAND.C F0359 LoadGameSettings (M12 menu selection)
 *   ReDMCSB PANEL.C F0354 / CLIKMENU.C F0365/F0366 / MOVESENS.C:475-538
 *   include/touch_click_zone_matrix_pc34_compat.h (sibling matrix)
 *   include/dm1_v2_hud_interaction_pc34.h (DM1 V2 sibling)
 *   include/theron_v2_phase_gate_pc34.h (V1 compatibility lock)
 *   include/theron_v2_presentation_mode_pc34.h (V2 presentation mode)
 *   include/theron_v2_hud_overlay_pc34.h (V2 HUD overlay surface)
 *   dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *
 * Architecture:
 *   The launch-mode gate is a thin presentation-only selector. It
 *   reads the phase-gate config, the V1-faithful flag, the V22
 *   modern-pack availability, and a caller-supplied requested mode.
 *   It resolves them down to one of OFF / OVERLAY / TOUCH /
 *   CONTROLLER. None of the resolution paths mutate V1 game-logic,
 *   input, champion, world, save, or Track 02 bank state.
 *
 *   Skip-safe defaults:
 *     - Default launch mode: OFF.
 *     - V1_FAITHFUL active: locked to OFF.
 *     - V2-off (phase gate): locked to OFF.
 *     - CONTROLLER + missing modern pack: downgraded to OVERLAY.
 *     - TOUCH without config-persistence: downgraded to OVERLAY.
 *     - OVERLAY alone: allowed when V2-on (no persistence needed).
 *
 *   No file I/O, no SDL, no real-asset dependency.
 */

#include "theron_v2_hud_launch_mode_pc34.h"
#include "theron_v2_phase_gate_pc34.h"

#include <string.h>

/* No forward-declared dependency on the sibling touch-zone matrix
 * module: this module's hit-test is presentation-only and stays
 * inside the documented Theron HUD overlay geometry. Sibling matrix
 * ordinals are recorded for callers that want to wire through
 * TOUCHCLICK_Compat_GetZone() but the launch-mode gate itself does
 * not call the sibling matrix at runtime. */

/* ── Module-level state ─────────────────────────────────────────────── */

static Theron_V2_HudLaunchModeState g_state;

static void launch_mode_apply_predicates(Theron_V2_HudLaunchMode resolved)
{
    g_state.resolved = resolved;
    g_state.overlayAllowed = (resolved == THERON_V2_HUD_LAUNCH_MODE_OVERLAY ||
                              resolved == THERON_V2_HUD_LAUNCH_MODE_TOUCH ||
                              resolved == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER) ? 1 : 0;
    g_state.touchAllowed = (resolved == THERON_V2_HUD_LAUNCH_MODE_TOUCH) ? 1 : 0;
    g_state.controllerAllowed = (resolved == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER) ? 1 : 0;

    g_state.controller_glyph_rail.visible =
        (resolved == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER) ? 1 : 0;
}

static void launch_mode_recompute(void)
{
    Theron_V2_HudLaunchMode resolved =
        theron_v2_hud_launch_mode_resolve(
            g_state.requested,
            g_state.v2PresentationEnabled,
            g_state.configPersistenceEnabled,
            g_state.v1FaithfulActive,
            g_state.modernPackAvailable);
    g_state.mode = resolved;
    launch_mode_apply_predicates(resolved);
}

/* ── Public API ─────────────────────────────────────────────────────── */

void theron_v2_hud_launch_mode_reset(void)
{
    memset(&g_state, 0, sizeof(g_state));
    g_state.requested = THERON_V2_HUD_LAUNCH_MODE_OFF;
    g_state.resolved = THERON_V2_HUD_LAUNCH_MODE_OFF;
    g_state.mode = THERON_V2_HUD_LAUNCH_MODE_OFF;
    g_state.v1FaithfulActive = 1;     /* V1-by-default */
    g_state.v2PresentationEnabled = 0;
    g_state.configPersistenceEnabled = 0;
    g_state.modernPackAvailable = 0;
    g_state.overlayAllowed = 0;
    g_state.touchAllowed = 0;
    g_state.controllerAllowed = 0;
    /* Glyph rail defaults. The rail paints inside the top-bar zone
     * (y=0..23); the documented anchor positions are mirror-aligned
     * with the existing THERON_V2_HUD_COMPASS_CX / _CY / _QUEST_X /
     * _DUNGEON_X / _RELIC_X zones. */
    g_state.controller_glyph_rail.x = 0;
    g_state.controller_glyph_rail.y = 0;
    g_state.controller_glyph_rail.w = 256; /* TQR_FB_W */
    g_state.controller_glyph_rail.h = 24;  /* top-bar height */
    g_state.controller_glyph_rail.visible = 0;
    launch_mode_apply_predicates(THERON_V2_HUD_LAUNCH_MODE_OFF);
}

void theron_v2_hud_launch_mode_set(Theron_V2_HudLaunchMode mode)
{
    g_state.requested = mode;
    g_state.setCount++;
    launch_mode_recompute();
}

void theron_v2_hud_launch_mode_set_phase_gate(int v2PresentationEnabled,
                                              int configPersistenceEnabled)
{
    g_state.v2PresentationEnabled = (v2PresentationEnabled != 0) ? 1 : 0;
    g_state.configPersistenceEnabled = (configPersistenceEnabled != 0) ? 1 : 0;
    launch_mode_recompute();
}

void theron_v2_hud_launch_mode_set_modern_pack_available(int available)
{
    g_state.modernPackAvailable = (available != 0) ? 1 : 0;
    launch_mode_recompute();
}

void theron_v2_hud_launch_mode_set_v1_faithful(int v1FaithfulActive)
{
    g_state.v1FaithfulActive = (v1FaithfulActive != 0) ? 1 : 0;
    launch_mode_recompute();
}

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get(void)
{
    return g_state.mode;
}

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get_requested(void)
{
    return g_state.requested;
}

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_get_resolved(void)
{
    return g_state.resolved;
}

const Theron_V2_HudLaunchModeState*
    theron_v2_hud_launch_mode_state(void)
{
    return &g_state;
}

int theron_v2_hud_launch_mode_is_off(void)
{
    return g_state.mode == THERON_V2_HUD_LAUNCH_MODE_OFF;
}

int theron_v2_hud_launch_mode_is_overlay(void)
{
    return g_state.mode == THERON_V2_HUD_LAUNCH_MODE_OVERLAY;
}

int theron_v2_hud_launch_mode_is_touch(void)
{
    return g_state.mode == THERON_V2_HUD_LAUNCH_MODE_TOUCH;
}

int theron_v2_hud_launch_mode_is_controller(void)
{
    return g_state.mode == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER;
}

int theron_v2_hud_launch_mode_allows_overlay(void)
{
    return g_state.overlayAllowed;
}

int theron_v2_hud_launch_mode_allows_touch(void)
{
    return g_state.touchAllowed;
}

int theron_v2_hud_launch_mode_allows_controller(void)
{
    return g_state.controllerAllowed;
}

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_resolve(
    Theron_V2_HudLaunchMode requested,
    int v2PresentationEnabled,
    int configPersistenceEnabled,
    int v1FaithfulActive,
    int modernPackAvailable)
{
    /* V1_FAITHFUL wins: HUD launch-mode is locked to OFF whenever the
     * launcher selected V1_FAITHFUL (V1 chrome must be preserved). */
    if (v1FaithfulActive) {
        return THERON_V2_HUD_LAUNCH_MODE_OFF;
    }
    /* V2-off (phase gate): same V1-locked behaviour, even if the
     * caller asked for an overlay. This is the Phase 0 V1 source-
     * locked contract: the launch-mode selector MUST NOT escalate
     * past V1 chrome when the gate denies presentation. */
    if (!v2PresentationEnabled) {
        return THERON_V2_HUD_LAUNCH_MODE_OFF;
    }
    /* V2-on + persistence-off: TOUCH and CONTROLLER cannot persist
     * per-zone state into M12 settings, so they are downgraded.
     * OVERLAY does not persist and stays allowed. */
    if (requested == THERON_V2_HUD_LAUNCH_MODE_TOUCH && !configPersistenceEnabled) {
        return THERON_V2_HUD_LAUNCH_MODE_OVERLAY;
    }
    if (requested == THERON_V2_HUD_LAUNCH_MODE_CONTROLLER) {
        if (!configPersistenceEnabled) {
            return THERON_V2_HUD_LAUNCH_MODE_OVERLAY;
        }
        /* When modern pack is missing, controller glyph rail has no
         * real art to point at; downgrade to OVERLAY so the launch
         * remains bounded and skip-safe. */
        if (!modernPackAvailable) {
            return THERON_V2_HUD_LAUNCH_MODE_OVERLAY;
        }
    }
    /* OFF / OVERLAY (when allowed) pass through. */
    return requested;
}

Theron_V2_HudLaunchMode theron_v2_hud_launch_mode_from_m11(int m11HudLaunchMode)
{
    /* M11 launches the launcher HUD launch-mode as a signed int
     * aligned with M12's M12_HUD_LAUNCH_MODE_* contract; we accept
     * the canonical 0..3 values and clamp everything else to OFF. */
    switch (m11HudLaunchMode) {
        case 0: return THERON_V2_HUD_LAUNCH_MODE_OFF;
        case 1: return THERON_V2_HUD_LAUNCH_MODE_OVERLAY;
        case 2: return THERON_V2_HUD_LAUNCH_MODE_TOUCH;
        case 3: return THERON_V2_HUD_LAUNCH_MODE_CONTROLLER;
        default: return THERON_V2_HUD_LAUNCH_MODE_OFF;
    }
}

/* ── Touch hit-test (TOUCH mode) ───────────────────────────────────── */

/* Sibling matrix ordinal table for the Theron HUD zones. The
 * ordinals map onto the existing TOUCHCLICK_Compat_* matrix in
 * src/shared/touch_click_zone_matrix_pc34_compat.c.  We enumerate
 * a contiguous ordinal range so the hit-test loop stays simple.
 *
 * Source-lock: see include/touch_click_zone_matrix_pc34_compat.h
 * for the canonical matrix shape; this table is the Theron-
 * specific HUD wrapper that targets zones 0..(THERON_V2_HUD_LAUNCH
 * _ZONE_COUNT - 1) below. */
typedef struct {
    Theron_V2_HudLaunchZone zone;
    int ordinal;          /* sibling matrix ordinal */
    int x;
    int y;
    int w;
    int h;
    int champion_index;   /* -1 unless champion bar */
    int action_index;     /* -1 unless action icon */
    const char* name;
} HudLaunchMode_ZoneTableEntry;

/* Geometry constants mirror theron_v2_hud_overlay_pc34.h. Kept
 * verbatim so a hit-test against this module's zones matches a
 * painted pixel from the HUD overlay. */
#define HUD_LAUNCH_MODE_TOP_BAR_H       24
#define HUD_LAUNCH_MODE_CHAMP_BAR_Y     184
#define HUD_LAUNCH_MODE_CHAMP_BAR_H     8
#define HUD_LAUNCH_MODE_CHAMP_BAR_W     60
#define HUD_LAUNCH_MODE_CHAMP_BAR_X0    4
#define HUD_LAUNCH_MODE_CHAMP_BAR_SPACING 2
#define HUD_LAUNCH_MODE_ACTION_STRIP_Y  208
#define HUD_LAUNCH_MODE_ACTION_STRIP_H  14
#define HUD_LAUNCH_MODE_ACTION_ICON_W   28
#define HUD_LAUNCH_MODE_ACTION_ICONS_X0 16
#define HUD_LAUNCH_MODE_ACTION_SPACING  4

static const HudLaunchMode_ZoneTableEntry kZoneTable[] = {
    /* Top-bar sub-zones (specific -> checked first). */
    { THERON_V2_HUD_LAUNCH_ZONE_COMPASS,          11, 8,    4,  16,  16, -1, -1, "COMPASS" },
    { THERON_V2_HUD_LAUNCH_ZONE_RUNE_INDICATOR,   15, 36,   4,  22,  14, -1, -1, "RUNE_INDICATOR" },
    { THERON_V2_HUD_LAUNCH_ZONE_QUEST_ITEMS,      12, 64,   0,  40,  24, -1, -1, "QUEST_ITEMS" },
    { THERON_V2_HUD_LAUNCH_ZONE_DUNGEON_PROGRESS, 13, 160,  0,  40,  24, -1, -1, "DUNGEON_PROGRESS" },
    { THERON_V2_HUD_LAUNCH_ZONE_RELIC_COUNTER,    14, 220,  0,  36,  24, -1, -1, "RELIC_COUNTER" },
    /* Champion mini-bar slots (THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_0..3). */
    { THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_0,   20, 4,   184, 60, 8,  0, -1, "CHAMPION_BAR_0" },
    { THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_1,   21, 66,  184, 60, 8,  1, -1, "CHAMPION_BAR_1" },
    { THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_2,   22, 128, 184, 60, 8,  2, -1, "CHAMPION_BAR_2" },
    { THERON_V2_HUD_LAUNCH_ZONE_CHAMPION_BAR_3,   23, 190, 184, 60, 8,  3, -1, "CHAMPION_BAR_3" },
    /* Action strip + per-action icons (specific -> checked first). */
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_ATTACK,    31, 16,  208, 28, 14, -1, 0, "ACTION_ATTACK" },
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_CAST,      32, 48,  208, 28, 14, -1, 1, "ACTION_CAST" },
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_USE,       33, 80,  208, 28, 14, -1, 2, "ACTION_USE" },
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_DROP,      34, 112, 208, 28, 14, -1, 3, "ACTION_DROP" },
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_MOVE,      35, 144, 208, 28, 14, -1, 4, "ACTION_MOVE" },
    /* Fallback zones (broad -> checked last). */
    { THERON_V2_HUD_LAUNCH_ZONE_TOP_BAR,          10, 0,   0,   256, 24, -1, -1, "TOP_BAR" },
    { THERON_V2_HUD_LAUNCH_ZONE_ACTION_STRIP,     30, 16,  208, 200, 14, -1, -1, "ACTION_STRIP" },
    { THERON_V2_HUD_LAUNCH_ZONE_CONTROLLER_GLYPH, 40, 0,   0,   256, 24, -1, -1, "CONTROLLER_GLYPH_RAIL" }
};

#define HUD_LAUNCH_MODE_ZONE_COUNT \
    (int)(sizeof(kZoneTable)/sizeof(kZoneTable[0]))

/* Internal predicate: does (x,y) live inside the rect (rx,ry,rw,rh)? */
static int launch_mode_point_in_rect(int x, int y, int rx, int ry, int rw, int rh)
{
    return (x >= rx && x < rx + rw && y >= ry && y < ry + rh) ? 1 : 0;
}

int theron_v2_hud_launch_mode_touch_hittest(int screenX, int screenY,
                                            Theron_V2_HudLaunchTouchResult* outResult)
{
    if (!outResult) {
        return 0;
    }
    /* Zero out for cleanliness. */
    memset(outResult, 0, sizeof(*outResult));
    outResult->zone = THERON_V2_HUD_LAUNCH_ZONE_NONE;
    outResult->champion_index = -1;
    outResult->action_index = -1;
    outResult->framebufferX = screenX;
    outResult->framebufferY = screenY;

    if (!g_state.touchAllowed) {
        /* Touch is not active under the resolved mode. Skip-safe
         * default: return NO HIT. */
        return 0;
    }

    /* Walk the zone table. First-hit wins (matches the documented
     * THERON_V2_HUD_* geometry). The sibling matrix ordinals are
     * pushed into the result for callers that want to wire through
     * TOUCHCLICK_Compat_GetZone(). The zone geometry here is the
     * authoritative Theron HUD geometry. */
    for (int i = 0; i < HUD_LAUNCH_MODE_ZONE_COUNT; ++i) {
        const HudLaunchMode_ZoneTableEntry* z = &kZoneTable[i];
        if (launch_mode_point_in_rect(screenX, screenY, z->x, z->y, z->w, z->h)) {
            outResult->hit = 1;
            outResult->zone = z->zone;
            outResult->ordinal = z->ordinal;
            outResult->champion_index = z->champion_index;
            outResult->action_index = z->action_index;
            outResult->zone_name = z->name;
            return 1;
        }
    }
    /* Outside any zone: not a hit. The sibling matrix is consulted
     * only when the caller's input lands in coordinates outside the
     * Theron HUD overlay surface, in which case the matrix may
     * report a sibling-game zone (DM1/CSB). We deliberately do NOT
     * call the sibling matrix here because the Theron launch-mode
     * gate is presentation-only and must not depend on cross-game
     * ordinal resolution. */
    return 0;
}

/* ── Controller glyph rail ─────────────────────────────────────────── */

void theron_v2_hud_launch_mode_controller_set_active(
    Theron_V2_HudLaunchCardinal cardinal,
    int active)
{
    if ((int)cardinal < 0 || (int)cardinal >= THERON_V2_HUD_LAUNCH_GLYPH_COUNT) {
        return;
    }
    g_state.controller_glyph_rail.cardinal_active[cardinal] = (active != 0) ? 1 : 0;
}

void theron_v2_hud_launch_mode_controller_set_action_active(
    Theron_V2_HudLaunchActionGlyph action,
    int active)
{
    if ((int)action < 0 || (int)action >= THERON_V2_HUD_LAUNCH_GLYPH_ACTION_COUNT) {
        return;
    }
    g_state.controller_glyph_rail.action_active[action] = (active != 0) ? 1 : 0;
}

void theron_v2_hud_launch_mode_controller_reset_active(void)
{
    memset(g_state.controller_glyph_rail.cardinal_active, 0,
           sizeof(g_state.controller_glyph_rail.cardinal_active));
    memset(g_state.controller_glyph_rail.action_active, 0,
           sizeof(g_state.controller_glyph_rail.action_active));
}

int theron_v2_hud_launch_mode_controller_should_render(void)
{
    return (g_state.controllerAllowed && g_state.controller_glyph_rail.visible) ? 1 : 0;
}

/* ── Source evidence ───────────────────────────────────────────────── */

const char* theron_v2_hud_launch_mode_name(Theron_V2_HudLaunchMode mode)
{
    switch (mode) {
        case THERON_V2_HUD_LAUNCH_MODE_OFF:        return "OFF";
        case THERON_V2_HUD_LAUNCH_MODE_OVERLAY:    return "OVERLAY";
        case THERON_V2_HUD_LAUNCH_MODE_TOUCH:      return "TOUCH";
        case THERON_V2_HUD_LAUNCH_MODE_CONTROLLER: return "CONTROLLER";
    }
    return "UNKNOWN";
}

const char* theron_v2_hud_launch_mode_source_evidence(void)
{
    return
        "Theron V2 HUD launch-mode gate (presentation-only):\n"
        "  THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE  (NEW, V2-eligible)\n"
        "  ReDMCSB COMMAND.C F0359 LoadGameSettings - M12 menu selection\n"
        "  ReDMCSB PANEL.C F0354 - champion status-box drawing (sibling)\n"
        "  ReDMCSB CLIKMENU.C F0365/F0366 - V1 source-locked turn/move\n"
        "  ReDMCSB MOVESENS.C:475-538 - move/turn sense table\n"
        "  THQUEST.ASM T080 - between-dungeon save/load (V1 source-locked)\n"
        "  THQUEST.ASM T400 - dungeon bank loading (V1 source-locked)\n"
        "  THQUEST.ASM T520 - party placement / start position\n"
        "  THQUEST.ASM T560 - dungeon loading (header parsing, dungeon_seed)\n"
        "  THQUEST.ASM T600 - UI overlay zones (top-bar / right / bottom)\n"
        "  THQUEST.ASM T700 - timers / world tick (V1 source-locked)\n"
        "  THQUEST.ASM T800 - champion persistence + inventory reset\n"
        "  THQUEST.ASM T900 - object database / thing list / rune magic\n"
        "  HuC6260/HuC6270 datasheet - PC Engine VDC + VCE\n"
        "  HuC6280 CPU datasheet - PC Engine CPU\n"
        "  include/touch_click_zone_matrix_pc34_compat.h - sibling matrix\n"
        "  include/dm1_v2_hud_interaction_pc34.h - DM1 V2 sibling\n"
        "  include/theron_v2_phase_gate_pc34.h - V1 compatibility lock\n"
        "  include/theron_v2_presentation_mode_pc34.h - V2 mode selector\n"
        "  include/theron_v2_hud_overlay_pc34.h - V2 HUD overlay surface\n"
        "  theron_v2_hud_launch_mode_pc34.c - this module\n"
        "  dmweb Theron overview - 7 dungeons + 7 relic goals + rune magic\n"
        "  docs/source-lock/tqr_v1_phase2_data_formats_H2339.md\n"
        "  Resolution table:\n"
        "    V1_FAITHFUL=1                -> OFF (V1 chrome preserved)\n"
        "    v2PresentationEnabled=0     -> OFF (V1-locked phase gate)\n"
        "    OVERLAY                     -> OVERLAY (allowed when V2-on)\n"
        "    TOUCH (persistence off)     -> OVERLAY (downgraded)\n"
        "    TOUCH (persistence on)      -> TOUCH\n"
        "    CONTROLLER (pack missing)   -> OVERLAY (skip-safe fallback)\n"
        "    CONTROLLER (pack present)   -> CONTROLLER\n"
        "  M11 launch spec hudLaunchMode int mapping:\n"
        "    0 -> OFF, 1 -> OVERLAY, 2 -> TOUCH, 3 -> CONTROLLER\n"
        "  No V1 mutation. No real-asset dependency. No file I/O.\n";
}
