/*
 * nexus_v2_phase_gate_pc34.c
 *
 * Nexus V2 Phase 0 - V1 Compatibility Lock
 * Implementation.
 *
 * Source-lock anchors (Saturn DM Nexus + ReDMCSB Common Toolchains):
 * - nexus_v1_iso_reader.c     Saturn ISO 9660 directory + DMDF parsing
 * - nexus_v1_dmdf_model.c     DMDF (Dungeon Master Data Format) decoder
 * - nexus_v1_dungeon.c        DGN level loader, 16 levels
 * - nexus_v1_engine.c         V1 engine singleton
 * - nexus_v1_game.c           state receipt; start selector unknown
 * - nexus_v1_champions.c      authenticated PLRD source rows
 * - nexus_v1_creatures.c      CRET/MNS source receipts; AI/render gated
 * - nexus_v1_movement.c       NEXUS_CMD_* source labels; producer unbound
 * - nexus_v1_combat.c         diagnostic melee/spell tables only
 * - nexus_v1_magic.c          diagnostic rune/mana tables only
 * - nexus_v1_inventory.c      source records; action consumer unbound
 * - nexus_v1_save_load.c      native save receipt; Saturn owner unbound
 * - nexus_v1_sound.c          SLEV/SAL/SDDRVS source receipts only
 * - nexus_v1_rasterizer.c     host raster probe; Saturn submit no-draw
 *
 * Phase 0 rule: V1 Nexus code compiles cleanly WITHOUT any unverified
 * Saturn presentation code being active. V2 probe modules (lighting, particles,
 * atmosphere, HUD overlay, smooth movement, touch/controller
 * affordances) MUST NOT mutate V1 game state.
 */

#include "nexus_v2_phase_gate_pc34.h"
#include <string.h>

/* ----------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------- */

static NEXUS_V2_PhaseGateDecision make_decision(
    int sourceLocked,
    int presentationAllowed,
    const char *anchor,
    const char *rule)
{
    NEXUS_V2_PhaseGateDecision out;
    out.v1SourceLocked       = sourceLocked ? 1 : 0;
    out.v2PresentationAllowed = presentationAllowed ? 1 : 0;
    out.sourceAnchor         = anchor;
    out.rule                 = rule;
    return out;
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void nexus_v2_phase_gate_defaults(NEXUS_V2_PhaseGateConfig *config)
{
    if (!config) return;
    /* Phase 0 default: V2 off, config persistence off. */
    config->v2PresentationEnabled       = 0;
    config->v2ConfigPersistenceEnabled = 0;
}

int nexus_v2_phase_gate_is_gameplay_domain(NEXUS_V2_PhaseDomain domain)
{
    switch (domain) {
        /* V1-source-locked gameplay domains */
        case NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING:
        case NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER:
        case NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT:
        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY:
        case NEXUS_V2_PHASE_DOMAIN_CREATURE_AI:
        case NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC:
        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:
        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:
        case NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER:
        case NEXUS_V2_PHASE_DOMAIN_RASTERIZER:
        case NEXUS_V2_PHASE_DOMAIN_INVENTORY:
            return 1;

        /* V2-presentation-eligible domains */
        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY:
        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:
        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE:
        case NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_UPSCALER:
            return 0;

        default:
            /* Unknown domains default to V1-locked for safety. */
            return 1;
    }
}

int nexus_v2_phase_gate_v2_active(const NEXUS_V2_PhaseGateConfig *config)
{
    return config && config->v2PresentationEnabled;
}

const char *nexus_v2_phase_gate_domain_name(NEXUS_V2_PhaseDomain domain)
{
    switch (domain) {
        case NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING:                 return "DMDF_DGN_LOADING";
        case NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER:                return "SATURN_ISO_READER";
        case NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT:                  return "GAME_STATE_INIT";
        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY:                   return "CHAMPION_PARTY";
        case NEXUS_V2_PHASE_DOMAIN_CREATURE_AI:                      return "CREATURE_AI";
        case NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC:                      return "SPELL_MAGIC";
        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:                         return "MOVEMENT";
        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:                        return "SAVE_LOAD";
        case NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER:                     return "SOUND_DRIVER";
        case NEXUS_V2_PHASE_DOMAIN_RASTERIZER:                       return "RASTERIZER";
        case NEXUS_V2_PHASE_DOMAIN_INVENTORY:                        return "INVENTORY";
        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:              return "RENDER_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:     return "SMOOTH_MOVEMENT_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:    return "DYNAMIC_LIGHTING_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY:                      return "HUD_OVERLAY";
        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:                 return "PARTICLE_EFFECTS";
        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE:                       return "ATMOSPHERE";
        case NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION:               return "INPUT_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:              return "CONFIG_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_UPSCALER:                         return "UPSCALER";
        default:                                                     return "UNKNOWN";
    }
}

const char *nexus_v2_phase_gate_source_evidence(void)
{
    return
        "Verified Saturn retail data: DM.BIN and LEV00.DGN through LEV15.DGN; "
        "V1 parser receipts cover bounded file formats and data readers; "
        "the executable entry point and runtime consumers remain unbound; "
        "no Saturn input, HUD, lighting, or movement-timing consumer receipt is admitted; "
        "ReDMCSB and Theron's Quest sources are comparisons only, not Saturn source evidence.";
}

NEXUS_V2_PhaseGateDecision nexus_v2_phase_gate_decide(
    const NEXUS_V2_PhaseGateConfig *config,
    NEXUS_V2_PhaseDomain domain)
{
    int v2Active = config && config->v2PresentationEnabled;
    int configPersistence = config && config->v2ConfigPersistenceEnabled;

    switch (domain) {

        /* ── V1-source-locked gameplay domains ── */

        case NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING:
            return make_decision(
                1, 0,
                "nexus_v1_dmdf_model.c; nexus_v1_dungeon.c (format readers)",
                "Format readers have bounded tests; full retail level loading "
                "and gameplay ownership are not established");

        case NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER:
            return make_decision(
                1, 0,
                "nexus_v1_iso_reader.c (ISO 9660 reader)",
                "Reader behavior does not establish the complete Saturn boot "
                "handoff or retail executable ownership");

        case NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT:
            return make_decision(
                1, 0,
                "nexus_v1_game.c; launcher lifecycle remains unbound",
                "Retail startup, selector, starting state, and level handoff "
                "remain unverified");

        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY:
            return make_decision(
                1, 0,
                "nexus_v1_champions.c (bounded PLRD records)",
                "Record decoding does not establish the retail party screen "
                "or champion-state consumers");

        case NEXUS_V2_PHASE_DOMAIN_CREATURE_AI:
            return make_decision(
                1, 0,
                "nexus_v1_creatures.c; CRET/MNS format probes",
                "AI, combat, drops, and creature presentation have no admitted "
                "retail consumer receipt");

        case NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC:
            return make_decision(
                1, 0,
                "nexus_v1_magic.c (diagnostic tables)",
                "Retail spell dispatch, rune UI, state mutation, and effects "
                "remain unverified");

        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:
            return make_decision(
                1, 0,
                "nexus_v1_movement.c (command labels only)",
                "Retail input dispatch, movement timing, collision response, "
                "and viewport ownership remain unverified");

        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:
            return make_decision(
                1, 0,
                "nexus_v1_save_load.c (save-format study)",
                "Retail save/load ownership is unverified; the local V2 "
                "settings gate is not a Saturn save receipt");

        case NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER:
            return make_decision(
                1, 0,
                "nexus_v1_sound.c (bounded sound-data readers)",
                "Retail SCSP dispatch, scheduling, and CD audio playback "
                "remain unverified");

        case NEXUS_V2_PHASE_DOMAIN_RASTERIZER:
            return make_decision(
                1, 0,
                "nexus_v1_rasterizer.c (host raster fixture)",
                "A host raster fixture does not establish Saturn VDP1 command "
                "submission or retail viewport composition");

        case NEXUS_V2_PHASE_DOMAIN_INVENTORY:
            return make_decision(
                1, 0,
                "nexus_v1_inventory.c (diagnostic state study)",
                "Retail inventory input, mutation, and presentation consumers "
                "remain unverified");

        /* ── V2-presentation-eligible domains ── */

        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_render_pipeline.c (diagnostic helper)",
                "The local gate permits this helper only in diagnostic builds; "
                "retail Saturn presentation remains blocked");

        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_smooth_movement.c (diagnostic helper)",
                "No retail movement-to-presentation timing receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_lighting.c (diagnostic math)",
                "No retail VDP1/VDP2 lighting consumer receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY:
            return make_decision(
                0, v2Active,
                "nexus_v2_hud_overlay.c (diagnostic overlay)",
                "No retail HUD widget or VDP placement receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:
            return make_decision(
                0, v2Active,
                "nexus_v2_particles.c (diagnostic helper)",
                "No retail particle effect or presentation receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE:
            return make_decision(
                0, v2Active,
                "nexus_v2_atmosphere.c (diagnostic helper)",
                "No retail atmosphere or camera-motion receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_touch_controller_affordance.c (experimental UI metadata)",
                "No retail Saturn input-dispatch or control-mapping receipt is admitted");

        case NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
            /* CONFIG_PRESENTATION requires BOTH v2PresentationEnabled AND
             * v2ConfigPersistenceEnabled. This is a stricter gate than
             * the other V2-eligible domains because config writes are
             * persistent state changes. */
            return make_decision(
                0, v2Active && configPersistence,
                "nexus_v2_config.c (local settings helper)",
                "The diagnostic settings helper requires both local toggles; "
                "this does not establish retail configuration behavior");

        case NEXUS_V2_PHASE_DOMAIN_UPSCALER:
            return make_decision(
                0, v2Active,
                "nexus_v2_upscaler.c (diagnostic helper)",
                "No retail framebuffer ownership or upscaler path is admitted");

        default:
            /* Unknown domains default to V1-locked. */
            return make_decision(
                1, 0,
                "default: unknown domain -> V1-locked for safety",
                "unknown domains default to V1-locked");
    }
}
