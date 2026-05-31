/*
 * nexus_v1_compat_gate.h — Nexus V1/V2 Phase Gate Definitions
 *
 * Phase 0: V1 Compatibility Lock Before V2 Work
 *
 * NEXUS_V1_PHASE_DOMAIN_LOCK is a compile-time flag that locks all
 * Nexus V1 behaviour (dungeon loading, rasterizer, champions, combat,
 * movement, magic, save/load, sound) and explicitly blocks V2-only
 * code paths from activating.
 *
 * Phase 0 rule: V1 Nexus code compiles cleanly WITHOUT any V2
 * presentation code being active. All V2 modules (atmosphere,
 * HUD overlay, smooth movement, lighting, particles, upscaler,
 * touch controller affordances) MUST NOT alter V1 game-logic state.
 *
 * V1 source-lock anchors (DM Nexus Saturn):
 *   docs/NEXUS_FILE_CLASSIFICATION.md — full 138-file Saturn asset catalog
 *   Greatstone DM Nexus (greatstone.free.fr/g_dm.html) — dungeon maps,
 *     creature atlas, graphics reference
 *   ReDMCSB DUNGEON.C — dungeon square type dispatch (shared with DM1/CSB)
 *   ReDMCSB COMMAND.C:2045-2155 — command queue dispatch (shared pattern)
 *   ReDMCSB CHAMPION.C F0309 — champion maximum load calculation
 *   ReDMCSB MOVESENS.C F0267 — movement result side effects
 *   Saturn VDP1 SDK — Color RAM BGR555 format, BITMAP command format
 *   Saturn SDDRVS.TSK — 26 KB sound driver task
 *
 * V2 presentation anchors (Firestaff only — no original source):
 *   nexus_v2_atmosphere.c          (Phase 4 — Saturn VDP2 atmosphere)
 *   nexus_v2_hud_overlay.c         (Phase 3 — HUD overlay rendering)
 *   nexus_v2_lighting.c            (Phase 4 — VDP2 enhanced lighting)
 *   nexus_v2_particles.c           (Phase 4 — particle effects)
 *   nexus_v2_render_pipeline.c     (Phase 2 — enhanced rasterizer)
 *   nexus_v2_touch_controller_affordance.c (Phase 6 — touch gestures)
 *   nexus_v2_upscaler.c            (Phase 2 — V2.1 upscaled output)
 *
 * Version: Phase 0 v1.0.0 — Firestaff Nexus
 */

#ifndef FIRESTAFF_NEXUS_V1_COMPAT_GATE_H
#define FIRESTAFF_NEXUS_V1_COMPAT_GATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * NEXUS_V1_PHASE_DOMAIN_LOCK — compile-time gate
 * ================================================================
 *
 * Set to 1 to lock the build to V1 source-locked behaviour.
 * V2-only paths are blocked when this is defined.
 *
 * V2 Phase 1+ modules that include this header can query the
 * runtime gate (via Nexus_V1_PhaseGateConfig) to determine whether
 * V2 presentation is active, but V1 game-logic is NEVER altered
 * by V2 code regardless of this flag's value.
 *
 * The macro form lets V2 code use #if NEXUS_V1_PHASE_DOMAIN_LOCK
 * for compile-time dead-code elimination of V2 paths in V1 builds.
 */
#ifndef NEXUS_V1_PHASE_DOMAIN_LOCK
#define NEXUS_V1_PHASE_DOMAIN_LOCK 1
#endif

/* ================================================================
 * Phase domains
 * ================================================================
 *
 * V1-source-locked domains: V2 must NOT alter behaviour.
 * V2-presentation-eligible domains: V2 may PRESENT but not alter
 * the underlying V1 game-logic state.
 */

typedef enum {
    /* ── V1-source-locked gameplay domains ── */

    NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING = 0,
        /* DM Nexus Saturn DGN level format; DMDF parser.
         * Source: docs/NEXUS_FILE_CLASSIFICATION.md DGN/DMDF sections;
         *   Greatstone DM Nexus dungeon map data.
         * V2 must not reinterpret level layout or square types. */

    NEXUS_V2_PHASE_DOMAIN_DUNGEON_RASTERIZER = 1,
        /* Saturn VDP1 rasterizer — 2D blit pipeline, BGR555 framebuffer.
         * Source: nexus_v1_rasterizer.c (ReDMCSB DRAWVIEW.C F2172,
         *   BLIT.C F0132, DUNGEON.C F0108, OBJECT.C F0841-F0843);
         *   Saturn VDP1 SDK BITMAP command.
         * V2 enhanced rendering must not alter dungeon square composition. */

    NEXUS_V2_PHASE_DOMAIN_CHAMPION_POOL = 2,
        /* Nexus champion pool — 6 slots, Nexus-specific stat growth.
         * Source: nexus_v1_champions.c (ReDMCSB CHAMPION.C F0309);
         *   docs/NEXUS_FILE_CLASSIFICATION.md CHAMPION.SAV section.
         * V2 HUD must not alter champion stats or party composition. */

    NEXUS_V2_PHASE_DOMAIN_COMBAT = 3,
        /* Nexus combat — champion attack, damage, experience.
         * Source: nexus_v1_combat.c (ReDMCSB COMBAT.C pattern).
         * V2 must not alter combat resolution, damage, or XP rules. */

    NEXUS_V2_PHASE_DOMAIN_MOVEMENT = 4,
        /* Nexus movement — click/touch command routing, input queue.
         * Source: nexus_v1_movement.c (ReDMCSB CLIKMENU.C F0366
         *   COMMAND_ProcessTypes3To6_MoveParty).
         * V2 smooth movement must not alter V1 movement cooldowns. */

    NEXUS_V2_PHASE_DOMAIN_MAGIC = 5,
        /* Nexus magic — rune spell casting, mana, element/form/align.
         * Source: nexus_v1_magic.c (ReDMCSB MAGIC.C pattern).
         * V2 must not alter spell selection or mana costs. */

    NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD = 6,
        /* Nexus save/load — Firestaff native format (FNXS magic).
         * Source: nexus_v1_save_load.c (ReDMCSB LOADSAVE.C F0433/F0434,
         *   SAVEHEAD.C F0429/F0430).
         * Saturn memory card format is proprietary (not reverse-engineered).
         * V2 config persistence must not alter V1 save payload. */

    NEXUS_V2_PHASE_DOMAIN_AUDIO = 7,
        /* Saturn CDDA audio + SDDRVS.TSK sound driver task.
         * Source: nexus_v1_sound.c (Saturn SDDRVS.TSK 26 KB driver;
         *   docs/NEXUS_FILE_CLASSIFICATION.md AUDIO.SF section).
         * V2 must not alter audio event timing or trigger rules. */

    /* ── V2-presentation-eligible domains ── */

    NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION = 8,
        /* V2 enhanced rendering: VDP1 upscaler, post-processing filters.
         * Source: nexus_v2_render_pipeline.c; nexus_v2_upscaler.c.
         * V2 may present the V1 dungeon picture with enhanced output
         * when v2PresentationEnabled is true. */

    NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION = 9,
        /* V2 smooth movement: interpolates visually between V1 walk/turn
         * states. V1 cooldowns, collision, and sensor timing unaffected.
         * Source: nexus_v2_render_pipeline.c.
         * V2 may only interpolate the PRESENTED position, never the
         * underlying V1 world position. */

    NEXUS_V2_PHASE_DOMAIN_LIGHTING_PRESENTATION = 10,
        /* V2 enhanced lighting: Saturn VDP2 local light sources on top
         * of the V1 canonical palette.
         * Source: nexus_v2_lighting.c.
         * V1 palette stays canonical; V2 light map is presentation-only. */

    NEXUS_V2_PHASE_DOMAIN_HUD_PRESENTATION = 11,
        /* V2 HUD overlay: compass, depth, gold counter, champion bars,
         * action strip for Nexus Saturn.
         * Source: nexus_v2_hud_overlay.c.
         * HUD is presentation-only and does not mutate V1 game state. */

    NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE_PRESENTATION = 12,
        /* V2 Saturn atmosphere: VDP2 background effects, sky gradient.
         * Source: nexus_v2_atmosphere.c (Phase 4).
         * Presentation-only; V1 dungeon state is not modified. */

    NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS = 13,
        /* V2 particle effects: magic, fire, explosions, ambient.
         * Source: nexus_v2_particles.c (Phase 4).
         * V2 must not alter V1 combat resolution or creature AI. */

    NEXUS_V2_PHASE_DOMAIN_TOUCH_PRESENTATION = 14,
        /* V2 touch/controller: gesture routing, controller affordances.
         * Source: nexus_v2_touch_controller_affordance.c (Phase 6).
         * V2 touch must map to V1 command ids (C001-C006) ONLY behind
         * the v2PresentationEnabled gate. */

    NEXUS_V2_PHASE_DOMAIN_COUNT
} Nexus_V2_PhaseDomain;

/* ================================================================
 * Phase gate config
 * ================================================================
 *
 * Controls which V2 presentation features are enabled at runtime.
 * Defaults to all-zero (V1-only boot).
 */

typedef struct {
    int v2PresentationEnabled;      /* master V2 presentation toggle    */
} Nexus_V1_PhaseGateConfig;

/* ================================================================
 * Phase gate decision
 * ================================================================ */

typedef struct {
    int v1SourceLocked;        /* 1 = this domain must stay V1      */
    int v2PresentationAllowed; /* 1 = V2 may present, never alter   */
    const char *sourceAnchor;   /* source reference citation          */
    const char *rule;           /* short gate rule description       */
} Nexus_V1_PhaseGateDecision;

/* ================================================================
 * Public API
 * ================================================================
 */

/* Set defaults: all V2 presentation features OFF (V1-only boot). */
void nexus_v1_phase_gate_defaults(Nexus_V1_PhaseGateConfig *config);

/* Returns 1 if domain is V1-source-locked (V2 must not alter). */
int nexus_v1_phase_gate_is_v1_locked(Nexus_V2_PhaseDomain domain);

/* Query whether V2 presentation is allowed for a domain. */
Nexus_V1_PhaseGateDecision nexus_v1_phase_gate_decide(
    const Nexus_V1_PhaseGateConfig *config,
    Nexus_V2_PhaseDomain domain);

/* Returns 1 if v2PresentationEnabled is set. */
int nexus_v1_phase_gate_v2_active(const Nexus_V1_PhaseGateConfig *config);

/* Human-readable domain name for debugging/logging. */
const char *nexus_v1_phase_gate_domain_name(Nexus_V2_PhaseDomain domain);

/* Source evidence string for verification scripts. */
const char *nexus_v1_phase_gate_source_evidence(void);

/* Compile-time gate value (reflects NEXUS_V1_PHASE_DOMAIN_LOCK). */
int nexus_v1_phase_gate_compile_lock(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V1_COMPAT_GATE_H */
