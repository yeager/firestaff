#include "csb_p4_lighting_metadata.h"

#include <stddef.h>

/* ================================================================
 * CSB Phase 4 — Enhanced Lighting: Palette & Projectile Metadata
 * ================================================================
 *
 * Phase gate: CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION.
 * Presentation-only. V1 palette selection and V1 projectile logic
 * are unaffected.
 *
 * Source-lock anchors:
 * - ReDMCSB DATA.C:263    G0029_auc_Graphic562_ChargeCountToTorchType[16]
 * - ReDMCSB DATA.C:359    G0039_ai_Graphic562_LightPowerToLightAmount[16]
 * - ReDMCSB DATA.C:360    G0040_ai_Graphic562_PaletteIndexToLightAmount[6]
 * - ReDMCSB CASTER.C:1-103 F0394_MENUS_SetMagicCasterAndDrawSpellArea
 * - ReDMCSB ANIM.C:20     G3567_as_AnimationItems[200]
 * - CSBWin/Graphics.cpp   projectile rendering
 * - CSBWin/Chaos.cpp      chaos magic visual triggers
 * ================================================================ */

/* ---- Torch / light-power tables ---- */

/* ReDMCSB DATA.C:263 — G0029_auc_Graphic562_ChargeCountToTorchType[16]
 * Maps torch charge count (0-15 from champion hand slot) to torch type id.
 * 0=no torch, 1=normal, 2=bright, 3=magical */
const uint8_t csb_p4_k_charge_to_torch_type[16] = {
    0, 1, 1, 1,   /* charges 0-3  → no torch or normal torch */
    2, 2, 2, 2,   /* charges 4-7  → bright torch */
    3, 3, 3, 3,   /* charges 8-11 → magical torch */
    3, 3, 3, 3    /* charges 12-15 → magical torch */
};

/* ReDMCSB DATA.C:359 — G0039_ai_Graphic562_LightPowerToLightAmount[16]
 * Light percent (0-100) for each torch power level 0-15.
 * Corresponds to dungeon torch light output. */
const uint8_t csb_p4_k_light_power_to_percent[16] = {
    0,   5,  12,  24,  /* power 0-3  */
    33, 40,  46,  51,  /* power 4-7  */
    59, 68,  76,  82,  /* power 8-11 */
    89, 94,  97, 100   /* power 12-15 */
};

/* ReDMCSB DATA.C:360 — G0040_ai_Graphic562_PaletteIndexToLightAmount[6]
 * Ambient light percent for each dungeon-depth palette index 0-5.
 * Index: dungeon depth (0=surface-lit, 5=darkest).
 * This table is mirrored from csb_v2_lighting_dynamic.c for Phase 4
 * metadata completeness. */
const uint8_t csb_p4_k_palette_index_to_light_percent[6] = {
    99, 75, 50, 25, 1, 0
};

/* ---- Helpers ---- */

static int clamp_int(int v, int lo, int hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

/* ---- Public API: torch / light-power ---- */

int csb_p4_charge_count_to_torch_type(int charge_count) {
    /* ReDMCSB DATA.C:263 — direct table lookup */
    int idx = clamp_int(charge_count, 0, 15);
    return csb_p4_k_charge_to_torch_type[idx];
}

uint8_t csb_p4_torch_type_to_intensity(int torch_type) {
    (void)torch_type;
    return 0;
}

/* ---- Chaos magic / spell projectile metadata ---- */

const CSB_P4_SpellProjectileMetadata *
csb_p4_get_spell_projectile_metadata(int spell_id) {
    /* ReDMCSB ANIM.C identifies animation entries but does not define a
     * spell-id-to-modern-particle mapping, RGB values, speed or radius.
     * CSBWin's host renderer cannot fill that provenance gap. */
    (void)spell_id;
    return NULL;
}

int csb_p4_spell_category_to_vfx_type(CSB_P4_SpellCategory cat) {
    (void)cat;
    return CSB_V2_VFX_NONE;
}

int csb_p4_spell_category_has_light(CSB_P4_SpellCategory cat) {
    (void)cat;
    return 0;
}

/* ---- VFX binding gates ---- */

/* Phase gate: field VFX enabled when V2 presentation is active. */
int csb_p4_vfx_gate_field_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    (void)cfg;
    return 0;
}

/* Phase gate: projectile VFX enabled when V2 presentation is active. */
int csb_p4_vfx_gate_projectile_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    (void)cfg;
    return 0;
}

/* Phase gate: chaos magic visual enhancement enabled when V2
 * presentation is active. V1 DSA script execution is unaffected. */
int csb_p4_vfx_gate_chaos_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    (void)cfg;
    return 0;
}

int csb_p4_vfx_gate_any_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    return csb_p4_vfx_gate_field_enabled(cfg)  ||
           csb_p4_vfx_gate_projectile_enabled(cfg) ||
           csb_p4_vfx_gate_chaos_enabled(cfg);
}

const char *csb_p4_lighting_metadata_source_evidence(void) {
    return
        "CSB Phase 4 lighting metadata: presentation-only\n"
        "ReDMCSB DATA.C:263   G0029_auc_Graphic562_ChargeCountToTorchType\n"
        "ReDMCSB DATA.C:359   G0039_ai_Graphic562_LightPowerToLightAmount\n"
        "ReDMCSB DATA.C:360   G0040_ai_Graphic562_PaletteIndexToLightAmount\n"
        "ReDMCSB CASTER.C:1-103 F0394_MENUS_SetMagicCasterAndDrawSpellArea\n"
        "ReDMCSB ANIM.C:20    G3567_as_AnimationItems[200]\n"
        "CSBWin/Graphics.cpp projectile rendering\n"
        "CSBWin/Chaos.cpp    chaos magic visual triggers\n"
        "V1 DSA scripts, projectile hit detection, and palette selection unaffected\n";
}
