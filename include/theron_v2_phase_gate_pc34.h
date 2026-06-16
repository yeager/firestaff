#ifndef FIRESTAFF_THERON_V2_PHASE_GATE_PC34_H
#define FIRESTAFF_THERON_V2_PHASE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Theron's Quest V2 Phase 0 - V1 Compatibility Lock
 * ============================================================
 *
 * This header defines the compile-time and runtime gates that
 * isolate Theron V1 game logic (PC Engine CD Track 02 bank,
 * dungeon progression, mechanics, save/load, shop, world state)
 * from V2 presentation code (presentation mode, texture upscale,
 * filter config, modern shape book).
 *
 * Phase 0 rule: V1 Theron code compiles cleanly WITHOUT any V2
 * presentation code being active. The V2 static library
 * (firestaff_theron_v2) MUST NOT alter V1 game-logic behaviour.
 *
 * Source-lock anchors (Theron's Quest + ReDMCSB Common Toolchains):
 * - theron_v1_track02.c     PC Engine CD Track 02 bank signal
 *                           (JP Rev 1 + US ISO MD5 hashes)
 * - theron_v1_boot.c        boot/profile state
 * - theron_v1_champions.c   4-champion party (Theron + 3 companions)
 * - theron_v1_dungeon_progression.c  dungeon progression / level
 * - theron_v1_mechanics.c   click routes, doors, pits, teleporters,
 *                           altars, combat, drops, sounds
 * - theron_v1_save_load.c   between-dungeon save/load (8 slots)
 * - theron_v1_shop.c        shop price table + purchase state
 * - theron_v1_tile_renderer.c  tile rendering
 * - theron_v1_viewport.c    viewport + presentation
 * - theron_v1_world.c       world model, map loading, party placement
 * - theron_v1_palette.c     16-color PC Engine palette
 * - theron_v1_ui_chrome.c   UI chrome (boxes, fonts, icons)
 *
 * V2 modules (presentation only, never mutate V1 state):
 * - theron_v2_presentation_mode_pc34.c  M12_PRESENTATION -> V2 mode
 * - theron_v2_texture_upscale_pc34.c    EPX + bilinear filter
 * - theron_v2_filter_config_pc34.c      V2.0 filter-chain config
 * - theron_v22_shapes.c                  V2.2 modern shape book
 *
 * Theron-specific references:
 * - THQUEST.ASM T080  between-dungeon save/load
 * - THQUEST.ASM T400  dungeon bank loading
 * - THQUEST.ASM T520  party placement / start position
 * - THQUEST.ASM T560  dungeon loading (header parsing, dungeon_seed)
 * - THQUEST.ASM T600  map transitions
 * - THQUEST.ASM T700  timers / world tick
 * - THQUEST.ASM T800  champion persistence + inventory reset
 * - THQUEST.ASM T900  object database / thing list
 * - HuC6260/HuC6270 VDC/VCE        PC Engine video chip datasheet
 * - HuC6280 CPU                    PC Engine CPU datasheet
 * - ADPCM                           PC Engine audio codec
 * - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 * - docs/source-lock/tqr_v1_phase1_boot_H2338.md
 * - docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

/* ============================================================
 * Phase domains - each domain is classified as V1-source-locked
 * or V2-presentation-eligible.
 * ============================================================ */

typedef enum {
    /* V1-source-locked gameplay domains */
    THERON_V2_PHASE_DOMAIN_TRACK02_BANK = 0,
        /* theron_v1_track02.c - PC Engine CD Track 02 bank signal.
         * JP Rev 1 + US ISO MD5 hashes gate the asset. V2 must not
         * reinterpret the bank layout. */

    THERON_V2_PHASE_DOMAIN_BOOT_PROFILE = 1,
        /* theron_v1_boot.c - boot sequence, profile loading, slot
         * enumeration. V1 only. */

    THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY = 2,
        /* theron_v1_champions.c - 4-champion party (Theron + 3
         * companions), stats, level-up, food/water/stamina, gold. */

    THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION = 3,
        /* theron_v1_dungeon_progression.c - dungeon index, level
         * transitions, seed-based map variation, completion state. */

    THERON_V2_PHASE_DOMAIN_MECHANICS = 4,
        /* theron_v1_mechanics.c - click routes, doors, pits,
         * teleporters, altars, combat, drops, sounds. DM1-compatible
         * with Theron-specific mechanics. */

    THERON_V2_PHASE_DOMAIN_SAVE_LOAD = 5,
        /* theron_v1_save_load.c - between-dungeon save/load (8 slots,
         * 64-byte header + champion blocks + footer). No in-dungeon
         * saves (TQ design restriction). V2 config persistence is
         * explicitly separate. */

    THERON_V2_PHASE_DOMAIN_SHOP = 6,
        /* theron_v1_shop.c - shop price table + purchase state
         * (gold deduction, item transfer). V1-owned. */

    THERON_V2_PHASE_DOMAIN_TILE_RENDERER = 7,
        /* theron_v1_tile_renderer.c - tile rendering, 16-color PC
         * Engine palette, ADPCM audio. */

    THERON_V2_PHASE_DOMAIN_VIEWPORT = 8,
        /* theron_v1_viewport.c - viewport + presentation. V1 framebuffer
         * stays V1-owned. V2 upscale happens on a read-only copy. */

    THERON_V2_PHASE_DOMAIN_WORLD_STATE = 9,
        /* theron_v1_world.c - world model, map loading, party
         * placement, map transitions, timers, object database. */

    THERON_V2_PHASE_DOMAIN_PALETTE = 10,
        /* theron_v1_palette.c - 16-color PC Engine palette selection. */

    THERON_V2_PHASE_DOMAIN_UI_CHROME = 11,
        /* theron_v1_ui_chrome.c - UI chrome (boxes, fonts, icons). */

    /* V2-presentation-eligible domains */
    THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE = 12,
        /* theron_v2_presentation_mode_pc34.c - M12_PRESENTATION enum
         * -> V1_FAITHFUL / V20_FILTERED / V21_UPSCALED / V22_MODERN. */

    THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE = 13,
        /* theron_v2_texture_upscale_pc34.c - EPX + bilinear filter.
         * Reads V1 framebuffer read-only. */

    THERON_V2_PHASE_DOMAIN_FILTER_CONFIG = 14,
        /* theron_v2_filter_config_pc34.c - V2.0 filter chain config
         * (CRT scanline, palette correction, etc.). M12 settings
         * persistence. */

    THERON_V2_PHASE_DOMAIN_MODERN_SHAPES = 15,
        /* theron_v22_shapes.c - V2.2 modern shape book (9-square
         * viewport + panel). */

    THERON_V2_PHASE_DOMAIN_COUNT
} THERON_V2_PhaseDomain;

/* ============================================================
 * Phase gate configuration. Phase 0 default: both disabled.
 * ============================================================ */

typedef struct {
    int v2PresentationEnabled;
    int v2ConfigPersistenceEnabled;
} THERON_V2_PhaseGateConfig;

/* ============================================================
 * Phase gate decision: per-domain allow/deny verdict with
 * source-lock citation.
 * ============================================================ */

typedef struct {
    int v1SourceLocked;       /* 1 = V1 must own this domain */
    int v2PresentationAllowed; /* 1 = V2 may present (if v2PresentationEnabled) */
    const char* sourceAnchor;  /* THQUEST.ASM / ReDMCSB / V2 source reference */
    const char* rule;          /* human-readable rule */
} THERON_V2_PhaseGateDecision;

/* ============================================================
 * Public API
 * ============================================================ */

void theron_v2_phase_gate_defaults(THERON_V2_PhaseGateConfig *config);

int theron_v2_phase_gate_is_gameplay_domain(THERON_V2_PhaseDomain domain);

THERON_V2_PhaseGateDecision theron_v2_phase_gate_decide(
    const THERON_V2_PhaseGateConfig *config,
    THERON_V2_PhaseDomain domain);

int theron_v2_phase_gate_v2_active(const THERON_V2_PhaseGateConfig *config);

const char *theron_v2_phase_gate_domain_name(THERON_V2_PhaseDomain domain);

const char *theron_v2_phase_gate_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_PHASE_GATE_PC34_H */
