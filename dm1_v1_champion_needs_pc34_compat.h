/*
 * dm1_v1_champion_needs_pc34_compat.h — DM1 V1 Champion Needs System
 *
 * Source-locked to ReDMCSB: CHAMPION.C (F0331_CHAMPION_ApplyTimeEffects_CPSF,
 * F0325_CHAMPION_DecrementStamina, F0306_CHAMPION_GetStaminaAdjustedValue),
 * GAMELOOP.C (F0002_MAIN_GameLoop_CPSDF), DEFS.H.
 *
 * Implements: food depletion per tick, water depletion per tick, stamina
 * regeneration (rest vs active), HP healing, starvation/dehydration damage.
 *
 * DM1 V1 uses signed int16_t for Food/Water with range [-1024, +2048].
 * Negative = hungry/thirsty; below -512 = starving/dehydrated (HP damage).
 */
#ifndef FIRESTAFF_DM1_V1_CHAMPION_NEEDS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_NEEDS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants from DEFS.H / CHAMPION.C ───────────────────────────── */

#define DM1_NEEDS_MAX_CHAMPIONS     4
#define DM1_NEEDS_FOOD_MIN        (-1024)
#define DM1_NEEDS_FOOD_MAX          2048
#define DM1_NEEDS_WATER_MIN       (-1024)
#define DM1_NEEDS_WATER_MAX         2048
#define DM1_NEEDS_STARVATION_THRESH (-512)
#define DM1_NEEDS_MAX_HEALTH         999
#define DM1_NEEDS_MAX_STAMINA       9999
#define DM1_NEEDS_MAX_MANA           900

/* ── Champion stat snapshot for needs processing ──────────────────── */
typedef struct {
    int16_t current_health;
    int16_t max_health;
    int16_t current_stamina;
    int16_t max_stamina;
    int16_t current_mana;
    int16_t max_mana;
    int16_t food;
    int16_t water;
    uint8_t vitality_current;
    uint8_t wisdom_current;
    uint16_t wounds;
    uint8_t poison_event_count;
    int     alive;
    int     has_ekkhard_cross;
    int     wizard_skill_level;
    int     priest_skill_level;
} DM1_ChampionNeeds;

/* ── Tick context ─────────────────────────────────────────────────── */
typedef struct {
    uint32_t game_time;
    uint32_t last_movement_time;
    int      party_is_resting;
} DM1_NeedsTickContext;

/* ── Result of a single champion needs tick ───────────────────────── */
typedef struct {
    int16_t stamina_delta;
    int16_t health_delta;
    int16_t mana_delta;
    int16_t food_after;
    int16_t water_after;
    int     starvation_damage;
} DM1_NeedsTickResult;

void dm1_needs_apply_time_effects(
    const DM1_ChampionNeeds *champ,
    const DM1_NeedsTickContext *ctx,
    DM1_NeedsTickResult *out
);

int dm1_needs_compute_stamina_amount(int max_stamina);

int dm1_needs_compute_health_gain(int max_health, int is_resting,
                                   int has_ekkhard_cross);

int dm1_needs_decrement_stamina(int16_t *current_stamina,
                                 int16_t max_stamina,
                                 int16_t decrement);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_NEEDS_PC34_COMPAT_H */
