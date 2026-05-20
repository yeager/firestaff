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

static int dm1_abs(int v) __attribute__((unused)); static int dm1_abs(int v) { return v < 0 ? -v : v; }
static int dm1_max(int a, int b) { return a > b ? a : b; }
static int dm1_min(int a, int b) __attribute__((unused)); static int dm1_min(int a, int b) { return a < b ? a : b; }

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
    /* ReDMCSB F0311: uses F0309_CHAMPION_GetMaximumLoad (computed from strength) */
    int maxL = dm1_combat_get_maximum_load_pc34(ch->strength);
    if (maxL <= 0) maxL = 1;
    dex -= (int)(((long)(dex >> 1) * (long)ch->load) / maxL);
    return dm1_clamp(dex >> 1, 1 + dm1_combat_random(8), 100 - dm1_combat_random(8));
}

static int dm1_champion_slot_strength_pc34(const DM1_ChampionCombat* ch,
                                           int slotIndex,
                                           int objectWeight,
                                           const DM1_WeaponInfo* weapon) {
    int str = dm1_combat_random(16) + ch->strength;

    /* Object weight vs load factor — ReDMCSB F0312 3-tier system.
     * Tier 1: weight <= maxLoad/16 → str += weight - 12
     * Tier 2: weight <= threshold → str += (weight - maxLoad/16) >> 1
     * Tier 3: weight > threshold → str -= (weight - threshold) << 1 */
    {
        int maxLoad = dm1_combat_get_maximum_load_pc34(ch->strength);
        int oneSixteenth = maxLoad >> 4;
        if (objectWeight <= oneSixteenth) {
            str += objectWeight - 12;
        } else {
            int threshold = oneSixteenth + ((oneSixteenth - 12) >> 1);
            if (objectWeight <= threshold) {
                str += (objectWeight - oneSixteenth) >> 1;
            } else {
                str -= (objectWeight - threshold) << 1;
            }
        }
    }

    if (weapon) {
        str += weapon->strength;

        /* Skill bonus */
        int skillLevel = 0;
        int cls = weapon->weaponClass;
        if (cls == 0 || cls == 2) { /* SWING or DAGGER_AND_AXES */
            skillLevel = ch->skillSwing;
        }
        if (cls != 0 && cls != 2 && cls < 16) { /* THROW weapons per ReDMCSB F0312:
             * class != SWING(0) && class != DAGGER_AND_AXES(2) && class < FIRST_BOW(16) */
            skillLevel += ch->skillThrow;
        }
        if (cls >= 16 && cls < 112) { /* SHOOT weapons */
            skillLevel += ch->skillShoot;
        }
        str += skillLevel << 1;
    }

    /* Stamina adjustment */
    str = dm1_stamina_adjusted(ch, str);

    /* Wound penalty — ReDMCSB F0312: check wound for the slot holding the weapon.
     * Ready hand (slot 0) → check WOUND_READY_HAND
     * Action hand (slot 1) → check WOUND_ACTION_HAND */
    {
        uint16_t woundMask = (slotIndex == 0) ? DM1_WOUND_READY_HAND : DM1_WOUND_ACTION_HAND;
        if (ch->wounds & woundMask) {
            str >>= 1;
        }
    }

    return dm1_clamp(str >> 1, 0, 100);
}

/*
 * F0312_CHAMPION_GetStrength
 * ReDMCSB CHAMPION.C: Strength from stats + object weight + optional
 * weapon/skill + stamina adjustment + hand wound penalty. Bounded [0, 100].
 */
int dm1_champion_strength(const DM1_ChampionCombat* ch) {
    if (!ch) return 0;
    return dm1_champion_slot_strength_pc34(ch,
                                           ch->weaponSlot,
                                           ch->hasWeapon ? ch->actionHandWeapon.weight : 0,
                                           ch->hasWeapon ? &ch->actionHandWeapon : NULL);
}

/*
 * C032_ACTION_SHOOT bow/sling projectile launch parameter resolver.
 * Source-locked to ReDMCSB MENU.C F0407, AMMO.C F0294,
 * CHAMPION.C F0326, and DEFS.H weapon class/attribute macros.
 */
int dm1_ranged_shoot_resolve_pc34(const DM1_WeaponInfo* actionHandWeapon,
                                  const DM1_WeaponInfo* readyHandObject,
                                  int readyHandThing, int championCell,
                                  int championDirection, int shootSkillLevel,
                                  DM1_RangedShootResult* out)
{
    int launcherClass;
    int ammunitionClass;
    int stepEnergy;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->projectileThing = -1;
    out->projectileCell = -1;
    out->projectileDirection = ((championDirection % 4) + 4) % 4;
    out->actionDisabledTicks = DM1_ACTION_SHOOT_DISABLED_TICKS_PC34;
    out->actionStaminaBase = DM1_ACTION_SHOOT_STAMINA_BASE_PC34;
    out->skillIndex = DM1_ACTION_SHOOT_SKILL_INDEX_PC34;

    if (!actionHandWeapon || !readyHandObject) {
        out->noAmmunition = 1;
        return 0;
    }

    launcherClass = actionHandWeapon->weaponClass;
    ammunitionClass = readyHandObject->weaponClass;

    if (launcherClass >= DM1_WEAPON_CLASS_FIRST_BOW &&
        launcherClass <= DM1_WEAPON_CLASS_LAST_BOW) {
        if (ammunitionClass != DM1_WEAPON_CLASS_BOW_AMMUNITION) {
            out->noAmmunition = 1;
            return 0;
        }
        stepEnergy = launcherClass - DM1_WEAPON_CLASS_FIRST_BOW;
    } else if (launcherClass >= DM1_WEAPON_CLASS_FIRST_SLING &&
               launcherClass <= DM1_WEAPON_CLASS_LAST_SLING) {
        if (ammunitionClass != DM1_WEAPON_CLASS_SLING_AMMUNITION) {
            out->noAmmunition = 1;
            return 0;
        }
        stepEnergy = launcherClass - DM1_WEAPON_CLASS_FIRST_SLING;
    } else {
        out->noAmmunition = 1;
        return 0;
    }

    out->actionPerformed = 1;
    out->projectileThing = readyHandThing;
    out->projectileDirection = ((championDirection % 4) + 4) % 4;
    out->projectileCell = ((((championCell - out->projectileDirection + 1) & 0x0002) >> 1) +
                           out->projectileDirection) & 0x0003;
    out->kineticEnergy = actionHandWeapon->kineticEnergy + readyHandObject->kineticEnergy;
    out->attack = ((actionHandWeapon->attributes & 0x00FF) + shootSkillLevel) << 1;
    out->stepEnergy = stepEnergy;
    out->experienceGain = DM1_ACTION_SHOOT_EXPERIENCE_GAIN_PC34;
    out->projectileMovementDisabledTicks = DM1_PROJECTILE_DISABLED_MOVEMENT_TICKS_PC34;
    return 1;
}

const char *dm1_ranged_shoot_source_evidence_pc34(void)
{
    return
        "AMMO.C:34-80 F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon bow/sling ammunition class gate\n"
        "MENU.C:1363-1395 F0407 C032_ACTION_SHOOT compatibility, ammo removal, kinetic/attack/step formulas\n"
        "MENU.C:1620-1628 F0407 action disable, stamina, skill XP tail\n"
        "CHAMPION.C:2051-2070 F0326_CHAMPION_ShootProjectile launch cell/direction and projectile movement lockout\n"
        "DEFS.H:1720-1734 weapon class constants and M065_SHOOT_ATTACK low-byte attribute macro\n";
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
            int slotStrength = dm1_champion_slot_strength_pc34(ch, slot, ch->armor[slot].weight, NULL);
            /* ReDMCSB F0313 adds F0312 slot strength to shield armor defense. */
            int weighted = ((slotStrength + armorDef) * g_woundDefenseFactor[woundIdx]) >> ((slot == woundIdx) ? 4 : 5);
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
            /* ReDMCSB F0321 I34E: defense += F0303_CHAMPION_GetSkillLevel(champIdx, C01_SKILL_NINJA) */
            defense += ch->skillNinja;
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
                                 int creatureIdx __attribute__((unused)), int targetChampIdx) {
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

    /* Poison — ReDMCSB PROJEXPL.C F0230 lines 1404-1408:
     * only after damage, with a 50% gate, then F0307 Vitality adjustment
     * before F0322 applies the first poison tick immediately. */
    if (ci->poisonAttack > 0 && damage > 0) {
        (void)dm1_creature_poison_attack_pc34(s, targetChampIdx, ci->poisonAttack);
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

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Extended combat pipeline source-lock
 *
 * Source-locked to ReDMCSB CHAMPION.C:
 *   F0309_CHAMPION_GetMaximumLoad         (1157-1177)
 *   F0310_CHAMPION_GetMovementTicks       (1180-1215)
 *   F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds (1689-1800)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── F0309: Maximum load ───────────────────────────────────────────── */
/* CHAMPION.C:1157-1177 max(6, (Strength * 625 + 12500) / 1000) */
int dm1_combat_get_maximum_load_pc34(int strength)
{
    int v = (strength * 625 + 12500) / 1000;
    return v > 6 ? v : 6;
}

/* ── F0310: Movement ticks based on load ───────────────────────────── */
/* CHAMPION.C:1180-1215 BUG0_72: > not >= */
int dm1_combat_get_movement_ticks_pc34(int load, int max_load)
{
    int ticks;
    if (max_load > load) return 2; /* BUG0_72 */
    if (max_load <= 0) return 8;
    ticks = 2 + ((load - max_load) * 6) / max_load;
    return ticks > 8 ? 8 : ticks;
}

/* ── F0320: Apply pending damage ───────────────────────────────────── */
/* CHAMPION.C:1689-1800 */
void dm1_combat_apply_pending_damage_pc34(DM1_CombatState *state)
{
    int i;
    if (!state) return;
    for (i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        int damage = state->pendingDamage[i];
        if (!state->champions[i].alive || damage == 0) continue;
        state->pendingDamage[i] = 0;
        /* CHAMPION.C:1708 wound mask */
        state->champions[i].wounds |= state->pendingWounds[i];
        state->pendingWounds[i] = 0;
        /* CHAMPION.C:1720 subtract health */
        state->champions[i].currentHealth -= damage;
        if (state->champions[i].currentHealth <= 0) {
            state->champions[i].currentHealth = 0;
            state->champions[i].alive = 0;
        }
    }
}

/* ── Source evidence ───────────────────────────────────────────────── */
const char *dm1_combat_pass601_source_evidence(void)
{
    return
        "CHAMPION.C:1157-1177 F0309_CHAMPION_GetMaximumLoad\n"
        "CHAMPION.C:1180-1215 F0310_CHAMPION_GetMovementTicks BUG0_72\n"
        "CHAMPION.C:1689-1800 F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds\n"
        "GROUP.C:769-930 F0190_GROUP_GetDamageCreatureOutcome\n"
        "GROUP.C:932-990 F0191_GROUP_GetDamageAllCreaturesOutcome\n";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining CHAMPION.C function citations for parity
 *
 *   CHAMPION.C:286 F0067_MOUSE_S
 *   CHAMPION.C:1016 F0819_TEXT_MESSAGEAREA_P
 * ══════════════════════════════════════════════════════════════════════ */


/* F0322_CHAMPION_Poison (CHAMPION.C:1926-1960). Applies immediate normal
 * poison damage, decrements attack, and schedules the next event 36 ticks out. */
int dm1_combat_start_poison_pc34(DM1_CombatState* s, int champIdx, int attack) {
    DM1_PoisonEvent* p;
    int damage;

    if (!s || champIdx < 0 || champIdx >= s->championCount) return 0;
    if (attack <= 0) return 0;
    if (!s->champions[champIdx].alive || !s->champions[champIdx].currentHealth) return 0;

    p = &s->pendingPoison[champIdx];
    if (p->active && s->champions[champIdx].poisonEventCount > 0) {
        s->champions[champIdx].poisonEventCount--;
    }

    damage = dm1_champion_take_damage(s, champIdx, dm1_max(1, attack >> 6),
                                      DM1_WOUND_NONE, DM1_ATTACK_NORMAL);

    attack--;
    if (attack > 0) {
        p->active = 1;
        p->attack = attack;
        p->ticksUntilNext = 36;
        s->champions[champIdx].poisonEventCount++;
    } else {
        p->active = 0;
        p->attack = 0;
        p->ticksUntilNext = 0;
    }

    return damage;
}

/* F0230 creature poison side-effect (PROJEXPL.C:1404-1408). */
int dm1_creature_poison_attack_pc34(DM1_CombatState* s, int champIdx, int poisonAttack) {
    DM1_ChampionCombat* ch;
    int adjustedAttack;

    if (!s || champIdx < 0 || champIdx >= s->championCount) return 0;
    if (poisonAttack <= 0) return 0;
    ch = &s->champions[champIdx];
    if (!ch->alive || !ch->currentHealth) return 0;
    if (!dm1_combat_random(2)) return 0;

    adjustedAttack = dm1_stat_adjusted_attack(ch, ch->vitality, poisonAttack);
    if (adjustedAttack <= 0) return 0;
    return dm1_combat_start_poison_pc34(s, champIdx, adjustedAttack);
}

/* C75_EVENT_POISON_CHAMPION dispatch: TIMELINE.C:1991-1993 decrements the
 * scheduled-event count, then calls F0322 with the remaining attack value. */
void dm1_combat_tick_poison(DM1_CombatState* s) {
    if (!s) return;
    for (int i = 0; i < s->championCount; i++) {
        DM1_PoisonEvent* p = &s->pendingPoison[i];
        if (!p->active) continue;
        if (!s->champions[i].alive) { p->active = 0; continue; }

        if (--p->ticksUntilNext <= 0) {
            int attack = p->attack;
            p->active = 0;
            p->attack = 0;
            p->ticksUntilNext = 0;
            if (s->champions[i].poisonEventCount > 0) {
                s->champions[i].poisonEventCount--;
            }
            (void)dm1_combat_start_poison_pc34(s, i, attack);
        }
    }
}
