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
 *   DM2_events_5BFB              c_events.cpp:170
 *   DM2_events_AB26              c_events.cpp:201
 *   DM2_events_38c8_0002         c_events.cpp:327
 *   DM2_events_38c8_0060         c_events.cpp:355
 *   DM2_events_13262             c_events.cpp:427
 *   DM2_ADD_RUNE_TO_TAIL         c_events.cpp:1871
 *   DM2_REMOVE_RUNE_FROM_TAIL    c_events.cpp:1931
 *   DM2_CLICK_MONEYBOX           c_events.cpp:1941
 *   DM2_events_2e62_0cfa         c_events.cpp:2153
 *   DM2_events_30DEA             c_events.cpp:2903
 *   DM2_events_443c_0434         c_events.cpp:3958
 *   DM2_events_2f3f_04ea         c_events.cpp:1996
 *   DM2_events_121e_0351         c_events.cpp:1298
 *   DM2_events_37BBB             c_events.cpp:1290
 *   DM2_events_121e_0003         c_events.cpp:1222
 *   DM2_events_121e_013a         c_events.cpp:974
 *   DM2_eventa_121e_0222         c_events.cpp:1051
 *   DM2_events_121e_03ae         c_events.cpp:1171
 *   DM2_events_32cb_0287         c_events.cpp:670
 *   DM2_events_32cb_03a6         c_events.cpp:736
 *   DM2_guivp_32cb_01b6          c_events.cpp:1345
 *   DM2_events_3C1E5             c_events.cpp:2949
 *
 * From c_input.cpp:
 *   DM2_ADJUST_UI_EVENT           c_input.cpp:112
 *   DM2_1031_03f2                 c_input.cpp:55
 *   DM2_0b36_129a                 c_input.cpp:522
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

/* ---- DM2_events_5BFB (c_events.cpp:170) ----
 * Sound/UI helper: sets v1d26a0/v1d26a2 and queues noise. */
typedef struct {
    int16_t *v1d26a0;
    int16_t *v1d26a2;
    int16_t v1e0270;
    int16_t v1e0272;
    void (*queue_noise_gen1)(void *ctx, int16_t cat, int16_t sub,
                             int16_t snd, int16_t vol, int16_t dur,
                             int16_t x, int16_t y, int16_t flags);
    void (*sound3)(void *ctx, int16_t a, int16_t b);
} DM2_V1_Events5BFBCallbacks;

void dm2_v1_events_5bfb(
    int16_t param1, int16_t param2,
    const DM2_V1_Events5BFBCallbacks *cb, void *ctx);

/* ---- DM2_events_38c8_0002 (c_events.cpp:327) ----
 * Viewport enter: sets v1e13f0 flag, hides mouse, fills halftone. */
typedef struct {
    int16_t *v1e13f0;
    int16_t *dialog2;
    int16_t *v1e00b8;
    int16_t *v1e0976;
    void (*champion_squad_recompute)(void *ctx);
    void (*update_right_panel)(void *ctx, int mode);
    void (*proc_1031_04f5)(void *ctx);
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*move_12b4_0092)(void *ctx);
    void (*fill_halftone_recti)(void *ctx, int16_t idx);
} DM2_V1_Events38c80002Callbacks;

typedef struct {
    int handled;
} DM2_V1_Events38c80002Receipt;

void dm2_v1_events_38c8_0002(
    const DM2_V1_Events38c80002Callbacks *cb, void *ctx,
    DM2_V1_Events38c80002Receipt *receipt);

/* ---- DM2_events_38c8_0060 (c_events.cpp:355) ----
 * Viewport exit: clears v1e13f0 flag, triggers redraw. */
typedef struct {
    int16_t *v1e13f0;
    int16_t *dialog2;
    int16_t *v1e0238;
    int16_t *v1e0976;
    int16_t *v1e0b6c;
    int16_t *v1e0b30;
    int16_t curacthero;
    void (*draw_wake_up_text)(void *ctx);
    void (*drawings_completed)(void *ctx);
    void (*guidraw_24a5_1798)(void *ctx, int16_t param);
    void (*init_backbuff)(void *ctx);
    void (*guidraw_29ee_000f)(void *ctx);
} DM2_V1_Events38c80060Callbacks;

void dm2_v1_events_38c8_0060(
    const DM2_V1_Events38c80060Callbacks *cb, void *ctx);

/* ---- DM2_events_13262 (c_events.cpp:427) ----
 * Click dispatch by zone: scans v1e02f0 array for matching
 * zone and routes to CLICK_VWPT. */
typedef struct {
    int16_t v1e0404;
    int16_t v1d26fc;
    int16_t v1d26fe;
    int16_t (*get_zone_b_0a)(void *ctx, int idx);
    int16_t (*get_zone_b_0b)(void *ctx, int idx);
    int16_t (*get_zone_rc_x)(void *ctx, int idx);
    int16_t (*get_zone_rc_y)(void *ctx, int idx);
    void (*click_vwpt)(void *ctx, int16_t x, int16_t y);
} DM2_V1_Events13262Callbacks;

void dm2_v1_events_13262(
    int16_t param,
    const DM2_V1_Events13262Callbacks *cb, void *ctx);

/* ---- DM2_ADD_RUNE_TO_TAIL (c_events.cpp:1871) ----
 * Add a rune symbol to the current spell sequence. */
typedef struct {
    int16_t curacthero;
    int16_t (*get_hero_nrunes)(void *ctx, int hero);
    void (*set_hero_nrunes)(void *ctx, int hero, int16_t n);
    int16_t (*get_hero_rune)(void *ctx, int hero, int idx);
    void (*set_hero_rune)(void *ctx, int hero, int idx, int16_t val);
    int16_t (*get_hero_curMP)(void *ctx, int hero);
    void (*set_hero_curMP)(void *ctx, int hero, int16_t mp);
    void (*or_hero_flag)(void *ctx, int hero, int16_t bits);
    const uint8_t *rune_cost_table; /* table1d67e0[nrunes][6] */
    const uint8_t *rune_power_table; /* table1d6797 */
    int16_t *v1e0b6c;
    void (*update_right_panel)(void *ctx, int mode);
    void (*proc_107b0)(void *ctx);
    void (*refresh_player_stat_disp)(void *ctx, int16_t hero);
} DM2_V1_AddRuneToTailCallbacks;

typedef struct {
    int handled;
    int rune_added;
} DM2_V1_AddRuneToTailReceipt;

void dm2_v1_add_rune_to_tail(
    int16_t rune_index,
    const DM2_V1_AddRuneToTailCallbacks *cb, void *ctx,
    DM2_V1_AddRuneToTailReceipt *receipt);

/* ---- DM2_REMOVE_RUNE_FROM_TAIL (c_events.cpp:1931) ----
 * Remove the last rune from the current spell sequence. */
typedef struct {
    int16_t curacthero;
    int16_t (*get_hero_nrunes)(void *ctx, int hero);
    void (*set_hero_nrunes)(void *ctx, int hero, int16_t n);
    void (*set_hero_rune)(void *ctx, int hero, int idx, int16_t val);
    int16_t *v1e0b6c;
    void (*update_right_panel)(void *ctx, int mode);
    void (*proc_107b0)(void *ctx);
} DM2_V1_RemoveRuneFromTailCallbacks;

void dm2_v1_remove_rune_from_tail(
    const DM2_V1_RemoveRuneFromTailCallbacks *cb, void *ctx);

/* ---- DM2_CLICK_MONEYBOX (c_events.cpp:1941) ----
 * Click on a moneybox slot: deposit or withdraw coins. */
typedef struct {
    int16_t (*get_v1d67bc)(void *ctx);
    int16_t curacthero;
    int16_t curactmode;
    int16_t (*get_hero_item)(void *ctx, int hero, int slot);
    int16_t (*get_held_item)(void *ctx);
    int16_t (*get_item_order_in_container)(void *ctx, int16_t container, int16_t slot);
    int16_t (*take_coin_from_wallet)(void *ctx, int16_t container, int16_t order);
    int (*add_coin_to_wallet)(void *ctx, int16_t container, int16_t coin);
    void (*take_object)(void *ctx, int16_t record, int mode);
    int (*remove_object_from_hand)(void *ctx);
    int16_t *v1e0b6c;
    void (*update_right_panel)(void *ctx, int mode);
    void (*calc_player_weight)(void *ctx, int16_t hero);
} DM2_V1_ClickMoneyboxCallbacks;

typedef struct {
    int handled;
    int deposited;
    int withdrawn;
} DM2_V1_ClickMoneyboxReceipt;

void dm2_v1_click_moneybox(
    int16_t slot_index,
    const DM2_V1_ClickMoneyboxCallbacks *cb, void *ctx,
    DM2_V1_ClickMoneyboxReceipt *receipt);

/* ---- DM2_events_2e62_0cfa (c_events.cpp:2153) ----
 * Refresh all player stat displays and right panel. */
typedef struct {
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_v1e0976)(void *ctx);
    int16_t (*get_hero_ench_power)(void *ctx, int hero);
    int16_t (*get_hero_ench_aura)(void *ctx, int hero);
    int16_t (*get_hero_flag)(void *ctx, int hero);
    void (*set_hero_flag)(void *ctx, int hero, int16_t flags);
    int (*get_vcapture3)(void *ctx);
    int16_t (*get_held_item)(void *ctx);
    void (*refresh_player_stat_disp)(void *ctx, int16_t hero);
    void (*update_right_panel)(void *ctx, int mode);
} DM2_V1_Events2e620cfaCallbacks;

void dm2_v1_events_2e62_0cfa(
    int16_t param,
    const DM2_V1_Events2e620cfaCallbacks *cb, void *ctx);

/* ---- DM2_events_30DEA (c_events.cpp:2903) ----
 * Item charge management for engage_command results. */
typedef struct {
    int16_t v1e0b7e;
    int16_t v1e0b7f;
    int16_t v1e0b80;
    int16_t v1e0b50;
    int16_t curactmode;
    int16_t (*query_cmdstr_entry)(void *ctx, int16_t cls1, int16_t cls2,
                                  int16_t idx, int16_t field);
    int16_t (*add_item_charge)(void *ctx, int16_t record, int16_t delta);
    int16_t (*query_gdat_dbspec_word)(void *ctx, int16_t record, int16_t field);
    void (*remove_possession)(void *ctx, int16_t hero, int16_t slot);
    void (*dealloc_record)(void *ctx, int16_t record);
} DM2_V1_Events30DEACallbacks;

typedef struct {
    int handled;
    int item_destroyed;
} DM2_V1_Events30DEAReceipt;

int dm2_v1_events_30dea(
    int16_t hero,
    const DM2_V1_Events30DEACallbacks *cb, void *ctx,
    DM2_V1_Events30DEAReceipt *receipt);

/* ---- DM2_events_443c_0434 (c_events.cpp:3958) ----
 * Refresh mouse cursor based on held item state. */
typedef struct {
    int16_t (*get_held_item)(void *ctx);
    void (*generate_cursor_from_item)(void *ctx);
    void (*refresh_mouse)(void *ctx);
} DM2_V1_Events443c0434Callbacks;

typedef struct {
    int handled;
    int cursor_changed;
} DM2_V1_Events443c0434Receipt;

void dm2_v1_events_443c_0434(
    const DM2_V1_Events443c0434Callbacks *cb, void *ctx,
    DM2_V1_Events443c0434Receipt *receipt);

/* ---- DM2_events_121e_0351 (c_events.cpp:1298) ----
 * Viewport click sub-handler: test click against arrow panel rects. */
typedef struct {
    int16_t v1e0530;
    int (*pt_in_expanded_rect)(void *ctx, int16_t rect_id, int16_t x, int16_t y);
    int (*hero_2c1d_1de2)(void *ctx, int16_t hero, int16_t param1, int16_t param2);
    int16_t (*get_event_heroidx)(void *ctx);
    int *v1e0488;
} DM2_V1_Events121e0351Callbacks;

void dm2_v1_events_121e_0351(
    int16_t x, int16_t y,
    const DM2_V1_Events121e0351Callbacks *cb, void *ctx);

/* ---- DM2_events_37BBB (c_events.cpp:1290) ----
 * Hero wall test helper: calls hero_2c1d_1de2 if hero exists. */
typedef struct {
    int16_t (*get_event_heroidx)(void *ctx);
    int (*hero_2c1d_1de2)(void *ctx, int16_t hero, int16_t param1, int16_t param2);
} DM2_V1_Events37BBBCallbacks;

int dm2_v1_events_37bbb(
    int16_t param,
    const DM2_V1_Events37BBBCallbacks *cb, void *ctx);

/* ---- DM2_events_121e_0003 (c_events.cpp:1222) ----
 * Viewport click sub-handler: compute target tile from click zone
 * and dispatch to events_3C1E5 (wall interaction). */
typedef struct {
    int16_t v1e0270;
    int16_t v1e0272;
    int16_t v1e0258;
    int16_t map_width;
    int16_t map_height;
    int16_t savegamewpc_w_00;
    const int16_t *dx_table;
    const int16_t *dy_table;
    void (*events_3c1e5)(void *ctx, int16_t x, int16_t y,
                          int16_t dir, int16_t param, int16_t record);
} DM2_V1_Events121e0003Callbacks;

void dm2_v1_events_121e_0003(
    int16_t param,
    const DM2_V1_Events121e0003Callbacks *cb, void *ctx);

/* ---- DM2_events_121e_013a (c_events.cpp:974) ----
 * Viewport click: pick up item from tile via events_32cb_03a6. */
typedef struct {
    int16_t v1e0270;
    int16_t v1e0272;
    int16_t v1e0258;
    int16_t (*get_event_heroidx)(void *ctx);
    int16_t (*get_zone_b_0a)(void *ctx, int idx);
    int16_t (*get_zone_b_0b)(void *ctx, int idx);
    int16_t (*get_zone_w_08)(void *ctx, int idx);
    const int16_t *dx_table;
    const int16_t *dy_table;
    int16_t (*events_32cb_03a6)(void *ctx, int16_t x, int16_t y,
        int16_t zone_b0a, int16_t tile_x, int16_t tile_y,
        int16_t record, int16_t zone_b0b, int argl3);
    int (*is_container_moneybox)(void *ctx, int16_t record);
    void (*move_record_to)(void *ctx, int16_t record, int16_t x, int16_t y,
                            int16_t dest, int16_t param);
    void (*take_object)(void *ctx, int16_t record, int mode);
    int *v1e0ff6;
    int *v1e0488;
} DM2_V1_Events121e013aCallbacks;

void dm2_v1_events_121e_013a(
    int16_t click_x, int16_t click_y, int16_t zone_idx,
    const DM2_V1_Events121e013aCallbacks *cb, void *ctx);

/* ---- DM2_eventa_121e_0222 (c_events.cpp:1051) ----
 * Viewport click: drop held item onto tile. */
typedef struct {
    int16_t v1e0258;
    int16_t (*get_event_heroidx)(void *ctx);
    int (*remove_object_from_hand)(void *ctx);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    void *(*query_creature_ai_spec)(void *ctx, int16_t creature);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    int (*is_container_moneybox)(void *ctx, int16_t record);
    void (*move_record_to)(void *ctx, int16_t record, int16_t dest_x,
                            int16_t dest_y, int16_t src_x, int16_t src_y);
    int *v1e0488;
} DM2_V1_Eventa121e0222Callbacks;

int16_t dm2_v1_eventa_121e_0222(
    int16_t x, int16_t y, int16_t dir_offset,
    const DM2_V1_Eventa121e0222Callbacks *cb, void *ctx);

/* ---- DM2_events_121e_03ae (c_events.cpp:1171) ----
 * Viewport click: try place item via 32cb_03a6, fall back to 0222. */
typedef struct {
    int16_t v1e0258;
    int16_t savegamewpc_w_00;
    int16_t (*events_32cb_03a6)(void *ctx, int16_t x, int16_t y,
        int16_t argw0, int16_t tile_x, int16_t tile_y,
        int16_t record, int16_t argw2, int argl3);
    int (*remove_object_from_hand)(void *ctx);
    int16_t (*eventa_121e_0222)(void *ctx, int16_t x, int16_t y, int16_t dir);
} DM2_V1_Events121e03aeCallbacks;

int dm2_v1_events_121e_03ae(
    int16_t click_x, int16_t click_y,
    int16_t tile_x, int16_t tile_y,
    int16_t argw0, int16_t argl1, int16_t argw2,
    const DM2_V1_Events121e03aeCallbacks *cb, void *ctx);

/* ---- DM2_events_32cb_0287 (c_events.cpp:670) ----
 * Viewport sub: compute creature/item visibility at stone room position. */
typedef struct {
    int16_t v1e12c8;
    int16_t v1e12ca;
    int16_t v1e12cc;
    void (*calc_vector_w_dir)(void *ctx, int16_t dir, int16_t dy, int16_t dx,
                               int16_t *out_y, int16_t *out_x);
    void (*summarize_stone_room)(void *ctx, void *out, int16_t dir,
                                  int16_t x, int16_t y);
    int16_t (*guivp_32cb_15b8)(void *ctx, int16_t idx, int16_t param, int16_t mode);
    int (*guivp_32cb_00f1)(void *ctx, int16_t x, int16_t y, int16_t param);
    void *(*alloc_lobigpool)(void *ctx, int16_t size);
    void (*dealloc_lobigpool)(void *ctx, int16_t size);
    void **ptr1e1044;
    int16_t img_width;
    int16_t img_height;
    const int16_t (*table1d6ad0)[2];
    const int16_t *table1d6afe;
} DM2_V1_Events32cb0287Callbacks;

int dm2_v1_events_32cb_0287(
    int16_t param, int16_t click_x, int16_t click_y,
    const DM2_V1_Events32cb0287Callbacks *cb, void *ctx);

/* ---- DM2_events_32cb_03a6 (c_events.cpp:736) ----
 * Viewport sub: draw and hit-test items on tile for click handling. */
typedef struct {
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    int16_t (*get_next_record_link)(void *ctx, int16_t record);
    int (*is_container_chest)(void *ctx, int16_t record);
    uint8_t (*query_cls2_from_record)(void *ctx, int16_t record);
    int (*query_gdat_entry_if_loadable)(void *ctx, int16_t cls1,
        uint8_t cls2, int16_t idx, int16_t param);
    void (*draw_item)(void *ctx, int16_t record, int16_t param1,
        int16_t param2, int16_t param3, int16_t param4);
    int (*guivp_32cb_00f1)(void *ctx, int16_t x, int16_t y, int16_t param);
    int16_t (*query_gdat_dbspec_word)(void *ctx, int16_t record, int16_t field);
} DM2_V1_Events32cb03a6Callbacks;

int16_t dm2_v1_events_32cb_03a6(
    int16_t click_x, int16_t click_y,
    int16_t zone_dir, int16_t tile_x,
    int16_t tile_y, int16_t record,
    int16_t zone_type, int argl3,
    const DM2_V1_Events32cb03a6Callbacks *cb, void *ctx);

/* ---- DM2_guivp_32cb_01b6 (c_events.cpp:1345) ----
 * Viewport sub: hit-test creature at tile for click handling. */
typedef struct {
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    void (*query_creature_picst)(void *ctx, int16_t mode,
        int16_t param, void *record, void *creature, int16_t id);
    void (*draw_temp_picst)(void *ctx);
    int (*guivp_32cb_00f1)(void *ctx, int16_t x, int16_t y, int16_t param);
    int16_t v1e12da;
    const int16_t *table1d6b15;
} DM2_V1_Guivp32cb01b6Callbacks;

int dm2_v1_guivp_32cb_01b6(
    int16_t click_x, int16_t click_y,
    int16_t tile_x, int16_t tile_y,
    int16_t *out_dir,
    const DM2_V1_Guivp32cb01b6Callbacks *cb, void *ctx);

/* ---- DM2_events_3C1E5 (c_events.cpp:2949) ----
 * Wall interaction: alcove insert/take, lever, sensor activation.
 * This is the most complex handler — many sub-interactions. */
typedef struct {
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_next_record_link)(void *ctx, int16_t record);
    int (*is_wall_ornate_alcove)(void *ctx, int16_t tile_record, int16_t record);
    int (*query_0cee_319e)(void *ctx, int16_t record);
    int (*is_container_chest)(void *ctx, int16_t record);
    uint8_t (*query_cls2_from_record)(void *ctx, int16_t record);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    int (*is_container_moneybox)(void *ctx, int16_t record);
    void (*move_record_to)(void *ctx, int16_t record, int16_t x, int16_t y,
                            int16_t dest, int16_t param);
    void (*take_object)(void *ctx, int16_t record, int mode);
    int (*remove_object_from_hand)(void *ctx);
    void (*invoke_message)(void *ctx, int16_t x, int16_t y,
                            int16_t param1, int16_t param2, int32_t tick);
    void (*queue_timer)(void *ctx, int16_t type, int16_t actor,
                         int16_t a_val, int32_t tick, int16_t map);
    void (*queue_noise_gen1)(void *ctx, int16_t cat, int16_t sub,
                              int16_t snd, int16_t vol, int16_t dur,
                              int16_t x, int16_t y, int16_t flags);
    int16_t (*get_event_heroidx)(void *ctx);
    int16_t v1e0270;
    int16_t v1e0272;
    int32_t gametick;
    int16_t v1d3248;
    int *v1e0488;
} DM2_V1_Events3C1E5Callbacks;

typedef struct {
    int handled;
    int action_type;
} DM2_V1_Events3C1E5Receipt;

void dm2_v1_events_3c1e5(
    int16_t x, int16_t y, int16_t dir,
    int16_t param, int16_t record,
    const DM2_V1_Events3C1E5Callbacks *cb, void *ctx,
    DM2_V1_Events3C1E5Receipt *receipt);

/* ---- DM2_ADJUST_UI_EVENT (c_input.cpp:112) ----
 * Adjust a UI event index based on hand cooldowns and item
 * activability. For idx 116-123 (action hand clicks): checks
 * the hero's hand cooldown timer and whether the wielded item
 * is activable; clears idx to 0 if not available. For idx
 * 95-98 (movement arrows): checks player position validity
 * and hand cooldown. */
typedef struct {
    int16_t (*get_curacthero)(void *ctx);
    int16_t (*get_hero_curHP)(void *ctx, int hero);
    int16_t (*get_hero_hand_cooldown)(void *ctx, int hero, int hand);
    int16_t (*get_hero_item)(void *ctx, int hero, int slot);
    int (*is_item_activable)(void *ctx, int16_t item);
    int16_t (*get_player_position)(void *ctx, int hero);
    int16_t v1e0288;
} DM2_V1_AdjustUiEventCallbacks;

typedef struct {
    int handled;
    int16_t original_idx;
    int16_t adjusted_idx;
    int suppressed;
} DM2_V1_AdjustUiEventReceipt;

void dm2_v1_adjust_ui_event(
    int16_t *idx, int16_t x, int16_t y,
    const DM2_V1_AdjustUiEventCallbacks *cb, void *ctx,
    DM2_V1_AdjustUiEventReceipt *receipt);

/* ---- DM2_1031_03f2 (c_input.cpp:55) ----
 * Recursive event table lookup. Walks table1d3ba0 entries
 * matching the current s_bbw value, returning the event index
 * from table1d3d23/v1d39bc. Used by the touch-click zone
 * matrix to resolve which handler a click dispatches to. */
typedef struct {
    int16_t s_bbw;
    const uint8_t *table1d3ba0;   /* event table: [n][3] packed entries */
    int16_t table1d3ba0_count;    /* number of entries */
    const int16_t *table1d3d23;   /* event index table */
    const int16_t *v1d39bc;       /* secondary event index table */
    int16_t (*recurse)(void *ctx, int16_t sub_bbw);
} DM2_V1_1031_03f2Callbacks;

typedef struct {
    int handled;
    int16_t event_index;
    int recursed;
} DM2_V1_1031_03f2Receipt;

int16_t dm2_v1_1031_03f2(
    const DM2_V1_1031_03f2Callbacks *cb, void *ctx,
    DM2_V1_1031_03f2Receipt *receipt);

/* ---- DM2_0b36_129a (c_input.cpp:522) ----
 * Draw a string to a button group bitmap during event execution.
 * Queries string metrics, then calls DM2_DRAW_STRING at the
 * computed position within the button group rectangle. */
typedef struct {
    int16_t (*query_str_width)(void *ctx, const char *str);
    int16_t (*query_str_height)(void *ctx, const char *str);
    void (*draw_string)(void *ctx, int16_t x, int16_t y,
                        const char *str, int16_t color);
    int16_t btn_x;
    int16_t btn_y;
    int16_t btn_w;
    int16_t btn_h;
} DM2_V1_0b36_129aCallbacks;

typedef struct {
    int handled;
    int16_t draw_x;
    int16_t draw_y;
} DM2_V1_0b36_129aReceipt;

void dm2_v1_0b36_129a(
    const char *str, int16_t color,
    const DM2_V1_0b36_129aCallbacks *cb, void *ctx,
    DM2_V1_0b36_129aReceipt *receipt);

/* ---- DM2_events_AB26 (c_events.cpp:201) ----
 * Game load dialogue: displays save/load slot selection UI. */
typedef struct {
    int16_t *v1d26a0;
    int16_t *v1d26a2;
    int (*query_gdat_entry_if_loadable)(void *ctx, int16_t cls1,
        uint8_t cls2, int16_t idx, int16_t param);
    void (*proc_0aaf_02f8)(void *ctx, int16_t param, int16_t mode);
    void (*query_gdat_text)(void *ctx, uint8_t cls1, uint8_t cls2,
                             uint8_t idx, char *out);
    void (*draw_vp_rc_str)(void *ctx, int16_t rect_id, int16_t color,
                            const char *str);
    void (*draw_static_pic)(void *ctx, int16_t cls1, uint8_t cls2,
                             uint8_t idx, int16_t rect_id, int alpha);
    void (*draw_gameload_dialogue_to_screen)(void *ctx);
    int16_t (*proc_0aaf_0067)(void *ctx, int16_t param);
    uint8_t (*palette_col11)(void *ctx);
    void (*events_5bfb)(void *ctx, int16_t param1, int16_t param2);
} DM2_V1_EventsAB26Callbacks;

typedef struct {
    int handled;
    int slot_selected;
    int16_t selected_slot;
} DM2_V1_EventsAB26Receipt;

void dm2_v1_events_ab26(
    const DM2_V1_EventsAB26Callbacks *cb, void *ctx,
    DM2_V1_EventsAB26Receipt *receipt);

/* ---- DM2_events_2f3f_04ea (c_events.cpp:1996) ----
 * Cryocell/champion release handler. */
typedef struct {
    int16_t v1e0288;
    int16_t v1d3248;
    int16_t heros_in_party;
    void (*change_current_map_to)(void *ctx, int16_t map);
    int (*remove_object_from_hand)(void *ctx);
    void (*draw_cryocell_lever)(void *ctx, int16_t param);
    void (*add_item_to_player)(void *ctx, int16_t hero, int16_t item);
    void (*cut_record_from)(void *ctx, int16_t record, int16_t x, int16_t y);
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_next_record_link)(void *ctx, int16_t record);
    void *(*get_address_of_record)(void *ctx, int16_t record);
    void (*display_hint_new_line)(void *ctx);
    void (*display_hint_text)(void *ctx, int16_t color, const char *text);
    void (*query_gdat_text)(void *ctx, uint8_t cls1, uint8_t cls2,
                             uint8_t idx, char *out);
    void (*guidraw_24a5_1798)(void *ctx, int16_t param);
    void (*events_38c8_0060)(void *ctx);
    void (*hide_mouse)(void *ctx);
    void (*show_mouse)(void *ctx);
    void (*select_champion_leader)(void *ctx, int16_t param);
    void (*fill_screen_rect)(void *ctx, int16_t rect_id, uint8_t color);
    int16_t v1d6a2d;
    int16_t *v1e098c;
    int16_t *v1e01a0_tick;
    int32_t gametick;
    const int16_t *dx_table;
    const int16_t *dy_table;
    const uint8_t *table1d69d0;
} DM2_V1_Events2f3f04eaCallbacks;

void dm2_v1_events_2f3f_04ea(
    int16_t click_x, int16_t click_y,
    int16_t dir, int16_t map,
    int16_t argw0,
    const DM2_V1_Events2f3f04eaCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_EVENT_HANDLERS_PC34_COMPAT_H */
