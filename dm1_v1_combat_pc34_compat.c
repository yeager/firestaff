/*
 * dm1_v1_combat_pc34_compat.c — DM1 V1 Combat & Damage System
 *
 * Source-locked to ReDMCSB:
 *   GROUP.C:    F0190_GROUP_GetDamageCreatureOutcome
 *               F0191_GROUP_GetDamageAllCreaturesOutcome
 *               F0192_GROUP_GetResistanceAdjustedPoisonAttack
 *               F0207_GROUP_IsCreatureAttacking (melee path)
 *   CHAMPION.C: F0306_CHAMPION_GetStaminaAdjustedValue
 *               F0307_CHAMPION_GetStatisticAdjustedAttack
 *               F0311_CHAMPION_GetDexterity
 *               F0312_CHAMPION_GetStrength
 *               F0313_CHAMPION_GetWoundDefense
 *               F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage
 *               F0324_CHAMPION_DamageAll_GetDamagedChampionCount
 *   DEFS.H:     Attack types, wound masks, defense formula
 */
#include "dm1_v1_combat_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

/* ── RNG (simple LCG, seedable for deterministic tests) ───────────── */
static uint32_t g_rng_state = 12345;

void dm1_combat_seed_rng(uint32_t seed) {
    g_rng_state = seed ? seed : 1;
}

int dm1_combat_random(int modulus) {
    if (modulus <= 0) return 0;
    g_rng_state = g_rng_state * 1103515245u + 12345u;
    return (int)((g_rng_state >> 16) & 0x7FFF) % modulus;
}

/* ── Helpers ──────────────────────────────────────────────────────── */
static int dm1_clamp(int val, int lo, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

static int dm1_abs(int v) { return v < 0 ? -v : v; }
static int dm1_max(int a, int b) { return a > b ? a : b; }
static int dm1_min(int a, int b) { return a < b ? a : b; }

/*
 * F0030_MAIN_GetScaledProduct
 * ReDMCSB: return (val * factor) >> shift
 * Used extensively in damage calculations.
 */
int dm1_scaled_product(int val, int shift, int factor) {
    return ((long)val * (long)factor) >> shift;
}

/* ── Init ─────────────────────────────────────────────────────────── */
void dm1_combat_init(DM1_CombatState* s) {
    if (!s) return;
    memset(s, 0, sizeof(DM1_CombatState));
}

void dm1_combat_init_champion(DM1_ChampionCombat* ch) {
    if (!ch) return;
    memset(ch, 0, sizeof(DM1_ChampionCombat));
    ch->alive = 1;
    ch->maxHealth = 100;
    ch->currentHealth = 100;
    ch->maxStamina = 100;
    ch->currentStamina = 100;
    ch->maxLoad = 200;
}

void dm1_combat_init_group(DM1_CreatureGroup* g) {
    if (!g) return;
    memset(g, 0, sizeof(DM1_CreatureGroup));
}

/*
 * F0143_DUNGEON_GetArmourDefense
 * ReDMCSB: Returns armor defense. If useSharpDefense, adds the sharp
 * defense bits (MASK0x0007_SHARP_DEFENSE).
 */
int dm1_armor_defense(const DM1_ArmorPiece* armor, int useSharpDefense) {
    if (!armor) return 0;
    int def = armor->defense;
    if (useSharpDefense) {
        def += armor->sharpDefense;
    }
    return def;
}

/*
 * F0306_CHAMPION_GetStaminaAdjustedValue
 * ReDMCSB CHAMPION.C: If stamina < half max, value is reduced
 * proportionally. val = (val/2) + (val/2 * stamina / halfMax)
 */
int dm1_stamina_adjusted(const DM1_ChampionCombat* ch, int value) {
    if (!ch) return value;
    int halfMax = ch->maxStamina >> 1;
    if (halfMax <= 0) return value;
    if (ch->currentStamina < halfMax) {
        value >>= 1;
        return value + (int)(((long)value * (long)ch->currentStamina) / halfMax);
    }
    return value;
}

/*
 * F0307_CHAMPION_GetStatisticAdjustedAttack
 * ReDMCSB CHAMPION.C: factor = 170 - statCurrent. If factor < 16,
 * return attack >> 3. Else return scaled_product(attack, 7, factor).
 */
int dm1_stat_adjusted_attack(const DM1_ChampionCombat* ch, int statValue, int attack) {
    (void)ch;
    int factor = 170 - statValue;
    if (factor < 16) {
        return attack >> 3;
    }
    return dm1_scaled_product(attack, 7, factor);
}

/*
 * F0311_CHAMPION_GetDexterity
 * ReDMCSB CHAMPION.C: Base dex + random(8), penalized by load,
 * bounded [1+random(8), 100-random(8)].
 */
int dm1_champion_dexterity(const DM1_ChampionCombat* ch) {
    if (!ch) return 1;
    int dex = dm1_combat_random(8) + ch->dexterity;
    int maxL = ch->maxLoad > 0 ? ch->maxLoad : 1;
    dex -= (int)(((long)(dex >> 1) * (long)ch->load) / maxL);
    return dm1_clamp(dex >> 1, 1 + dm1_combat_random(8), 100 - dm1_combat_random(8));
}

/*
 * F0312_CHAMPION_GetStrength
 * ReDMCSB CHAMPION.C: Strength from stats + weapon + skill + stamina adj,
 * wound penalty. Bounded [0, 100].
 */
int dm1_champion_strength(const DM1_ChampionCombat* ch) {
    if (!ch) return 0;
    int str = dm1_combat_random(16) + ch->strength;

    /* Weapon weight vs load factor */
    if (ch->hasWeapon) {
        str += ch->actionHandWeapon.strength;

        /* Skill bonus */
        int skillLevel = 0;
        int cls = ch->actionHandWeapon.weaponClass;
        if (cls == 0 || cls == 2) { /* SWING or DAGGER_AND_AXES */
            skillLevel = ch->skillSwing;
        }
        if (cls != 0 && cls < 16) { /* THROW weapons (class 1-15 excl swing) */
            skillLevel += ch->skillThrow;
        }
        if (cls >= 16 && cls < 112) { /* SHOOT weapons */
            skillLevel += ch->skillShoot;
        }
        str += skillLevel << 1;
    }

    /* Stamina adjustment */
    str = dm1_stamina_adjusted(ch, str);

    /* Wound penalty: action hand wound halves strength */
    if (ch->wounds & DM1_WOUND_ACTION_HAND) {
        str >>= 1;
    }

    return dm1_clamp(str >> 1, 0, 100);
}

/*
 * G0050_auc_Graphic562_WoundDefenseFactor — per-wound defense weight.
 * ReDMCSB uses these factors in F0313.
 */
static const int g_woundDefenseFactor[DM1_WOUND_IDX_COUNT] = {
    5, 5, 4, 6, 3, 2
};

/*
 * F0313_CHAMPION_GetWoundDefense
 * ReDMCSB CHAMPION.C: Compute defense for a wound location.
 *  - Shields in hand slots contribute weighted defense
 *  - Vitality contributes random(vitality/8 + 1)
 *  - Sharp halves the vitality contribution
 *  - Action defense + champion shield + party shield + armor shield
 *  - Body slot armor defense added
 *  - Existing wound on that slot reduces defense
 *  - Bounded [0, 100]
 */
int dm1_wound_defense(const DM1_CombatState* s, int champIdx,
                      int woundIdx, int useSharpDefense) {
    if (!s || champIdx < 0 || champIdx >= s->championCount) return 0;
    if (woundIdx < 0 || woundIdx >= DM1_WOUND_IDX_COUNT) return 0;

    const DM1_ChampionCombat* ch = &s->champions[champIdx];
    int shieldDef = 0;

    /* Shields in hand slots (slots 0 and 1) */
    for (int slot = 0; slot <= 1; slot++) {
        if (ch->hasArmor[slot] && ch->armor[slot].isShield) {
            int armorDef = dm1_armor_defense(&ch->armor[slot], useSharpDefense);
            /* Shield defense weighted by wound location factor */
            int weighted = (armorDef * g_woundDefenseFactor[woundIdx]) >> ((slot == woundIdx) ? 4 : 5);
            shieldDef += weighted;
        }
    }

    /* Vitality contribution */
    int vitalDiv = (ch->vitality >> 3) + 1;
    int woundDef = dm1_combat_random(vitalDiv > 0 ? vitalDiv : 1);
    if (useSharpDefense) {
        woundDef >>= 1;
    }

    woundDef += ch->actionDefense + ch->shieldDefense +
                s->partyShieldDefense + shieldDef;

    /* Body slot armor (slots 2-5: head, torso, legs, feet) */
    if (woundIdx > 1 && ch->hasArmor[woundIdx]) {
        woundDef += dm1_armor_defense(&ch->armor[woundIdx], useSharpDefense);
    }

    /* Existing wound reduces defense */
    if (ch->wounds & (1 << woundIdx)) {
        woundDef -= 8 + dm1_combat_random(4);
    }

    return dm1_clamp(woundDef >> 1, 0, 100);
}

/*
 * F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage
 * ReDMCSB CHAMPION.C: Main damage pipeline for champions.
 *
 * For NORMAL attacks: no wound/defense processing, just raw damage.
 * For typed attacks:
 *  1. Compute average wound defense across allowed wounds
 *  2. Apply type-specific reductions (fire→antifire-fireShield,
 *     magic→antimagic-spellShield, psychic→wisdom, self→halve defense)
 *  3. Scaled product: attack * (130 - defense) / 128
 *  4. Wound probability: if attack > stat-adjusted vitality random,
 *     apply wounds
 */
int dm1_champion_take_damage(DM1_CombatState* s, int champIdx, int attack,
                             uint16_t allowedWounds, int attackType) {
    if (!s || champIdx < 0 || champIdx >= s->championCount) return 0;
    DM1_ChampionCombat* ch = &s->champions[champIdx];
    if (!ch->alive || !ch->currentHealth) return 0;
    if (attack <= 0) return 0;

    if (attackType != DM1_ATTACK_NORMAL) {
        /* Compute average defense across allowed wound slots */
        int woundCount = 0;
        int defense = 0;
        int useSharp = (attackType == DM1_ATTACK_SHARP) ? 1 : 0;

        for (int w = 0; w < DM1_WOUND_IDX_COUNT; w++) {
            if (allowedWounds & (1 << w)) {
                woundCount++;
                defense += dm1_wound_defense(s, champIdx, w, useSharp);
            }
        }
        if (woundCount > 0) {
            defense /= woundCount;
        }

        /* Type-specific attack modifications */
        switch (attackType) {
        case DM1_ATTACK_PSYCHIC: {
            int wisdomFactor = 115 - ch->wisdom;
            if (wisdomFactor <= 0) {
                attack = 0;
            } else {
                attack = dm1_scaled_product(attack, 6, wisdomFactor);
            }
            break;
        }
        case DM1_ATTACK_MAGIC:
            attack = dm1_stat_adjusted_attack(ch, ch->antimagic, attack);
            attack -= s->partySpellShieldDefense;
            break;
        case DM1_ATTACK_FIRE:
            attack = dm1_stat_adjusted_attack(ch, ch->antifire, attack);
            attack -= s->partyFireShieldDefense;
            if (attack <= 0) return 0;
            break;
        case DM1_ATTACK_SELF:
            defense >>= 1;
            break;
        case DM1_ATTACK_BLUNT:
        case DM1_ATTACK_SHARP:
        case DM1_ATTACK_LIGHTNING:
            break;
        default:
            break;
        }

        if (attack <= 0) return 0;

        /* Scale attack by defense: attack * (130 - defense) / 128 */
        attack = dm1_scaled_product(attack, 6, 130 - defense);
        if (attack <= 0) return 0;

        /* Wound probability check (F0307 with vitality) */
        int adjAttack = dm1_stat_adjusted_attack(ch, ch->vitality,
                                                  dm1_combat_random(128) + 10);
        if (attack > adjAttack) {
            /* Apply wounds */
            do {
                s->pendingWounds[champIdx] |= (uint16_t)((1 << dm1_combat_random(8)) & allowedWounds);
            } while ((attack > (adjAttack <<= 1)) && adjAttack);
        }
    }

    /* Accumulate pending damage */
    s->pendingDamage[champIdx] += attack;
    return attack;
}

/*
 * F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds
 * ReDMCSB CHAMPION.C: Apply accumulated damage and wounds.
 */
void dm1_apply_pending_damage(DM1_CombatState* s) {
    if (!s) return;

    for (int i = 0; i < s->championCount; i++) {
        DM1_ChampionCombat* ch = &s->champions[i];

        /* Apply wounds */
        ch->wounds |= s->pendingWounds[i];
        s->pendingWounds[i] = 0;

        int dmg = s->pendingDamage[i];
        s->pendingDamage[i] = 0;
        if (dmg <= 0) continue;
        if (!ch->currentHealth) continue;

        int hp = ch->currentHealth - dmg;
        if (hp <= 0) {
            ch->currentHealth = 0;
            ch->alive = 0;
        } else {
            ch->currentHealth = hp;
        }
    }
}

/*
 * F0190_GROUP_GetDamageCreatureOutcome
 * ReDMCSB GROUP.C: Damage a single creature in a group.
 * If creature health <= damage, the creature dies.
 * If it was the only creature, whole group dies → KILLED_ALL.
 * If multiple, remove creature → KILLED_SOME.
 * Archenemy (defense 255) cannot be damaged.
 */
int dm1_creature_take_damage(DM1_CreatureGroup* group, int creatureIdx,
                             int damage) {
    if (!group) return DM1_OUTCOME_KILLED_NONE;
    if (creatureIdx < 0 || creatureIdx > group->count) return DM1_OUTCOME_KILLED_NONE;

    /* Archenemy immune */
    if (group->info.defense == 255) {
        return DM1_OUTCOME_KILLED_NONE;
    }

    if (group->creatures[creatureIdx].health <= damage) {
        /* Creature killed */
        if (group->count == 0) {
            /* Last creature in group */
            group->creatures[0].health = 0;
            return DM1_OUTCOME_KILLED_ALL;
        } else {
            /* Shift remaining creatures down */
            for (int i = creatureIdx; i < group->count; i++) {
                group->creatures[i] = group->creatures[i + 1];
            }
            group->count--;
            return DM1_OUTCOME_KILLED_SOME;
        }
    }

    if (damage > 0) {
        group->creatures[creatureIdx].health -= damage;
    }
    return DM1_OUTCOME_KILLED_NONE;
}

/*
 * F0191_GROUP_GetDamageAllCreaturesOutcome
 * ReDMCSB GROUP.C: Damage all creatures in group with attack ± attack/8.
 */
int dm1_damage_all_creatures(DM1_CreatureGroup* group, int attack) {
    if (!group || attack <= 0) return DM1_OUTCOME_KILLED_NONE;

    int randomAttack = (attack >> 3) + 1;
    int baseAttack = attack - randomAttack;
    randomAttack <<= 1;

    int killedSome = 0;
    int killedAll = 1;

    /* Iterate backwards (higher indices first) since removal shifts */
    int creatureIdx = group->count;
    do {
        int actualAttack = dm1_max(1, baseAttack + dm1_combat_random(randomAttack > 0 ? randomAttack : 1));
        int outcome = dm1_creature_take_damage(group, creatureIdx, actualAttack);
        if (outcome) {
            killedSome = 1;
        }
        if (!outcome) {
            killedAll = 0;
        }
    } while (creatureIdx--);

    if (killedAll && killedSome) return DM1_OUTCOME_KILLED_ALL;
    if (killedSome) return DM1_OUTCOME_KILLED_SOME;
    return DM1_OUTCOME_KILLED_NONE;
}

/*
 * F0192_GROUP_GetResistanceAdjustedPoisonAttack
 * ReDMCSB GROUP.C: Adjusts poison attack by creature's poison resistance.
 * If immune (resistance == 15), return 0.
 * Otherwise: ((poisonAttack + random(4)) << 3) / (resistance + 1)
 */
int dm1_poison_adjusted_attack(int poisonResistance, int poisonAttack) {
    if (!poisonAttack || poisonResistance == 15) return 0;
    return ((poisonAttack + dm1_combat_random(4)) << 3) / (poisonResistance + 1);
}

/*
 * F0230_GROUP_GetChampionDamage (referenced from F0207)
 * ReDMCSB GROUP.C: Calculate damage from creature melee attack to champion.
 *
 * Creature attack value is computed, then champion takes damage with
 * wound probabilities from the creature info.
 */
int dm1_creature_attack_champion(DM1_CombatState* s, const DM1_CreatureGroup* group,
                                 int creatureIdx, int targetChampIdx) {
    if (!s || !group) return 0;
    if (targetChampIdx < 0 || targetChampIdx >= s->championCount) return 0;

    const DM1_CreatureInfo* ci = &group->info;

    /* Base attack: creature attack + random(attack >> 2) + random(4) */
    int atk = ci->attack;
    int randAtk = (atk >> 2) + 1;
    atk += dm1_combat_random(randAtk > 0 ? randAtk : 1);
    atk += dm1_combat_random(4);

    /* Compute allowed wounds from creature wound probabilities */
    uint16_t allowedWounds = 0;
    if (dm1_combat_random(16) < ci->woundProbHead)  allowedWounds |= DM1_WOUND_HEAD;
    if (dm1_combat_random(16) < ci->woundProbTorso) allowedWounds |= DM1_WOUND_TORSO;
    if (dm1_combat_random(16) < ci->woundProbLegs)  allowedWounds |= DM1_WOUND_LEGS;
    if (dm1_combat_random(16) < ci->woundProbFeet)  allowedWounds |= DM1_WOUND_FEET;

    /* Hands can always be wounded */
    allowedWounds |= DM1_WOUND_READY_HAND | DM1_WOUND_ACTION_HAND;

    int damage = dm1_champion_take_damage(s, targetChampIdx, atk,
                                          allowedWounds, ci->attackType);

    /* Poison */
    if (ci->poisonAttack > 0 && damage > 0) {
        /* Simplified poison: add small normal damage */
        int poisonDmg = dm1_max(1, ci->poisonAttack >> 6);
        dm1_champion_take_damage(s, targetChampIdx, poisonDmg,
                                DM1_WOUND_NONE, DM1_ATTACK_NORMAL);
    }

    return damage;
}

/*
 * F0177_GROUP_GetMeleeTargetCreatureOrdinal
 * ReDMCSB GROUP.C: Determines which creature in a group is the melee
 * target based on champion cell position relative to group cells.
 *
 * Simplified: finds first creature in the ordered attack cells.
 */
int dm1_get_melee_target(const DM1_CreatureGroup* group, int championCell,
                         int partyDirection, int groupDirection) {
    if (!group) return -1;
    (void)groupDirection;

    /* Ordered cells to attack based on champion position and party direction.
     * ReDMCSB F0229_GROUP_SetOrderedCellsToAttack creates a priority order.
     * Simplified: check the cell facing the champion first, then adjacent. */
    int cells[4];
    cells[0] = (championCell + 2) & 3;  /* Opposite cell (directly facing) */
    cells[1] = (championCell + 3) & 3;  /* Adjacent clockwise */
    cells[2] = (championCell + 1) & 3;  /* Adjacent counter-clockwise */
    cells[3] = championCell;             /* Same cell */
    (void)partyDirection;

    for (int c = 0; c < 4; c++) {
        for (int i = group->count; i >= 0; i--) {
            if (group->creatures[i].cell == cells[c]) {
                return i;
            }
        }
    }
    return -1;
}

/*
 * dm1_melee_action_damage — Champion melee attack against creature
 *
 * Source: MENUS.C F0402_MENUS_GetActionDamage calls F0312 for strength,
 * then F0190 for creature damage. Attack value is
 * strength * 2 + random(16) with creature defense subtraction.
 */
int dm1_melee_action_damage(DM1_CombatState* s, int champIdx,
                            DM1_CreatureGroup* group, int creatureIdx) {
    if (!s || !group) return 0;
    if (champIdx < 0 || champIdx >= s->championCount) return 0;
    if (creatureIdx < 0 || creatureIdx > group->count) return 0;

    DM1_ChampionCombat* ch = &s->champions[champIdx];
    if (!ch->alive) return 0;

    /* Champion attack strength */
    int str = dm1_champion_strength(ch);
    int attack = (str << 1) + dm1_combat_random(16);

    /* Subtract creature defense */
    int creatureDef = group->info.defense;
    if (creatureDef == 255) return 0; /* Immune */

    /* Dexterity hit check: random(dex) must beat random(creature dex) */
    int champDex = dm1_champion_dexterity(ch);
    int creaDex = group->info.dexterity;
    if (dm1_combat_random(dm1_max(1, champDex)) < dm1_combat_random(dm1_max(1, creaDex >> 1))) {
        return 0; /* Miss */
    }

    /* Apply creature defense */
    attack = dm1_max(0, attack - dm1_combat_random(creatureDef + 1));

    /* Non-material creatures can only be hit by specific means */
    if (group->info.nonMaterial && !ch->hasWeapon) {
        return 0;
    }

    /* Apply damage */
    return dm1_creature_take_damage(group, creatureIdx, attack);
}

/*
 * F0324_CHAMPION_DamageAll_GetDamagedChampionCount
 * ReDMCSB CHAMPION.C: Damage all champions with attack ± attack/8.
 */
int dm1_damage_all_champions(DM1_CombatState* s, int attack,
                             uint16_t wounds, int attackType) {
    if (!s || !attack) return 0;

    int randomAttack = (attack >> 3) + 1;
    int baseAttack = attack - randomAttack;
    randomAttack <<= 1;

    int damagedCount = 0;
    for (int i = 0; i < s->championCount; i++) {
        int actualAttack = dm1_max(1, baseAttack + dm1_combat_random(randomAttack > 0 ? randomAttack : 1));
        if (dm1_champion_take_damage(s, i, actualAttack, wounds, attackType)) {
            damagedCount++;
        }
    }
    return damagedCount;
}
