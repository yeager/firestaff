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
 * - nexus_v1_game.c           state init, level load, CD track map
 * - nexus_v1_champions.c      4-champion party
 * - nexus_v1_creatures.c      creature AI + render
 * - nexus_v1_movement.c       NEXUS_CMD_* (F0365/F0366 analogues)
 * - nexus_v1_combat.c         melee + spell combat
 * - nexus_v1_magic.c          spell casting, mana, runes
 * - nexus_v1_inventory.c      inventory + chest pickup
 * - nexus_v1_save_load.c      save/load round-trip
 * - nexus_v1_sound.c          Saturn SCSP sound driver
 * - nexus_v1_rasterizer.c     320x200 indexed framebuffer
 *
 * Phase 0 rule: V1 Nexus code compiles cleanly WITHOUT any V2
 * presentation code being active. V2 modules (lighting, particles,
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
        "nexus_v1_iso_reader.c (Saturn ISO 9660 + DMDF interleaving); "
        "nexus_v1_dmdf_model.c (DMDF decoder); "
        "nexus_v1_dungeon.c (DGN loader, 16 levels); "
        "nexus_v1_engine.c (V1 engine singleton); "
        "nexus_v1_game.c (state init, CD track map 2-9); "
        "nexus_v1_champions.c (4-champion party); "
        "nexus_v1_creatures.c (MNS-driven creature AI + render); "
        "nexus_v1_movement.c (NEXUS_CMD_FORWARD/BACKWARD/TURN_*/STRAFE_*); "
        "nexus_v1_combat.c (melee + spell combat); "
        "nexus_v1_magic.c (spell casting, mana, runes); "
        "nexus_v1_inventory.c (inventory + chest pickup); "
        "nexus_v1_save_load.c (CDL save/load round-trip); "
        "nexus_v1_sound.c (Saturn SCSP sound driver); "
        "nexus_v1_rasterizer.c (320x200 indexed framebuffer); "
        "ReDMCSB CLIKMENU.C:142 F0365 turn; CLIKMENU.C:180 F0366 move; "
        "COMMAND.C:2045-2155 F0380 input wait loop; "
        "MOVESENS.C:316-345 F0267 move-result side effects; "
        "NEXUS.C / NEXUS2.C (Saturn DM Nexus engine lifecycle); "
        "NEXUS.BIN (Saturn game binary); "
        "HuC6260/HuC6270 VDC/VCE datasheet; "
        "THQUEST.ASM T400/T520/T560/T600/T700/T800/T900 (Theron's Quest parallels).";
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
                "nexus_v1_dmdf_model.c; nexus_v1_dungeon.c (16-level DGN loader)",
                "DMDF format parsing, 16-level DGN loader, square types, and "
                "first-thing chain walk stay V1-source-locked; V2 must not "
                "reinterpret layout");

        case NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER:
            return make_decision(
                1, 0,
                "nexus_v1_iso_reader.c (Saturn ISO 9660 + DMDF interleaving)",
                "ISO 9660 directory + DMDF interleaving stays V1-owned; V2 "
                "must not change the read path or file handle ownership");

        case NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT:
            return make_decision(
                1, 0,
                "nexus_v1_game.c; nexus_v1_launcher.c (party spawn 11,29,N)",
                "Game state init, party spawn, level load, and CD audio "
                "track mapping (2-9) stay V1-source-locked");

        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY:
            return make_decision(
                1, 0,
                "nexus_v1_champions.c (4-champion party, stats, level-up, resurrect)",
                "Champion stats, level-up, food/water/stamina, and resurrect "
                "stay V1-source-locked; V2 must not mutate champion state");

        case NEXUS_V2_PHASE_DOMAIN_CREATURE_AI:
            return make_decision(
                1, 0,
                "nexus_v1_creatures.c (MNS-driven AI); nexus_v1_combat.c",
                "Creature groups, AI behavior, melee/spell combat, and drops "
                "stay V1-source-locked; V2 may only render the existing AI state");

        case NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC:
            return make_decision(
                1, 0,
                "nexus_v1_magic.c (rune UI, mana, projectile spells)",
                "Spell casting, rune UI, mana cost, and projectile behaviour "
                "stay V1-source-locked; V2 may only render spell visuals");

        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:
            return make_decision(
                1, 0,
                "nexus_v1_movement.c (NEXUS_CMD_*); ReDMCSB CLIKMENU.C:142/180",
                "Party movement, turn, strafe, and collision response stay "
                "V1-source-locked; V2 smooth movement may only interpolate "
                "the visual presentation between V1 ticks");

        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:
            return make_decision(
                1, 0,
                "nexus_v1_save_load.c (CDL format round-trip)",
                "V1 CDL save/load payload stays V1-source-locked; V2 config "
                "persistence is explicitly separate and gated by "
                "v2ConfigPersistenceEnabled");

        case NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER:
            return make_decision(
                1, 0,
                "nexus_v1_sound.c (Saturn SCSP driver + CD audio)",
                "SCSP sound driver, SFX scheduling, and CD audio track 2-9 "
                "stay V1-owned; V2 enhanced audio is presentation-only");

        case NEXUS_V2_PHASE_DOMAIN_RASTERIZER:
            return make_decision(
                1, 0,
                "nexus_v1_rasterizer.c (320x200 indexed framebuffer, 9-square viewport)",
                "V1 320x200 indexed framebuffer layout and 9-square viewport "
                "stays V1-source-locked; V2 upscale happens on a read-only copy");

        case NEXUS_V2_PHASE_DOMAIN_INVENTORY:
            return make_decision(
                1, 0,
                "nexus_v1_inventory.c (inventory, chest pickup, weight)",
                "Inventory, chest pickup, encumbrance, and item identification "
                "stay V1-source-locked; V2 may only render the inventory UI");

        /* ── V2-presentation-eligible domains ── */

        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_render_pipeline.c (V1 fb -> V2 RGBA)",
                "V2 render pipeline may only consume a read-only V1 framebuffer; "
                "presentation only when v2PresentationEnabled");

        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_smooth_movement.c (ease-out cubic / quad / ease-in-out cubic)",
                "V2 smooth movement interpolates the visual state over exactly "
                "1 V1 tick (55ms); V1 game state advances on V1 ticks only");

        case NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_lighting.c (per-vertex torch/spell lighting)",
                "V2 dynamic lighting is presentation-only; V1 tile lighting "
                "stays in the V1 rasterizer");

        case NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY:
            return make_decision(
                0, v2Active,
                "nexus_v2_hud_overlay.c (minimap, damage numbers, journal)",
                "V2 HUD overlay is presentation-only; never alters V1 game state");

        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:
            return make_decision(
                0, v2Active,
                "nexus_v2_particles.c (particle systems, weather, fire)",
                "V2 particle effects are presentation-only; never alter V1 "
                "game state");

        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE:
            return make_decision(
                0, v2Active,
                "nexus_v2_atmosphere.c (fog, AO, reflective floors, camera bob)",
                "V2 atmosphere is presentation-only; never alters V1 game state");

        case NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION:
            return make_decision(
                0, v2Active,
                "nexus_v2_touch_controller_affordance.c (touch swipe, edge strafe, D-pad)",
                "V2 touch/controller affordances are presentation-only; V1 "
                "mouse/touch/click parity preserved when v2PresentationEnabled=0");

        case NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION:
            /* CONFIG_PRESENTATION requires BOTH v2PresentationEnabled AND
             * v2ConfigPersistenceEnabled. This is a stricter gate than
             * the other V2-eligible domains because config writes are
             * persistent state changes. */
            return make_decision(
                0, v2Active && configPersistence,
                "nexus_v2_config.c (M12 settings persistence, separate from V1 save/load)",
                "V2 config presentation is allowed only when BOTH "
                "v2PresentationEnabled=1 AND v2ConfigPersistenceEnabled=1; "
                "config writes are persistent state changes and require the "
                "config-persistence toggle");

        case NEXUS_V2_PHASE_DOMAIN_UPSCALER:
            return make_decision(
                0, v2Active,
                "nexus_v2_upscaler.c (EPX + bilinear filter, V1 fb read-only)",
                "V2 upscaler consumes a read-only V1 framebuffer; presentation "
                "only when v2PresentationEnabled");

        default:
            /* Unknown domains default to V1-locked. */
            return make_decision(
                1, 0,
                "default: unknown domain -> V1-locked for safety",
                "unknown domains default to V1-locked");
    }
}
