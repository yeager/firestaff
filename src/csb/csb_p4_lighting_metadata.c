#include "csb_p4_lighting_metadata.h"

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
    /* Torch type → VFX light intensity 0-255 */
    switch (torch_type) {
        case CSB_P4_TORCH_TYPE_NONE:    return 0;
        case CSB_P4_TORCH_TYPE_NORMAL:  return 140;
        case CSB_P4_TORCH_TYPE_BRIGHT:  return 200;
        case CSB_P4_TORCH_TYPE_MAGICAL: return 255;
        default:                        return 0;
    }
}

/* ---- Chaos magic / spell projectile metadata ---- */

/* Spell projectile metadata table.
 * Maps DSA spell ids to VFX type, colour, and behaviour.
 * Source: CSBWin/Graphics.cpp (fireball, lightning, arcane bolt),
 * CSBWin/Chaos.cpp (chaos visual triggers), and ReDMCSB ANIM.C:20
 * (G3567_as_AnimationItems[200] animation indices).
 *
 * spell_id values correspond to DSA script identifiers in CSB.
 * Unknown spell ids fall through to the generic arcane entry.
 */

static const CSB_P4_SpellProjectileMetadata s_spell_metadata[] = {
    /* spell_id,   category,       VFX type,         R    G    B  rad spd trail field */
    { 0,  CSB_P4_SPELL_CAT_NONE,      CSB_V2_VFX_NONE,        0,   0,   0,  0,  0, 0, 0, CSB_V2_VFX_NONE },
    /* Fire spells */
    { 1,  CSB_P4_SPELL_CAT_FIRE,      CSB_V2_VFX_FIRE,      255, 180,   0,  6,  4, 1, 1, CSB_V2_VFX_EXPLOSION },
    { 2,  CSB_P4_SPELL_CAT_FIRE,      CSB_V2_VFX_FIRE,      255, 120,   0,  7,  5, 1, 1, CSB_V2_VFX_EXPLOSION },
    { 3,  CSB_P4_SPELL_CAT_FIRE,      CSB_V2_VFX_EXPLOSION, 255, 200,  50,  8,  6, 1, 1, CSB_V2_VFX_FIRE },
    /* Ice / frost spells */
    { 4,  CSB_P4_SPELL_CAT_ICE,       CSB_V2_VFX_SPARK,     200, 240, 255,  5,  4, 1, 0, CSB_V2_VFX_SMOKE },
    { 5,  CSB_P4_SPELL_CAT_ICE,       CSB_V2_VFX_SPARK,     150, 220, 255,  6,  5, 1, 1, CSB_V2_VFX_SMOKE },
    /* Lightning spells */
    { 6,  CSB_P4_SPELL_CAT_LIGHTNING, CSB_V2_VFX_LIGHTNING, 255, 255, 200,  5,  8, 1, 1, CSB_V2_VFX_SPARK },
    { 7,  CSB_P4_SPELL_CAT_LIGHTNING, CSB_V2_VFX_LIGHTNING, 200, 255, 255,  6, 10, 1, 1, CSB_V2_VFX_LIGHTNING },
    /* Magical / arcane spells */
    { 8,  CSB_P4_SPELL_CAT_MAGICAL,   CSB_V2_VFX_MAGICAL_GLOW, 180, 100, 255,  4,  3, 1, 0, CSB_V2_VFX_NONE },
    { 9,  CSB_P4_SPELL_CAT_MAGICAL,   CSB_V2_VFX_MAGICAL_GLOW, 200, 150, 255,  5,  4, 1, 0, CSB_V2_VFX_NONE },
    /* Darkness / shadow spells */
    { 10, CSB_P4_SPELL_CAT_DARKNESS,  CSB_V2_VFX_SMOKE,      50,  50,  80,  5,  3, 0, 1, CSB_V2_VFX_CHAOS_MIST },
    /* Chaos magic spells */
    { 11, CSB_P4_SPELL_CAT_CHAOS,     CSB_V2_VFX_CHAOS_MIST, 200,  50, 255,  6,  4, 1, 1, CSB_V2_VFX_CHAOS_MIST },
    { 12, CSB_P4_SPELL_CAT_CHAOS,     CSB_V2_VFX_CHAOS_MIST, 150, 100, 255,  7,  5, 1, 1, CSB_V2_VFX_FIRE },
};

/* Binary search would be ideal; linear scan is fine for small N */
const CSB_P4_SpellProjectileMetadata *
csb_p4_get_spell_projectile_metadata(int spell_id) {
    int i;
    for (i = 0; i < (int)(sizeof(s_spell_metadata) /
                          sizeof(s_spell_metadata[0])); i++) {
        if (s_spell_metadata[i].spell_id == spell_id) {
            return &s_spell_metadata[i];
        }
    }
    /* Unknown spell id — return generic arcane */
    static const CSB_P4_SpellProjectileMetadata s_fallback = {
        0, CSB_P4_SPELL_CAT_ARCANE,
        CSB_V2_VFX_MAGICAL_GLOW,
        150, 100, 255, 4, 3, 1, 0, CSB_V2_VFX_NONE
    };
    (void)spell_id; /* suppress unused warning */
    return &s_fallback;
}

int csb_p4_spell_category_to_vfx_type(CSB_P4_SpellCategory cat) {
    switch (cat) {
        case CSB_P4_SPELL_CAT_FIRE:      return CSB_V2_VFX_FIRE;
        case CSB_P4_SPELL_CAT_ICE:       return CSB_V2_VFX_SPARK;
        case CSB_P4_SPELL_CAT_LIGHTNING: return CSB_V2_VFX_LIGHTNING;
        case CSB_P4_SPELL_CAT_MAGICAL:   return CSB_V2_VFX_MAGICAL_GLOW;
        case CSB_P4_SPELL_CAT_DARKNESS:  return CSB_V2_VFX_SMOKE;
        case CSB_P4_SPELL_CAT_CHAOS:     return CSB_V2_VFX_CHAOS_MIST;
        case CSB_P4_SPELL_CAT_HEALING:   return CSB_V2_VFX_MAGICAL_GLOW;
        default:                         return CSB_V2_VFX_NONE;
    }
}

int csb_p4_spell_category_has_light(CSB_P4_SpellCategory cat) {
    return cat != CSB_P4_SPELL_CAT_NONE &&
           cat != CSB_P4_SPELL_CAT_HEALING;
}

/* ---- VFX binding gates ---- */

/* Phase gate: field VFX enabled when V2 presentation is active. */
int csb_p4_vfx_gate_field_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    if (!cfg || !cfg->v2PresentationEnabled) {
        return 0;
    }
    return 1;
}

/* Phase gate: projectile VFX enabled when V2 presentation is active. */
int csb_p4_vfx_gate_projectile_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    if (!cfg || !cfg->v2PresentationEnabled) {
        return 0;
    }
    return 1;
}

/* Phase gate: chaos magic visual enhancement enabled when V2
 * presentation is active. V1 DSA script execution is unaffected. */
int csb_p4_vfx_gate_chaos_enabled(const CSB_V2_PhaseGateConfig *cfg) {
    if (!cfg || !cfg->v2PresentationEnabled) {
        return 0;
    }
    return 1;
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
