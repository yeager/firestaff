#ifndef FIRESTAFF_DM2_V1_COMBAT_DAMAGE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_COMBAT_DAMAGE_PC34_COMPAT_H

/*
 * dm2_v1_combat_damage_pc34_compat.h — DM2 combat damage pipeline.
 *
 * Ports DM2_CALC_PLAYER_ATTACK_DAMAGE, DM2_WOUND_PLAYER, and
 * DM2_ATTACK_PARTY from skproject/SKULLWIN/c_hero.cpp.
 *
 * CALC_PLAYER_ATTACK_DAMAGE: hit/miss check (dexterity vs creature
 * defense + random), then strength-based damage roll, armor reduction,
 * poison application, and skill experience award.
 *
 * WOUND_PLAYER: applies damage to a hero, distributing across armor
 * slots based on damage type, reducing HP, potentially killing.
 *
 * ATTACK_PARTY: creature attacks all party members with randomized
 * damage per hero, calling WOUND_PLAYER for each.
 *
 * Source: c_hero.cpp:232-430, 1496-1740, 3346-3394
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Damage types from skengage.cpp CMDSTR entry 0x10 */
typedef enum {
    DM2_DMG_PHYSICAL   = 0,
    DM2_DMG_FIRE       = 1,
    DM2_DMG_LIGHTNING  = 2,
    DM2_DMG_MAGIC      = 3,
    DM2_DMG_SHARP      = 4,
    DM2_DMG_POISON     = 5
} DM2_V1_DamageType;

/* CALC_PLAYER_ATTACK_DAMAGE request */
typedef struct {
    int hero_index;
    int16_t hero_hp;
    int16_t hero_dexterity;
    int16_t hero_strength;
    int16_t hero_luck;
    int16_t hero_skill_level;
    int16_t hand;
    int16_t item_handle;
    int16_t creature_record;
    int16_t creature_defense;
    int16_t creature_armor;
    int16_t creature_poison_resist;
    int16_t target_x;
    int16_t target_y;
    int16_t damage_type;
    int16_t skill_type;
    int16_t power_base;
    int16_t power_random;
} DM2_V1_CalcAttackDamageRequest;

/* CALC_PLAYER_ATTACK_DAMAGE receipt */
typedef struct {
    int valid;
    int fail_closed;
    int hit;
    int miss;
    int16_t raw_damage;
    int16_t final_damage;
    int16_t poison_applied;
    int16_t skill_exp_awarded;
} DM2_V1_CalcAttackDamageReceipt;

/* WOUND_PLAYER request */
typedef struct {
    int16_t hero_index;
    int16_t wound_amount;
    int16_t damage_type;
    uint16_t armor_mask;
    int16_t hero_hp;
    int16_t hero_max_hp;
    int16_t hero_armor_slots[6];
    int16_t hero_vitality_skill;
} DM2_V1_WoundPlayerRequest;

/* WOUND_PLAYER receipt */
typedef struct {
    int valid;
    int fail_closed;
    int hero_wounded;
    int hero_killed;
    int16_t damage_absorbed;
    int16_t damage_dealt;
    int16_t hp_remaining;
} DM2_V1_WoundPlayerReceipt;

/* ATTACK_PARTY request */
typedef struct {
    int16_t base_damage;
    int16_t damage_type;
    int16_t damage_flags;
    int16_t heroes_in_party;
} DM2_V1_AttackPartyRequest;

/* ATTACK_PARTY receipt */
typedef struct {
    int valid;
    int fail_closed;
    uint32_t heroes_hit_mask;
    int16_t heroes_wounded;
} DM2_V1_AttackPartyReceipt;

int dm2_v1_calc_player_attack_damage(
    const DM2_V1_CalcAttackDamageRequest *request,
    DM2_V1_CalcAttackDamageReceipt *receipt);

int dm2_v1_wound_player(
    const DM2_V1_WoundPlayerRequest *request,
    DM2_V1_WoundPlayerReceipt *receipt);

int dm2_v1_attack_party(
    const DM2_V1_AttackPartyRequest *request,
    DM2_V1_AttackPartyReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_COMBAT_DAMAGE_PC34_COMPAT_H */
