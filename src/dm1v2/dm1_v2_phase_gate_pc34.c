#include "dm1_v2_phase_gate_pc34.h"

/* DM1 V2 Phase 0/1 gate.
 *
 * Source-lock anchors:
 * - ReDMCSB DEFS.H:238-243 owns command ids C001..C006.
 * - ReDMCSB COMMAND.C:2045-2155 pops one queued command and dispatches turns
 *   to F0365 and steps to F0366.
 * - ReDMCSB CLIKMENU.C:142-346 owns turn/step side effects, source-locked
 *   wall/door/fakewall/group collision checks, and movement cooldown writes.
 * - ReDMCSB GAMELOOP.C:150-155 decrements disabled movement ticks; GAMELOOP.C:215-219
 *   loops through F0380 until input waiting stops and game time is ticking.
 * - ReDMCSB MOVESENS.C:316-345 owns F0267 move-result side effects.
 * - ReDMCSB LOADSAVE.C:1520-1534 and LOADSAVE.C:2730-2742 serialize and
 *   restore party position, direction, movement timing, and global state.
 * - ReDMCSB COORD.C:1721-1722 plus DUNVIEW.C:2999-3000 keep the original
 *   224x136 viewport as the gameplay-space picture that V2 may present.
 *
 * Phase 0 rule: DM1 V2 must not reinterpret command semantics, dungeon
 * timing, source-locked collisions, save/load data, or ReDMCSB-backed rules.
 * Phase 1 rule: V2 render/input/config work is presentation-only and requires
 * an explicit V2 presentation toggle; V1 remains the default boot/runtime path.
 */

static DM1_V2_PhaseGateDecision decision(int sourceLocked,
                                         int presentationAllowed,
                                         const char* sourceAnchor,
                                         const char* rule) {
    DM1_V2_PhaseGateDecision out;
    out.v1SourceLocked = sourceLocked ? 1 : 0;
    out.v2PresentationAllowed = presentationAllowed ? 1 : 0;
    out.sourceAnchor = sourceAnchor;
    out.rule = rule;
    return out;
}

void dm1_v2_phase_gate_defaults(DM1_V2_PhaseGateConfig* config) {
    if (!config) return;
    config->v2PresentationEnabled = 0;
    config->v2ConfigPersistenceEnabled = 0;
}

int dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PhaseDomain domain) {
    return domain == DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS ||
           domain == DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING ||
           domain == DM1_V2_PHASE_DOMAIN_COLLISION_RULES ||
           domain == DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA ||
           domain == DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES;
}

DM1_V2_PhaseGateDecision dm1_v2_phase_gate_decide(
    const DM1_V2_PhaseGateConfig* config,
    DM1_V2_PhaseDomain domain) {
    int presentationEnabled = config && config->v2PresentationEnabled;
    int configPersistenceEnabled = config && config->v2ConfigPersistenceEnabled;

    switch (domain) {
        case DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS:
            return decision(1, 0, "DEFS.H:238-243; COMMAND.C:2045-2155",
                            "commands stay V1 source ids; V2 may only route presentation adapters");
        case DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING:
            return decision(1, 0, "GAMELOOP.C:150-155; GAMELOOP.C:215-219; CLIKMENU.C:330-346",
                            "disabled movement ticks and command wait loop stay V1-owned");
        case DM1_V2_PHASE_DOMAIN_COLLISION_RULES:
            return decision(1, 0, "CLIKMENU.C:278-323; MOVESENS.C:316-345",
                            "wall, door, fakewall, group, and move-result behavior stay source-locked");
        case DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA:
            return decision(1, 0, "LOADSAVE.C:1520-1534; LOADSAVE.C:2730-2742",
                            "V2 presentation state must not alter V1 save/load payload semantics");
        case DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES:
            return decision(1, 0, "ReDMCSB primary source required before gameplay rule changes",
                            "ReDMCSB-backed rules remain V1 unless a future source-locked gameplay lane exists");
        case DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
            return decision(0, presentationEnabled, "COORD.C:1721-1722; DUNVIEW.C:2999-3000",
                            "V2 may present the 224x136 V1 picture only when V2 presentation is explicit");
        case DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
            return decision(0, presentationEnabled, "DEFS.H:238-243; COMMAND.C:2045-2155",
                            "V2 input affordances may map onto V1 command ids only behind the presentation toggle");
        case DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
            return decision(0, presentationEnabled && configPersistenceEnabled,
                            "COORD.C:1721-1722; DUNVIEW.C:2999-3000",
                            "deterministic V2 config persistence is presentation-only and explicitly enabled");
        default:
            return decision(1, 0, "unknown domain", "unknown domains are locked to V1 by default");
    }
}
