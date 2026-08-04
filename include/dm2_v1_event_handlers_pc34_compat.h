#ifndef FIRESTAFF_DM2_V1_EVENT_HANDLERS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_EVENT_HANDLERS_PC34_COMPAT_H

/*
 * dm2_v1_event_handlers_pc34_compat.h — DM2 UI event handler callbacks.
 *
 * Source: skproject c_events.cpp
 *
 * Implements the click/interaction handlers that the touch-click zone
 * matrix dispatches to. Each handler uses a callback struct so the
 * module can be tested without linking the full runtime.
 *
 * Currently implemented:
 *   DM2_CLICK_ITEM_SLOT          c_events.cpp:45
 *   DM2_CLICK_VWPT               c_events.cpp:457
 *   DM2_CLICK_MAGICAL_MAP_RUNE   c_events.cpp:395
 *   DM2_CLICK_INVENTORY_EYE      c_events.cpp:1846
 *   DM2_ACTIVATE_ACTION_HAND     c_events.cpp:2784
 *   DM2_PROCEED_COMMAND_SLOT     c_events.cpp:2818
 *   DM2_PLAYER_TESTING_WALL      c_events.cpp:625
 *   DM2_PUSH_PULL_RIGID_BODY     c_events.cpp:467
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_CLICK_ITEM_SLOT (c_events.cpp:45) ----
 * Handle clicking an inventory slot. Swaps the held item with
 * the slot contents, triggering equip/unequip side effects. */
typedef struct {
    /* Party/hero state queries */
    int16_t (*get_event_heroidx)(void *ctx);
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_hero_curHP)(void *ctx, int hero);
    int16_t (*get_hero_item)(void *ctx, int hero, int slot);
    int16_t (*get_hand_container_item)(void *ctx, int slot);
    int16_t (*get_held_item)(void *ctx);
    int16_t (*get_v1e0976)(void *ctx);
    int16_t (*get_curacthero)(void *ctx);
    int16_t (*get_v1e0288)(void *ctx);

    /* Item fitting */
    int (*is_item_fit_for_equip)(void *ctx, int16_t item, int slot, int mode);

    /* Mutation */
    int (*remove_object_from_hand)(void *ctx);
    void (*remove_possession)(void *ctx, int hero, int slot);
    void (*take_object)(void *ctx, int16_t record, int mode);
    void (*equip_item_to_hand)(void *ctx, int hero, int16_t item, int slot);

    /* GUI */
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*update_right_panel)(void *ctx, int mode);
    void (*events_2e62_0cfa)(void *ctx, int param);
    void (*events_443c_0434)(void *ctx);

    /* Redraw flags */
    int16_t *v1e0b6c;
    int16_t *v1e0254;
} DM2_V1_ClickItemSlotCallbacks;

typedef struct {
    int handled;
    int item_swapped;
    int panel_refreshed;
} DM2_V1_ClickItemSlotReceipt;

void dm2_v1_click_item_slot(
    int16_t slot_index,
    const DM2_V1_ClickItemSlotCallbacks *cb, void *ctx,
    DM2_V1_ClickItemSlotReceipt *receipt);

/* ---- DM2_CLICK_VWPT (c_events.cpp:457) ----
 * Handle click in the 3D viewport area. Routes to
 * DM2_PLAYER_TESTING_WALL or DM2_PUSH_PULL_RIGID_BODY. */
typedef struct {
    int16_t (*get_held_item)(void *ctx);
    int16_t (*get_v1e0b28)(void *ctx);
    void (*player_testing_wall)(void *ctx, int16_t param);
    void (*push_pull_rigid_body)(void *ctx, int16_t param);
} DM2_V1_ClickVwptCallbacks;

void dm2_v1_click_vwpt(
    int16_t click_param,
    const DM2_V1_ClickVwptCallbacks *cb, void *ctx);

/* ---- DM2_CLICK_MAGICAL_MAP_RUNE (c_events.cpp:395) ----
 * Toggle a rune on the magical map spell panel. */
typedef struct {
    const int16_t *rune_table;   /* table1d67fe[9] */
    int16_t *v1e0b62;           /* current rune mask */
    void (*add_rune)(void *ctx, int16_t rune_mask);
    void (*remove_rune)(void *ctx, int16_t rune_mask);
    void (*queue_noise)(void *ctx, int16_t cat, int16_t idx);
} DM2_V1_ClickMagicalMapRuneCallbacks;

void dm2_v1_click_magical_map_rune(
    int16_t rune_index,
    const DM2_V1_ClickMagicalMapRuneCallbacks *cb, void *ctx);

/* ---- DM2_CLICK_INVENTORY_EYE (c_events.cpp:1846) ----
 * Toggle inventory detail view for a champion. */
typedef struct {
    int16_t (*get_event_heroidx)(void *ctx);
    void (*set_v1e0976)(void *ctx, int16_t val);
    void (*update_right_panel)(void *ctx, int mode);
} DM2_V1_ClickInventoryEyeCallbacks;

void dm2_v1_click_inventory_eye(
    const DM2_V1_ClickInventoryEyeCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_ACTION_HAND (c_events.cpp:2784) ----
 * Activate the action hand (select champion for action). */
typedef struct {
    int16_t (*get_event_heroidx)(void *ctx);
    int16_t (*get_curacthero)(void *ctx);
    void (*set_curacthero)(void *ctx, int16_t hero);
    void (*update_champion_display)(void *ctx, int hero);
} DM2_V1_ActivateActionHandCallbacks;

void dm2_v1_activate_action_hand(
    const DM2_V1_ActivateActionHandCallbacks *cb, void *ctx);

/* ---- DM2_PROCEED_COMMAND_SLOT (c_events.cpp:2818) ----
 * Execute a command from the action menu. Routes through the
 * CMDSTR/engage_command system. */
typedef struct {
    int16_t (*get_curacthero)(void *ctx);
    int16_t (*get_hero_curHP)(void *ctx, int hero);
    int (*query_cmdstr_entry)(void *ctx, int cls1, int cls2, int idx, int field,
                              uint16_t *out_val);
    int (*engage_command)(void *ctx, int hero, int cmd_idx, int16_t delay);
    void (*queue_noise)(void *ctx, int16_t sound_id);
    void (*update_right_panel)(void *ctx, int mode);

    /* Hero wielded item query */
    int16_t (*get_wielded_item_cls1)(void *ctx, int hero);
    int16_t (*get_wielded_item_cls2)(void *ctx, int hero);
} DM2_V1_ProceedCommandSlotCallbacks;

typedef struct {
    int handled;
    int command_engaged;
    int16_t delay_used;
} DM2_V1_ProceedCommandSlotReceipt;

void dm2_v1_proceed_command_slot(
    int16_t slot_index,
    const DM2_V1_ProceedCommandSlotCallbacks *cb, void *ctx,
    DM2_V1_ProceedCommandSlotReceipt *receipt);

/* ---- DM2_PLAYER_TESTING_WALL (c_events.cpp:625) ----
 * Test a wall tile for ornament interaction (alcove put/take,
 * lever toggle, keyhole). */
typedef struct {
    int16_t (*get_facing_tile_class)(void *ctx);
    int16_t (*get_facing_x)(void *ctx);
    int16_t (*get_facing_y)(void *ctx);
    int16_t (*get_held_item)(void *ctx);
    int (*is_wall_ornate_alcove)(void *ctx, int16_t x, int16_t y);
    int (*try_insert_into_alcove)(void *ctx, int16_t x, int16_t y, int16_t item);
    int (*try_take_from_alcove)(void *ctx, int16_t x, int16_t y);
    int (*try_toggle_lever)(void *ctx, int16_t x, int16_t y);
    int (*try_use_keyhole)(void *ctx, int16_t x, int16_t y, int16_t item);
    void (*queue_noise)(void *ctx, int16_t sound_id);
} DM2_V1_PlayerTestingWallCallbacks;

typedef struct {
    int handled;
    int action_type; /* 0=none 1=alcove_put 2=alcove_take 3=lever 4=keyhole */
} DM2_V1_PlayerTestingWallReceipt;

void dm2_v1_player_testing_wall(
    int16_t param,
    const DM2_V1_PlayerTestingWallCallbacks *cb, void *ctx,
    DM2_V1_PlayerTestingWallReceipt *receipt);

/* ---- DM2_PUSH_PULL_RIGID_BODY (c_events.cpp:467) ----
 * Push or pull a rigid object in the viewport. */
typedef struct {
    int16_t party_x;
    int16_t party_y;
    int16_t party_dir;
    int (*get_tile_class)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int (*is_tile_blocked)(void *ctx, int16_t x, int16_t y);
    int (*try_push_object_to)(void *ctx, int32_t record, int16_t x, int16_t y,
                              int16_t *out_x, int16_t *out_y);
    int16_t (*get_tile_record)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_record_cls1)(void *ctx, int16_t record);
    void (*move_record)(void *ctx, int32_t record, int16_t from_x, int16_t from_y,
                        int16_t to_x, int16_t to_y);
    void (*queue_noise)(void *ctx, int16_t sound_id);
    void (*redraw_viewport)(void *ctx);
    const int16_t *dx_table;
    const int16_t *dy_table;
} DM2_V1_PushPullRigidBodyCallbacks;

typedef struct {
    int handled;
    int pushed;
    int16_t target_x;
    int16_t target_y;
} DM2_V1_PushPullRigidBodyReceipt;

void dm2_v1_push_pull_rigid_body(
    int16_t direction_param,
    const DM2_V1_PushPullRigidBodyCallbacks *cb, void *ctx,
    DM2_V1_PushPullRigidBodyReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_EVENT_HANDLERS_PC34_COMPAT_H */
