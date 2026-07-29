#ifndef FIRESTAFF_DM2_V1_HERO_STATS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_HERO_STATS_PC34_COMPAT_H

/*
 * DM2 hero stat computation functions.
 * Source: skproject c_hero.cpp:80-147 (get_adj_ability1/2, get_stamina_adj,
 *         get_max_load, use_luck).
 *
 * These are the c_hero member functions ported as standalone pure-computation
 * functions that take hero state as input and return receipts.
 */

#include <stdint.h>

#define DM2_V1_HERO_NUM_ABILITIES 7

typedef enum {
    DM2_V1_HERO_ABILITY_FIGHTER    = 0,
    DM2_V1_HERO_ABILITY_NINJA      = 1,
    DM2_V1_HERO_ABILITY_PRIEST     = 2,
    DM2_V1_HERO_ABILITY_WIZARD     = 3,
    DM2_V1_HERO_ABILITY_STRENGTH   = 4,
    DM2_V1_HERO_ABILITY_DEXTERITY  = 5,
    DM2_V1_HERO_ABILITY_LUCK       = 6
} DM2_V1_HeroAbility;

typedef struct {
    uint8_t current;
    uint8_t maximum;
} DM2_V1_HeroAbilityPair;

typedef struct {
    DM2_V1_HeroAbilityPair ability[DM2_V1_HERO_NUM_ABILITIES];
    int8_t eability[DM2_V1_HERO_NUM_ABILITIES];
    uint8_t ench_aura;
    uint8_t ench_power;
    uint16_t cur_stamina;
    uint16_t max_stamina;
    uint8_t body_flag;
} DM2_V1_HeroStats;

typedef struct {
    int valid;
    int16_t value;
} DM2_V1_HeroGetAdjAbility1Receipt;

typedef struct {
    int valid;
    int32_t value;
} DM2_V1_HeroGetAdjAbility2Receipt;

typedef struct {
    int valid;
    int16_t value;
} DM2_V1_HeroGetStaminaAdjReceipt;

typedef struct {
    int valid;
    int16_t value;
} DM2_V1_HeroGetMaxLoadReceipt;

typedef struct {
    int valid;
    int result;
    uint8_t luck_delta;
    uint8_t new_luck_current;
    int consumed_rand16_ability;
    int consumed_rand16_luck;
    int consumed_randbit;
} DM2_V1_HeroUseLuckReceipt;

int dm2_v1_hero_get_adj_ability1(
    const DM2_V1_HeroStats *hero,
    DM2_V1_HeroAbility ability,
    int use_current,
    int16_t rand16_value,
    DM2_V1_HeroGetAdjAbility1Receipt *out);

int dm2_v1_hero_get_adj_ability2(
    const DM2_V1_HeroStats *hero,
    DM2_V1_HeroAbility ability,
    int16_t input,
    int16_t rand16_value,
    DM2_V1_HeroGetAdjAbility2Receipt *out);

int dm2_v1_hero_get_stamina_adj(
    const DM2_V1_HeroStats *hero,
    int16_t input,
    DM2_V1_HeroGetStaminaAdjReceipt *out);

int dm2_v1_hero_get_max_load(
    const DM2_V1_HeroStats *hero,
    int16_t rand16_value,
    DM2_V1_HeroGetMaxLoadReceipt *out);

#endif
