#ifndef FIRESTAFF_CSB_P4_LIGHTING_METADATA_H
#define FIRESTAFF_CSB_P4_LIGHTING_METADATA_H

#include <stdbool.h>
#include <stdint.h>
#include "csb_v2_phase_gate_pc34.h"
#include "csb_v2_vfx_particles.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * CSB Phase 4 — Enhanced Lighting: Palette & Projectile Metadata
 * ================================================================
 *
 * Phase gate: CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION.
 * Presentation-only: V1 palette selection (PANEL.C:418-428) and
 * V1 projectile hit detection are unaffected.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 * - DATA.C:263     G0029_auc_Graphic562_ChargeCountToTorchType[16]
 *                  — maps torch charge count (0..15) to torch type id.
 * - DATA.C:359     G0039_ai_Graphic562_LightPowerToLightAmount[16]
 *                  — maps torch light power (0..15) to light percent.
 * - DATA.C:360     G0040_ai_Graphic562_PaletteIndexToLightAmount[6]
 *                  — maps dungeon depth palette index to ambient %.
 * - CASTER.C:1-103 F0394_MENUS_SetMagicCasterAndDrawSpellArea
 *                  — spell area, magic caster champion selection.
 * - CASTER.C       chaos magic cast triggers (DSA script calls).
 * - ANIM.C:20      G3567_as_AnimationItems[200] — animation item table.
 * - CSBWin/Graphics.cpp — projectile rendering (fireball, lightning).
 * - CSBWin/Chaos.cpp    — chaos magic visual triggers.
 *
 * What this module provides:
 * 1. Canonical torch/magical-light metadata mirrors ReDMCSB DATA.C.
 * 2. Projectile spell metadata: which VFX type maps to each spell id.
 * 3. VFX binding gates for field/projectile enhanced effects.
 *
 * See also: csb_v2_lighting_dynamic.h, csb_v2_vfx_particles.h.
 * ================================================================ */

/* ---- Torch / light-power metadata ---- */

/* ReDMCSB DATA.C:263
 * G0029_auc_Graphic562_ChargeCountToTorchType[16]
 * Maps raw charge count (0-15) read from champion hand slots to a
 * logical torch type id used for rendering.
 *   0 = no torch,  1 = normal torch,  2 = bright torch,  3 = magical */
#define CSB_P4_TORCH_TYPE_NONE    0
#define CSB_P4_TORCH_TYPE_NORMAL  1
#define CSB_P4_TORCH_TYPE_BRIGHT  2
#define CSB_P4_TORCH_TYPE_MAGICAL 3

/* ReDMCSB DATA.C:359
 * G0039_ai_Graphic562_LightPowerToLightAmount[16]
 * Light percentage (0-100) for each torch power level 0-15.
 * Index = torch power, Value = percent illumination. */
extern const uint8_t  csb_p4_k_light_power_to_percent[16];

/* ReDMCSB DATA.C:360
 * G0040_ai_Graphic562_PaletteIndexToLightAmount[6]
 * Ambient light percent for each dungeon-depth palette index 0-5.
 * Index = dungeon depth palette, Value = ambient light percent. */
extern const uint8_t  csb_p4_k_palette_index_to_light_percent[6];

/* Maps a champion's raw torch charge count (0-15) to torch type id.
 * Mirrors ReDMCSB DATA.C:263 inline lookup. */
int csb_p4_charge_count_to_torch_type(int charge_count);

/* Maps torch type id to light intensity 0-255 for the VFX system.
 * Torch type 0 → 0, type 1 → 140, type 2 → 200, type 3 → 255. */
uint8_t csb_p4_torch_type_to_intensity(int torch_type);

/* ---- Chaos magic / spell projectile metadata ---- */

/* CSB chaos magic spell categories.
 * Mirrors the spell families used by CSBWin/Chaos.cpp. */
typedef enum {
    CSB_P4_SPELL_CAT_NONE = 0,
    CSB_P4_SPELL_CAT_ARCANE,      /* generic arcane bolt */
    CSB_P4_SPELL_CAT_FIRE,         /* fireball, flame, fire bolt */
    CSB_P4_SPELL_CAT_ICE,          /* frost, ice bolt */
    CSB_P4_SPELL_CAT_LIGHTNING,    /* lightning bolt, electric */
    CSB_P4_SPELL_CAT_MAGICAL,      /* magical glow, arcane shimmer */
    CSB_P4_SPELL_CAT_DARKNESS,     /* darkness, shadow */
    CSB_P4_SPELL_CAT_CHAOS,        /* chaos surge, swirl */
    CSB_P4_SPELL_CAT_HEALING,      /* restorative / healing */
    CSB_P4_SPELL_CAT_COUNT
} CSB_P4_SpellCategory;

/* Spell projectile metadata record.
 * Describes the VFX and lighting properties of one chaos magic spell. */
typedef struct {
    int spell_id;              /* DSA script / spell identifier */
    CSB_P4_SpellCategory category;
    int vfx_type;              /* CSB_V2_VFXType for the projectile */
    uint8_t light_r;           /* VFX light colour R 0-255 */
    uint8_t light_g;           /* VFX light colour G 0-255 */
    uint8_t light_b;           /* VFX light colour B 0-255 */
    uint8_t light_radius;      /* VFX light radius in tile units */
    uint8_t speed_tiles_per_sec; /* projectile speed */
    uint8_t has_trail;         /* 1 = trail particles behind projectile */
    uint8_t has_field_on_hit;  /* 1 = field VFX spawned on target tile */
    int field_vfx_type;        /* CSB_V2_VFXType for field on-hit effect */
} CSB_P4_SpellProjectileMetadata;

/* Number of known spell projectile metadata records. */
#define CSB_P4_SPELL_PROJECTILE_METADATA_COUNT 12

/* Returns the spell projectile metadata for a given spell id.
 * Returns NULL if the spell id is unknown. */
const CSB_P4_SpellProjectileMetadata *
csb_p4_get_spell_projectile_metadata(int spell_id);

/* Returns the VFX type (CSB_V2_VFXType) for a spell category.
 * Used by the VFX binding layer. */
int csb_p4_spell_category_to_vfx_type(CSB_P4_SpellCategory cat);

/* Returns 1 if the spell category produces a light effect. */
int csb_p4_spell_category_has_light(CSB_P4_SpellCategory cat);

/* ---- VFX binding layer ---- */

/* Initialise the VFX binding layer and underlying VFX particle system. */
void csb_p4_binding_init(void);

/* Reset all active bindings (does not reset the VFX particle system). */
void csb_p4_binding_reset(void);

/* Fire a spell projectile VFX from (sx,sy) to (tx,ty).
 * Looks up spell metadata and delegates to csb_v2_vfx_fire_projectile.
 * Returns binding index (>= 0) or -1 on failure / gate closed. */
int csb_p4_binding_fire_projectile(const CSB_V2_PhaseGateConfig *cfg,
                                    int spell_id,
                                    float sx, float sy,
                                    float tx, float ty);

/* Place a field VFX on dungeon tile (tx,ty) triggered by spell_id.
 * Also spawns a looping emitter for area-effect spells.
 * Returns binding index (>= 0) or -1 on failure / gate closed. */
int csb_p4_binding_add_field(const CSB_V2_PhaseGateConfig *cfg,
                              int spell_id,
                              int tile_x, int tile_y);

/* Trigger chaos magic visual enhancement (light event).
 * Mirrors csb_v2_chaos_on_trigger but gated on phase config. */
void csb_p4_binding_trigger_chaos(const CSB_V2_PhaseGateConfig *cfg,
                                   int spell_id);

/* Add a torch light source to the dynamic lighting system.
 * Returns light source index or -1. */
int csb_p4_binding_add_torch_light(const CSB_V2_PhaseGateConfig *cfg,
                                    float x, float y,
                                    int torch_type);

/* Tick all active bindings and expire done VFX entries. */
void csb_p4_binding_tick(float dt_seconds);

/* Count of active bound projectiles. */
int csb_p4_binding_active_projectile_count(void);

/* Count of active bound fields. */
int csb_p4_binding_active_field_count(void);

/* Returns 1 if any binding is active. */
int csb_p4_binding_any_active(void);

/* ---- VFX binding gates ---- */

/* Phase gate for field VFX.
 * Returns 1 if enhanced field VFX (fire, smoke, magical glow on dungeon
 * tiles) is allowed given the current phase config. */
int csb_p4_vfx_gate_field_enabled(const CSB_V2_PhaseGateConfig *cfg);

/* Phase gate for projectile VFX.
 * Returns 1 if enhanced projectile VFX (fireball, lightning bolt) is allowed. */
int csb_p4_vfx_gate_projectile_enabled(const CSB_V2_PhaseGateConfig *cfg);

/* Phase gate for chaos magic visual enhancement.
 * Returns 1 if chaos magic trigger effects (glow, pulse, chaos surge) are
 * allowed. V1 DSA script execution is unaffected. */
int csb_p4_vfx_gate_chaos_enabled(const CSB_V2_PhaseGateConfig *cfg);

/* Returns 1 if any Phase 4 VFX feature is active. */
int csb_p4_vfx_gate_any_enabled(const CSB_V2_PhaseGateConfig *cfg);

/* Source evidence string for verification scripts. */
const char *csb_p4_lighting_metadata_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_P4_LIGHTING_METADATA_H */
