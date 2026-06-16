#ifndef FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H
#define FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Nexus V2 Phase 0 - V1 Compatibility Lock
 * ============================================================
 *
 * This header defines the compile-time and runtime gates that
 * isolate Nexus V1 game logic (DMDF/DGN loading, Saturn BIOS
 * boot, dungeon rendering, mechanics, save/load) from V2
 * presentation code (smooth movement, dynamic lighting, HUD
 * overlay, touch/controller affordances, upscaler, particles,
 * atmosphere).
 *
 * Phase 0 rule: V1 Nexus code compiles cleanly WITHOUT any V2
 * presentation code being active. The V2 static library
 * (firestaff_nexus_v2) MUST NOT alter V1 game-logic behaviour.
 *
 * Source-lock anchors (Saturn DM Nexus + ReDMCSB Common Toolchains):
 * - nexus_v1_iso_reader.c     Saturn ISO 9660 directory + DMDF parsing
 * - nexus_v1_dmdf_model.c     DMDF (Dungeon Master Data Format) decoder
 * - nexus_v1_dungeon.c        DGN level loader, 16 levels
 * - nexus_v1_engine.c         V1 engine singleton (data + state)
 * - nexus_v1_game.c           game state init, level load, CD track map
 * - nexus_v1_champions.c      4-champion party (DM1 + Nexus variants)
 * - nexus_v1_creatures.c      creature AI + render (MNS files)
 * - nexus_v1_movement.c       NEXUS_CMD_* (F0365/F0366 analogues)
 * - nexus_v1_combat.c         melee + spell combat
 * - nexus_v1_magic.c          spell casting, mana, runes
 * - nexus_v1_inventory.c      inventory + chest pickup
 * - nexus_v1_save_load.c      save/load round-trip (CDL format)
 * - nexus_v1_sound.c          Saturn SCSP sound driver
 * - nexus_v1_rasterizer.c     320x200 indexed framebuffer rasterizer
 * - nexus_v1_palette.c        256-color VGA palette
 * - nexus_v1_math3d.c         3D math (vec3, mat4, project)
 * - nexus_v1_launcher.c       singleton engine lifecycle
 * - nexus_v1_compat_gate.c    V1-source-locked read gate
 *
 * V2 modules (presentation only, never mutate V1 state):
 * - nexus_v2_config.c                mode + upscale + filter config
 * - nexus_v2_render_pipeline.c       V1 framebuffer -> V2 RGBA
 * - nexus_v2_upscaler.c               EPX + bilinear filter
 * - nexus_v2_smooth_movement.c       visual walk/turn/stairs interp
 * - nexus_v2_lighting.c              per-vertex torch/spell lighting
 * - nexus_v2_particles.c             particle systems
 * - nexus_v2_atmosphere.c            fog + AO + reflective floors
 * - nexus_v2_hud_overlay.c           minimap + damage numbers
 * - nexus_v2_touch_controller_affordance.c  touch swipe / edge strafe
 *
 * Saturn-specific references:
 * - NEXUS.C / NEXUS2.C           DM Nexus engine lifecycle
 * - NEXUS.BIN                    Saturn game binary
 * - ReDMCSB boot/disk loading    parallels
 * - THQUEST.ASM T400/T520/T560   Theron's Quest boot (sister game)
 * - HuC6260/HuC6270 VDC/VCE      Saturn video chip datasheet
 */

/* ============================================================
 * Phase domains - each domain is classified as V1-source-locked
 * or V2-presentation-eligible.
 *
 * V1-source-locked domains MUST NOT have their behaviour altered
 * by any V2 code. V2-presentation-eligible domains MAY receive
 * enhanced visual presentation when v2PresentationEnabled is true,
 * but V2 code MUST NOT change the underlying V1 game-logic state.
 * ============================================================ */

typedef enum {
    /* V1-source-locked gameplay domains */
    NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING = 0,
        /* nexus_v1_dmdf_model.c + nexus_v1_dungeon.c -
         * DMDF format parsing, 16-level DGN loader, square types,
         * first-thing chain walk. V2 must not reinterpret layout. */

    NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER = 1,
        /* nexus_v1_iso_reader.c - ISO 9660 directory + DMDF
         * interleaving, file seek/read via Saturn BIOS / host shim. */

    NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT = 2,
        /* nexus_v1_game.c + nexus_v1_launcher.c - state init,
         * party spawn (11,29,N), level load, CD audio track map. */

    NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY = 3,
        /* nexus_v1_champions.c - 4-champion party, stats, level-up,
         * food/water/stamina, resurrect logic. DM1-compatible
         * with Nexus-specific champion variants. */

    NEXUS_V2_PHASE_DOMAIN_CREATURE_AI = 4,
        /* nexus_v1_creatures.c + nexus_v1_combat.c - creature groups
         * (MNS files), AI behavior, melee/spell combat, drops.
         * Source-locked to original DM Nexus + ReDMCSB parallels. */

    NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC = 5,
        /* nexus_v1_magic.c - spell casting, rune UI, mana, projectile
         * spells (NUKE / FLAME / HEAL / etc). */

    NEXUS_V2_PHASE_DOMAIN_MOVEMENT = 6,
        /* nexus_v1_movement.c - NEXUS_CMD_FORWARD/BACKWARD/TURN_*
         * /STRAFE_* + F0365/F0366 analogues + collision response. */

    NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD = 7,
        /* nexus_v1_save_load.c - V1 save/load round-trip (CDL format).
         * V2 config persistence is explicitly separate and gated. */

    NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER = 8,
        /* nexus_v1_sound.c - Saturn SCSP sound driver, MNS-driven
         * SFX + CD audio track 2-9. */

    NEXUS_V2_PHASE_DOMAIN_RASTERIZER = 9,
        /* nexus_v1_rasterizer.c - 320x200 indexed framebuffer, 9-square
         * viewport. The framebuffer layout is V1 source-locked; V2
         * upscaling happens on the V2 side. */

    NEXUS_V2_PHASE_DOMAIN_INVENTORY = 10,
        /* nexus_v1_inventory.c - inventory + chest pickup, weight,
         * encumbrance, item identification. */

    /* V2-presentation-eligible domains */
    NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION = 11,
        /* nexus_v2_render_pipeline.c - 320x200 indexed -> RGBA upscale
         * + lighting + particles + atmosphere. */

    NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION = 12,
        /* nexus_v2_smooth_movement.c - visual walk/turn/stairs
         * interpolation. V1 tick (55ms) is preserved exactly;
         * V2 only interpolates the visual state. */

    NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION = 13,
        /* nexus_v2_lighting.c - per-vertex torch/spell lighting.
         * V1 tile lighting stays in the V1 rasterizer. */

    NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY = 14,
        /* nexus_v2_hud_overlay.c - minimap, damage numbers, journal,
         * achievements. */

    NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS = 15,
        /* nexus_v2_particles.c - particle systems, weather, fire. */

    NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE = 16,
        /* nexus_v2_atmosphere.c - fog, ambient occlusion, reflective
         * floors, camera bob, screen shake. */

    NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION = 17,
        /* nexus_v2_touch_controller_affordance.c - V2 touch swipe,
         * edge strafe, D-pad, dual analog stick. V1 mouse/touch/click
         * parity preserved. */

    NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION = 18,
        /* nexus_v2_config.c - mode toggle, upscale factor, filter,
         * widescreen, render width/height, smooth movement, dynamic
         * lighting, texture filtering, ambient occlusion, particles,
         * fog, reflective floors, enhanced creatures, camera bob,
         * screen shake, weather, enhanced audio, minimap, damage
         * numbers, journal, achievements, footstep audio. Persists
         * via M12 settings (separate from V1 save/load payload). */

    NEXUS_V2_PHASE_DOMAIN_UPSCALER = 19,
        /* nexus_v2_upscaler.c - EPX + bilinear upscale.
         * V1 rasterizer output is consumed read-only. */

    NEXUS_V2_PHASE_DOMAIN_COUNT
} NEXUS_V2_PhaseDomain;

/* ============================================================
 * Phase gate configuration. Phase 0 default: both disabled.
 * ============================================================ */

typedef struct {
    int v2PresentationEnabled;
    int v2ConfigPersistenceEnabled;
} NEXUS_V2_PhaseGateConfig;

/* ============================================================
 * Phase gate decision: per-domain allow/deny verdict with
 * source-lock citation.
 * ============================================================ */

typedef struct {
    int v1SourceLocked;       /* 1 = V1 must own this domain */
    int v2PresentationAllowed; /* 1 = V2 may present (if v2PresentationEnabled) */
    const char* sourceAnchor;  /* ReDMCSB / CSBWin / NEXUS.C reference */
    const char* rule;          /* human-readable rule */
} NEXUS_V2_PhaseGateDecision;

/* ============================================================
 * Public API
 * ============================================================ */

void nexus_v2_phase_gate_defaults(NEXUS_V2_PhaseGateConfig *config);

int nexus_v2_phase_gate_is_gameplay_domain(NEXUS_V2_PhaseDomain domain);

NEXUS_V2_PhaseGateDecision nexus_v2_phase_gate_decide(
    const NEXUS_V2_PhaseGateConfig *config,
    NEXUS_V2_PhaseDomain domain);

int nexus_v2_phase_gate_v2_active(const NEXUS_V2_PhaseGateConfig *config);

const char *nexus_v2_phase_gate_domain_name(NEXUS_V2_PhaseDomain domain);

const char *nexus_v2_phase_gate_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H */
