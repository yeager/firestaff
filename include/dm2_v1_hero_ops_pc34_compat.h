#ifndef FIRESTAFF_DM2_V1_HERO_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_HERO_OPS_PC34_COMPAT_H

/*
 * dm2_v1_hero_ops_pc34_compat.h — DM2 V1 hero/champion operations from
 * skproject/SKULLWIN/c_hero.cpp + SKWIN/SkWinCore2.cpp.
 *
 * Callback-based implementations of:
 *   DM2_ADJUST_STAMINA              c_hero.cpp:1722
 *   DM2_CURE_POISON                 c_hero.cpp:2580 / SkWinCore2.cpp:455
 *   DM2_PROCESS_POISON              c_hero.cpp:3397
 *   DM2_ADD_COIN_TO_WALLET          c_hero.cpp:3506
 *   DM2_TAKE_COIN_FROM_WALLET       c_hero.cpp:3451
 *   DM2_PERFORM_TURN_SQUAD          c_hero.cpp:2887
 *   DM2_RESUME_FROM_WAKE            c_hero.cpp:2961
 *   DM2_SET_SPELLING_CHAMPION       c_hero.cpp:2971
 *   CURE_PLAGUE                     SkWinCore2.cpp:455
 *   PROCESS_PLAGUE                  SkWinCore2.cpp:475
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Hero state for callback-based operations ---- */
typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    int16_t cur_stamina;
    int16_t max_stamina;
    int16_t cur_mp;
    int16_t max_mp;
    int16_t cur_water;
    int16_t cur_food;
    uint16_t hero_flag;
    int16_t poison_value;
    int16_t plague_value;
    uint8_t hero_type;
    uint8_t party_pos;
    uint8_t ench_aura;
    uint8_t ench_power;
    int16_t timer_idx;
} DM2_V1_HeroState;

#define DM2_V1_HERO_FLAG_0800  0x0800
#define DM2_V1_HERO_FLAG_2000  0x2000
#define DM2_V1_HERO_FLAG_4000  0x4000
#define DM2_V1_WATER_MIN       (-1024)

/* ---- DM2_ADJUST_STAMINA (c_hero.cpp:1722) ----
 * Subtract stamina_cost from hero's current stamina.
 * If stamina drops below 0, wounds the hero for half the deficit.
 * If abs(stamina_cost) >= 10, sets HERO_FLAG_0800.
 * Returns abs(stamina_cost). -1 if hero_idx == -1. */
typedef struct {
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int damage,
                         int type, int flags);
} DM2_V1_StaminaCallbacks;

int16_t dm2_v1_adjust_stamina(
    int hero_idx, int16_t stamina_cost,
    const DM2_V1_StaminaCallbacks *cb, void *ctx);

/* ---- DM2_CURE_POISON (c_hero.cpp:2580, SkWinCore2.cpp:455) ----
 * Removes all poison timers for a hero and zeroes poison value. */
typedef struct {
    int (*get_timer_count)(void *ctx);
    uint8_t (*get_timer_type)(void *ctx, int timer_idx);
    uint8_t (*get_timer_actor)(void *ctx, int timer_idx);
    void (*delete_timer)(void *ctx, int timer_idx);
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
} DM2_V1_CurePoisonCallbacks;

int dm2_v1_cure_poison(
    int hero_idx,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx);

/* ---- CURE_PLAGUE (SkWinCore2.cpp:455) ----
 * Same structure as cure_poison but for plague timers. */
int dm2_v1_cure_plague(
    int hero_idx, uint8_t plague_timer_type,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_POISON (c_hero.cpp:3397) ----
 * Apply one poison tick: damage hero, reduce water, requeue timer.
 * Returns 1 if processed, 0 if hero is dead or invalid. */
typedef struct {
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int damage,
                         int type, int flags);
    void (*adjust_stamina)(void *ctx, int hero_idx, int16_t amount);
    void (*queue_poison_timer)(void *ctx, int hero_idx, int16_t counters,
                               uint32_t delay);
} DM2_V1_ProcessPoisonCallbacks;

int dm2_v1_process_poison(
    int hero_idx, int16_t counters,
    const DM2_V1_ProcessPoisonCallbacks *cb, void *ctx);

/* ---- DM2_ADD_COIN_TO_WALLET / DM2_TAKE_COIN_FROM_WALLET ----
 * Wallet is an array of 4 coin slots (platinum/gold/silver/copper).
 * add_coin returns 1 on success, 0 if wallet full.
 * take_coin returns 1 on success, 0 if wallet empty. */
int dm2_v1_add_coin_to_wallet(int16_t *wallet, int wallet_size,
                               int16_t coin_type);
int dm2_v1_take_coin_from_wallet(int16_t *wallet, int wallet_size,
                                  int16_t coin_type);

/* ---- DM2_PERFORM_TURN_SQUAD (c_hero.cpp:2887) ----
 * Rotate entire party's positions by delta (+1 or -1).
 * Updates each hero's party_pos. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
} DM2_V1_SquadCallbacks;

void dm2_v1_perform_turn_squad(
    int delta,
    const DM2_V1_SquadCallbacks *cb, void *ctx);

/* ---- DM2_SET_SPELLING_CHAMPION (c_hero.cpp:2971) ----
 * Set which hero is currently casting spells. -1 = none. */
typedef struct {
    int16_t *spelling_champion;
} DM2_V1_SpellingState;

void dm2_v1_set_spelling_champion(DM2_V1_SpellingState *state, int hero_idx);

/* ---- DM2_PROCEED_ENCHANTMENT_SELF (c_hero.cpp:724) ----
 * Apply self-targeted enchantment to party heroes.
 * hero_mask: bitmask of which heroes (bits 0-3).
 * aura_type: enchantment aura identifier.
 * power: enchantment power value (halved if any hero already > 50).
 * duration: tick offset added to current game tick for timer. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int (*get_timer_count)(void *ctx);
    uint8_t (*get_timer_type)(void *ctx, int timer_idx);
    uint8_t (*get_timer_actor)(void *ctx, int timer_idx);
    void (*set_timer_actor)(void *ctx, int timer_idx, uint8_t actor);
    void (*delete_timer)(void *ctx, int timer_idx);
    void (*queue_enchant_timer)(void *ctx, uint8_t actor_mask,
                                int16_t power, uint16_t duration);
} DM2_V1_EnchantmentCallbacks;

void dm2_v1_proceed_enchantment_self(
    uint16_t hero_mask, uint8_t aura_type, int16_t power, uint16_t duration,
    const DM2_V1_EnchantmentCallbacks *cb, void *ctx);

/* ---- PROCEED_GLOBAL_EFFECT_TIMERS (SkWinCore.cpp:2426) ----
 * Iterate all timers and accumulate global spell effects. */
typedef struct {
    int16_t light;
    int16_t invisibility;
    int16_t see_thru_walls;
} DM2_V1_GlobalSpellEffects;

typedef struct {
    int timer_count;
    uint8_t (*get_timer_type)(void *ctx, int idx);
    uint16_t (*get_timer_actor)(void *ctx, int idx);
    int16_t (*get_timer_value)(void *ctx, int idx);
    void (*process_timer_0e)(void *ctx, int idx);
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    const int16_t *light_level_table;
    int light_level_table_size;
} DM2_V1_GlobalEffectCallbacks;

void dm2_v1_proceed_global_effect_timers(
    DM2_V1_GlobalSpellEffects *effects,
    const DM2_V1_GlobalEffectCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_TIMER_0C (c_tim_proc.cpp:30) ----
 * Hero timer expired: clear timer index, set flag 0x800 if alive. */
int dm2_v1_process_timer_0c(DM2_V1_HeroState *hero);

/* ---- DM2_RESUME_FROM_WAKE (c_hero.cpp:2961) ----
 * Resume from sleep/wake state. Sets flags in the wake state struct. */
typedef struct {
    int *wake_flag;      /* ddat.v1e0488 */
    int *sleep_flag;     /* ddat.v1e0238 */
    int *tick_trigger;   /* ddat.ticktrig */
    void (*init_backbuff)(void *ctx);
    int (*display_mode)(void *ctx, int mode);
} DM2_V1_WakeCallbacks;

int dm2_v1_resume_from_wake(const DM2_V1_WakeCallbacks *cb, void *ctx);

/* ---- DM2_CHAMPION_SQUAD_RECOMPUTE_POSITION (c_hero.cpp:4141) ----
 * If a pending position change exists, reapply it. Trivial wrapper. */
typedef struct {
    int16_t pending_pos;
    int16_t pending_flag;
    void (*change_player_pos)(void *ctx, int16_t packed);
} DM2_V1_SquadRecomputeCallbacks;

void dm2_v1_champion_squad_recompute_position(
    const DM2_V1_SquadRecomputeCallbacks *cb, void *ctx);

/* ---- DM2_hero_2c1d_0186 (c_hero.cpp:840) ----
 * Cast an enchantment on the party. If requires_mp, deducts 4 MP. */
typedef struct {
    void (*proceed_enchantment_self)(void *ctx, uint16_t mask, uint8_t aura,
                                     int16_t power, uint16_t duration);
} DM2_V1_CastEnchantCallbacks;

int dm2_v1_hero_cast_enchantment(
    DM2_V1_HeroState *hero, uint8_t enchant_type,
    int16_t power, int requires_mp,
    const DM2_V1_CastEnchantCallbacks *cb, void *ctx);

/* ---- DM2_SHOOT_CHAMPION_MISSILE (c_hero.cpp:688) ----
 * Fire a projectile from a champion. */
typedef struct {
    void (*shoot_item)(void *ctx, uint16_t item, int16_t x, int16_t y,
                       int direction, int facing, uint8_t damage,
                       uint8_t speed, uint8_t power);
} DM2_V1_ShootMissileCallbacks;

typedef struct {
    uint8_t abs_dir;
    uint8_t party_pos;
    int16_t party_x;
    int16_t party_y;
} DM2_V1_ShootMissileState;

void dm2_v1_shoot_champion_missile(
    const DM2_V1_ShootMissileState *state,
    uint16_t item, int16_t damage, int16_t kinetic_energy, int16_t spell_power,
    const DM2_V1_ShootMissileCallbacks *cb, void *ctx);

/* ---- DM2_CAST_CHAMPION_MISSILE_SPELL (c_hero.cpp:3840) ----
 * Cast a missile spell if hero has enough MP. */
int dm2_v1_cast_champion_missile_spell(
    DM2_V1_HeroState *hero, const DM2_V1_ShootMissileState *state,
    uint16_t spell_item, int16_t spell_power, int16_t mp_cost,
    const DM2_V1_ShootMissileCallbacks *cb, void *ctx);

/* ---- R_36EFE + DM2_BRING_CHAMPION_TO_LIFE (c_hero.cpp:873+916) ----
 * Resurrect a dead champion. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int16_t (*get_player_at_position)(void *ctx, uint8_t pos);
    uint8_t formation_start;
    void (*post_resurrect)(void *ctx, int hero_idx);
} DM2_V1_ResurrectCallbacks;

void dm2_v1_bring_champion_to_life(
    int hero_idx,
    const DM2_V1_ResurrectCallbacks *cb, void *ctx);

/* ---- DM2_ADD_ITEM_TO_PLAYER (c_hero.cpp:2188) ----
 * Auto-equip an item to the first matching inventory slot. */
typedef struct {
    uint16_t slot_start;
    uint16_t slot_end;
    int16_t category_filter;
} DM2_V1_SlotGroup;

typedef struct {
    int (*is_item_fit)(void *ctx, uint16_t item, int slot, int mode);
    void (*equip_item)(void *ctx, int hero_idx, uint16_t item, int slot);
} DM2_V1_AddItemCallbacks;

int dm2_v1_add_item_to_player(
    int hero_idx, int16_t *hero_items, int item_count,
    uint16_t item_record, uint16_t item_db_type,
    const DM2_V1_SlotGroup *groups, int group_count,
    const DM2_V1_AddItemCallbacks *cb, void *ctx);

/* ---- DM2_BURN_PLAYER_LIGHTING_ITEMS (c_hero.cpp:2262) ----
 * Decrement charges on lit torches/lanterns. */
typedef struct {
    int hero_count;
    int16_t (*get_hero_hand_item)(void *ctx, int hero_idx, int hand);
    uint16_t (*query_item_flags)(void *ctx, uint16_t item);
    int16_t (*add_item_charge)(void *ctx, uint16_t item, int16_t delta);
    void (*set_item_importance)(void *ctx, uint16_t item, int val);
    void (*recalc_light)(void *ctx);
} DM2_V1_BurnLightCallbacks;

int dm2_v1_burn_player_lighting_items(
    const DM2_V1_BurnLightCallbacks *cb, void *ctx);

/* ---- DM2_DROP_PLAYER_ITEMS (c_hero.cpp:2535) ----
 * Drop all 30 inventory items of a hero onto the floor. */
typedef struct {
    int16_t (*remove_possession)(void *ctx, int hero_idx, int slot);
    void (*move_record_to)(void *ctx, uint16_t record, int16_t x, int16_t y,
                            uint8_t pos);
} DM2_V1_DropItemsCallbacks;

int dm2_v1_drop_player_items(
    int hero_idx, uint8_t party_pos,
    int16_t tile_x, int16_t tile_y,
    const uint8_t *slot_order, int slot_count,
    const DM2_V1_DropItemsCallbacks *cb, void *ctx);

/* ---- DM2_PUT_ITEM_TO_PLAYER (c_hero.cpp:2838) ----
 * Place cursor item into hero backpack (slots 13-29). */
typedef struct {
    int16_t cursor_item;
    int16_t (*remove_object_from_hand)(void *ctx);
    void (*equip_item)(void *ctx, int hero_idx, uint16_t item, int slot);
} DM2_V1_PutItemCallbacks;

int dm2_v1_put_item_to_player(
    int hero_idx, DM2_V1_HeroState *hero,
    int16_t *hero_items, int item_count,
    const DM2_V1_PutItemCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HERO_OPS_PC34_COMPAT_H */
