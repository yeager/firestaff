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
    int16_t poison_value;    /* hero->poison */
    int16_t poisoned_count;  /* hero->poisoned */
    int16_t plague_value;
    uint8_t hero_type;
    uint8_t party_pos;
    uint8_t ench_aura;
    uint8_t ench_power;
    int16_t timer_idx;
    uint8_t absdir;           /* hero->absdir */
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

/* ---- DM2_CURE_POISON (c_hero.cpp:2580) ----
 * Partial poison cure: walks poison timers (type 0x4B) for the hero,
 * subtracting cure_amount from timer counters until exhausted. Reduces
 * hero->poison proportionally, deletes fully cured timers, decrements
 * hero->poisoned count. Stops when all cure is applied or no more timers. */
typedef struct {
    int (*get_timer_count)(void *ctx);
    uint8_t (*get_timer_type)(void *ctx, int timer_idx);
    uint8_t (*get_timer_actor)(void *ctx, int timer_idx);
    int16_t (*get_timer_value)(void *ctx, int timer_idx);
    void (*set_timer_value)(void *ctx, int timer_idx, int16_t value);
    void (*delete_timer)(void *ctx, int timer_idx);
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
} DM2_V1_CurePoisonCallbacks;

int dm2_v1_cure_poison(
    int hero_idx, int16_t cure_amount,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx);

/* ---- CURE_PLAGUE (SkWinCore2.cpp:455) ----
 * Same structure as cure_poison but for plague timers. */
int dm2_v1_cure_plague(
    int hero_idx, uint8_t plague_timer_type,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_POISON (c_hero.cpp:3397) ----
 * Apply one poison tick: wound = MAX(1, (counters+0x1E)>>6), wound player,
 * set flags 0x2800, decrement counter; if remaining > 0, accumulate into
 * hero->poison (capped at 0xC00), increment hero->poisoned, requeue timer
 * with delay 0x24. Skips if hero_idx == last hero (ddat.v1e0288 check). */
typedef struct {
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
    int (*is_last_hero)(void *ctx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int damage,
                         int type, int flags);
    void (*queue_poison_timer)(void *ctx, int hero_idx, int16_t counters,
                               uint32_t delay);
} DM2_V1_ProcessPoisonCallbacks;

int dm2_v1_process_poison(
    int hero_idx, int16_t counters,
    const DM2_V1_ProcessPoisonCallbacks *cb, void *ctx);

/* ---- DM2_ADD_COIN_TO_WALLET / DM2_TAKE_COIN_FROM_WALLET
 * (SKWINDOS/dm2byh.cpp:1451 SKW_ADD_COIN_TO_WALLET,
 *  SKWINDOS/dm2byc.cpp:1172 SKW_TAKE_COIN_FROM_WALLET) ----
 * A "wallet" is a container item record (moneybox); coins are item
 * records held in the container's linked list (NEXT pointer). Coins of
 * the same denomination are collapsed into a single record with a
 * packed charge/quantity count (0..0x3F) rather than one record per
 * coin.
 *
 * add_coin: container_ref must be a moneybox (is_container_moneybox)
 * and coin_item_ref must be a currency item (is_item_currency). Walks
 * the container's item list; if it hits a non-currency item, returns
 * whether that slot is occupied. If an existing coin record of the
 * same denomination is found with charge < 0x3F, increments its charge
 * and deallocates the incoming record. Otherwise appends the incoming
 * record to the end of the list. Returns 1 on success, 0 if the
 * container/item preconditions fail.
 *
 * take_coin: coin_type_index selects a denomination via the
 * coin_type_table (ddata.v1e0394). Walks the container's item list
 * remembering the last matching-denomination record. On reaching the
 * end of the (currency-only) run: if a match was found and its charge
 * is nonzero, decrements the charge and allocates a fresh single-coin
 * record of that denomination to return; if the match's charge is
 * zero (last coin in the stack), cuts that record out of the
 * container list and returns it directly. Returns DM2_V1_OBJECT_NULL
 * (0xFFFF) if no matching coin was found or the list contains a
 * non-currency item before reaching the end. */
#define DM2_V1_OBJECT_NULL 0xFFFFu
#define DM2_V1_OBJECT_END  0xFFFEu
#define DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY 0xA
#define DM2_V1_COIN_CHARGE_MAX 0x3F

typedef struct {
    int (*is_container_moneybox)(void *ctx, uint16_t container_ref);
    int (*is_item_currency)(void *ctx, uint16_t item_ref);
    uint16_t (*get_item_db_type)(void *ctx, uint16_t item_ref);
    uint16_t (*get_distinctive_item_type)(void *ctx, uint16_t item_ref);
    uint16_t (*get_container_head)(void *ctx, uint16_t container_ref);
    uint16_t (*get_next_item_in_list)(void *ctx, uint16_t item_ref);
    uint8_t (*get_item_charge)(void *ctx, uint16_t item_ref);
    void (*set_item_charge)(void *ctx, uint16_t item_ref, uint8_t charge);
    void (*append_item_to_container)(void *ctx, uint16_t item_ref,
                                     uint16_t container_ref);
    void (*cut_item_from_container)(void *ctx, uint16_t item_ref,
                                    uint16_t container_ref);
    void (*dealloc_record)(void *ctx, uint16_t item_ref);
    uint16_t (*alloc_new_dbitem)(void *ctx, uint16_t denomination);
    const uint16_t *coin_type_table; /* ddata.v1e0394 */
    int coin_type_table_size;
} DM2_V1_WalletCallbacks;

int dm2_v1_add_coin_to_wallet(uint16_t container_ref, uint16_t coin_item_ref,
                               const DM2_V1_WalletCallbacks *cb, void *ctx);
uint16_t dm2_v1_take_coin_from_wallet(uint16_t container_ref,
                                      int16_t coin_type_index,
                                      const DM2_V1_WalletCallbacks *cb,
                                      void *ctx);

/* ---- DM2_PERFORM_TURN_SQUAD (c_hero.cpp:2887) ----
 * Turn the party: sets ddat.v1e0488, calls RESET_SQUAD_DIR (set all
 * heroes' absdir to ddat.v1e0258), checks tile type for stairs (type 3
 * → move_12b4_00af) or teleporter (→ map_3BF83), otherwise does
 * moverec_3CE7D + party.rotate + moverec_3CE7D. Delta: 1 = right turn
 * (rotation +3), 2 = about-face (rotation +1). */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int hero_idx);
    void (*reset_squad_dir)(void *ctx);
    uint8_t (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    int (*get_teleporter_detail)(void *ctx, int16_t x, int16_t y,
                                  uint8_t *out_b1, uint8_t *out_b2,
                                  uint8_t *out_b3, uint8_t *out_b4);
    void (*move_stairs)(void *ctx, int stair_flag);
    void (*map_teleport)(void *ctx, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b1);
    void (*moverec)(void *ctx, int16_t x, int16_t y, int16_t dir, int a, int b, int c);
    void (*rotate_party)(void *ctx, int16_t new_dir);
    int16_t party_x;
    int16_t party_y;
    int16_t party_dir;
} DM2_V1_SquadCallbacks;

void dm2_v1_perform_turn_squad(
    int delta,
    const DM2_V1_SquadCallbacks *cb, void *ctx);

/* ---- DM2_SET_SPELLING_CHAMPION (c_hero.cpp:2971) ----
 * Activate spell casting for a hero: checks curHP != 0, sets
 * curactmode=2, curacthero=hero_idx+1, recomputes squad position,
 * updates right panel. */
typedef struct {
    int16_t *spelling_champion;  /* party.curacthero */
    int16_t curactmode;
    int16_t spell_active;        /* ddat.v1e0b6c */
    int16_t spell_symbol_count;  /* ddat.v1e0b62 */
    void (*update_panel)(void *panel_ctx, int mode);
    void *panel_ctx;
} DM2_V1_SpellingState;

void dm2_v1_set_spelling_champion(
    DM2_V1_SpellingState *state, int hero_idx,
    const DM2_V1_SquadCallbacks *cb, void *ctx);

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
 * Fire a projectile from a champion. After DM2_SHOOT_ITEM, skproject
 * also writes two globals (c_hero.cpp:719-720):
 *   ddat.v1e025e = 4     (missile-noise/animation trigger)
 *   ddat.v1e0274 = direction (last-shot direction) */
typedef struct {
    void (*shoot_item)(void *ctx, uint16_t item, int16_t x, int16_t y,
                       int direction, int facing, uint8_t damage,
                       uint8_t speed, uint8_t power);
    void (*set_v1e025e)(void *ctx, int16_t value);
    void (*set_v1e0274)(void *ctx, uint8_t direction);
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
 * Resurrect a dead champion. R_36EFE always assigns
 * hero->absdir = ddat.v1e0258 (c_hero.cpp:911, `formation_start` here)
 * once the party position has been (re)assigned, regardless of whether
 * a rescan for a free slot happened. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int16_t (*get_player_at_position)(void *ctx, uint8_t pos);
    uint8_t formation_start;
    void (*clear_hero_items)(void *ctx, int hero_idx);
    void (*post_resurrect)(void *ctx, int hero_idx);
} DM2_V1_ResurrectCallbacks;

void dm2_v1_hero_bring_to_life(
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
 * Decrement charges on lit torches/lanterns. Excludes the last party
 * hero from the burn loop when ddat.v1e0288 is set (c_hero.cpp:2271-2277:
 * `vl_00 = heros_in_party - (ddat.v1e0288 != 0)`), the same "is last
 * hero" condition used by DM2_PROCESS_POISON. */
typedef struct {
    int hero_count;
    int (*is_last_hero)(void *ctx, int hero_idx); /* ddat.v1e0288 check */
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

/* ---- c_hero::init (c_hero.cpp:40) ----
 * Zero all hero fields. */
void dm2_v1_hero_init(DM2_V1_HeroState *hero);

/* ---- c_party::init (c_hero.cpp:153) ----
 * Initialize party: zero all heroes and party-level state. */
typedef struct {
    int max_heroes;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
} DM2_V1_PartyInitCallbacks;

void dm2_v1_party_init(
    int *hero_count, int *curactmode, int *curacthero,
    const DM2_V1_PartyInitCallbacks *cb, void *ctx);

/* ---- c_party::rotate (c_hero.cpp:168) ----
 * Rotate party facing. Updates each hero's partypos and absdir. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int16_t *party_dir;      /* ddat.v1e0258 */
    int16_t *party_absdir;   /* ddat.v1e0264 */
    int16_t v1e0234;         /* staircase flag */
    uint8_t party_absdir_val; /* party.absdir for staircase mode */
} DM2_V1_RotateCallbacks;

void dm2_v1_party_rotate(
    int16_t new_dir,
    const DM2_V1_RotateCallbacks *cb, void *ctx);

/* ---- c_party::set_hero_flags (c_hero.cpp:190) ----
 * Set bit 0x4000 on every hero's heroflag. */
void dm2_v1_party_set_hero_flags(
    int hero_count,
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx), void *ctx);

/* ---- c_party::get_player_weight (c_hero.cpp:197) ----
 * Get hero's carried weight; adds hand-held item weight if event hero. */
int16_t dm2_v1_get_player_weight(
    const DM2_V1_HeroState *hero,
    int is_event_hero, int16_t hand_weight);

/* ---- c_party::calc_player_weight (c_hero.cpp:208) ----
 * Sum weights of all items in hero's inventory slots. */
typedef struct {
    int16_t (*query_item_weight)(void *ctx, uint16_t item);
    int (*is_container_chest)(void *ctx, uint16_t item);
    uint16_t *hero_items;
    int item_count;
    uint16_t *hand_container;
    int container_size;
    int is_active_hero;    /* heroidx == curacthero-1 && curactmode < 2 */
    uint16_t active_hand_item; /* handitems[h+1][curactmode] */
} DM2_V1_CalcWeightCallbacks;

int16_t dm2_v1_calc_player_weight(
    DM2_V1_HeroState *hero,
    const DM2_V1_CalcWeightCallbacks *cb, void *ctx);

/* ---- DM2_RESET_SQUAD_DIR (c_hero.cpp:2939) ----
 * Reset all heroes' absdir to the party's current facing direction. */
void dm2_v1_reset_squad_dir(
    int hero_count, uint8_t party_dir,
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx), void *ctx);

/* ---- DM2_SELECT_CHAMPION_LEADER (c_hero.cpp:2325) ----
 * Change the active champion leader. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int16_t *event_heroidx;
    int16_t v1e0288;  /* last hero check */
} DM2_V1_SelectLeaderCallbacks;

void dm2_v1_select_champion_leader(
    int hero_idx,
    const DM2_V1_SelectLeaderCallbacks *cb, void *ctx);

/* ---- DM2_EQUIP_ITEM_TO_HAND (c_hero.cpp:2161) ----
 * Place an item into a hero's slot (or party hand container if slot >= 30). */
typedef struct {
    uint16_t *hero_items;
    int item_count;
    uint16_t *hand_container;
    int container_size;
    void (*process_item_bonus)(void *ctx, int hero_idx, uint16_t item,
                                int slot, int mode);
} DM2_V1_EquipCallbacks;

void dm2_v1_equip_item_to_hand(
    int hero_idx, uint16_t item, int slot,
    const DM2_V1_EquipCallbacks *cb, void *ctx);

/* ---- DM2_REMOVE_POSSESSION (c_hero.cpp:2485) ----
 * Remove an item from a hero's slot. Returns the item record or -1. */
typedef struct {
    uint16_t *hero_items;
    int item_count;
    uint16_t *hand_container;
    int container_size;
    void (*process_item_bonus)(void *ctx, int hero_idx, uint16_t item,
                                int slot, int mode);
    void (*display_right_panel)(void *ctx);
    int16_t curacthero;    /* party.curacthero */
    int16_t curactmode;    /* party.curactmode */
} DM2_V1_RemovePossessionCallbacks;

int16_t dm2_v1_remove_possession(
    int hero_idx, int slot,
    const DM2_V1_RemovePossessionCallbacks *cb, void *ctx);

/* ---- DM2_GET_PARTY_SPECIAL_FORCE (c_hero.cpp:2407) ----
 * Sum the "special force" value across all party members. */
typedef struct {
    int hero_count;
    DM2_V1_HeroState *(*get_hero)(void *ctx, int idx);
    int16_t (*get_player_weight)(void *ctx, int hero_idx);
} DM2_V1_SpecialForceCallbacks;

int dm2_v1_get_party_special_force(
    const DM2_V1_SpecialForceCallbacks *cb, void *ctx);

/* ---- DM2_ADJUST_HAND_COOLDOWN (c_hero.cpp:2432) ----
 * Set hand cooldown timers after an action. */
#define DM2_V1_MAX_HANDS 4

typedef struct {
    uint8_t hand_cooldown[DM2_V1_MAX_HANDS];
} DM2_V1_HeroCooldownState;

void dm2_v1_adjust_hand_cooldown(
    DM2_V1_HeroCooldownState *hero,
    int16_t cooldown_ticks, int hand_idx,
    int easy_mode);

/* ---- DM2_ATTACK_PARTY (c_hero.cpp:3346) ----
 * Creature attacks all party members. Returns bitmask of wounded heroes. */
typedef struct {
    int hero_count;
    int16_t (*wound_player)(void *ctx, int hero_idx, int16_t damage,
                             int body_parts, int damage_type);
    int16_t (*rand16)(void *ctx, int16_t max);
    int16_t (*max16)(void *ctx, int16_t a, int16_t b);
} DM2_V1_HeroAttackPartyCallbacks;

int dm2_v1_hero_attack_party(
    int16_t total_damage, int body_parts, int damage_type,
    const DM2_V1_HeroAttackPartyCallbacks *cb, void *ctx);

/* ---- DM2_REMOVE_OBJECT_FROM_HAND (c_hero.cpp:2354) ----
 * Remove the item currently held in the cursor hand. */
typedef struct {
    int16_t *cursor_item;   /* ddat.savegamewpc.w_00 */
    int16_t *cursor_extra;  /* ddat.savegamewpc.w_18 */
    int16_t *cursor_weight; /* ddat.savegamewpc.weight */
    int8_t *cursor_type;    /* ddat.savegamewpc.b_16 */
    void (*hide_cursor_item)(void *ctx);
    void (*process_item_bonus)(void *ctx, int hero_idx, uint16_t item,
                                int slot, int mode);
    void (*moverec)(void *ctx);
    int16_t event_heroidx;
} DM2_V1_RemoveFromHandCallbacks;

int16_t dm2_v1_remove_object_from_hand(
    const DM2_V1_RemoveFromHandCallbacks *cb, void *ctx);

/* DM2_PROCESS_PLAYERS_DAMAGE and DM2_PLAYER_DEFEATED are declared in
 * dm2_v1_runtime_narrow_pc34_compat.h with their authoritative callback
 * structs (DM2_V1_ProcessPlayersDamageCallbacks, DM2_V1_PlayerDefeatedInfo,
 * DM2_V1_PlayerDefeatedCallbacks). Include that header for those APIs. */

/* ---- DM2_USE_DEXTERITY_ATTRIBUTE (c_hero.cpp:2026) ----
 * Compute effective dexterity check value. Pure computation. */
typedef struct {
    int valid;
    int16_t value;
} DM2_V1_UseDexterityResult;

int dm2_v1_use_dexterity_attribute(
    int16_t adj_dex,          /* get_adj_ability1(DEXTERITY, CUR) */
    int16_t player_weight,    /* get_player_weight() */
    int16_t max_load,         /* get_max_load() */
    int is_sleeping,          /* ddat.v1e0238 != 0 */
    int16_t rand_7a,          /* DM2_RAND() & 7 */
    int16_t rand_7b,          /* DM2_RAND() & 7 */
    int16_t rand_7c,          /* DM2_RAND() & 7 */
    DM2_V1_UseDexterityResult *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HERO_OPS_PC34_COMPAT_H */
