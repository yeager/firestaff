#ifndef FIRESTAFF_DM2_V1_RUNTIME_NARROW_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_RUNTIME_NARROW_PC34_COMPAT_H

/*
 * dm2_v1_runtime_narrow_pc34_compat.h — narrow callback-based stubs for the
 * remaining DM2 skproject runtime functions (timer processors, hero combat
 * ops, creature AI ops, item ops, move-record ops, record recycling, and
 * light ops).
 *
 * Each function captures the essential parameters and return type of the
 * corresponding skproject function and delegates all external state access
 * (dungeon data, other subsystems, globals) to callbacks. Complex functions
 * (ENGAGE_COMMAND, ACTIVATE_ITEM_TELEPORT, PLAYER_CONSUME_OBJECT) delegate
 * their routing/switch logic to a single dispatch callback.
 *
 * Source references (skproject SKULLWIN):
 *   c_tim_proc.cpp, c_hero.cpp, c_creature.cpp, c_item.cpp, c_moverec.cpp,
 *   c_record.cpp, c_light.cpp, c_engage.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * c_tim_proc.cpp — timer processors
 * ===================================================================== */

/* ---- DM2_PROCESS_TIMER_RESURRECTION (c_tim_proc.cpp:39) ----
 * Timer fired: resurrect the actor identified by the timer. */
typedef struct {
    uint8_t (*get_timer_actor)(void *ctx, int timer_idx);
    int16_t (*get_timer_value)(void *ctx, int timer_idx);
    void (*bring_champion_to_life)(void *ctx, int hero_idx);
    void (*delete_timer)(void *ctx, int timer_idx);
} DM2_V1_TimerResurrectionCallbacks;

void dm2_v1_process_timer_resurrection(
    int timer_idx, const DM2_V1_TimerResurrectionCallbacks *cb, void *ctx);

/* ---- DM2_CONTINUE_ORNATE_NOISE (c_tim_proc.cpp:1092) ----
 * Continue a periodic ornament noise timer, requeueing it. */
typedef struct {
    int16_t (*get_timer_value)(void *ctx, int timer_idx);
    void (*play_sound)(void *ctx, int16_t x, int16_t y, int sample);
    void (*requeue_timer)(void *ctx, int timer_idx, uint32_t delay);
    void (*delete_timer)(void *ctx, int timer_idx);
} DM2_V1_ContinueOrnateNoiseCallbacks;

void dm2_v1_continue_ornate_noise_cb(
    int timer_idx, int16_t x, int16_t y, uint32_t delay,
    const DM2_V1_ContinueOrnateNoiseCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_ORNATE_ANIMATOR (c_tim_proc.cpp:1325) ----
 * Advance a one-shot wall-ornament animation frame. */
typedef struct {
    int16_t (*get_ornament_frame)(void *ctx, int16_t x, int16_t y);
    void (*set_ornament_frame)(void *ctx, int16_t x, int16_t y, int16_t frame);
    void (*invalidate_view)(void *ctx, int16_t x, int16_t y);
} DM2_V1_ActivateOrnateAnimatorCallbacks;

void dm2_v1_activate_ornate_animator(
    int16_t x, int16_t y, int frame_count,
    const DM2_V1_ActivateOrnateAnimatorCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_CONTINUOUS_ORNATE_ANIMATOR (c_tim_proc.cpp:1503) ----
 * Advance a looping wall-ornament animation frame and requeue timer. */
typedef struct {
    int16_t (*get_ornament_frame)(void *ctx, int16_t x, int16_t y);
    void (*set_ornament_frame)(void *ctx, int16_t x, int16_t y, int16_t frame);
    void (*requeue_timer)(void *ctx, int timer_idx, uint32_t delay);
} DM2_V1_ActivateContinuousOrnateAnimatorCallbacks;

void dm2_v1_activate_continuous_ornate_animator(
    int timer_idx, int16_t x, int16_t y, int frame_count, uint32_t delay,
    const DM2_V1_ActivateContinuousOrnateAnimatorCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_SHOOTER (c_tim_proc.cpp:1610) ----
 * Fire a wall shooter mechanism at the timer's target tile. */
typedef struct {
    int16_t (*get_timer_actor)(void *ctx, int timer_idx);
    void (*shoot_item)(void *ctx, int16_t x, int16_t y, uint16_t item,
                       int direction);
} DM2_V1_ActivateShooterCallbacks;

void dm2_v1_activate_shooter_narrow(
    int timer_idx, int16_t x, int16_t y, uint16_t item, int direction,
    const DM2_V1_ActivateShooterCallbacks *cb, void *ctx);

/* ---- DM2_TRY_ORNATE_NOISE (c_tim_proc.cpp:4591) ----
 * Roll a chance-based ornament noise and queue a timer if it triggers. */
typedef struct {
    int (*random)(void *ctx, int max);
    void (*queue_timer)(void *ctx, int16_t x, int16_t y, uint32_t delay);
} DM2_V1_TryOrnateNoiseCallbacks;

int dm2_v1_try_ornate_noise(
    int16_t x, int16_t y, int chance, uint32_t delay,
    const DM2_V1_TryOrnateNoiseCallbacks *cb, void *ctx);

/* ---- DM2_ANIMATE_CREATURE (c_tim_proc.cpp:2859) ----
 * Advance a creature's animation frame counter. */
typedef struct {
    int16_t anim_frame;
    int16_t anim_max;
} DM2_V1_CreatureAnimState;

void dm2_v1_animate_creature(
    DM2_V1_CreatureAnimState *anim, int16_t direction, int16_t step);

/* ---- DM2_ADVANCE_TILES_TIME (c_tim_proc.cpp:3533) ----
 * Advance a tile's own timer/animation state by delta ticks. */
typedef struct {
    void (*advance_tile_state)(void *ctx, int16_t x, int16_t y, int16_t delta);
} DM2_V1_AdvanceTilesTimeCallbacks;

void dm2_v1_advance_tiles_time(
    int16_t x, int16_t y, int16_t delta,
    const DM2_V1_AdvanceTilesTimeCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_ACTUATOR_TICK_GENERATOR (c_tim_proc.cpp:4395) ----
 * Iterate world actuator generators and tick each one. */
typedef struct {
    int generator_count;
    int (*get_generator_active)(void *ctx, int idx);
    void (*tick_generator)(void *ctx, int idx);
} DM2_V1_ActuatorTickGeneratorCallbacks;

void dm2_v1_process_actuator_tick_generator(
    const DM2_V1_ActuatorTickGeneratorCallbacks *cb, void *ctx);

/* ---- DM2_OPERATE_PIT_TELE_TILE (c_tim_proc.cpp:4513) ----
 * Move objects through a pit or teleporter tile.
 * Returns 1 if the tile operated on something, 0 otherwise. */
typedef struct {
    int (*get_things_on_tile)(void *ctx, int16_t x, int16_t y);
    void (*teleport_thing)(void *ctx, int16_t x, int16_t y, int thing_idx);
} DM2_V1_OperatePitTeleTileCallbacks;

int dm2_v1_operate_pit_tele_tile(
    int16_t x, int16_t y, int tele_mode,
    const DM2_V1_OperatePitTeleTileCallbacks *cb, void *ctx);

/* =====================================================================
 * c_hero.cpp — hero combat / progression ops
 * ===================================================================== */

/* ---- DM2_CALC_PLAYER_ATTACK_DAMAGE (c_hero.cpp:232) ----
 * Compute a champion's melee/ranged attack damage against a creature.
 * skproject rolls a to-hit check gated by dexterity vs. the target's
 * defense/level, then (on a hit) accumulates a base strength-derived
 * roll, several small d32/d16/d4 dice, a skill-threshold bonus, and a
 * poison-resistance roll, before granting fighter/ninja skill exp via
 * ADJUST_SKILLS and charging stamina. On a miss it still applies a
 * small stamina/skill tick. Returns the final damage roll. */
typedef struct {
    int16_t cur_hp;
    int16_t skill_fighter;   /* DM2_QUERY_PLAYER_SKILL_LV(fighter) cache */
    int16_t skill_ninja;     /* DM2_QUERY_PLAYER_SKILL_LV(ninja) cache */
    int16_t dexterity;       /* DM2_USE_DEXTERITY_ATTRIBUTE(heroidx) */
    uint8_t weapon_poison_class; /* dbspec byte at offset 0x8 in weapon record; 0xff = unarmed */
} DM2_V1_AttackDamageHero;

typedef struct {
    int (*random)(void *ctx, int max);     /* DM2_RAND()-style: [0,max) */
    int (*random_dir)(void *ctx);          /* DM2_RANDDIR(): small signed jitter */
    int (*random_bit)(void *ctx);          /* DM2_RANDBIT(): 0 or 1 */
    int (*use_luck)(void *ctx, int hero_idx, int16_t threshold); /* hero->use_luck() */
    int16_t (*creature_defense_class)(void *ctx, int creature_idx); /* byte at 0x8 of AI spec */
    int16_t (*creature_armour_class)(void *ctx, int creature_idx);  /* word at 0x16 >>8 &0xf */
    int (*is_creature_asleep)(void *ctx, int creature_idx);
    int16_t (*apply_creature_poison_resistance)(void *ctx, int creature_idx, int16_t poison_class);
    void (*adjust_stamina)(void *ctx, int hero_idx, int16_t amount);
    int32_t (*adjust_skills)(void *ctx, int hero_idx, int32_t skill_id, int32_t exp_gain);
} DM2_V1_AttackDamageCallbacks;

int16_t dm2_v1_calc_player_attack_damage(
    int hero_idx, const DM2_V1_AttackDamageHero *hero, int creature_idx,
    int16_t action_strength, int32_t skill_id,
    const DM2_V1_AttackDamageCallbacks *cb, void *ctx);

/* ---- DM2_hero_39796 (c_hero.cpp:464) ----
 * NOTE: at c_hero.cpp:464 skproject's function is the champion
 * name-entry UI loop (keyboard text edit for name1/name2), not a
 * numeric regen tick — there is no meaningful non-UI state logic to
 * port. We keep the narrow per-tick regen contract documented by the
 * original stub (stamina regen + dirty-flag) since the caller expects
 * a tick hook here; see c_hero.cpp:464 for the actual (UI-only)
 * source function this identifier names. */
typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    int16_t cur_stamina;
    int16_t max_stamina;
    uint16_t hero_flag;
} DM2_V1_HeroRegenState;

void dm2_v1_hero_39796(DM2_V1_HeroRegenState *hero);

/* ---- DM2_hero_2c1d_135d (c_hero.cpp:1403) ----
 * Compute a hero's damage-reduction percentage (0-100) from worn armour
 * across hand + body slots, applying wear-and-tear (armourClass halving
 * on the "used" flag), shield double-count, body-status penalty, and a
 * sleep-halving. Iterates ARMOUR_SLOTS (6) body slots then the 2 hand
 * slots. Returns the defense percentage in [0,100]. */
typedef struct {
    int (*random)(void *ctx, int max);
    int16_t (*item_armour_class)(void *ctx, uint16_t item); /* dbspec word 0xb, hi bit = shield */
    int16_t (*hand_defense_class)(void *ctx, int hero_idx, int hand);
    int16_t (*ability_defense_bonus)(void *ctx, int hero_idx); /* get_adj_ability1(DEXTERITY) */
    int (*is_asleep)(void *ctx, int hero_idx);
} DM2_V1_DefenseCallbacks;

int32_t dm2_v1_hero_2c1d_135d(
    int hero_idx, uint16_t body_item_slot[6], uint16_t hand_item[2],
    int is_used, uint32_t body_flag,
    const DM2_V1_DefenseCallbacks *cb, void *ctx);

/* ---- DM2_ADJUST_SKILLS (c_hero.cpp:1166) ----
 * Grant experience to a hero's skill (or skill-group), applying a
 * time-window halving/doubling around a global exp-scale window,
 * scaling by the current dbspec exp multiplier, adding it to both the
 * sub-skill and (if a sub-skill) the parent skill-group, then looping
 * "level up" passes while the new level exceeds the loop count: each
 * pass rolls ability/HP/stamina/MP gains (varying by skill group 0-3:
 * fighter/ninja/wizard/priest) and clamps maxHP<=999, maxStamina<=9999,
 * maxMP<=900. Returns the resulting skill level. */
typedef struct {
    int16_t (*get_gametick)(void *ctx);
    int16_t (*get_exp_window_tick)(void *ctx); /* ddat.v1d26a4 */
    int16_t (*get_exp_scale)(void *ctx);       /* ddat.v1e03c0->w_0c >> 12 */
    int16_t (*get_skill_lv)(void *ctx, int hero_idx, int skill_group, int with_bonus);
    int16_t (*get_skill_exp)(void *ctx, int hero_idx, int skill_id);
    void (*add_skill_exp)(void *ctx, int hero_idx, int skill_id, int32_t amount);
    int (*random_bit)(void *ctx);
    int16_t (*get_max_hp)(void *ctx, int hero_idx);
    void (*set_max_hp)(void *ctx, int hero_idx, int16_t v);
    int16_t (*get_max_stamina)(void *ctx, int hero_idx);
    void (*set_max_stamina)(void *ctx, int hero_idx, int16_t v);
    int16_t (*get_max_mp)(void *ctx, int hero_idx);
    void (*set_max_mp)(void *ctx, int hero_idx, int16_t v);
    void (*add_ability_max)(void *ctx, int hero_idx, int ability, int16_t delta);
    void (*mark_hero_dirty)(void *ctx, int hero_idx);
    void (*display_level_up_hint)(void *ctx, int hero_idx, int skill_group);
} DM2_V1_AdjustSkillsCallbacks;

int32_t dm2_v1_adjust_skills(
    int hero_idx, int32_t skill_id, int32_t exp_gain,
    const DM2_V1_AdjustSkillsCallbacks *cb, void *ctx);

/* ---- DM2_WOUND_PLAYER (c_hero.cpp:1496) ----
 * Queue damage against a hero (does not touch curHP directly — actual
 * HP is applied later by PROCESS_PLAYERS_DAMAGE). Validates hero_idx
 * and curHP != 0, halves/screens the wound through worn-armour defense
 * (DM2_hero_2c1d_135d) unless the "ignore armour" high bit of body_mask
 * is set, applies special per-slot reduction for slot 0 (head/vs fire),
 * slot 4 (feet/vs antimagic), slot 5 (shield/vs strength), rolls a
 * shock/stun body-status flag when the wound is large, wakes a sleeping
 * hero, and finally accumulates the wound into the hero's pending
 * damage accumulator. Returns the (possibly reduced) wound applied. */
typedef struct {
    int16_t cur_hp;
    uint16_t hero_flag;
} DM2_V1_WoundPlayerHero;

typedef struct {
    int (*hero_exists)(void *ctx, int hero_idx); /* hero_idx < heros_in_party */
    int (*is_sleeping)(void *ctx);               /* ddat.v1e0240 (global sleep lock) */
    int32_t (*hero_defense_pct)(void *ctx, int hero_idx, uint32_t body_mask, int is_used);
    int (*random)(void *ctx, int max);
    int16_t (*skill_lv)(void *ctx, int hero_idx, int skill_group, int with_bonus);
    void (*resume_from_wake)(void *ctx);
    void (*add_body_status)(void *ctx, int hero_idx, uint16_t status_bits);
    void (*accumulate_pending_damage)(void *ctx, int hero_idx, int16_t wound);
} DM2_V1_WoundPlayerCallbacks;

int16_t dm2_v1_wound_player(
    int hero_idx, DM2_V1_WoundPlayerHero *hero, int16_t wound,
    uint32_t body_mask,
    const DM2_V1_WoundPlayerCallbacks *cb, void *ctx);

/* ---- DM2_UPDATE_CHAMPIONS_STATS (c_hero.cpp:1757) ----
 * Periodic (every 0x38 ticks, capped at 0x80) pass over all living
 * heroes (skipping the "held item" phantom hero slot) recomputing MP
 * regen and other derived stats. */
typedef struct {
    int hero_count;
    int16_t (*get_stat_tick_accum)(void *ctx);
    void (*set_stat_tick_accum)(void *ctx, int16_t v);
    int (*is_held_item_slot)(void *ctx, int hero_idx); /* hero_idx+1 == ddat.v1e0288 */
    int (*hero_alive)(void *ctx, int hero_idx);
    void (*recompute_hero_stats)(void *ctx, int hero_idx);
} DM2_V1_UpdateChampionsStatsCallbacks;

void dm2_v1_update_champions_stats(
    const DM2_V1_UpdateChampionsStatsCallbacks *cb, void *ctx);

/* ---- DM2_WIELD_WEAPON (c_hero.cpp:2064) ----
 * Equip an item into a hero's hand as an attack: validates a live
 * target creature is present, checks the hero isn't facing the wrong
 * way relative to an ally in melee range (redirecting the strike into
 * "occupied" refusal), checks throw-only restriction via the creature's
 * AI spec flags, then computes attack damage via
 * dm2_v1_calc_player_attack_damage and records it as the pending combat
 * damage. Returns true on success. */
typedef struct {
    int (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int (*get_player_at_position)(void *ctx, int position);
    int (*creature_ai_throw_only)(void *ctx, int creature_idx);
    int16_t (*calc_attack_damage)(void *ctx, int hero_idx, int creature_idx,
                                   int16_t action_strength, int32_t skill_id);
    void (*set_pending_combat_damage)(void *ctx, int16_t damage);
} DM2_V1_WieldWeaponCallbacks;

int dm2_v1_wield_weapon(
    int hero_idx, int16_t x, int16_t y, int hero_partypos, int hero_absdir,
    int16_t action_strength, int require_melee,
    const DM2_V1_WieldWeaponCallbacks *cb, void *ctx);

/* ---- DM2_REMOVE_OBJECT_FROM_HAND (c_hero.cpp:2354) ----
 * Remove and return the item currently held by the mouse/cursor "hand",
 * clearing weight/flags, releasing the item-bonus effect it granted,
 * and re-linking the item's move-record back into the world at the
 * current view tile. Returns the removed item, or -1 if hand is empty. */
typedef struct {
    int16_t hand_item;      /* ddat.savegamewpc.w_00 */
    int16_t hand_weight;
    int8_t hand_flags;
} DM2_V1_RemoveFromHandState;

typedef struct {
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*process_item_bonus_release)(void *ctx, int event_hero_idx, uint16_t item);
    void (*relink_item_to_view_tile)(void *ctx, uint16_t item);
} DM2_V1_RemoveFromHandCallbacks;

int32_t dm2_v1_remove_object_from_hand_ex(
    DM2_V1_RemoveFromHandState *state, int event_hero_idx,
    const DM2_V1_RemoveFromHandCallbacks *cb, void *ctx);

/* ---- DM2_PLAYER_DEFEATED (c_hero.cpp:2636) ----
 * Handle a hero reaching 0 HP: refresh the squad-hands panel if the
 * defeated hero was the active one, zero curHP/hand cooldowns, mark the
 * "dead" flag, release any active mouse capture the dead hero held
 * (drag, taken item, or event capture), drop the hero's items to a new
 * world record at the party's tile, and refresh the champion display if
 * it was the last champion shown. */
typedef struct {
    int hero_idx;
    int is_active_hero;      /* hero_idx == curacthero - 1 */
    int is_last_visible_champion; /* hero_idx + 1 == ddat.v1e0976 */
    int is_mouse_drag_capture;    /* ddat.vcapture3 */
    int is_mouse_select_capture;  /* ddat.vcapture2 */
    int is_event_capture;         /* ddat.vcapture1 && hero_idx == event_heroidx */
} DM2_V1_PlayerDefeatedInfo;

typedef struct {
    void (*refresh_squad_hands_panel)(void *ctx);
    void (*release_mouse_capture)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*display_taken_item_name)(void *ctx, uint16_t item);
    int16_t (*get_hand_item)(void *ctx);
    void (*refresh_champion_display)(void *ctx, int mode);
    void (*drop_player_items)(void *ctx, int hero_idx);
    void (*mark_hero_dead)(void *ctx, int hero_idx);
} DM2_V1_PlayerDefeatedCallbacks;

void dm2_v1_player_defeated(
    const DM2_V1_PlayerDefeatedInfo *info,
    const DM2_V1_PlayerDefeatedCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_PLAYERS_DAMAGE (c_hero.cpp:2777) ----
 * Per-tick pass over the party: merges any pending body-status flags
 * into the hero's status, then applies any pending wound accumulated by
 * WOUND_PLAYER to curHP. If HP survives, records damagesuffered, sets
 * the "damaged" flag, and (re)arms a short (5-tick) hit-flash timer
 * (reusing the hero's existing timer if it has one). If HP would drop
 * to <=0, calls PLAYER_DEFEATED instead. */
typedef struct {
    int hero_count;
    uint16_t (*take_pending_body_status)(void *ctx, int hero_idx); /* returns & clears */
    void (*merge_body_status)(void *ctx, int hero_idx, uint16_t status_bits);
    int16_t (*take_pending_damage)(void *ctx, int hero_idx); /* returns & clears */
    int16_t (*get_cur_hp)(void *ctx, int hero_idx);
    void (*set_cur_hp)(void *ctx, int hero_idx, int16_t v);
    void (*set_damage_suffered)(void *ctx, int hero_idx, int16_t v);
    void (*mark_hero_dirty)(void *ctx, int hero_idx);
    int (*get_hero_timer)(void *ctx, int hero_idx); /* -1 if none armed */
    void (*rearm_hero_timer)(void *ctx, int hero_idx, int timer_idx, int32_t delay_ticks);
    void (*queue_hero_hit_timer)(void *ctx, int hero_idx, int32_t delay_ticks);
    void (*player_defeated)(void *ctx, int hero_idx);
} DM2_V1_ProcessPlayersDamageCallbacks;

void dm2_v1_process_players_damage(
    const DM2_V1_ProcessPlayersDamageCallbacks *cb, void *ctx);

/* ---- DM2_PLAYER_CONSUME_OBJECT (c_hero.cpp:2998) ----
 * Eat/drink/consume/use an item; the original is a large per-item-type
 * router (food, potions, scrolls, torches, etc.) gated on whether the
 * item is in-hand (item == -1, using mouse capture) or a stack slot.
 * Narrow form resolves the item category, decrements a stackable
 * charge/quantity where applicable, and delegates the effect switch to
 * a dispatch callback keyed by category. Returns the dispatch result. */
typedef struct {
    int (*dispatch)(void *ctx, int hero_idx, uint16_t item, int category);
    int (*get_item_category)(void *ctx, uint16_t item);
    int (*item_is_stackable_charge)(void *ctx, uint16_t item);
    void (*decrement_item_charge)(void *ctx, uint16_t item);
} DM2_V1_PlayerConsumeObjectCallbacks;

int dm2_v1_player_consume_object(
    int hero_idx, uint16_t item,
    const DM2_V1_PlayerConsumeObjectCallbacks *cb, void *ctx);

/* ---- DM2_CHANGE_PLAYER_POS (c_hero.cpp:4037) ----
 * Reorder party formation: the low byte packs a source slot, the high
 * bit selects "swap absolute direction too". If a drag-in-progress flag
 * (ddat.v1e00b8) is set, it swaps the dragged hero and the target-slot
 * hero's partypos (and absdir when the high bit was set); otherwise (no
 * drag) it performs the UI icon-swap animation path, which the narrow
 * form represents as a plain position swap. */
typedef struct {
    int drag_in_progress;      /* ddat.v1e00b8 != 0 */
    int drag_source_position;  /* (ddat.v1e00b8 - 1 + rotation) & 3 */
    int also_swap_direction;   /* packed_pos high bit */
} DM2_V1_ChangePlayerPosState;

typedef struct {
    int (*get_player_at_position)(void *ctx, int position);
    void (*swap_positions)(void *ctx, int hero_a, int hero_b);
    void (*set_direction)(void *ctx, int hero_idx, int8_t direction);
    int8_t (*get_party_facing)(void *ctx);
} DM2_V1_ChangePlayerPosCallbacks;

void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosState *state,
    const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx);

/* ---- DM2_hero_2c1d_1de2 (c_hero.cpp:3903) ----
 * Throw an item held by a hero: if throwing directly from the mouse
 * "hand" (hand_slot < 0), first swaps it into inventory slot 1;
 * otherwise removes the item from the given inventory slot. Computes
 * the throw strength via the attack/throw strength formula, queues the
 * launch noise, and shoots the item as a missile from the party's tile.
 * Returns 1 if an item was actually thrown. */
typedef struct {
    int16_t (*remove_from_hand)(void *ctx);
    int16_t (*remove_possession)(void *ctx, int hero_idx, int slot);
    void (*set_hand_slot1)(void *ctx, int hero_idx, int16_t item);
    int16_t (*compute_throw_strength)(void *ctx, int hero_idx, int slot);
    void (*queue_launch_noise)(void *ctx, uint16_t item, int16_t x, int16_t y);
    void (*shoot_item)(void *ctx, uint16_t item, int16_t x, int16_t y,
                       int direction, int16_t strength);
} DM2_V1_ThrowItemCallbacks;

int32_t dm2_v1_hero_2c1d_1de2(
    int hero_idx, int slot, int16_t x, int16_t y, int direction,
    const DM2_V1_ThrowItemCallbacks *cb, void *ctx);

/* ---- DM2_REVIVE_PLAYER (c_hero.cpp:957, "belongs to SELECT_CHAMPION") ----
 * Initialize a brand-new hero record of a given champion type at party
 * creation: resets item slots to empty, formation position/facing to
 * the party's current heading, copies the champion's name/HP/stamina/MP
 * (x10) and 7 base abilities (clamped to a minimum of 30) from the game
 * data table, seeds the 4 skill groups x 4 sub-skills from a
 * `0x40 << level` table and sums them into the group totals, and rolls
 * initial food/water reserves (rand()+1500). */
typedef struct {
    int16_t hp_base, stamina_base, mp_base;      /* dp[0..2] */
    int16_t ability_base[7];                      /* dp[3..9], clamped >=30 */
    int16_t skill_level[16];                      /* dp[10..25], per sub-skill */
    int (*random)(void *ctx);
    int (*find_free_position)(void *ctx, int8_t base_dir); /* returns free partypos slot */
} DM2_V1_RevivePlayerCallbacks;

typedef struct {
    uint16_t item[30];
    char name1[8];
    char name2[20];
    int16_t cur_hp, max_hp;
    int16_t cur_stamina, max_stamina;
    int16_t cur_mp, max_mp;
    int16_t ability_cur[7], ability_max[7];
    int32_t skill[4][4]; /* [group][sub] */
    uint16_t food, water;
    int8_t partypos;
    int8_t absdir;
} DM2_V1_HeroRecord;

void dm2_v1_revive_player(
    DM2_V1_HeroRecord *hero, int8_t hero_type, int8_t direction,
    const DM2_V1_RevivePlayerCallbacks *cb, void *ctx);

/* ---- DM2_SELECT_CHAMPION (c_hero.cpp:1052, "was SKW_2f3f_0343") ----
 * Add a new champion to the party at game start: bails out if a hand
 * item is captured or the party is already full (4). Switches to the
 * champion-creation map, finds the champion's spawn tile record to read
 * back its assigned hero_type/direction, calls REVIVE_PLAYER, bumps
 * party.heros_in_party, selects it as leader if it is the first
 * champion, scavenges any items the creation-tile record deposited near
 * the champion into its inventory, refreshes the champion strip, then
 * restores the previous map and recomputes the new hero's carried
 * weight. */
typedef struct {
    int already_full;          /* party.heros_in_party >= 4 */
    int hand_item_captured;    /* ddat.savegamewpc.w_00 != -1 */
    int8_t hero_type;
    int8_t direction;
} DM2_V1_SelectChampionState;

typedef struct {
    void (*change_current_map)(void *ctx, int map_id);
    void (*revive_player)(void *ctx, int8_t hero_type, int8_t direction);
    void (*select_leader)(void *ctx, int hero_idx);
    void (*scavenge_creation_tile_items)(void *ctx, int hero_idx);
    void (*refresh_champion_strip)(void *ctx);
    void (*recompute_player_weight)(void *ctx, int hero_idx);
} DM2_V1_SelectChampionCallbacks;

void dm2_v1_select_champion_narrow(
    const DM2_V1_SelectChampionState *state, int creation_map_id,
    int previous_map_id, int new_hero_idx,
    const DM2_V1_SelectChampionCallbacks *cb, void *ctx);

/* =====================================================================
 * c_engage.cpp — combat command dispatch
 * ===================================================================== */

/* ---- DM2_ENGAGE_COMMAND (c_engage.cpp:24) ----
 * Top-level combat action command dispatcher (~850 lines of routing in
 * the original). Narrow form preserves skproject's outer validation
 * gate and delegates the per-command switch (c_engage.cpp:179 onward)
 * to a dispatch callback.
 *
 * skproject packs the raw command word passed in edx as:
 *   bit 15      -> "alt hand"/queued-flag bit (vw_48 = RG1L & 0x8000)
 *   bits 0..14  -> raw command selector (vol_00.and16(0x7fff))
 * The raw selector is written into hero->handcmd[curactmode] and then
 * re-read back through DM2_QUERY_CUR_CMDSTR_ENTRY(2) as vw_54; the
 * live command id actually dispatched is vw_54, not the raw input.
 * skproject then validates: `if ((u16)(vw_54 - 1) > 0x35) goto L_fin;`
 * i.e. the resolved command id must be in [1, 0x35] (1..53 inclusive)
 * or the whole call is a no-op returning 0. Hero HP is checked first
 * (`if (hero->curHP == 0) return 0;`). */
#define DM2_V1_ENGAGE_CMD_ALT_FLAG   0x8000
#define DM2_V1_ENGAGE_CMD_MASK       0x7fff
#define DM2_V1_ENGAGE_CMD_MIN        1
#define DM2_V1_ENGAGE_CMD_MAX        0x35

typedef struct {
    /* Returns nonzero if the hero's curHP == 0 (dead/incapacitated). */
    int (*hero_is_dead)(void *ctx, int hero_idx);
    /* Writes the raw command selector into hero->handcmd[curactmode]
     * and returns the resolved command id from
     * DM2_QUERY_CUR_CMDSTR_ENTRY(2) (skproject vw_54). */
    int (*set_handcmd_and_resolve)(void *ctx, int hero_idx, int raw_cmd);
    /* Dispatches the validated command (c_engage.cpp switch body). */
    int32_t (*dispatch)(void *ctx, int hero_idx, int resolved_cmd, int alt_flag);
} DM2_V1_EngageCommandCallbacks;

int32_t dm2_v1_engage_command_narrow(
    int hero_idx, int command_id,
    const DM2_V1_EngageCommandCallbacks *cb, void *ctx);

/* =====================================================================
 * c_creature.cpp — creature AI ops
 * ===================================================================== */

/* ---- DM2_WOUND_CREATURE (c_creature.cpp:165) ----
 * Apply damage to a creature record, matching skproject's HP-clamp,
 * death and flee-check logic:
 *  - defense_type == 0xff marks a record with no HP (immune/inanimate);
 *    always returns 0.
 *  - kill_flag & 1 selects the "instant delete" record kind (word@0 bit0
 *    in skproject): when set, lethal damage fully deletes the record and
 *    the function returns 1; when clear, lethal damage zeroes HP and
 *    plays the death animation/AI transition but the record itself is
 *    not deleted here (return 0).
 *  - ai_flags1 & 0x4 gates a cross-map "wake" notification sent when the
 *    creature is hit while off the party's current map.
 *  - ai_flags1 & 0x8 gates a "can this creature actually die" check
 *    (bosses / plot creatures) plus a wander timeout reset on death.
 *  - ai_flags1 & 0x80 gates a flee roll when damage is non-lethal:
 *    RAND()&7==0 always flees; otherwise a wound-percentage threshold
 *    check (3*maxhp/100 vs current hp, and 5*curhp_word/100) decides,
 *    falling back to an aggression-flag / random-direction check. */
typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    uint8_t defense_type;   /* byte@0x2 of the creature/ai record; 0xff = no HP */
    uint8_t kill_flag;      /* word@0x0 bit0: 1 = instant-delete record kind */
    uint8_t ai_flags1;      /* AI spec flags byte: bit2 map-wake, bit3 boss,
                              * bit7 can-flee */
} DM2_V1_WoundCreatureState;

typedef struct {
    /* Cross-map hit notification (word bit0==0, ai_flags1&4==0). */
    void (*notify_cross_map_wake)(void *ctx, int creature_idx);
    /* ai_flags1&8: query whether this creature is allowed to actually
     * die right now (skproject DM2_1c9a_17c7); returning false aborts
     * the kill entirely (creature survives at 0 recorded damage). */
    int (*can_creature_die)(void *ctx, int creature_idx);
    /* ai_flags1&8 death path: reset the creature's wander/AI timeout
     * (skproject ddat.v1e0526 = 0xa0). */
    void (*set_ai_wander_timeout)(void *ctx, int creature_idx, int timeout);
    /* Non-deleting death path: play death animation / AI retreat state
     * (skproject DM2_ai_13e4_0360(..., 0x13, 1)). */
    void (*play_death_animation)(void *ctx, int creature_idx);
    /* Deleting death path: fully remove the creature record
     * (skproject DM2_DELETE_CREATURE_RECORD). */
    void (*creature_defeated)(void *ctx, int creature_idx);
    /* Flee roll: returns nonzero chance-of-flee RNG (RAND()&7). */
    int (*random)(void *ctx, int max);
    /* Flee roll: queue the "creature flees" noise/animation. */
    void (*queue_flee_noise)(void *ctx, int creature_idx);
} DM2_V1_WoundCreatureCallbacks;

int32_t dm2_v1_wound_creature(
    int creature_idx, DM2_V1_WoundCreatureState *creature, int16_t damage,
    const DM2_V1_WoundCreatureCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ATTACKS_PLAYER (c_creature.cpp:651) ----
 * Resolve a creature melee attack against a hero: computes to-hit from
 * the creature's AI attack spec, hero dexterity and a d32 roll (or an
 * automatic hit for asleep/surprise attacks and attack types 8/9), then
 * on a hit rolls damage via a 3-stage random cascade and applies it via
 * wound_player, optionally triggering poison. Returns the actual damage
 * applied to the hero (0 on a miss), matching skproject's RG5L return. */
typedef struct {
    int16_t to_hit_base;    /* ai spec byte@0x8: base to-hit value */
    int attack_type;        /* ai spec byte@0x1c: attack/weapon type */
    int16_t max_damage;     /* ai spec byte@0x6: base damage roll cap */
    int16_t poison_chance;  /* ai spec byte@0x7: poison-on-hit chance, 0 = none */
    int is_surprise_attack; /* ddat.v1e0238 != 0: asleep party, auto-hit */
} DM2_V1_CreatureAttackProfile;

typedef struct {
    int (*get_attack_profile)(void *ctx, int creature_idx,
                              DM2_V1_CreatureAttackProfile *out);
    int (*get_hero_dexterity)(void *ctx, int hero_idx);
    int (*hero_use_luck)(void *ctx, int hero_idx, int chance);
    int (*get_player_fight_skill)(void *ctx, int hero_idx);
    int (*random)(void *ctx, int max);
    int (*rand_bit)(void *ctx);
    int (*rand_dir)(void *ctx);
    int16_t (*wound_player)(void *ctx, int hero_idx, int16_t damage);
    void (*queue_hit_noise)(void *ctx, int creature_idx, int hero_idx,
                            int16_t damage);
    int (*hero_resist_poison)(void *ctx, int hero_idx, int16_t poison_chance);
    void (*process_poison)(void *ctx, int hero_idx, int amount);
    void (*resume_from_wake)(void *ctx);
} DM2_V1_CreatureAttacksPlayerCallbacks;

int32_t dm2_v1_creature_attacks_player(
    int creature_idx, int hero_idx,
    const DM2_V1_CreatureAttacksPlayerCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ATTACKS_CREATURE (c_creature.cpp:906) ----
 * Resolve a creature-vs-creature melee attack: looks up the defender via
 * get_creature_at, aborts if the defender is invulnerable (ai spec
 * byte@0x8 == 0xff), rolls to-hit (attacker roll vs defender roll, both
 * RAND()&0x1f, with a random-direction fallback check), then on a hit
 * rolls damage via the same 3-stage cascade as CREATURE_ATTACKS_PLAYER
 * and forwards it to DM2_ATTACK_CREATURE. Returns the rolled damage
 * amount (0 on a miss, -1 if no defender is present). */
typedef struct {
    int (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int (*get_to_hit)(void *ctx, int creature_idx);      /* ai spec byte@0x8 */
    int (*get_defense)(void *ctx, int defender_idx);     /* ai spec byte@0x8 */
    int (*get_max_damage)(void *ctx, int attacker_idx);  /* ai spec byte@0x6 */
    int (*random)(void *ctx, int max);
    int (*rand_bit)(void *ctx);
    int (*rand_dir)(void *ctx);
    void (*apply_creature_damage)(void *ctx, int attacker_idx, int defender_idx,
                                  int16_t damage);
} DM2_V1_CreatureAttacksCreatureCallbacks;

int32_t dm2_v1_creature_attacks_creature(
    int attacker_idx, int16_t target_x, int16_t target_y,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CAN_HANDLE_IT (c_creature.cpp:1344) ----
 * Check whether a creature type can pick up/handle an item's class.
 * item classes 0x3e/0x3f are sentinels (empty slot / illegal) and never
 * handled. Class 0x29 (container) gets special moneybox handling: a
 * moneybox creatures cannot loot is only takeable once already opened
 * (flags & 0x80, i.e. a forced pickpocket/steal), otherwise the regular
 * per-type handle_mask bit for the item's class decides. */
typedef struct {
    uint32_t handle_mask;   /* bit N set => creature type can handle class N */
} DM2_V1_CreatureHandleCaps;

typedef struct {
    int (*is_container_moneybox)(void *ctx, uint16_t item);
    int (*moneybox_already_opened)(void *ctx, uint16_t item);
} DM2_V1_CreatureCanHandleCallbacks;

int32_t dm2_v1_creature_can_handle_it(
    const DM2_V1_CreatureHandleCaps *caps, uint16_t item, int flags,
    const DM2_V1_CreatureCanHandleCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CAST_SPELL (c_creature.cpp:1865) ----
 * A creature casts an offensive spell: rolls a 3-stage additive damage
 * cascade (d = base/4+1; d += rand(d); d += rand(d)), scales it per
 * spell-item id (some ids *4, a range <<3), clips to [4,255], buckets
 * it into a discrete power tier (1/2/3/7 depending on thresholds
 * 8/16/32), shoots the spell item, and — unless the "no recoil" flag is
 * set — applies self-inflicted recoil damage to the caster via
 * WOUND_CREATURE. Returns 1 if recoil damage was applied. */
typedef struct {
    uint16_t spell_item;   /* ai spec byte@0x1e: spell/item id to shoot */
    int direction;         /* creature byte@0x1c */
    int16_t base_power;    /* creature byte@0x1b: base damage input */
    int8_t speed;          /* creature byte@0x8 */
    int no_recoil;         /* creature flags byte@0x9 bit 0x10 */
} DM2_V1_CreatureSpellProfile;

typedef struct {
    int (*get_spell_profile)(void *ctx, int creature_idx,
                             DM2_V1_CreatureSpellProfile *out);
    int (*random)(void *ctx, int max);
    int (*rand_bit)(void *ctx);
    int (*rand_dir)(void *ctx);
    void (*shoot_item)(void *ctx, int creature_idx, uint16_t spell_item,
                       int direction, int8_t damage, int8_t speed,
                       int8_t power);
    int32_t (*wound_self)(void *ctx, int creature_idx, int16_t recoil_damage);
    void (*mark_creature_dirty)(void *ctx, int creature_idx);
} DM2_V1_CreatureCastSpellCallbacks;

int32_t dm2_v1_creature_cast_spell(
    int creature_idx, const DM2_V1_CreatureCastSpellCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_STEAL_FROM_CHAMPION (c_creature.cpp:1972) ----
 * A creature steals an item from a champion sharing its tile: aborts
 * unless the creature is on the party's current position, rolls a
 * dexterity-based luck check (100 - dexterity vs hero_use_luck), then
 * weighs the hero's two hand items by CREATURE_CAN_HANDLE_IT + item
 * weight and steals the heavier eligible one (falling back to the other
 * hand, or nothing). On success, appends the item to the creature's
 * inventory and, if the party was surprised, may resume it from wake.
 * Returns 1 on failure to steal, 0 on a successful theft (matching
 * skproject's inverted RG6l-based return). */
typedef struct {
    int (*same_tile_as_creature)(void *ctx, int creature_idx);
    int (*get_player_at_creature_position)(void *ctx, int creature_idx);
    int (*get_hero_dexterity)(void *ctx, int hero_idx);
    int (*hero_use_luck)(void *ctx, int hero_idx, int chance);
    int (*random)(void *ctx, int max);
    int (*get_hero_hand_item)(void *ctx, int hero_idx, int hand_slot);
    int (*creature_can_handle_it)(void *ctx, int creature_idx, uint16_t item);
    int (*get_item_weight)(void *ctx, uint16_t item);
    int32_t (*remove_possession)(void *ctx, int hero_idx, int hand_slot);
    void (*give_creature_item)(void *ctx, int creature_idx, int32_t item);
    int (*is_party_surprised)(void *ctx);
    void (*clear_party_surprised)(void *ctx);
    int (*get_player_skill_lv)(void *ctx, int hero_idx, int skill);
    void (*resume_from_wake)(void *ctx);
} DM2_V1_CreatureStealCallbacks;

int32_t dm2_v1_creature_steal_from_champion(
    int creature_idx, const DM2_V1_CreatureStealCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CCM0B (c_creature.cpp:2145) ----
 * Creature combat-manager state 0B step: try to path to the tracked
 * target tile; if blocked, stay in this state (return 1). If reached
 * but the tile kind isn't the expected sensor kind (0xb), also stay
 * (return 1). Once on the sensor tile, invoke the tile's message event
 * and advance out of this state (return 0). */
typedef struct {
    int (*go_there)(void *ctx, int creature_idx);
    int (*get_target_tile_kind)(void *ctx, int creature_idx);
    void (*invoke_message)(void *ctx, int creature_idx, int event_id);
} DM2_V1_CreatureCcm0bCallbacks;

int32_t dm2_v1_creature_ccm0b(
    int creature_idx, const DM2_V1_CreatureCcm0bCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CCM0C (c_creature.cpp:2247) ----
 * Creature combat-manager state 0C step: a two-phase "approach then
 * pick up items" state driven by a phase counter (byte@0x1f). Phase 0
 * re-runs CCM06 (facing/approach) and arms the item-pickup target;
 * phase 1 re-runs CCM06 and then attempts CREATURE_TAKES_ITEM, whose
 * result becomes the step's return value. The phase counter is always
 * incremented afterwards. */
typedef struct {
    uint8_t (*get_phase)(void *ctx, int creature_idx);
    void (*ccm06)(void *ctx, int creature_idx);
    void (*arm_item_pickup_target)(void *ctx, int creature_idx);
    int32_t (*takes_item)(void *ctx, int creature_idx);
    void (*advance_phase)(void *ctx, int creature_idx);
} DM2_V1_CreatureCcm0cCallbacks;

int32_t dm2_v1_creature_ccm0c(
    int creature_idx, const DM2_V1_CreatureCcm0cCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_USES_LADDER_HOLE (c_creature.cpp:1709) ----
 * Move a creature through a ladder/hole/pit tile: first tries to path
 * onto the tile (go_there); bails (return 1) if blocked. Checks the
 * tile-flags table for "is traversable hole" (table1d613a bit 0x4);
 * bails if not set. For ladder tile kinds (0x39 up / 0x3a down) finds
 * the matching landing spot on the destination level via
 * find_ladder_landing, falling back to a random direction if none is
 * found; pit tile kinds (0x35/0x36) query the destination level
 * directly. Operates any pit/teleport tile the creature is leaving,
 * moves the record to the destination level, and on success repositions
 * the creature from the resolved landing coordinates and operates the
 * landing tile too; decrements the creature's pending-move counter.
 * Returns 1 if the move did not happen, 0 on success. */
typedef struct {
    int (*go_there)(void *ctx, int creature_idx);
    int (*get_tile_traversable)(void *ctx, int creature_idx); /* table1d613a & 4 */
    int (*get_tile_kind)(void *ctx, int creature_idx);
    int (*is_leaving_pit_tele_tile)(void *ctx, int creature_idx);
    void (*operate_pit_tele_tile)(void *ctx, int16_t x, int16_t y, int entering);
    /* Resolve the destination level + landing (x,y,dir) for a ladder;
     * returns 0 if no matching landing tile exists on the other level. */
    int (*find_ladder_landing)(void *ctx, int creature_idx, int going_up,
                               int *out_level, int16_t *out_x, int16_t *out_y,
                               int *out_dir);
    int (*rand_dir)(void *ctx);
    /* Move the creature's record to the resolved destination level;
     * returns 0 on success, nonzero if the destination is full/blocked. */
    int32_t (*move_creature_to_level)(void *ctx, int creature_idx, int level,
                                      int16_t x, int16_t y, int dir);
    void (*reposition_from_landing)(void *ctx, int creature_idx);
    void (*decrement_pending_move_counter)(void *ctx, int creature_idx);
    void (*mark_creature_dirty)(void *ctx, int creature_idx);
} DM2_V1_CreatureUsesLadderHoleCallbacks;

int32_t dm2_v1_creature_uses_ladder_hole(
    int creature_idx, const DM2_V1_CreatureUsesLadderHoleCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_WALK_NOW (c_creature.cpp:2845) ----
 * Resolve one creature movement step: if the creature's tile is marked
 * "party present", it attacks the party first (CREATURE_ATTACKS_PARTY)
 * after forcing an attack pose. Then tries to path to the tracked
 * target tile (go_there); bails (return 1) if blocked. Checks the
 * tile-flags table for "walkable" (table1d613a bit 0x4); bails if not
 * set. Operates any pit/teleport tile being left, moves the creature's
 * record to the target tile, and on success repositions the creature
 * from the resolved landing coordinates, operates the landing tile too,
 * and decrements the pending-move counter. Returns 1 if the creature
 * did not move this tick, 0 if it did. */
typedef struct {
    int (*is_party_on_tile)(void *ctx, int creature_idx);
    void (*force_attack_pose)(void *ctx, int creature_idx);
    void (*attacks_party)(void *ctx, int creature_idx);
    int (*go_there)(void *ctx, int creature_idx);
    int (*get_tile_walkable)(void *ctx, int creature_idx); /* table1d613a & 4 */
    int (*is_leaving_pit_tele_tile)(void *ctx, int creature_idx);
    void (*operate_pit_tele_tile)(void *ctx, int16_t x, int16_t y, int entering);
    int32_t (*move_creature_to_target)(void *ctx, int creature_idx);
    void (*reposition_from_landing)(void *ctx, int creature_idx);
    void (*decrement_pending_move_counter)(void *ctx, int creature_idx);
    void (*mark_creature_dirty)(void *ctx, int creature_idx);
} DM2_V1_CreatureWalkNowCallbacks;

int32_t dm2_v1_creature_walk_now(
    int creature_idx, const DM2_V1_CreatureWalkNowCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ACTIVATES_WALL (c_creature.cpp:2564) ----
 * A creature triggers a wall mechanism (lever/button) by proximity:
 * checks the creature can reach/face the wall tile (go_there variant
 * with the "wall proximity" flag); bails (return 1) if not. Unless the
 * activation kind is 2 ("no item needed") and the creature is carrying
 * no item, tries to pick up an item to use on the mechanism first,
 * bailing (return 1) if none is available. Dispatches the wall event
 * with the resolved item (or -1). Returns 0 on success. */
typedef struct {
    int (*go_there_wall)(void *ctx, int creature_idx);
    int (*get_activation_kind)(void *ctx, int creature_idx); /* byte@0x20 */
    int (*get_held_item)(void *ctx, int creature_idx);       /* byte@0x1e */
    int (*pick_up_item_for_activation)(void *ctx, int creature_idx);
    void (*activate_wall_event)(void *ctx, int creature_idx, int32_t item);
} DM2_V1_CreatureActivatesWallCallbacks;

int32_t dm2_v1_creature_activates_wall(
    int creature_idx, const DM2_V1_CreatureActivatesWallCallbacks *cb, void *ctx);

/* ---- DM2_PLACE_MERCHANDISE (c_creature.cpp:2411) ----
 * A merchant creature (ai spec flags bit0 set) offloads items from its
 * carried inventory onto its shop tile, one at a time, looping while
 * the creature's "bulk transfer" flag (byte@0x1e bit 0x40) stays set.
 * Moneybox containers get flagged as "already opened" once placed. The
 * first placed item queues a shop noise. Returns 1 if nothing was
 * placed (not a merchant, or no eligible items), 0 if at least one item
 * was placed. */
typedef struct {
    int (*is_merchant)(void *ctx, int creature_idx);
    int32_t (*next_inventory_item)(void *ctx, int creature_idx); /* -1 = none left */
    int (*bulk_transfer_active)(void *ctx, int creature_idx);
    int (*is_container_moneybox)(void *ctx, int32_t item);
    void (*mark_moneybox_opened)(void *ctx, int32_t item);
    void (*place_item_on_shop_tile)(void *ctx, int creature_idx, int32_t item);
    void (*queue_shop_noise)(void *ctx, int creature_idx, int32_t item);
} DM2_V1_PlaceMerchandiseCallbacks;

int32_t dm2_v1_place_merchandise(
    int creature_idx, const DM2_V1_PlaceMerchandiseCallbacks *cb, void *ctx);

/* ---- DM2_TAKE_MERCHANDISE (c_creature.cpp:2509) ----
 * A merchant creature (ai spec flags bit0 set) reclaims unsold
 * merchandise from its shop tile back into its own inventory, looping
 * while the creature's "bulk transfer" flag stays set. Returns 1 if
 * nothing was taken back, 0 if at least one item was reclaimed. */
typedef struct {
    int (*is_merchant)(void *ctx, int creature_idx);
    int32_t (*next_shop_item)(void *ctx, int creature_idx); /* -1 = none left */
    int (*bulk_transfer_active)(void *ctx, int creature_idx);
    void (*take_item_into_inventory)(void *ctx, int creature_idx, int32_t item);
} DM2_V1_TakeMerchandiseCallbacks;

int32_t dm2_v1_take_merchandise(
    int creature_idx, const DM2_V1_TakeMerchandiseCallbacks *cb, void *ctx);

/* =====================================================================
 * c_item.cpp — item ops
 * ===================================================================== */

/* ---- DM2_MOVE_ITEM_TO (c_item.cpp:554) ----
 * Move an item record to a new dungeon tile position. */
typedef struct {
    void (*unlink_from_source)(void *ctx, uint16_t item);
    void (*link_to_tile)(void *ctx, uint16_t item, int16_t x, int16_t y);
} DM2_V1_MoveItemToCallbacks;

void dm2_v1_move_item_to(
    uint16_t item, int16_t x, int16_t y, int16_t flags,
    const DM2_V1_MoveItemToCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_ITEM_TELEPORT (c_item.cpp:651) ----
 * Item teleporter mechanism (~320 lines of routing in the original).
 * Narrow form delegates the full logic to a dispatch callback. */
typedef struct {
    int32_t (*dispatch)(void *ctx, uint16_t item, int16_t src_x, int16_t src_y,
                        int16_t dst_x, int16_t dst_y, int32_t mode);
} DM2_V1_ActivateItemTeleportCallbacks;

int32_t dm2_v1_activate_item_teleport_narrow(
    uint16_t item, int16_t src_x, int16_t src_y,
    int16_t dst_x, int16_t dst_y, int32_t mode,
    const DM2_V1_ActivateItemTeleportCallbacks *cb, void *ctx);

/* ---- DM2_SHOOT_ITEM (c_item.cpp:1043) ----
 * Launch a projectile item from a tile in a direction. */
typedef struct {
    void (*spawn_missile)(void *ctx, uint16_t item, int16_t x, int16_t y,
                          int direction, int8_t damage, int8_t speed,
                          int8_t power);
} DM2_V1_ShootItemCallbacks;

void dm2_v1_shoot_item(
    uint16_t item, int16_t x, int16_t y, int direction,
    int8_t damage, int8_t speed, int8_t power,
    const DM2_V1_ShootItemCallbacks *cb, void *ctx);

/* =====================================================================
 * c_moverec.cpp — move-record ops
 * ===================================================================== */

/* ---- DM2_TRY_PUSH_OBJECT_TO (c_moverec.cpp:25) ----
 * Attempt to push a movable record onto a target tile.
 * Returns 1 if the push succeeded, writing the resolved coords. */
typedef struct {
    int (*is_tile_free)(void *ctx, int16_t x, int16_t y);
    void (*move_record_to)(void *ctx, int32_t record, int16_t x, int16_t y);
} DM2_V1_TryPushObjectToCallbacks;

int32_t dm2_v1_try_push_object_to(
    int32_t record, int16_t dst_x, int16_t dst_y,
    int16_t *out_x, int16_t *out_y,
    const DM2_V1_TryPushObjectToCallbacks *cb, void *ctx);

/* ---- DM2_2fcf_0234 (c_moverec.cpp:144) ----
 * Move-record helper: relink a record between tile lists. */
typedef struct {
    void (*unlink_record)(void *ctx, int32_t record, int16_t x, int16_t y);
    void (*link_record)(void *ctx, int32_t record, int16_t dst_x, int16_t dst_y);
} DM2_V1_Moverec2fcf0234Callbacks;

void dm2_v1_moverec_2fcf_0234(
    int32_t record, int16_t src_x, int16_t src_y,
    int16_t dst_x, int16_t dst_y,
    const DM2_V1_Moverec2fcf0234Callbacks *cb, void *ctx);

/* ---- DM2_moverec_3CE7D (c_moverec.cpp:1147) ----
 * Move-record dispatcher for a specific record kind/flags combination. */
typedef struct {
    int32_t (*dispatch)(void *ctx, int32_t record, int16_t x, int16_t y,
                        int32_t kind, int32_t flags);
} DM2_V1_Moverec3ce7dCallbacks;

void dm2_v1_moverec_3ce7d(
    int32_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    const DM2_V1_Moverec3ce7dCallbacks *cb, void *ctx);

/* =====================================================================
 * c_record.cpp — record pool management
 * ===================================================================== */

/* ---- DM2_RECYCLE_A_RECORD_FROM_THE_WORLD (c_record.cpp:544) ----
 * Reclaim the oldest/least-important world record for reuse.
 * Returns the recycled record index, or -1 if none available. */
typedef struct {
    int record_count;
    int (*get_record_importance)(void *ctx, int idx);
    void (*free_record)(void *ctx, int idx);
} DM2_V1_RecycleRecordCallbacks;

int32_t dm2_v1_recycle_a_record_from_the_world(
    int32_t requested_kind, const DM2_V1_RecycleRecordCallbacks *cb, void *ctx);

/* ---- DM2_DELETE_MISSILE_RECORD (c_record.cpp:1427) ----
 * Remove a missile projectile record from the world. */
typedef struct {
    void (*unlink_missile)(void *ctx, int32_t record, int16_t x, int16_t y);
    void (*free_record)(void *ctx, int32_t record);
} DM2_V1_DeleteMissileRecordCallbacks;

void dm2_v1_delete_missile_record(
    int32_t record, int16_t x, int16_t y,
    const DM2_V1_DeleteMissileRecordCallbacks *cb, void *ctx);

/* =====================================================================
 * c_light.cpp — light ops
 * ===================================================================== */

/* ---- DM2_ADD_BACKGROUND_LIGHT_FROM_TILE (c_light.cpp:202) ----
 * Accumulate a tile's static light contribution into the light map. */
typedef struct {
    int16_t (*get_tile_light)(void *ctx, int16_t x, int16_t y);
    void (*add_light)(void *ctx, int16_t x, int16_t y, int16_t amount);
} DM2_V1_AddBackgroundLightCallbacks;

void dm2_v1_add_background_light_from_tile(
    int16_t x, int16_t y, int16_t radius,
    const DM2_V1_AddBackgroundLightCallbacks *cb, void *ctx);

/* ---- DM2_CHECK_RECOMPUTE_LIGHT (c_light.cpp:490) ----
 * Check the dirty flag and recompute the light map if needed.
 * Returns 1 if a recompute was performed. */
typedef struct {
    int (*is_light_dirty)(void *ctx);
    void (*recompute_light_map)(void *ctx);
    void (*clear_light_dirty)(void *ctx);
} DM2_V1_CheckRecomputeLightCallbacks;

int32_t dm2_v1_check_recompute_light(
    const DM2_V1_CheckRecomputeLightCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RUNTIME_NARROW_PC34_COMPAT_H */
