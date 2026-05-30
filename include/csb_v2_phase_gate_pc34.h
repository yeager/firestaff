#ifndef FIRESTAFF_CSB_V2_PHASE_GATE_PC34_H
#define FIRESTAFF_CSB_V2_PHASE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * CSB V2 Phase 0 — V1 Compatibility Lock
 * ============================================================
 *
 * This header defines the compile-time and runtime gates that
 * isolate CSB V1 game logic (dungeon loading, combat, movement,
 * chaos magic, champion management) from V2 presentation code
 * (enhanced rendering, smooth movement, dynamic lighting, minimap,
 * touch/controller affordances).
 *
 * Phase 0 rule: V1 CSB code compiles cleanly WITHOUT any V2
 * presentation code being active. The V2 static library
 * (firestaff_csb_v2) MUST NOT alter V1 game-logic behaviour.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 * - COMMAND.C:108-113   mouse movement zones (C001-C006)
 * - COMMAND.C:254-291   keyboard mapping for C001..C006
 * - COMMAND.C:1379      F0358_COMMAND_GetCommandFromMouseInput_CPSC
 * - COMMAND.C:1452      F0359_COMMAND_ProcessClick_CPSC
 * - COMMAND.C:1641,1643  primary-to-secondary mouse queue dispatch
 * - COMMAND.C:1693      F0360_COMMAND_ProcessPendingClick
 * - COMMAND.C:2045-2155 pops queued command, dispatches F0365/F0366
 * - CLIKMENU.C:142      F0365_COMMAND_ProcessTypes1To2_TurnParty
 * - CLIKMENU.C:180      F0366_COMMAND_ProcessTypes3To6_MoveParty
 * - CLIKMENU.C:278-323  wall/door/fakewall/group collision checks
 * - CLIKMENU.C:330-346  movement cooldown writes
 * - GAMELOOP.C:150-155  disabled-movement-tick decrement
 * - GAMELOOP.C:164-219  V1 input wait / command queue loop
 * - GAMELOOP.C:215-219  F0380 until input waiting stops
 * - MOVESENS.C:316-345  F0267 move-result side effects
 * - LOADSAVE.C:1520-1534, 2730-2742  save/load party position/timing
 * - DUNGEON.C:          CSB dungeon format, square types, level count
 * - CHAMPION.C:         champion stats, level-up, resurrect
 * - ENTRANCE.C:         CSB intro sequence, prison door
 * - PANEL.C:418-428     canonical dungeon palette selection
 *
 * CSB-specific references:
 * - CSBWin/Chaos.cpp:60-69  DSA call dispatch (_CALL0-_CALL9)
 * - CSBWin/DSA.cpp          DSA interpreter (chaos magic scripts)
 * - CSBWin/Viewport.cpp     CSB viewport (7290 lines)
 * - CSBWin/Graphics.cpp      CSB graphics (3186 lines)
 * - CSBWin/champion.cpp      champion resurrect logic
 * - csb_v1_chaos_magic_pc34_compat.c:50  csb_v1_chaos_trigger
 */

/* ============================================================
 * Phase domains — each domain is classified as V1-source-locked
 * or V2-presentation-eligible.
 *
 * V1-source-locked domains MUST NOT have their behaviour altered
 * by any V2 code. V2-presentation-eligible domains MAY receive
 * enhanced visual presentation when v2PresentationEnabled is true,
 * but V2 code MUST NOT change the underlying V1 game-logic state.
 * ============================================================ */

typedef enum {
    /* V1-source-locked gameplay domains */
    CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS = 0,
        /* ReDMCSB COMMAND.C:2045-2155 — command ids and queue dispatch
         * stay V1 source ids; V2 may only route presentation adapters. */

    CSB_V2_PHASE_DOMAIN_DUNGEON_LOADING = 1,
        /* ReDMCSB DUNGEON.C — CSB dungeon format, level count, square
         * types, first-thing parsing. V2 must not reinterpret layout. */

    CSB_V2_PHASE_DOMAIN_DUNGEON_TIMING = 2,
        /* ReDMCSB GAMELOOP.C:150-155, 215-219; CLIKMENU.C:330-346 —
         * disabled movement ticks and command-wait loop stay V1-owned. */

    CSB_V2_PHASE_DOMAIN_COLLISION_RULES = 3,
        /* ReDMCSB CLIKMENU.C:278-323; MOVESENS.C:316-345 —
         * wall, door, fakewall, group, move-result behaviour
         * stay source-locked to ReDMCSB. */

    CSB_V2_PHASE_DOMAIN_SAVE_LOAD_DATA = 4,
        /* ReDMCSB LOADSAVE.C:1520-1534, 2730-2742 —
         * V2 presentation state must not alter V1 save/load payload. */

    CSB_V2_PHASE_DOMAIN_CHAMPION_RESURRECT = 5,
        /* CSBWin champion.cpp — champion death, resurrect, and stat
         * management stay CSBWin/BeipDev source-locked. */

    CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS = 6,
        /* CSBWin/Chaos.cpp:60-69; CSBWin/DSA.cpp —
         * DSA script dispatch and chaos magic trigger logic stay V1. */

    /* V2-presentation-eligible domains */
    CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION = 7,
        /* V2 may present the V1 dungeon picture with enhanced lighting,
         * custom backgrounds, and CSB-specific wall sets only when
         * v2PresentationEnabled is true. */

    CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION = 8,
        /* V2 smooth movement interpolates visually between V1 walk/turn
         * states. V1 cooldowns, collision, and sensor timing are
         * unaffected. V2 may only interpolate the PRESENTED position. */

    CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION = 9,
        /* V2 dynamic lighting adds local light sources on top of the
         * V1 canonical palette (PANEL.C:418-428). Source palette stays
         * V1; V2 light map is presentation-only. */

    CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION = 10,
        /* V2 minimap renders a CSB dungeon overview. V1 dungeon state
         * is not modified. */

    CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION = 11,
        /* V2 touch swipes and controller inputs map to V1 C001-C006
         * command ids ONLY behind the v2PresentationEnabled gate. */

    CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION = 12,
        /* Deterministic V2 config persistence is presentation-only
         * and requires explicit v2ConfigPersistenceEnabled. */

    CSB_V2_PHASE_DOMAIN_COUNT
} CSB_V2_PhaseDomain;

/* ============================================================
 * Phase gate config — controls which V2 presentation features
 * are enabled at runtime. Both default to 0 (V1-only boot).
 * ============================================================ */

typedef struct {
    int v2PresentationEnabled;       /* master V2 presentation toggle */
    int v2ConfigPersistenceEnabled; /* V2 config save/load toggle */
} CSB_V2_PhaseGateConfig;

/* ============================================================
 * Phase gate decision — result of querying a domain.
 * ============================================================ */

typedef struct {
    int v1SourceLocked;        /* 1 = this domain must stay V1 */
    int v2PresentationAllowed; /* 1 = V2 may present, never alter */
    const char *sourceAnchor;  /* ReDMCSB / CSBWin citation */
    const char *rule;          /* short rule description */
} CSB_V2_PhaseGateDecision;

/* ============================================================
 * Public API
 * ============================================================ */

/* Set defaults: all V2 presentation features OFF (V1-only boot). */
void csb_v2_phase_gate_pc34_defaults(CSB_V2_PhaseGateConfig *config);

/* Returns 1 if the domain is V1-source-locked (V2 may not alter). */
int csb_v2_phase_gate_pc34_is_gameplay_domain(CSB_V2_PhaseDomain domain);

/* Query whether V2 presentation is allowed for a domain. */
CSB_V2_PhaseGateDecision csb_v2_phase_gate_pc34_decide(
    const CSB_V2_PhaseGateConfig *config,
    CSB_V2_PhaseDomain domain);

/* Returns 1 if v2PresentationEnabled is set. Shortcut for
 * config && config->v2PresentationEnabled. */
int csb_v2_phase_gate_pc34_v2_active(const CSB_V2_PhaseGateConfig *config);

/* Human-readable domain name for debugging/logging. */
const char *csb_v2_phase_gate_pc34_domain_name(CSB_V2_PhaseDomain domain);

/* Source evidence string for verification scripts. */
const char *csb_v2_phase_gate_pc34_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_PHASE_GATE_PC34_H */
