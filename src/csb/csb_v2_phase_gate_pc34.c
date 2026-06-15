/**
 * CSB V2 Phase 0 — V1 Compatibility Lock
 * Implementation
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 * - COMMAND.C:2045-2155  command queue dispatch (F0359)
 * - CLIKMENU.C:142       F0365_COMMAND_ProcessTypes1To2_TurnParty
 * - CLIKMENU.C:180       F0366_COMMAND_ProcessTypes3To6_MoveParty
 * - CLIKMENU.C:278-323   wall/door/fakewall/group collision
 * - CLIKMENU.C:330-346   movement cooldown writes
 * - GAMELOOP.C:150-155   disabled movement tick decrement
 * - GAMELOOP.C:215-219   F0380 input wait loop
 * - MOVESENS.C:316-345   F0267 move-result side effects
 * - LOADSAVE.C:1520-1534, 2730-2742  save/load state
 * - DUNGEON.C            CSB dungeon format
 * - CHAMPION.C           champion stats, resurrect
 * - PANEL.C:418-428      canonical dungeon palette
 *
 * CSB-specific references:
 * - CSBWin/Chaos.cpp:60-69   DSA call dispatch
 * - CSBWin/DSA.cpp            DSA interpreter
 * - CSBWin/champion.cpp       champion resurrect logic
 * - csb_v1_chaos_magic_pc34_compat.c:50  csb_v1_chaos_trigger
 */

#include "csb_v2_phase_gate_pc34.h"
#include <string.h>

/* ----------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------- */

static CSB_V2_PhaseGateDecision make_decision(
    int sourceLocked,
    int presentationAllowed,
    const char *anchor,
    const char *rule)
{
    CSB_V2_PhaseGateDecision out;
    out.v1SourceLocked     = sourceLocked ? 1 : 0;
    out.v2PresentationAllowed = presentationAllowed ? 1 : 0;
    out.sourceAnchor       = anchor;
    out.rule               = rule;
    return out;
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void csb_v2_phase_gate_pc34_defaults(CSB_V2_PhaseGateConfig *config)
{
    if (!config) return;
    config->v2PresentationEnabled       = 0;
    config->v2ConfigPersistenceEnabled = 0;
}

int csb_v2_phase_gate_pc34_is_gameplay_domain(CSB_V2_PhaseDomain domain)
{
    switch (domain) {
        /* V1-source-locked gameplay domains — V2 must not alter */
        case CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS:
        case CSB_V2_PHASE_DOMAIN_DUNGEON_LOADING:
        case CSB_V2_PHASE_DOMAIN_DUNGEON_TIMING:
        case CSB_V2_PHASE_DOMAIN_COLLISION_RULES:
        case CSB_V2_PHASE_DOMAIN_SAVE_LOAD_DATA:
        case CSB_V2_PHASE_DOMAIN_CHAMPION_RESURRECT:
        case CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS:
            return 1;

        /* V2-presentation-eligible domains — V2 may present but not alter */
        case CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
        case CSB_V2_PHASE_DOMAIN_HUD:
            return 0;

        default:
            return 1; /* unknown domains default to V1-locked */
    }
}

CSB_V2_PhaseGateDecision csb_v2_phase_gate_pc34_decide(
    const CSB_V2_PhaseGateConfig *config,
    CSB_V2_PhaseDomain domain)
{
    int v2Active = config && config->v2PresentationEnabled;

    switch (domain) {

        /* ── V1-source-locked gameplay domains ── */

        case CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS:
            return make_decision(
                1, 0,
                "ReDMCSB COMMAND.C:2045-2155 F0359; ReDMCSB COMMAND.C:1379 F0358; ReDMCSB COMMAND.C:1452 F0359",
                "command ids and queue dispatch stay V1 source-locked; "
                "V2 may only route presentation adapters behind the toggle");

        case CSB_V2_PHASE_DOMAIN_DUNGEON_LOADING:
            return make_decision(
                1, 0,
                "DUNGEON.C (ReDMCSB); csb_v1_dungeon_loader_pc34_compat.c",
                "CSB dungeon format, level count, square types, and first-thing "
                "parsing stay V1-source-locked; V2 must not reinterpret layout");

        case CSB_V2_PHASE_DOMAIN_DUNGEON_TIMING:
            return make_decision(
                1, 0,
                "ReDMCSB GAMELOOP.C:150-155; ReDMCSB GAMELOOP.C:215-219; ReDMCSB CLIKMENU.C:330-346",
                "disabled movement ticks and command-wait loop stay V1-owned; "
                "V2 smooth movement may only interpolate the visual presentation");

        case CSB_V2_PHASE_DOMAIN_COLLISION_RULES:
            return make_decision(
                1, 0,
                "ReDMCSB CLIKMENU.C:278-323; ReDMCSB MOVESENS.C:316-345 F0267",
                "wall, door, fakewall, group, and move-result behaviour "
                "stay source-locked to ReDMCSB; V2 collision is presentation-only");

        case CSB_V2_PHASE_DOMAIN_SAVE_LOAD_DATA:
            return make_decision(
                1, 0,
                "ReDMCSB LOADSAVE.C:1520-1534; ReDMCSB LOADSAVE.C:2730-2742",
                "V2 presentation state must not alter V1 save/load payload semantics; "
                "V2 config persistence is explicitly gated and separate");

        case CSB_V2_PHASE_DOMAIN_CHAMPION_RESURRECT:
            return make_decision(
                1, 0,
                "CHAMPION.C (ReDMCSB); CSBWin/champion.cpp champion resurrect",
                "champion death, resurrect, and stat management stay "
                "CSBWin/BeipDev source-locked; V2 may not alter champion state");

        case CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS:
            return make_decision(
                1, 0,
                "CSBWin/Chaos.cpp:60-69 DSA call dispatch; CSBWin/DSA.cpp; "
                "csb_v1_chaos_magic_pc34_compat.c:50 csb_v1_chaos_trigger",
                "DSA script dispatch and chaos magic trigger logic stay V1; "
                "V2 chaos-enhanced visual feedback is presentation-only");

        /* ── V2-presentation-eligible domains ── */

        case CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
            return make_decision(
                0, v2Active,
                "Viewport.cpp (CSBWin 7290 lines); DUNGEON.C; csb_v1_viewport_pc34_compat.c",
                "V2 may present CSB dungeon with enhanced wall sets, custom backgrounds, "
                "and CSB-specific rendering only when v2PresentationEnabled is true");

        case CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "ReDMCSB COMMAND.C:2045-2155; ReDMCSB GAMELOOP.C:215-219; "
                "csb_v2_smooth_movement.h; dm1_v2_anim_timing.h",
                "V2 smooth movement interpolates visually between V1 walk/turn states. "
                "V1 cooldowns, collision, and sensor timing are unaffected; "
                "V2 may only interpolate the presented position");

        case CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:
            return make_decision(
                0, v2Active,
                "ReDMCSB PANEL.C:418-428 canonical palette; csb_v2_lighting_dynamic.h",
                "V2 dynamic lighting adds local light sources on top of the V1 canonical "
                "palette. Source palette stays V1; V2 light map is presentation-only");

        case CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION:
            return make_decision(
                0, v2Active,
                "ReDMCSB DUNGEON.C; csb_v2_minimap.h",
                "V2 minimap renders a CSB dungeon overview. V1 dungeon state "
                "is not modified by minimap rendering");

        case CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "ReDMCSB COMMAND.C:108-113 mouse zones; ReDMCSB COMMAND.C:254-291 keyboard map; "
                "csb_v2_touch_controller_affordance.h",
                "V2 touch swipes and controller inputs map to V1 C001-C006 command ids "
                "ONLY behind the v2PresentationEnabled gate; V1 mouse/touch/click route "
                "is the sole input path when V2 is disabled");

        case CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
            return make_decision(
                0, v2Active && config && config->v2ConfigPersistenceEnabled,
                "ReDMCSB LOADSAVE.C:1520-1534; ReDMCSB DUNGEON.C",
                "V2 config persistence is presentation-only and requires both "
                "v2PresentationEnabled and v2ConfigPersistenceEnabled to be set; "
                "V1 save/load payload semantics are unaffected");

        case CSB_V2_PHASE_DOMAIN_HUD:
            /* HUD is a PROFILE-domain feature: it activates only when both
             * v2PresentationEnabled is set AND the caller has set up the
             * HUD overlay state (csb_v2_hud_runtime). Unlike LAUNCH/PROFILE
             * which are hard gates, HUD presentation is gated on v2Active
             * but the caller manages the overlay lifetime.
             * Source: CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines)
             *         CSBWin/Graphics.cpp (CSB graphics, 3186 lines)
             *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
             *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
             *         ReDMCSB COMMAND.C action feedback gates
             *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
             *         DM2 DM2_V2_PHASE_DOMAIN_HUD pattern (Phase 3 HUD gate) */
            return make_decision(
                0, v2Active,
                "CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines); "
                "CSBWin/Graphics.cpp (CSB graphics, 3186 lines); "
                "ReDMCSB PANEL.C F0354; ReDMCSB DUNGEON.C F0260; "
                "ReDMCSB COMMAND.C; ReDMCSB DISPLAY.C (2 Hz pulse)",
                "Phase 3: V2 HUD overlay (compass/depth/gold/champion bars/"
                "action strip/chaos indicator) is presentation-only and "
                "activates when v2PresentationEnabled is true; "
                "V1 command routes and inventory transactions are unaffected");

        default:
            return make_decision(
                1, 0,
                "unknown domain",
                "unknown domains are V1-locked by default (fail-safe)");
    }
}

int csb_v2_phase_gate_pc34_v2_active(const CSB_V2_PhaseGateConfig *config)
{
    return config && config->v2PresentationEnabled;
}

const char *csb_v2_phase_gate_pc34_domain_name(CSB_V2_PhaseDomain domain)
{
    switch (domain) {
        case CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS:           return "COMMAND_SEMANTICS";
        case CSB_V2_PHASE_DOMAIN_DUNGEON_LOADING:             return "DUNGEON_LOADING";
        case CSB_V2_PHASE_DOMAIN_DUNGEON_TIMING:              return "DUNGEON_TIMING";
        case CSB_V2_PHASE_DOMAIN_COLLISION_RULES:             return "COLLISION_RULES";
        case CSB_V2_PHASE_DOMAIN_SAVE_LOAD_DATA:              return "SAVE_LOAD_DATA";
        case CSB_V2_PHASE_DOMAIN_CHAMPION_RESURRECT:          return "CHAMPION_RESURRECT";
        case CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS:         return "CHAOS_MAGIC_SCRIPTS";
        case CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION:         return "RENDER_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION: return "SMOOTH_MOVEMENT_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION: return "DYNAMIC_LIGHTING_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION:        return "MINIMAP_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION:          return "INPUT_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:         return "CONFIG_PRESENTATION";
        case CSB_V2_PHASE_DOMAIN_HUD:                            return "HUD";
        default:                                              return "UNKNOWN";
    }
}

const char *csb_v2_phase_gate_pc34_source_evidence(void)
{
    return "csb_v2_phase_gate_pc34.c v1.0.0 "
           "(CSB V2 Phase 0 V1 compatibility lock) "
           "Source: ReDMCSB WIP20210206 "
           "(COMMAND.C, CLIKMENU.C, GAMELOOP.C, MOVESENS.C, LOADSAVE.C, DUNGEON.C, "
           "CHAMPION.C, PANEL.C) + CSBWin/Chaos.cpp, CSBWin/DSA.cpp, "
           "CSBWin/champion.cpp, CSBWin/Viewport.cpp";
}
