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

void dm2_v1_continue_ornate_noise(
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

void dm2_v1_activate_shooter(
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
 * Compute a champion's melee/ranged attack damage. */
typedef struct {
    int16_t cur_hp;
    int16_t skill_fighter;
    int16_t skill_ninja;
} DM2_V1_AttackDamageHero;

int16_t dm2_v1_calc_player_attack_damage(
    const DM2_V1_AttackDamageHero *hero, int16_t weapon_damage,
    int16_t action_strength, int16_t to_hit_bonus, int16_t stealth_bonus);

/* ---- DM2_hero_39796 (c_hero.cpp:464) ----
 * Per-tick hero regeneration/stat update pass. */
typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    int16_t cur_stamina;
    int16_t max_stamina;
    uint16_t hero_flag;
} DM2_V1_HeroRegenState;

void dm2_v1_hero_39796(DM2_V1_HeroRegenState *hero);

/* ---- DM2_hero_2c1d_135d (c_hero.cpp:1403) ----
 * Compute skill-experience gain for an action. */
int dm2_v1_hero_2c1d_135d(int16_t skill_value, int16_t action_difficulty);

/* ---- DM2_ADJUST_SKILLS (c_hero.cpp:1166) ----
 * Grant experience to a hero's skill, updating level thresholds.
 * Returns the new skill level. */
typedef struct {
    int16_t *skill_exp;
    int16_t *skill_level;
    const int16_t *level_thresholds;
    int level_threshold_count;
} DM2_V1_AdjustSkillsState;

int32_t dm2_v1_adjust_skills(
    int hero_idx, DM2_V1_AdjustSkillsState *state, int32_t exp_gain);

/* ---- DM2_WOUND_PLAYER (c_hero.cpp:1496) ----
 * Apply damage to a hero, clamping HP and triggering death handling.
 * Returns actual damage applied. */
typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    uint16_t hero_flag;
} DM2_V1_WoundPlayerHero;

typedef struct {
    void (*player_defeated)(void *ctx, int hero_idx);
} DM2_V1_WoundPlayerCallbacks;

int16_t dm2_v1_wound_player(
    int hero_idx, DM2_V1_WoundPlayerHero *hero, int16_t damage, int flags,
    const DM2_V1_WoundPlayerCallbacks *cb, void *ctx);

/* ---- DM2_UPDATE_CHAMPIONS_STATS (c_hero.cpp:1757) ----
 * Iterate all heroes and refresh derived stats (load, aura, etc). */
typedef struct {
    int hero_count;
    void (*recompute_hero_stats)(void *ctx, int hero_idx);
} DM2_V1_UpdateChampionsStatsCallbacks;

void dm2_v1_update_champions_stats(
    const DM2_V1_UpdateChampionsStatsCallbacks *cb, void *ctx);

/* ---- DM2_WIELD_WEAPON (c_hero.cpp:2064) ----
 * Equip a weapon into a hero's hand slot.
 * Returns true on success. */
typedef struct {
    int (*is_item_fit)(void *ctx, uint16_t item, int hand_slot);
    void (*equip_item)(void *ctx, int hero_idx, uint16_t item, int hand_slot);
} DM2_V1_WieldWeaponCallbacks;

int dm2_v1_wield_weapon(
    int hero_idx, uint16_t item, int hand_slot, int forced,
    const DM2_V1_WieldWeaponCallbacks *cb, void *ctx);

/* ---- DM2_REMOVE_OBJECT_FROM_HAND (c_hero.cpp:2354) ----
 * Remove and return the item currently held by the cursor/hand. */
typedef struct {
    int16_t *hand_item;
} DM2_V1_RemoveFromHandState;

int32_t dm2_v1_remove_object_from_hand(DM2_V1_RemoveFromHandState *state);

/* ---- DM2_PLAYER_DEFEATED (c_hero.cpp:2636) ----
 * Handle a hero reaching 0 HP: drop items, mark dead, check party wipe. */
typedef struct {
    void (*drop_player_items)(void *ctx, int hero_idx);
    void (*mark_hero_dead)(void *ctx, int hero_idx);
    int (*count_living_heroes)(void *ctx);
    void (*trigger_game_over)(void *ctx);
} DM2_V1_PlayerDefeatedCallbacks;

void dm2_v1_player_defeated(
    int hero_idx, const DM2_V1_PlayerDefeatedCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_PLAYERS_DAMAGE (c_hero.cpp:2777) ----
 * Iterate all heroes and apply queued/pending damage. */
typedef struct {
    int hero_count;
    int16_t (*get_pending_damage)(void *ctx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int16_t damage);
    void (*clear_pending_damage)(void *ctx, int hero_idx);
} DM2_V1_ProcessPlayersDamageCallbacks;

void dm2_v1_process_players_damage(
    const DM2_V1_ProcessPlayersDamageCallbacks *cb, void *ctx);

/* ---- DM2_PLAYER_CONSUME_OBJECT (c_hero.cpp:2998) ----
 * Eat/drink/consume an item; complex per-item-type routing.
 * Delegates to a dispatch callback keyed by item category. */
typedef struct {
    int (*dispatch)(void *ctx, int hero_idx, uint16_t item, int category);
    int (*get_item_category)(void *ctx, uint16_t item);
} DM2_V1_PlayerConsumeObjectCallbacks;

int dm2_v1_player_consume_object(
    int hero_idx, uint16_t item,
    const DM2_V1_PlayerConsumeObjectCallbacks *cb, void *ctx);

/* ---- DM2_CHANGE_PLAYER_POS (c_hero.cpp:4037) ----
 * Change party formation position, packed hero/position argument. */
typedef struct {
    void (*swap_positions)(void *ctx, int hero_idx, uint8_t new_pos);
} DM2_V1_ChangePlayerPosCallbacks;

void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx);

/* ---- DM2_hero_2c1d_1de2 (c_hero.cpp:3903) ----
 * Compute a hero's saving-throw style resistance check.
 * Returns 1 if the hero resists. */
int32_t dm2_v1_hero_2c1d_1de2(int hero_idx, int16_t resist_stat, int16_t power);

/* ---- DM2_REVIVE_PLAYER (c_hero.cpp:957) ----
 * Create a fresh hero of the given type/direction at game start. */
typedef struct {
    void (*init_hero_stats)(void *ctx, int8_t hero_type);
    void (*set_hero_direction)(void *ctx, int8_t direction);
} DM2_V1_RevivePlayerCallbacks;

void dm2_v1_revive_player(
    int8_t hero_type, int8_t direction,
    const DM2_V1_RevivePlayerCallbacks *cb, void *ctx);

/* ---- DM2_SELECT_CHAMPION (c_hero.cpp:1052) ----
 * Select a champion panel/state by index and mode. */
typedef struct {
    void (*set_selected_champion)(void *ctx, int hero_idx);
    void (*refresh_panel)(void *ctx, int mode);
} DM2_V1_SelectChampionCallbacks;

void dm2_v1_select_champion(
    int hero_idx, int panel_slot, int mode,
    const DM2_V1_SelectChampionCallbacks *cb, void *ctx);

/* =====================================================================
 * c_engage.cpp — combat command dispatch
 * ===================================================================== */

/* ---- DM2_ENGAGE_COMMAND (c_engage.cpp:24) ----
 * Top-level combat action command dispatcher (~850 lines of routing in
 * the original). Narrow form delegates the entire switch to a dispatch
 * callback keyed by command id, and returns the callback's result. */
typedef struct {
    int32_t (*dispatch)(void *ctx, int hero_idx, int command_id);
} DM2_V1_EngageCommandCallbacks;

int32_t dm2_v1_engage_command(
    int hero_idx, int command_id,
    const DM2_V1_EngageCommandCallbacks *cb, void *ctx);

/* =====================================================================
 * c_creature.cpp — creature AI ops
 * ===================================================================== */

/* ---- DM2_WOUND_CREATURE (c_creature.cpp:165) ----
 * Apply damage to a creature record. Returns actual damage applied. */
typedef struct {
    int16_t cur_hp;
} DM2_V1_WoundCreatureState;

typedef struct {
    void (*creature_defeated)(void *ctx, int creature_idx);
} DM2_V1_WoundCreatureCallbacks;

int32_t dm2_v1_wound_creature(
    int creature_idx, DM2_V1_WoundCreatureState *creature, int16_t damage,
    const DM2_V1_WoundCreatureCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ATTACKS_PLAYER (c_creature.cpp:651) ----
 * Resolve a creature melee attack against a hero.
 * Returns 1 if the attack hit. */
typedef struct {
    int (*roll_to_hit)(void *ctx, int creature_idx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int16_t damage);
} DM2_V1_CreatureAttacksPlayerCallbacks;

int32_t dm2_v1_creature_attacks_player(
    int creature_idx, int hero_idx, int16_t damage,
    const DM2_V1_CreatureAttacksPlayerCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ATTACKS_CREATURE (c_creature.cpp:906) ----
 * Resolve a creature-vs-creature melee attack. */
typedef struct {
    int (*roll_to_hit)(void *ctx, int attacker_idx, int defender_idx);
    void (*wound_creature)(void *ctx, int defender_idx, int16_t damage);
} DM2_V1_CreatureAttacksCreatureCallbacks;

int32_t dm2_v1_creature_attacks_creature(
    int attacker_idx, int defender_idx, int16_t damage,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CAN_HANDLE_IT (c_creature.cpp:1344) ----
 * Check whether a creature type can pick up/handle an item type. */
typedef struct {
    uint32_t handle_mask;
} DM2_V1_CreatureHandleCaps;

int32_t dm2_v1_creature_can_handle_it(
    const DM2_V1_CreatureHandleCaps *caps, int item_class);

/* ---- DM2_CREATURE_CAST_SPELL (c_creature.cpp:1865) ----
 * A creature casts an offensive/defensive spell. */
typedef struct {
    void (*cast_spell)(void *ctx, int creature_idx, int spell_id);
} DM2_V1_CreatureCastSpellCallbacks;

int32_t dm2_v1_creature_cast_spell(
    int creature_idx, int spell_id,
    const DM2_V1_CreatureCastSpellCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_STEAL_FROM_CHAMPION (c_creature.cpp:1972) ----
 * A creature steals an item from a champion's inventory. */
typedef struct {
    int32_t (*pick_random_item_slot)(void *ctx, int hero_idx);
    int32_t (*remove_possession)(void *ctx, int hero_idx, int slot);
    void (*give_creature_item)(void *ctx, int creature_idx, int32_t item);
} DM2_V1_CreatureStealCallbacks;

int32_t dm2_v1_creature_steal_from_champion(
    int creature_idx, int hero_idx,
    const DM2_V1_CreatureStealCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CCM0B (c_creature.cpp:2145) ----
 * Creature combat-manager state 0B step. */
typedef struct {
    int32_t (*step)(void *ctx, int creature_idx);
} DM2_V1_CreatureCcm0bCallbacks;

int32_t dm2_v1_creature_ccm0b(
    int creature_idx, const DM2_V1_CreatureCcm0bCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CCM0C (c_creature.cpp:2247) ----
 * Creature combat-manager state 0C step. */
typedef struct {
    int32_t (*step)(void *ctx, int creature_idx);
} DM2_V1_CreatureCcm0cCallbacks;

int32_t dm2_v1_creature_ccm0c(
    int creature_idx, const DM2_V1_CreatureCcm0cCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_USES_LADDER_HOLE (c_creature.cpp:1709) ----
 * Check/apply whether a creature falls through or climbs a hole. */
typedef struct {
    int (*get_tile_type)(void *ctx, int16_t x, int16_t y);
    void (*move_creature)(void *ctx, int creature_idx, int16_t x, int16_t y,
                          int level_delta);
} DM2_V1_CreatureUsesLadderHoleCallbacks;

int32_t dm2_v1_creature_uses_ladder_hole(
    int creature_idx, int16_t x, int16_t y,
    const DM2_V1_CreatureUsesLadderHoleCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_WALK_NOW (c_creature.cpp:2845) ----
 * Decide whether a creature is allowed to move this tick. */
typedef struct {
    int32_t move_counter;
    int32_t speed;
} DM2_V1_CreatureWalkState;

int32_t dm2_v1_creature_walk_now(DM2_V1_CreatureWalkState *state);

/* ---- DM2_CREATURE_ACTIVATES_WALL (c_creature.cpp:2564) ----
 * A creature triggers a wall mechanism (lever/button) by proximity. */
typedef struct {
    void (*activate_wall_item)(void *ctx, int16_t x, int16_t y, int dir);
} DM2_V1_CreatureActivatesWallCallbacks;

int32_t dm2_v1_creature_activates_wall(
    int16_t x, int16_t y, int dir,
    const DM2_V1_CreatureActivatesWallCallbacks *cb, void *ctx);

/* ---- DM2_PLACE_MERCHANDISE (c_creature.cpp:2411) ----
 * A merchant creature places an item for sale on its tile. */
typedef struct {
    void (*place_item_on_tile)(void *ctx, int16_t x, int16_t y, uint16_t item);
} DM2_V1_PlaceMerchandiseCallbacks;

int32_t dm2_v1_place_merchandise(
    int creature_idx, int16_t x, int16_t y, uint16_t item,
    const DM2_V1_PlaceMerchandiseCallbacks *cb, void *ctx);

/* ---- DM2_TAKE_MERCHANDISE (c_creature.cpp:2509) ----
 * A merchant creature reclaims unsold merchandise. */
typedef struct {
    int32_t (*remove_item_from_tile)(void *ctx, int16_t x, int16_t y);
} DM2_V1_TakeMerchandiseCallbacks;

int32_t dm2_v1_take_merchandise(
    int creature_idx, int16_t x, int16_t y,
    const DM2_V1_TakeMerchandiseCallbacks *cb, void *ctx);

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

int32_t dm2_v1_activate_item_teleport(
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
