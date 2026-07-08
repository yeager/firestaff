/*
 * dm1_v1_action_xp_graphic560_pc34_compat.c
 *
 * DM1 V1 (PC 3.4 English) action→skill / action→XP routing fixture.
 * Source-locked to ReDMCSB MENU.C G0496 (skill) and G0497 (XP gain)
 * through the shared PC 3.4 EN source-lock accessors.
 *
 * See header for full provenance and citation table.
 */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "firestaff/dm1/v1/G0496_pc34_compat.h"
#include "firestaff/dm1/v1/G0497_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0494_pc34_compat.h"

/* ReDMCSB CHAMPION.C F0304 line ~874: base skill = (sub - 4) >> 2.
 * For base skills (0..3) the mapping is identity. */
static int sub_skill_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= 20) return 0;
    if (skillIndex < 4) return skillIndex;
    return (skillIndex - 4) >> 2;
}

int dm1_v1_action_xp_route(int actionIndex, DM1_ActionXpRoute* out) {
    int skillIndex;
    int experienceGain;
    if (!out) return 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    skillIndex = dm1_v1_graphic560_action_skill_index_get_pc34(actionIndex);
    experienceGain = dm1_v1_g0497_get_pc34(actionIndex);
    if (skillIndex < 0 || experienceGain < 0) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    out->valid = 1;
    out->skillIndex = skillIndex;
    out->baseSkillIndex = sub_skill_base_index(out->skillIndex);
    out->experienceGain = experienceGain;
    return 1;
}

int dm1_v1_action_is_melee_contact_f0407_pc34(int actionIndex) {
    int damageFactor;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) return 0;
    if (actionIndex == DM1_ACTION_BLOCK) return 0;
    /* ReDMCSB: MENU.C G0492 lines 202-245 plus F0407 lines 1308-1342.
     * BLOCK has a damage factor but does not enter the F0402/F0231 melee
     * contact case; PARRY does. */
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex);
    return damageFactor > 0;
}

int dm1_v1_action_is_party_shield_f0407_pc34(int actionIndex) {
    return actionIndex == DM1_ACTION_SPELLSHIELD ||
           actionIndex == DM1_ACTION_FIRESHIELD;
}

int dm1_v1_action_halves_xp_on_f0327_failure_pc34(int actionIndex) {
    /* ReDMCSB: MENU.C F0407 lines 1280-1305 routes FIREBALL, DISPELL,
     * LIGHTNING, and SPIT through F0327 and halves G0497 XP on failure.
     * INVOKE reaches the same T0407014 path from lines 1480-1493. */
    switch (actionIndex) {
        case DM1_ACTION_FIREBALL:
        case DM1_ACTION_DISPELL:
        case DM1_ACTION_LIGHTNING:
        case DM1_ACTION_INVOKE:
        case DM1_ACTION_SPIT:
            return 1;
        default:
            return 0;
    }
}

int dm1_v1_action_stamina_cost_f0407_pc34(int actionIndex,
                                          int championIndex,
                                          unsigned int gameTick) {
    int base;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) return 0;
    if (championIndex < 0) return 0;
    /* ReDMCSB: MENU.C F0407 line 1272 uses G0494[action] + RANDOM(2).
     * M11 supplies deterministic jitter through the same parity model it
     * already used before this helper was introduced. */
    base = dm1_v1_graphic560_action_stamina_get_pc34(actionIndex);
    if (base < 0) return 0;
    return base + (int)((gameTick + (unsigned int)championIndex +
                         (unsigned int)actionIndex) & 1u);
}

int dm1_v1_action_f0407_tail_pc34(int actionIndex,
                                  DM1_ActionF0407TailPc34* out) {
    int damageFactor;
    int staminaBase;
    if (!out) return 0;
    out->valid = 0;
    out->damageFactor = 0;
    out->staminaBase = 0;
    out->isMeleeContact = 0;
    out->isPartyShield = 0;
    out->halvesXpOnF0327Failure = 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        return 0;
    }
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(actionIndex);
    staminaBase = dm1_v1_graphic560_action_stamina_get_pc34(actionIndex);
    if (damageFactor < 0 || staminaBase < 0) return 0;
    out->valid = 1;
    out->damageFactor = damageFactor;
    out->staminaBase = staminaBase;
    out->isMeleeContact = dm1_v1_action_is_melee_contact_f0407_pc34(actionIndex);
    out->isPartyShield = dm1_v1_action_is_party_shield_f0407_pc34(actionIndex);
    out->halvesXpOnF0327Failure =
        dm1_v1_action_halves_xp_on_f0327_failure_pc34(actionIndex);
    return 1;
}
