#ifndef FIRESTAFF_DM2_V1_INPUT_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_INPUT_PC34_COMPAT_H

/*
 * dm2_v1_input_pc34_compat.h — DM2 input dispatcher and event interpreter.
 *
 * Ports the input handling from skproject/SKWINSPX/src/v5/c_input.cpp.
 * Covers UI event dispatch (handle_ui_event), bytecode event execution
 * (exec_event), input polling (input_check), and the outer event loop.
 *
 * Source: skproject/SKWINSPX/src/v5/c_input.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── handle_ui_event ─────────────────────────────────────────────── */

typedef struct {
    int16_t event_idx;
    int16_t x;
    int16_t y;
} DM2_V1_HandleUiEventRequest;

typedef struct {
    void (*release_mouse)(void *ctx);
    void (*clear_1031)(void *ctx);
    void (*perform_turn)(void *ctx, int16_t direction);
    void (*perform_move)(void *ctx, int16_t direction);
    void (*click_item_slot)(void *ctx, int16_t slot);
    void (*put_item_to_player)(void *ctx, int16_t player);
    void (*change_player_pos)(void *ctx, int16_t pos);
    void (*click_moneybox)(void *ctx, int16_t box);
    void (*rotate_direction)(void *ctx, int16_t dir);
    void (*select_leader)(void *ctx, int16_t leader);
    void (*panel_button)(void *ctx, int16_t button);
    void (*try_cast_spell)(void *ctx);
    void (*remove_rune)(void *ctx);
    void (*add_rune)(void *ctx, int16_t rune);
    void (*proceed_command_slot)(void *ctx, int16_t slot);
    void (*activate_action_hand)(void *ctx, int16_t hand);
    void (*set_spelling_champion)(void *ctx, int16_t champ);
    void (*consume_object)(void *ctx);
    void (*click_eye)(void *ctx);
    void (*click_hero_stat)(void *ctx);
    void (*click_viewport)(void *ctx, int16_t x, int16_t y);
    void (*event_13262)(void *ctx, int16_t param);
    void (*click_map_at)(void *ctx);
    void (*click_map_rune)(void *ctx, int16_t rune);
    void (*game_save_menu)(void *ctx);
    void (*sleep_wake)(void *ctx, int16_t param);
    void (*show_credits)(void *ctx);
    void (*prepare_exit)(void *ctx);
    void (*set_savegame_flag)(void *ctx, int16_t flag, int16_t value);
} DM2_V1_HandleUiEventCallbacks;

typedef struct {
    int16_t handled;
    int16_t dispatched_idx;
    int16_t dispatch_category;
} DM2_V1_HandleUiEventReceipt;

/* ── exec_event ──────────────────────────────────────────────────── */

typedef struct {
    void (*set_delay)(void *ctx, uint8_t delay);
    void (*set_event_unk02)(void *ctx);
    void (*set_v1e0478)(void *ctx, int16_t val);
    void (*set_v1e048c)(void *ctx, int16_t val);
    void (*push_event_state)(void *ctx, uint8_t state);
    void (*handle_ui_event)(void *ctx, int16_t idx, int16_t x, int16_t y);
    void (*draw_gdat_image)(void *ctx, uint8_t a, uint8_t b, uint8_t c,
                            int16_t x, int16_t y);
    void (*draw_squad_icon)(void *ctx, uint8_t icon);
    int16_t (*resolve_hero)(void *ctx, int16_t param);
    void (*draw_rune)(void *ctx, uint8_t rune);
    void (*draw_spell)(void *ctx);
    void (*draw_command_slot)(void *ctx, uint8_t slot);
    void (*refresh_stat)(void *ctx, uint8_t stat);
    void (*draw_guide)(void *ctx);
    void (*gameload_dialogue)(void *ctx, uint8_t param);
} DM2_V1_ExecEventCallbacks;

/* ── input_check ─────────────────────────────────────────────────── */

typedef struct {
    int (*has_key)(void *ctx);
    int16_t (*get_key)(void *ctx);
    int (*has_event)(void *ctx);
    int16_t (*get_event)(void *ctx, int16_t *out_x, int16_t *out_y);
    const int8_t *(*event_table)(void *ctx, int16_t event_idx);
    void (*set_ui_event)(void *ctx, int16_t event_idx, int16_t x, int16_t y);
    void (*transmit_ui_event)(void *ctx, int16_t event_idx, int16_t x,
                              int16_t y);
    void (*exec_event)(void *ctx, const int8_t *bytecode);
} DM2_V1_InputCheckCallbacks;

typedef struct {
    int16_t keys_processed;
    int16_t events_processed;
} DM2_V1_InputCheckReceipt;

/* ── bytecode event routing table ────────────────────────────────── */

extern const int8_t dm2_v1_table1d3efd[236];

/* ── function declarations ───────────────────────────────────────── */

void dm2_v1_handle_ui_event(
    const DM2_V1_HandleUiEventCallbacks *cb,
    void *ctx,
    const DM2_V1_HandleUiEventRequest *req,
    DM2_V1_HandleUiEventReceipt *receipt);

void dm2_v1_exec_event(
    const DM2_V1_ExecEventCallbacks *cb,
    void *ctx,
    const int8_t *bytecode);

void dm2_v1_input_check(
    const DM2_V1_InputCheckCallbacks *cb,
    void *ctx,
    DM2_V1_InputCheckReceipt *receipt);

void dm2_v1_event_loop(
    const DM2_V1_InputCheckCallbacks *cb,
    void *ctx);

int16_t dm2_v1_1031_03f2(
    int16_t key,
    const int16_t *tree,
    int16_t tree_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INPUT_PC34_COMPAT_H */
