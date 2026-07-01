/*
 * theron_v2_phase_gate_pc34.c
 *
 * Theron's Quest V2 Phase 0 - V1 Compatibility Lock
 * Implementation.
 *
 * Source-lock anchors (Theron's Quest + ReDMCSB Common Toolchains):
 * - theron_v1_track02.c     PC Engine CD Track 02 bank signal
 * - theron_v1_boot.c        boot/profile state
 * - theron_v1_champions.c   4-champion party (Theron + 3 companions)
 * - theron_v1_dungeon_progression.c  dungeon progression
 * - theron_v1_mechanics.c   click routes, doors, pits, teleporters
 * - theron_v1_save_load.c   between-dungeon save/load
 * - theron_v1_shop.c        shop price table + purchase state
 * - theron_v1_tile_renderer.c  tile rendering
 * - theron_v1_viewport.c    viewport + presentation
 * - theron_v1_world.c       world model, map loading, party placement
 * - theron_v1_palette.c     16-color PC Engine palette
 * - theron_v1_ui_chrome.c   UI chrome
 *
 * Phase 0 rule: V1 Theron code compiles cleanly WITHOUT any V2
 * presentation code being active. V2 modules (presentation mode,
 * texture upscale, filter config, modern shapes) MUST NOT mutate
 * V1 game state.
 */

#include "theron_v2_phase_gate_pc34.h"
#include <string.h>

/* ----------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------- */

static THERON_V2_PhaseGateDecision make_decision(
    int sourceLocked,
    int presentationAllowed,
    const char *anchor,
    const char *rule)
{
    THERON_V2_PhaseGateDecision out;
    out.v1SourceLocked       = sourceLocked ? 1 : 0;
    out.v2PresentationAllowed = presentationAllowed ? 1 : 0;
    out.sourceAnchor         = anchor;
    out.rule                 = rule;
    return out;
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void theron_v2_phase_gate_defaults(THERON_V2_PhaseGateConfig *config)
{
    if (!config) return;
    /* Phase 0 default: V2 off, config persistence off. */
    config->v2PresentationEnabled       = 0;
    config->v2ConfigPersistenceEnabled = 0;
}

int theron_v2_phase_gate_is_gameplay_domain(THERON_V2_PhaseDomain domain)
{
    switch (domain) {
        /* V1-source-locked gameplay domains */
        case THERON_V2_PHASE_DOMAIN_TRACK02_BANK:
        case THERON_V2_PHASE_DOMAIN_BOOT_PROFILE:
        case THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY:
        case THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION:
        case THERON_V2_PHASE_DOMAIN_MECHANICS:
        case THERON_V2_PHASE_DOMAIN_SAVE_LOAD:
        case THERON_V2_PHASE_DOMAIN_SHOP:
        case THERON_V2_PHASE_DOMAIN_TILE_RENDERER:
        case THERON_V2_PHASE_DOMAIN_VIEWPORT:
        case THERON_V2_PHASE_DOMAIN_WORLD_STATE:
        case THERON_V2_PHASE_DOMAIN_PALETTE:
        case THERON_V2_PHASE_DOMAIN_UI_CHROME:
            return 1;

        /* V2-presentation-eligible domains */
        case THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE:
        case THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE:
        case THERON_V2_PHASE_DOMAIN_FILTER_CONFIG:
        case THERON_V2_PHASE_DOMAIN_MODERN_SHAPES:
        case THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE:
            return 0;

        default:
            /* Unknown domains default to V1-locked for safety. */
            return 1;
    }
}

int theron_v2_phase_gate_v2_active(const THERON_V2_PhaseGateConfig *config)
{
    return config && config->v2PresentationEnabled;
}

const char *theron_v2_phase_gate_domain_name(THERON_V2_PhaseDomain domain)
{
    switch (domain) {
        case THERON_V2_PHASE_DOMAIN_TRACK02_BANK:         return "TRACK02_BANK";
        case THERON_V2_PHASE_DOMAIN_BOOT_PROFILE:         return "BOOT_PROFILE";
        case THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY:       return "CHAMPION_PARTY";
        case THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION:  return "DUNGEON_PROGRESSION";
        case THERON_V2_PHASE_DOMAIN_MECHANICS:            return "MECHANICS";
        case THERON_V2_PHASE_DOMAIN_SAVE_LOAD:            return "SAVE_LOAD";
        case THERON_V2_PHASE_DOMAIN_SHOP:                 return "SHOP";
        case THERON_V2_PHASE_DOMAIN_TILE_RENDERER:        return "TILE_RENDERER";
        case THERON_V2_PHASE_DOMAIN_VIEWPORT:             return "VIEWPORT";
        case THERON_V2_PHASE_DOMAIN_WORLD_STATE:          return "WORLD_STATE";
        case THERON_V2_PHASE_DOMAIN_PALETTE:              return "PALETTE";
        case THERON_V2_PHASE_DOMAIN_UI_CHROME:            return "UI_CHROME";
        case THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE:    return "PRESENTATION_MODE";
        case THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE:      return "TEXTURE_UPSCALE";
        case THERON_V2_PHASE_DOMAIN_FILTER_CONFIG:        return "FILTER_CONFIG";
        case THERON_V2_PHASE_DOMAIN_MODERN_SHAPES:        return "MODERN_SHAPES";
        case THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE:      return "HUD_LAUNCH_MODE";
        default:                                          return "UNKNOWN";
    }
}

const char *theron_v2_phase_gate_source_evidence(void)
{
    return
        "theron_v1_track02.c (PC Engine CD Track 02 bank signal); "
        "theron_v1_boot.c (boot/profile state); "
        "theron_v1_champions.c (4-champion party: Theron + 3 companions); "
        "theron_v1_dungeon_progression.c (dungeon index, level transitions, dungeon_seed); "
        "theron_v1_mechanics.c (click routes, doors, pits, teleporters, altars, combat, drops, sounds); "
        "theron_v1_save_load.c (between-dungeon save/load, 8 slots, no in-dungeon saves); "
        "theron_v1_shop.c (shop price table + purchase state); "
        "theron_v1_tile_renderer.c (16-color PC Engine tile rendering); "
        "theron_v1_viewport.c (V1 viewport + presentation); "
        "theron_v1_world.c (world model, map loading, party placement); "
        "theron_v1_palette.c (PC Engine 16-color palette); "
        "theron_v1_ui_chrome.c (UI chrome: boxes, fonts, icons); "
        "theron_v2_hud_launch_mode_pc34.c (OFF/OVERLAY/TOUCH/CONTROLLER gate); "
        "THQUEST.ASM T080 (between-dungeon save/load); "
        "THQUEST.ASM T400 (dungeon bank loading); "
        "THQUEST.ASM T520 (party placement / start position); "
        "THQUEST.ASM T560 (dungeon loading: header parsing, dungeon_seed); "
        "THQUEST.ASM T600 (map transitions); "
        "THQUEST.ASM T700 (timers / world tick); "
        "THQUEST.ASM T800 (champion persistence + inventory reset); "
        "THQUEST.ASM T900 (object database / thing list); "
        "HuC6260/HuC6270 (PC Engine VDC/VCE datasheet); "
        "HuC6280 (PC Engine CPU datasheet); "
        "ADPCM (PC Engine audio codec); "
        "docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md; "
        "docs/source-lock/tqr_v1_phase1_boot_H2338.md; "
        "docs/source-lock/tqr_v1_phase2_data_formats_H2339.md; "
        "ReDMCSB CLIKMENU.C:142 F0365 turn; CLIKMENU.C:180 F0366 move; "
        "COMMAND.C:2045-2155 F0380 input wait; "
        "MOVESENS.C:316-345 F0267 move-result side effects.";
}

THERON_V2_PhaseGateDecision theron_v2_phase_gate_decide(
    const THERON_V2_PhaseGateConfig *config,
    THERON_V2_PhaseDomain domain)
{
    int v2Active = config && config->v2PresentationEnabled;
    int configPersistence = config && config->v2ConfigPersistenceEnabled;

    switch (domain) {

        /* ── V1-source-locked gameplay domains ── */

        case THERON_V2_PHASE_DOMAIN_TRACK02_BANK:
            return make_decision(
                1, 0,
                "theron_v1_track02.c (JP Rev 1 + US ISO MD5 hashes)",
                "PC Engine CD Track 02 bank signal stays V1-source-locked; "
                "JP Rev 1 and US ISO MD5 hashes gate the asset, V2 must not "
                "reinterpret the bank layout");

        case THERON_V2_PHASE_DOMAIN_BOOT_PROFILE:
            return make_decision(
                1, 0,
                "theron_v1_boot.c (boot sequence, profile loading, slot enumeration)",
                "Boot sequence, profile loading, and slot enumeration stay "
                "V1-source-locked");

        case THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY:
            return make_decision(
                1, 0,
                "theron_v1_champions.c (4-champion party, THQUEST.ASM T800)",
                "Champion stats, level-up, food/water/stamina, gold, and "
                "inventory reset per dungeon stay V1-source-locked; V2 must "
                "not mutate champion state");

        case THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION:
            return make_decision(
                1, 0,
                "theron_v1_dungeon_progression.c (THQUEST.ASM T560 dungeon_seed)",
                "Dungeon index, level transitions, and seed-based map "
                "variation stay V1-source-locked; V2 may only render the "
                "existing dungeon state");

        case THERON_V2_PHASE_DOMAIN_MECHANICS:
            return make_decision(
                1, 0,
                "theron_v1_mechanics.c (click routes, doors, pits, teleporters, altars, combat, drops, sounds)",
                "Click routes, doors, pits, teleporters, altars, combat, "
                "drops, and sounds stay V1-source-locked; V2 may only "
                "render the existing mechanics state");

        case THERON_V2_PHASE_DOMAIN_SAVE_LOAD:
            return make_decision(
                1, 0,
                "theron_v1_save_load.c (between-dungeon save/load, 8 slots, no in-dungeon)",
                "Between-dungeon save/load (8 slots, 64-byte header + "
                "champion blocks + footer, no in-dungeon saves) stays "
                "V1-source-locked; V2 config persistence is explicitly "
                "separate and gated by v2ConfigPersistenceEnabled");

        case THERON_V2_PHASE_DOMAIN_SHOP:
            return make_decision(
                1, 0,
                "theron_v1_shop.c (shop price table + purchase state)",
                "Shop price table and purchase state (gold deduction, "
                "item transfer) stay V1-source-locked; V2 may only "
                "render the shop UI");

        case THERON_V2_PHASE_DOMAIN_TILE_RENDERER:
            return make_decision(
                1, 0,
                "theron_v1_tile_renderer.c (16-color PC Engine palette, ADPCM audio)",
                "16-color PC Engine tile rendering and ADPCM audio "
                "stay V1-owned; V2 upscale happens on a read-only copy");

        case THERON_V2_PHASE_DOMAIN_VIEWPORT:
            return make_decision(
                1, 0,
                "theron_v1_viewport.c (V1 viewport + presentation)",
                "V1 viewport + presentation stays V1-source-locked; "
                "V2 presentation mode reads a read-only copy of the "
                "V1 framebuffer");

        case THERON_V2_PHASE_DOMAIN_WORLD_STATE:
            return make_decision(
                1, 0,
                "theron_v1_world.c (world model, map loading, party placement)",
                "World model, map loading, party placement, map "
                "transitions, timers, and object database stay "
                "V1-source-locked; V2 may only render the existing "
                "world state");

        case THERON_V2_PHASE_DOMAIN_PALETTE:
            return make_decision(
                1, 0,
                "theron_v1_palette.c (16-color PC Engine palette selection)",
                "PC Engine 16-color palette selection stays V1-owned; "
                "V2 palette correction reads the V1 palette as input");

        case THERON_V2_PHASE_DOMAIN_UI_CHROME:
            return make_decision(
                1, 0,
                "theron_v1_ui_chrome.c (UI chrome: boxes, fonts, icons)",
                "UI chrome (boxes, fonts, icons) stays V1-owned; "
                "V2 may only add overlay presentation on top");

        /* ── V2-presentation-eligible domains ── */

        case THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE:
            return make_decision(
                0, v2Active,
                "theron_v2_presentation_mode_pc34.c (M12_PRESENTATION -> V1/V20/V21/V22)",
                "V2 presentation mode is presentation-only; never alters "
                "V1 game state. Maps M12_PRESENTATION enum onto the V2 "
                "runtime with V22->V21 fallback when modern pack absent");

        case THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE:
            return make_decision(
                0, v2Active,
                "theron_v2_texture_upscale_pc34.c (EPX + bilinear filter)",
                "V2 texture upscale consumes a read-only V1 framebuffer; "
                "presentation only when v2PresentationEnabled");

        case THERON_V2_PHASE_DOMAIN_FILTER_CONFIG:
            /* FILTER_CONFIG requires BOTH v2PresentationEnabled AND
             * v2ConfigPersistenceEnabled because it writes to M12
             * settings (persistent state). */
            return make_decision(
                0, v2Active && configPersistence,
                "theron_v2_filter_config_pc34.c (M12 settings persistence)",
                "V2 filter config presentation is allowed only when BOTH "
                "v2PresentationEnabled=1 AND v2ConfigPersistenceEnabled=1; "
                "filter writes are persistent state changes and require "
                "the config-persistence toggle");

        case THERON_V2_PHASE_DOMAIN_MODERN_SHAPES:
            return make_decision(
                0, v2Active,
                "theron_v22_shapes.c (V2.2 modern shape book, 9-square viewport + panel)",
                "V2.2 modern shape book is presentation-only; never "
                "alters V1 game state");

        case THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE:
            /* HUD launch-mode selector (OFF / OVERLAY / TOUCH /
             * CONTROLLER). Presentation-only by construction. We
             * expose it via v2PresentationEnabled=1; the selector
             * itself internally gates TOUCH and CONTROLLER on
             * v2ConfigPersistenceEnabled because per-zone hit-test
             * state persists into M12 settings. */
            return make_decision(
                0, v2Active,
                "theron_v2_hud_launch_mode_pc34.c (OFF/OVERLAY/TOUCH/CONTROLLER gate)",
                "V2 HUD launch-mode selector is presentation-only; never "
                "alters V1 input / champion / world / save / Track 02 "
                "bank state. TOUCH and CONTROLLER additionally require "
                "v2ConfigPersistenceEnabled=1; OVERLAY does not. See "
                "theron_v2_hud_launch_mode_pc34.c resolution table.");

        default:
            /* Unknown domains default to V1-locked. */
            return make_decision(
                1, 0,
                "default: unknown domain -> V1-locked for safety",
                "unknown domains default to V1-locked");
    }
}
