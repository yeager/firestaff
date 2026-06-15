/*
 * nexus_v1_compat_gate.c — Nexus V1/V2 Phase Gate Implementation
 *
 * Phase 0: V1 Compatibility Lock Before V2 Work
 *
 * NEXUS_V1_PHASE_DOMAIN_LOCK gate: all V1 game-logic domains are
 * permanently source-locked. V2 presentation modules (atmosphere, HUD,
 * smooth movement, lighting, particles, upscaler, touch affordances)
 * may activate ONLY when v2PresentationEnabled is set AND the domain
 * is a V2-presentation-eligible domain (never a V1-source-locked
 * gameplay domain).
 *
 * V1 source-lock anchors:
 *   docs/NEXUS_FILE_CLASSIFICATION.md — 138-file Saturn asset catalog
 *   Greatstone DM Nexus (greatstone.free.fr/g_dm.html) — maps/atlas
 *   ReDMCSB DUNGEON.C — square type dispatch
 *   ReDMCSB COMMAND.C:2045-2155 — command queue dispatch
 *   ReDMCSB CHAMPION.C F0309 — champion maximum load
 *   ReDMCSB MOVESENS.C F0267 — movement result side effects
 *   ReDMCSB GAMELOOP.C:47-50 — 55 ms world tick cadence
 *   Saturn VDP1 SDK — BGR555 framebuffer, BITMAP command
 *   Saturn SDDRVS.TSK — 26 KB sound driver task
 *
 * Phase 0 rules:
 *   All V1 domains (DUNGEON_LOADING..AUDIO) are permanently source-locked.
 *   V2 modules may present their output in V2-presentation-eligible domains
 *   only when v2PresentationEnabled is true.
 *   V2 must NEVER alter any V1 game-logic state.
 *
 * Version: Phase 0 v1.0.0
 */

#include "nexus/nexus_v1_compat_gate.h"
#include <string.h>

/* ================================================================
 * Internal helpers
 * ================================================================ */

static Nexus_V1_PhaseGateDecision make_decision(
    int sourceLocked,
    int v2Allowed,
    const char *anchor,
    const char *rule)
{
    Nexus_V1_PhaseGateDecision out;
    out.v1SourceLocked = sourceLocked ? 1 : 0;
    out.v2PresentationAllowed = v2Allowed ? 1 : 0;
    out.sourceAnchor = anchor;
    out.rule = rule;
    return out;
}

/* Returns 1 if domain is a V1-source-locked gameplay domain. */
static int is_v1_locked_domain(Nexus_V2_PhaseDomain domain)
{
    switch (domain) {
        /* V1-source-locked gameplay domains */
        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING:
        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_RASTERIZER:
        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_POOL:
        case NEXUS_V2_PHASE_DOMAIN_COMBAT:
        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:
        case NEXUS_V2_PHASE_DOMAIN_MAGIC:
        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:
        case NEXUS_V2_PHASE_DOMAIN_AUDIO:
            return 1;

        /* V2-presentation-eligible domains */
        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_LIGHTING_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_HUD_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE_PRESENTATION:
        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:
        case NEXUS_V2_PHASE_DOMAIN_TOUCH_PRESENTATION:
            return 0;

        default:
            return 1; /* unknown → V1-locked (fail-safe) */
    }
}

/* ================================================================
 * Public API
 * ================================================================ */

void nexus_v1_phase_gate_defaults(Nexus_V1_PhaseGateConfig *config)
{
    /* Default: all V2 presentation OFF — full V1 source-locked boot.
     * Caller must explicitly set v2PresentationEnabled to activate
     * any V2 presentation. */
    if (!config) return;
    config->v2PresentationEnabled = 0;
}

int nexus_v1_phase_gate_is_v1_locked(Nexus_V2_PhaseDomain domain)
{
    return is_v1_locked_domain(domain);
}

Nexus_V1_PhaseGateDecision nexus_v1_phase_gate_decide(
    const Nexus_V1_PhaseGateConfig *config,
    Nexus_V2_PhaseDomain domain)
{
    int v2Active = config && config->v2PresentationEnabled;

    switch (domain) {

        /* ── V1-source-locked gameplay domains ── */

        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING:
            /* DM Nexus Saturn DGN level format + DMDF parser.
             * Source: docs/NEXUS_FILE_CLASSIFICATION.md (DGN/DMDF);
             *   Greatstone DM Nexus dungeon map data.
             * V2 must not reinterpret level layout, square types, or
             * teleporter/stairs/door placement. */
            return make_decision(
                1, 0,
                "docs/NEXUS_FILE_CLASSIFICATION.md DGN/DMDF sections; "
                "Greatstone DM Nexus dungeon maps; "
                "nexus_v1_dmdf_model.c; nexus_v1_dungeon.c",
                "DGN level format and DMDF parser stay V1-source-locked; "
                "V2 may not reinterpret level layout");

        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_RASTERIZER:
            /* Saturn VDP1 rasterizer — 2D blit pipeline to BGR555 FB.
             * Source: nexus_v1_rasterizer.c
             *   (ReDMCSB DRAWVIEW.C F2172, BLIT.C F0132,
             *    DUNGEON.C F0108, OBJECT.C F0841-F0843);
             *   Saturn VDP1 SDK BITMAP command format.
             * V2 enhanced rendering must not alter dungeon square
             * composition or texture atlas selection. */
            return make_decision(
                1, 0,
                "nexus_v1_rasterizer.c "
                "(ReDMCSB DRAWVIEW.C F2172, BLIT.C F0132, "
                "DUNGEON.C F0108, OBJECT.C F0841-F0843); "
                "Saturn VDP1 SDK BITMAP command",
                "VDP1 rasterizer pipeline stays V1-source-locked; "
                "V2 enhanced rendering presents but never alters "
                "dungeon square composition");

        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_POOL:
            /* Nexus champion pool — 6 slots, Nexus-specific stat growth.
             * Source: nexus_v1_champions.c
             *   (ReDMCSB CHAMPION.C F0309 maximum load calculation;
             *    docs/NEXUS_FILE_CLASSIFICATION.md CHAMPION.SAV);
             *   nexus_v1_mechanics.c
             * V2 HUD must not alter champion stats, party order, or
             * maximum load calculations. */
            return make_decision(
                1, 0,
                "nexus_v1_champions.c (ReDMCSB CHAMPION.C F0309); "
                "nexus_v1_mechanics.c "
                "(ReDMCSB COMMAND.C, MOVESENS.C, CHAMPION.C, CREATURE.C); "
                "docs/NEXUS_FILE_CLASSIFICATION.md CHAMPION.SAV",
                "champion pool, stat growth, and party management stay "
                "V1-source-locked; V2 HUD is presentation-only");

        case NEXUS_V2_PHASE_DOMAIN_COMBAT:
            /* Nexus combat — champion attack, damage, experience.
             * Source: nexus_v1_combat.c (ReDMCSB COMBAT.C pattern).
             * V2 must not alter combat resolution, damage, or XP rules. */
            return make_decision(
                1, 0,
                "nexus_v1_combat.c (ReDMCSB COMBAT.C pattern); "
                "ReDMCSB CHAMPION.C champion stat mechanics",
                "combat resolution, damage calculation, and XP gain "
                "stay V1-source-locked; V2 is presentation-only");

        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:
            /* Nexus movement — click/touch command routing, input queue.
             * Source: nexus_v1_movement.c
             *   (ReDMCSB CLIKMENU.C F0366
             *    COMMAND_ProcessTypes3To6_MoveParty);
             *   nexus_v1_mechanics.c command queue.
             * V2 smooth movement must not alter V1 movement cooldowns,
             * collision checks, or sensor timing. */
            return make_decision(
                1, 0,
                "nexus_v1_movement.c "
                "(ReDMCSB CLIKMENU.C F0366 "
                "COMMAND_ProcessTypes3To6_MoveParty); "
                "nexus_v1_mechanics.c command queue; "
                "ReDMCSB MOVESENS.C F0267",
                "movement command routing and input queue stay V1; "
                "V2 smooth movement may interpolate presented position "
                "only; V1 cooldowns and collision are unaffected");

        case NEXUS_V2_PHASE_DOMAIN_MAGIC:
            /* Nexus magic — rune spell casting, mana, element/form/align.
             * Source: nexus_v1_magic.c (ReDMCSB MAGIC.C pattern).
             * V2 must not alter spell selection or mana costs. */
            return make_decision(
                1, 0,
                "nexus_v1_magic.c (ReDMCSB MAGIC.C pattern); "
                "docs/NEXUS_FILE_CLASSIFICATION.md MAGIC.SCR section",
                "rune spell casting, mana costs, and spell effects "
                "stay V1-source-locked; V2 particle effects are "
                "presentation-only and must not alter spell resolution");

        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:
            /* Nexus save/load — Firestaff native format (FNXS magic).
             * Source: nexus_v1_save_load.c
             *   (ReDMCSB LOADSAVE.C F0433/F0434,
             *    SAVEHEAD.C F0429/F0430);
             *   docs/NEXUS_FILE_CLASSIFICATION.md CHAMPION.SAV section.
             * Saturn memory card format is proprietary (not reverse-
             * engineered). V2 config persistence must not alter the
             * V1 save payload. */
            return make_decision(
                1, 0,
                "nexus_v1_save_load.c "
                "(ReDMCSB LOADSAVE.C F0433/F0434, "
                "SAVEHEAD.C F0429/F0430); "
                "docs/NEXUS_FILE_CLASSIFICATION.md CHAMPION.SAV; "
                "Greatstone DM Nexus save format",
                "V1 save/load payload semantics stay V1-source-locked; "
                "V2 config persistence is explicitly separate and "
                "presentation-only");

        case NEXUS_V2_PHASE_DOMAIN_AUDIO:
            /* Saturn CDDA audio + SDDRVS.TSK sound driver task.
             * Source: nexus_v1_sound.c
             *   (Saturn SDDRVS.TSK 26 KB sound driver task;
             *    docs/NEXUS_FILE_CLASSIFICATION.md AUDIO.SF section).
             * V2 must not alter audio event timing or trigger rules. */
            return make_decision(
                1, 0,
                "nexus_v1_sound.c "
                "(Saturn SDDRVS.TSK 26 KB sound driver task; "
                "docs/NEXUS_FILE_CLASSIFICATION.md AUDIO.SF); "
                "Greatstone DM Nexus audio reference",
                "audio event triggers and timing stay V1-source-locked; "
                "V2 enhanced audio is presentation-only");

        /* ── V2-presentation-eligible domains ── */

        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:
            /* V2 enhanced rendering: VDP1 upscaler, post-processing.
             * Source: nexus_v2_render_pipeline.c; nexus_v2_upscaler.c.
             * V2 may present V1 dungeon with enhanced output when
             * v2PresentationEnabled is true. */
            return make_decision(
                0, v2Active,
                "nexus_v2_render_pipeline.c (Phase 2); "
                "nexus_v2_upscaler.c (Phase 2.1); "
                "nexus_v1_rasterizer.c (V1 rasterizer anchor)",
                v2Active
                    ? "V2 RENDER: enhanced VDP1 rendering active"
                    : "V2 RENDER: V1 rasterizer source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:
            /* V2 smooth movement: visual interpolation only.
             * Source: nexus_v2_render_pipeline.c.
             * V2 may interpolate presented position; V1 cooldowns,
             * collision, and sensor timing are unaffected. */
            return make_decision(
                0, v2Active,
                "nexus_v2_render_pipeline.c (Phase 5); "
                "nexus_v1_movement.c (V1 movement anchor); "
                "ReDMCSB GAMELOOP.C:47-50 (55 ms world tick)",
                v2Active
                    ? "V2 SMOOTH_MOVEMENT: interpolation active"
                    : "V2 SMOOTH_MOVEMENT: V1 movement source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_LIGHTING_PRESENTATION:
            /* V2 enhanced lighting: VDP2 local light sources.
             * Source: nexus_v2_lighting.c (Phase 4).
             * V1 canonical palette stays; V2 light map is
             * presentation-only. */
            return make_decision(
                0, v2Active,
                "nexus_v2_lighting.c (Phase 4); "
                "nexus_v1_palette.c (V1 palette anchor; "
                "ReDMCSB PALETTE.C canonical palette); "
                "Saturn VDP2 SDK",
                v2Active
                    ? "V2 LIGHTING: Saturn VDP2 enhanced lighting active"
                    : "V2 LIGHTING: V1 canonical palette source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_HUD_PRESENTATION:
            /* V2 HUD overlay: compass, depth, gold, champion bars.
             * Source: nexus_v2_hud_overlay.c (Phase 3).
             * HUD is presentation-only and does not mutate V1 game state. */
            return make_decision(
                0, v2Active,
                "nexus_v2_hud_overlay.c (Phase 3); "
                "nexus_v1_champions.c (champion bar data anchor); "
                "nexus_v1_dungeon.c (depth counter anchor); "
                "ReDMCSB PANEL.C F0120-F0125 panel element drawing",
                v2Active
                    ? "V2 HUD: Phase 3 HUD overlay active"
                    : "V2 HUD: V1 panel source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE_PRESENTATION:
            /* V2 Saturn atmosphere: VDP2 background effects, sky.
             * Source: nexus_v2_atmosphere.c (Phase 4).
             * Presentation-only; V1 dungeon state not modified. */
            return make_decision(
                0, v2Active,
                "nexus_v2_atmosphere.c (Phase 4); "
                "Saturn VDP2 SDK background effects; "
                "docs/NEXUS_FILE_CLASSIFICATION.md TITLE.CG section",
                v2Active
                    ? "V2 ATMOSPHERE: Saturn VDP2 atmosphere active"
                    : "V2 ATMOSPHERE: V1 source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:
            /* V2 particle effects: magic, fire, ambient.
             * Source: nexus_v2_particles.c (Phase 4).
             * V2 must not alter V1 combat resolution or creature AI. */
            return make_decision(
                0, v2Active,
                "nexus_v2_particles.c (Phase 4); "
                "nexus_v1_combat.c (combat anchor); "
                "nexus_v1_creatures.c (creature AI anchor)",
                v2Active
                    ? "V2 PARTICLES: particle effects active"
                    : "V2 PARTICLES: V1 combat/AI source-locked "
                      "(v2PresentationEnabled=0)");

        case NEXUS_V2_PHASE_DOMAIN_TOUCH_PRESENTATION:
            /* V2 touch/controller: gesture routing, affordances.
             * Source: nexus_v2_touch_controller_affordance.c (Phase 6).
             * V2 touch must map to V1 command ids (C001-C006) ONLY
             * behind the v2PresentationEnabled gate. */
            return make_decision(
                0, v2Active,
                "nexus_v2_touch_controller_affordance.c (Phase 6); "
                "nexus_v1_movement.c (V1 command routing anchor; "
                "ReDMCSB COMMAND.C:2045-2155 command queue); "
                "ReDMCSB COMMAND.C:108-113 mouse movement zones",
                v2Active
                    ? "V2 TOUCH: touch/controller affordances active"
                    : "V2 TOUCH: V1 input routing source-locked "
                      "(v2PresentationEnabled=0)");

        default:
            return make_decision(
                1, 0,
                "unknown phase domain",
                "unknown domains are V1-locked by default (fail-safe)");
    }
}

int nexus_v1_phase_gate_v2_active(const Nexus_V1_PhaseGateConfig *config)
{
    return config && config->v2PresentationEnabled;
}

const char *nexus_v1_phase_gate_domain_name(Nexus_V2_PhaseDomain domain)
{
    switch (domain) {
        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING:               return "DUNGEON_LOADING";
        case NEXUS_V2_PHASE_DOMAIN_DUNGEON_RASTERIZER:            return "DUNGEON_RASTERIZER";
        case NEXUS_V2_PHASE_DOMAIN_CHAMPION_POOL:                 return "CHAMPION_POOL";
        case NEXUS_V2_PHASE_DOMAIN_COMBAT:                        return "COMBAT";
        case NEXUS_V2_PHASE_DOMAIN_MOVEMENT:                      return "MOVEMENT";
        case NEXUS_V2_PHASE_DOMAIN_MAGIC:                         return "MAGIC";
        case NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD:                     return "SAVE_LOAD";
        case NEXUS_V2_PHASE_DOMAIN_AUDIO:                         return "AUDIO";
        case NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION:           return "RENDER_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION:  return "SMOOTH_MOVEMENT_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_LIGHTING_PRESENTATION:        return "LIGHTING_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_HUD_PRESENTATION:              return "HUD_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE_PRESENTATION:       return "ATMOSPHERE_PRESENTATION";
        case NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS:              return "PARTICLE_EFFECTS";
        case NEXUS_V2_PHASE_DOMAIN_TOUCH_PRESENTATION:            return "TOUCH_PRESENTATION";
        default:                                                   return "UNKNOWN";
    }
}

const char *nexus_v1_phase_gate_source_evidence(void)
{
    return "nexus_v1_compat_gate.c Phase 0 v1.0.0 "
           "(Nexus V1/V2 domain separation) "
           "V1 anchors: docs/NEXUS_FILE_CLASSIFICATION.md, "
           "Greatstone DM Nexus (greatstone.free.fr/g_dm.html), "
           "ReDMCSB DUNGEON.C/COMMAND.C/CHAMPION.C/MOVESENS.C/GAMELOOP.C, "
           "nexus_v1_*.c (all 27 source files source-locked). "
           "V2 anchors: nexus_v2_*.c (atmosphere, hud, lighting, "
           "particles, render_pipeline, touch, upscaler) Phase 0 locked.";
}

int nexus_v1_phase_gate_compile_lock(void)
{
    return NEXUS_V1_PHASE_DOMAIN_LOCK;
}
