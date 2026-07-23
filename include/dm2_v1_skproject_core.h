#ifndef FIRESTAFF_DM2_V1_SKPROJECT_CORE_H
#define FIRESTAFF_DM2_V1_SKPROJECT_CORE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} DM2_V1_SkprojectRect;

typedef struct {
    DM2_V1_SkprojectRect rects[4];
    uint8_t next_index;
} DM2_V1_SkprojectTempRectRing;

typedef struct {
    int valid;
    uint8_t slot;
    uint8_t next_slot;
    DM2_V1_SkprojectRect rect;
    uint32_t receipt_hash;
} DM2_V1_SkprojectTempRectReceipt;

typedef struct {
    uint32_t random;
} DM2_V1_SkprojectRandomData;

typedef struct {
    int valid;
    int blocked_missing_output;
    uint8_t dir;
    int16_t input_xx;
    int16_t input_yy;
    int16_t initial_x;
    int16_t initial_y;
    int16_t forward_dx;
    int16_t forward_dy;
    int16_t side_dx;
    int16_t side_dy;
    int16_t final_x;
    int16_t final_y;
} DM2_V1_SkprojectVectorWDirReceipt;

typedef struct {
    int valid;
    int tied_axes;
    int consumed_randbit;
    int blocked_missing_random;
    int16_t from_x;
    int16_t from_y;
    int16_t to_x;
    int16_t to_y;
    int16_t delta_x;
    int16_t delta_y;
    int16_t abs_delta_x;
    int16_t abs_delta_y;
    uint8_t randbit;
    uint8_t dir;
} DM2_V1_SkprojectVectorDirReceipt;

typedef struct {
    int valid;
    int blocked_missing_table;
    int16_t value;
    uint16_t entries;
    uint16_t written_entries;
} DM2_V1_SkprojectFillI16TableReceipt;

typedef struct {
    int valid;
    int blocked_missing_rect;
    DM2_V1_SkprojectRect rect;
    int16_t point_x;
    int16_t point_y;
    uint8_t result;
} DM2_V1_SkprojectPtInRectReceipt;

typedef struct {
    int valid;
    int blocked_missing_origin;
    int blocked_missing_source;
    int blocked_missing_output;
    DM2_V1_SkprojectRect origin;
    DM2_V1_SkprojectRect source;
    DM2_V1_SkprojectRect output;
} DM2_V1_SkprojectOffsetRectReceipt;

typedef struct {
    int valid;
    uint32_t initial_offset;
    int32_t delta;
    uint32_t final_offset;
    uint8_t blocked_out_of_bounds;
} DM2_V1_SkprojectPtrAdvanceReceipt;

typedef struct {
    int valid;
    uint32_t offset;
    uint16_t value;
    uint8_t width_bytes;
    uint8_t blocked_missing_buffer;
    uint8_t blocked_out_of_bounds;
    uint8_t blocked_unsupported_width;
} DM2_V1_SkprojectCursorAccessReceipt;

#define DM2_V1_SKPROJECT_RECT_TABLE_MAX_NODES 64u
#define DM2_V1_SKPROJECT_RECT_TABLE_PAYLOAD_CAPACITY 8192u

typedef struct {
    uint16_t next_index;
    uint16_t min_rect;
    uint16_t max_rect;
    uint8_t mask;
    uint8_t common_x;
    uint32_t payload_offset;
    uint32_t payload_size;
} DM2_V1_SkprojectRectNode;

typedef struct {
    DM2_V1_SkprojectRectNode nodes[DM2_V1_SKPROJECT_RECT_TABLE_MAX_NODES];
    uint8_t payload[DM2_V1_SKPROJECT_RECT_TABLE_PAYLOAD_CAPACITY];
    uint16_t node_count;
    uint32_t payload_used;
} DM2_V1_SkprojectRectTable;

typedef struct {
    int valid;
    int blocked_bad_magic;
    int blocked_missing_input;
    int blocked_missing_output;
    int blocked_group_overflow;
    int blocked_payload_overflow;
    int blocked_malformed_range;
    int blocked_malformed_words;
    uint16_t group_count;
    uint16_t node_count;
    uint32_t consumed_words;
    uint32_t payload_used;
    uint32_t table_hash;
} DM2_V1_SkprojectCompressRectsReceipt;

typedef struct {
    int valid;
    int blocked_zero_rect;
    int blocked_missing_table;
    int blocked_not_found;
    int blocked_payload_bounds;
    uint16_t rectno;
    uint16_t node_index;
    uint16_t local_index;
    uint8_t mask;
    uint8_t row_size;
    DM2_V1_SkprojectRect rect;
    uint32_t row_offset;
    uint32_t table_hash;
} DM2_V1_SkprojectQueryRectReceipt;

typedef struct {
    int valid;
    int blocked_missing_input;
    int immediate_colors_before;
    int driver_setcolors_requested;
    uint32_t converted_entries;
    uint8_t dmpal6[256][3];
    uint32_t dmpal_hash;
} DM2_V1_SkprojectDriverPaletteReceipt;

typedef struct {
    int valid;
    uint8_t set;
    int fade_to_black_requested;
    int driver_setcolors_requested;
    int immediate_colors_after;
    uint16_t vsync_waits;
    uint32_t receipt_hash;
} DM2_V1_SkprojectPaletteSetReceipt;

typedef struct {
    int valid;
    int blocked_missing_output;
    uint16_t colors_before;
    uint16_t colors_after;
    uint16_t converted_colors;
    uint8_t large_palette_copy;
    uint8_t palette[256];
    uint32_t palette_hash;
} DM2_V1_SkprojectXlatPaletteReceipt;

typedef struct {
    int valid;
    uint16_t record_word_e;
    int16_t direction_delta;
    uint8_t facing;
    uint8_t relative_direction;
    uint8_t creature_5x5_pos;
    int8_t creature_x_offset;
    int side_direction;
    int side_offset_nonzero;
} DM2_V1_SkprojectMoveSideOffsetReceipt;

typedef struct {
    uint8_t rgb6[256][3];
    uint8_t base_rgb6[16][3];
    uint8_t active_set;
    uint8_t update_palette;
} DM2_V1_SkprojectIbmioPaletteState;

typedef struct {
    int valid;
    int blocked_missing_palette;
    uint8_t set;
    uint8_t driver_update_requested;
    uint8_t base_palette_updated;
    uint32_t palette_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectIbmioPaletteReceipt;

typedef struct {
    int valid;
    int blocked_missing_buffer;
    int blocked_missing_palette;
    int blocked_out_of_bounds;
    uint16_t off_src_pixels;
    uint16_t off_dst_pixels;
    uint16_t width_pixels;
    uint32_t copied_pixels;
    uint32_t palette_hash;
    uint32_t dest_hash;
} DM2_V1_SkprojectIbmioBlit4To8Receipt;

typedef struct {
    int valid;
    int blocked_missing_buffer;
    int blocked_out_of_bounds;
    uint16_t off_src_pixels;
    uint16_t off_dst_pixels;
    uint16_t width_pixels;
    uint32_t copied_pixels;
    uint32_t buffer_hash;
} DM2_V1_SkprojectAnimCopy4BppReceipt;

typedef struct {
    uint16_t hide_depth;
    uint16_t cursor_shape;
    uint16_t cursor_bounds[4];
    uint16_t cursor_bounds_mode;
    uint8_t cursor_bounds_dirty;
    uint8_t cursor_redraws;
    uint8_t event_lock_depth;
} DM2_V1_SkprojectMouseState;

typedef struct {
    int valid;
    uint16_t hide_depth_before;
    uint16_t hide_depth_after;
    uint8_t locked_mouse_event;
    uint8_t redrew_cursor;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseHideReceipt;

typedef struct {
    int valid;
    uint16_t shape_before;
    uint16_t shape_after;
    uint8_t redrew_before_shape_change;
    uint8_t redrew_after_shape_change;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseShapeReceipt;

typedef struct {
    int valid;
    int blocked_missing_bounds;
    uint16_t bounds[4];
    uint16_t mode;
    uint8_t bounds_dirty;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseBoundsReceipt;

/* SKWIN/SkWinCore.cpp:^443C UI tracking list and mouse-event lock family.
   Models the source sk0cea linked list ordered by b3_0_3 priority,
   the _4976_5dae head, and the _01b0_0ca4 cursor-bounds context. */
#define DM2_V1_SKPROJECT_UI_TRACK_MAX_OBJECTS 16u

typedef struct {
    uint16_t id;          /* w0_0_d() rect/token identifier */
    uint8_t priority;     /* b3_0_3() insertion order key */
    uint8_t tracked;      /* b3_7_7() list membership flag */
    uint8_t absolute;     /* w0_f_f() absolute-position flag */
    uint8_t has_bounds;   /* b5() geometry-present flag */
    int8_t prev;
    int8_t next;
} DM2_V1_SkprojectUiTrackingObject;

typedef struct {
    DM2_V1_SkprojectUiTrackingObject objects[DM2_V1_SKPROJECT_UI_TRACK_MAX_OBJECTS];
    int8_t head;
    uint8_t count;
    uint16_t context_ref;
    int16_t track_start_x; /* _4976_5d98 / _4976_5daa */
    int16_t track_end_x;   /* _4976_5da8 / _4976_5dae.rc4.x */
    int16_t track_start_y; /* _4976_5d9c / _4976_5daa */
    int16_t track_end_y;   /* _4976_5d9e / _4976_5dae.rc4.y */
    DM2_V1_SkprojectMouseState mouse_state;
} DM2_V1_SkprojectUiTrackingState;

typedef struct {
    int valid;
    uint8_t lock_depth_before;
    uint8_t lock_depth_after;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseEventLockReceipt;

typedef struct {
    int valid;
    uint8_t lock_depth_before;
    uint8_t lock_depth_after;
    uint8_t underflow;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseEventUnlockReceipt;

typedef struct {
    int valid;
    uint8_t hide_requested;
    uint8_t show_requested;
    uint8_t bounds_requested;
    DM2_V1_SkprojectRect reset_rect;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseTrackingResetReceipt;

typedef struct {
    int valid;
    int blocked_missing_state;
    uint16_t context_ref;
    int16_t track_start_x;
    int16_t track_end_x;
    int16_t track_start_y;
    int16_t track_end_y;
    uint16_t bounds[4];
    uint8_t bounds_mode;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMouseTrackingContextReceipt;

typedef struct {
    int valid;
    int blocked_missing_object;
    int blocked_already_tracked;
    int blocked_list_full;
    uint16_t object_id;
    uint8_t priority;
    uint8_t inserted;
    int8_t prev_id;
    int8_t next_id;
    uint8_t bounds_requested;
    DM2_V1_SkprojectRect bounds_rect;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiTrackingInsertReceipt;

typedef struct {
    int valid;
    int blocked_missing_object;
    int blocked_not_tracked;
    int blocked_not_found;
    uint16_t object_id;
    uint8_t removed;
    int8_t prev_id;
    int8_t next_id;
    uint8_t reset_requested;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiTrackingRemoveReceipt;

typedef struct {
    uint32_t interrupt_ff_vector;
    uint32_t interrupt_fe_vector;
    uint32_t active_interrupt_fe_vector;
    uint16_t timer_reload_ticks;
    int32_t anim_countdown;
    DM2_V1_SkprojectRect screen_rect;
    uint8_t sound_card_type;
    uint8_t display_mode_active;
    uint8_t display_callback_installed;
    uint16_t event_queue[10];
    uint8_t event_read_index;
    uint8_t event_count;
} DM2_V1_SkprojectAnimRuntimeState;

typedef struct {
    int valid;
    uint32_t captured_vector;
    uint32_t receipt_hash;
} DM2_V1_SkprojectAnimVectorReceipt;

typedef struct {
    int valid;
    uint32_t captured_vector;
    uint32_t active_vector;
    uint16_t timer_reload_ticks;
    uint8_t callback_installed;
    uint32_t receipt_hash;
} DM2_V1_SkprojectAnimTimerInstallReceipt;

typedef struct {
    int valid;
    int32_t countdown_before;
    int32_t countdown_after;
    uint16_t timer_reload_ticks;
    uint32_t receipt_hash;
} DM2_V1_SkprojectAnimTimerTickReceipt;

typedef struct {
    int valid;
    uint8_t display_callback_called;
    uint8_t event_available;
    uint8_t event_count;
    uint32_t receipt_hash;
} DM2_V1_SkprojectIbmioPollReceipt;

typedef struct {
    int valid;
    int blocked_no_event;
    uint16_t event_word;
    uint8_t event_count_before;
    uint8_t event_count_after;
    uint8_t event_read_index_after;
    uint32_t receipt_hash;
} DM2_V1_SkprojectIbmioWaitEventReceipt;

typedef struct {
    int valid;
    DM2_V1_SkprojectRect rect;
    uint8_t color;
    uint32_t filled_pixels;
    uint32_t receipt_hash;
} DM2_V1_SkprojectScreenRectFillReceipt;

typedef struct {
    int valid;
    uint32_t lfsr_lines_visited;
    uint32_t lines_filled;
    uint8_t color;
    uint32_t receipt_hash;
} DM2_V1_SkprojectScreenClearReceipt;

typedef struct {
    int valid;
    uint8_t sound_card_type;
    uint8_t available;
    uint32_t receipt_hash;
} DM2_V1_SkprojectSoundAvailableReceipt;

enum {
    DM2_V1_SKPROJECT_UI_PRED_RETURN_1 = 0,
    DM2_V1_SKPROJECT_UI_PRED_IS_GAME_ENDED = 1,
    DM2_V1_SKPROJECT_UI_PRED_1031_0023 = 2,
    DM2_V1_SKPROJECT_UI_PRED_1031_003E = 3,
    DM2_V1_SKPROJECT_UI_PRED_1031_007B = 4,
    DM2_V1_SKPROJECT_UI_PRED_1031_009E = 5,
    DM2_V1_SKPROJECT_UI_PRED_1031_00C5 = 6,
    DM2_V1_SKPROJECT_UI_PRED_1031_00F3 = 7,
    DM2_V1_SKPROJECT_UI_PRED_1031_012D = 8,
    DM2_V1_SKPROJECT_UI_PRED_1031_014F = 9,
    DM2_V1_SKPROJECT_UI_PRED_1031_0184 = 10,
    DM2_V1_SKPROJECT_UI_PRED_1031_01BA = 11
};

typedef struct {
    uint8_t b0;
    uint8_t b1;
    uint16_t w2;
} DM2_V1_SkprojectUiNodeRef;

typedef struct {
    uint8_t game_has_ended;
    uint8_t selected_panel_token;
    uint8_t champion_inventory;
    uint16_t champion_hp[4];
    int8_t player_at_position[4];
    uint8_t player_dir;
    uint8_t toggle_5dbc;
    uint8_t champion_index;
    uint8_t selected_spell_panel;
    uint8_t champion_runes_count[4];
    uint16_t magical_map_flags;
    uint8_t right_panel_type;
} DM2_V1_SkprojectUiPredicateState;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_ref;
    int blocked_unknown_predicate;
    int blocked_champion_index;
    uint8_t predicate_index;
    uint8_t ref_b0;
    uint8_t ref_b1;
    uint16_t ref_w2;
    uint8_t player_position_index;
    int8_t player_at_position;
    uint8_t result;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiPredicateReceipt;

typedef struct {
    uint16_t w0;
    uint16_t w2;
    uint16_t w4;
    uint8_t b6;
} DM2_V1_SkprojectUiLeafMeta;

typedef struct {
    int valid;
    uint16_t child_offset;
    uint8_t first_child_index;
    uint8_t first_child_has_stop_bit;
    uint8_t blocked_missing_ref;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_child_offset;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiChildListReceipt;

typedef struct {
    int valid;
    uint16_t input_rectno;
    uint16_t base_rectno;
    uint16_t offset_rectno;
    uint8_t applied_8000_offset;
    uint8_t applied_4000_offset;
    uint8_t blocked_missing_rects;
    uint8_t blocked_missing_output;
    uint8_t blocked_rect_out_of_bounds;
    DM2_V1_SkprojectRect rect;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiResolveRectReceipt;

typedef struct {
    int valid;
    uint16_t visited_nodes;
    uint16_t marked_leaves;
    uint16_t recursed_nodes;
    uint16_t rejected_nodes;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_nodes;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_node_index;
    uint8_t blocked_child_offset;
    uint8_t blocked_leaf_index;
    uint8_t blocked_recursion_limit;
    uint8_t last_predicate_index;
    uint8_t last_node_index;
    uint32_t leaf_meta_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiTraverseReceipt;

typedef struct {
    uint16_t w0;
    uint16_t w2;
    uint16_t w4;
} DM2_V1_SkprojectUiAction;

typedef struct {
    uint16_t event_code;
    uint16_t rectno;
    uint16_t mask;
} DM2_V1_SkprojectUiResolvedAction;

typedef struct {
    int valid;
    uint16_t leaf_index;
    uint16_t action_index;
    uint8_t found;
    uint8_t blocked_missing_ref;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_leaf_index;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiActionListReceipt;

typedef struct {
    int valid;
    uint16_t visited_nodes;
    uint16_t tested_leaves;
    uint16_t recursed_nodes;
    uint16_t rejected_nodes;
    uint16_t selected_event;
    uint16_t selected_leaf_index;
    uint16_t selected_action_index;
    uint16_t selected_rectno;
    uint16_t selected_action_ordinal;
    int16_t point_x;
    int16_t point_y;
    uint16_t action_mask;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_nodes;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_missing_actions;
    uint8_t blocked_missing_rects;
    uint8_t blocked_node_index;
    uint8_t blocked_child_offset;
    uint8_t blocked_leaf_index;
    uint8_t blocked_action_index;
    uint8_t blocked_rect_lookup;
    uint8_t blocked_recursion_limit;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiHitTestReceipt;

typedef struct {
    int valid;
    uint16_t visited_nodes;
    uint16_t tested_leaves;
    uint16_t recursed_nodes;
    uint16_t selected_event;
    uint16_t selected_leaf_index;
    uint16_t selected_action_index;
    uint16_t searched_action_code;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_nodes;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_missing_actions;
    uint8_t blocked_node_index;
    uint8_t blocked_child_offset;
    uint8_t blocked_leaf_index;
    uint8_t blocked_action_index;
    uint8_t blocked_recursion_limit;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiActionSearchReceipt;

typedef struct {
    int valid;
    uint16_t searched_action_code;
    uint16_t found_action_index;
    uint16_t found_leaf_index;
    uint8_t found;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_nodes;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_missing_actions;
    uint8_t blocked_node_index;
    uint8_t blocked_child_offset;
    uint8_t blocked_leaf_index;
    uint8_t blocked_action_index;
    uint8_t blocked_recursion_limit;
    uint32_t visited_nodes;
    uint32_t recursed_nodes;
    uint32_t tested_leaves;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiSearchActionReceipt;

typedef struct {
    int valid;
    uint16_t input_event_code;
    uint8_t found_action;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_runtime;
    uint8_t blocked_missing_nodes;
    uint8_t blocked_missing_child_bytes;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_missing_actions;
    uint8_t blocked_missing_rects;
    uint8_t blocked_action_index;
    uint8_t blocked_rect_lookup;
    DM2_V1_SkprojectRect queued_rect;
    uint16_t queued_action_value;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiQueueEventReceipt;

typedef struct {
    int valid;
    uint16_t scanned_actions;
    uint16_t selected_event;
    uint16_t selected_action_index;
    uint16_t selected_action_ordinal;
    uint16_t selected_rectno;
    uint16_t selected_offset_rectno;
    uint16_t selected_event_delta;
    int16_t selected_x;
    int16_t selected_y;
    int16_t point_x;
    int16_t point_y;
    uint16_t action_mask;
    uint8_t found;
    uint8_t blocked_missing_runtime;
    uint8_t blocked_missing_actions;
    uint8_t blocked_missing_rects;
    uint8_t blocked_action_index;
    uint8_t blocked_rect_lookup;
    DM2_V1_SkprojectRect rect;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiActionResolveReceipt;

typedef struct {
    uint8_t flags_b3;
    uint8_t refresh_link_1;
    uint8_t refresh_link_2;
} DM2_V1_SkprojectUiClickRectNode;

typedef struct {
    int valid;
    int16_t previous_tree;
    int16_t selected_tree;
    uint16_t scanned_leaves;
    uint16_t activated_leaves;
    uint16_t deactivated_leaves;
    uint16_t clickrect_refresh_1;
    uint16_t clickrect_refresh_2;
    uint8_t requested_event_reset;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_roots;
    uint8_t blocked_missing_leaf_meta;
    uint8_t blocked_missing_clickrects;
    uint8_t blocked_tree_index;
    DM2_V1_SkprojectUiTraverseReceipt traverse_receipt;
    uint32_t leaf_meta_hash;
    uint32_t clickrect_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiSelectTreeReceipt;

typedef struct {
    int valid;
    uint8_t cleared_vcaptures;
    uint8_t cleared_pending_redraw;
    uint8_t cleared_event_table;
    uint8_t requested_squad_recompute;
    uint8_t requested_mouse_release_capture;
    int16_t capture_count_before;
    int16_t capture_count_after;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiResetCaptureReceipt;

typedef struct {
    int valid;
    uint8_t remapped_v1d338c;
    uint8_t remapped_v1d39bc;
    uint8_t remapped_table1d3cd0;
    uint8_t remapped_table1d3ba0;
    uint8_t remapped_table1d3ed5;
    uint8_t remapped_table1d3d23;
    uint8_t remapped_clickrects;
    uint16_t v1d338c_count;
    uint16_t v1d39bc_count;
    uint16_t table1d3cd0_count;
    uint8_t blocked_missing_tables;
    uint32_t table_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiTableRemapReceipt;

typedef struct {
    int valid;
    int16_t input_x;
    int16_t input_y;
    uint16_t input_code;
    uint16_t item_in_hand;
    uint8_t blocked_missing_hero;
    uint8_t blocked_missing_item;
    uint8_t blocked_not_magical_map;
    uint8_t blocked_not_map_chip;
    uint8_t blocked_missing_minion;
    uint8_t blocked_invalid_tile;
    uint8_t blocked_same_position;
    int16_t map_origin_x;
    int16_t map_origin_y;
    int16_t cell_stride_x;
    int16_t cell_stride_y;
    int16_t map_offset_x;
    int16_t map_offset_y;
    int16_t target_map;
    int16_t target_x;
    int16_t target_y;
    uint8_t requested_change_map;
    uint8_t requested_set_destination;
    uint8_t requested_1c9a_0247;
    uint8_t requested_update_right_panel;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiMagicalMapClickReceipt;

typedef struct {
    uint16_t button;
    int16_t x;
    int16_t y;
} DM2_V1_SkprojectUiMouseEvent;

typedef struct {
    uint8_t pending_capture_redraw;
    uint8_t show_item_stats;
    uint8_t capture_item_stats;
    uint8_t capture_panel;
    uint8_t mouse_visibility;
    uint8_t requested_guidraw_29ee_000f;
    uint8_t requested_mouse_release_capture;
    uint8_t requested_show_mouse_cursor;
    uint8_t queue_busy;
    uint8_t pending_mouse_event;
    DM2_V1_SkprojectUiMouseEvent pending_event;
    DM2_V1_SkprojectUiMouseEvent event_queue[11];
    uint8_t event_read_index;
    uint8_t event_write_index;
    uint8_t event_count;
    uint8_t filter_active;
    uint16_t active_tree;
    uint16_t previous_tree;
    uint16_t saved_tree;
    uint16_t selected_rectno;
    uint16_t selected_offset_rectno;
    int16_t selected_x;
    int16_t selected_y;
    uint16_t ui_event_code;
    uint16_t ui_event_delta;
    uint16_t queued_action_code;
} DM2_V1_SkprojectUiRuntimeState;

typedef struct {
    int valid;
    uint8_t cleared_pending_capture_redraw;
    uint8_t requested_guidraw_29ee_000f;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiPendingRedrawReceipt;

typedef struct {
    int valid;
    uint8_t cleared_sources;
    uint8_t requested_mouse_release_capture;
    uint8_t requested_show_mouse_cursor;
    uint8_t mouse_visibility;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiMouseCaptureReceipt;

typedef struct {
    int valid;
    uint8_t drained_host_events;
    uint8_t kept_events;
    uint8_t dropped_events;
    uint8_t queued_pending_event;
    uint16_t previous_event_code;
    uint16_t event_code_after;
    uint16_t selected_rectno_after;
    uint32_t queue_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiEventResetReceipt;

typedef struct {
    uint16_t button_dbidx;
    DM2_V1_SkprojectRect rect;
    uint8_t allocated_clickrectdata;
    uint8_t copied_mouse_rect;
} DM2_V1_SkprojectUiButtonGroup;

typedef struct {
    int valid;
    uint16_t button_dbidx_before;
    uint16_t button_dbidx_after;
    DM2_V1_SkprojectRect mouse_rect;
    DM2_V1_SkprojectRect container_rect;
    DM2_V1_SkprojectRect centered_rect;
    uint16_t width;
    uint16_t height;
    uint8_t copied_mouse_rect;
    uint8_t requested_alloc_clickrectdata;
    uint8_t blocked_missing_group;
    uint8_t blocked_missing_container;
    uint8_t blocked_missing_output;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUiCenteredButtonReceipt;

typedef struct {
    uint16_t requested_move;
    uint16_t current_tile_value;
    uint16_t destination_tile_value;
    int destination_tile_blocked;
    int16_t destination_x;
    int16_t destination_y;
    int16_t creature_at_destination;
    uint16_t creature_ai_flags;
    int side_offset_nonzero;
    int16_t secondary_query_creature;
    uint16_t secondary_query_ai_flags;
} DM2_V1_SkprojectMoveAdmissionRequest;

typedef struct {
    int valid;
    int16_t stored_creature;
    uint8_t current_tile_type;
    uint8_t destination_tile_type;
    uint8_t result_code;
    int used_side_offset_test;
    int used_secondary_query;
} DM2_V1_SkprojectMoveAdmissionReceipt;

typedef struct {
    int valid;
    int in_bounds;
    int16_t previous_map;
    int16_t selected_map;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t previous_record_w6;
    uint16_t new_record_w6;
} DM2_V1_SkprojectMinionDestinationReceipt;

typedef struct {
    int valid;
    uint16_t random_input;
    uint16_t divisor;
    uint16_t range_input;
    uint16_t savegame_seed;
    uint32_t mixed_value;
    uint16_t result;
} DM2_V1_SkprojectMapRandomReceipt;

typedef struct {
    int valid;
    int blocked_missing_tiles;
    int blocked_out_of_bounds;
    int16_t dir;
    int16_t forward;
    int16_t side;
    int16_t input_x;
    int16_t input_y;
    int16_t tile_x;
    int16_t tile_y;
    uint8_t tile_value;
} DM2_V1_SkprojectMapTileVectorReceipt;

typedef struct {
    int valid;
    int blocked_missing_tiles;
    int blocked_missing_passage;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    int in_bounds;
    int x_in_bounds;
    int y_in_bounds;
    int16_t probed_x;
    int16_t probed_y;
    uint8_t returned_tile_value;
    uint8_t returned_boundary_mask;
    uint8_t returned_blocked_value;
    uint8_t used_left_boundary;
    uint8_t used_right_boundary;
    uint8_t used_top_boundary;
    uint8_t used_bottom_boundary;
    uint8_t used_corner_boundary;
    uint8_t checked_primary_passage;
    uint8_t checked_side_passage;
} DM2_V1_SkprojectGetTileValueReceipt;

typedef struct {
    int valid;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t fill;
    uint16_t aligned_width;
    uint32_t pixel_count;
    uint8_t requested_fill_rect_any;
    uint8_t requested_offset_rect;
    uint8_t requested_dirty_rect;
    int blocked_missing_buffer;
    int blocked_missing_rect;
} DM2_V1_SkprojectFillReceipt;

typedef struct {
    int valid;
    int blocked_missing_buffer;
    uint16_t count;
    uint16_t delta;
    uint8_t value;
    uint16_t written_entries;
    uint16_t last_offset;
    uint32_t buffer_hash;
} DM2_V1_SkprojectFillStrReceipt;

typedef struct {
    int valid;
    int blocked_missing_rect;
    int blocked_missing_pixels;
    int used_query_expanded_rect;
    uint16_t rectno;
    DM2_V1_SkprojectRect rect;
    uint16_t stride;
    uint16_t visited_pixels;
    uint16_t cleared_pixels;
    uint32_t pixel_hash;
} DM2_V1_SkprojectHalftoneRectReceipt;

typedef struct {
    int valid;
    int16_t previous_capture_count;
    int16_t new_capture_count;
    uint8_t requested_driver_command;
} DM2_V1_SkprojectMouseReleaseCaptureReceipt;

typedef struct {
    int valid;
    uint8_t cls4_input;
    uint8_t cls4_drawn;
    uint16_t rectno;
    uint8_t bright;
    uint8_t requested_hide_mouse;
    uint8_t requested_query_rect;
    uint8_t requested_fill_entire_pict;
    uint8_t requested_draw_icon_entry;
    uint8_t requested_show_mouse;
    uint8_t requested_wait_refresh;
} DM2_V1_SkprojectHighlightArrowPanelReceipt;

typedef struct {
    int valid;
    int16_t previous_current_map;
    int16_t new_v1e0270;
    int16_t new_v1e0272;
    int requested_change_to_previous_map;
} DM2_V1_SkprojectMap3B001Receipt;

typedef struct {
    int valid;
    uint8_t blocked_zero_gate;
    uint8_t blocked_empty_table;
    uint16_t candidate_count;
    uint16_t selected_index;
    uint16_t selected_value;
    uint32_t mixed_random;
} DM2_V1_SkprojectMap1815Receipt;

typedef struct {
    int valid;
    uint16_t values[4];
    uint8_t sanitized[4];
    uint8_t blocked_missing_output;
} DM2_V1_SkprojectMap185AReceipt;

typedef struct {
    int16_t map_width;
    int16_t map_height;
    int16_t current_map_index;
    int16_t x;
    int16_t y;
    uint16_t dungeon_seed;
    uint16_t wall_random_decoration_count;
    uint16_t gates[4];
    uint16_t rotation;
    const uint8_t *candidate_table;
    uint16_t candidate_table_count;
    const uint8_t *ornate_alcove_flags;
} DM2_V1_SkprojectMap0CEEWallDecorationState;

typedef struct {
    int valid;
    uint16_t divisor;
    uint16_t range_input;
    uint16_t step_plus_one;
    uint16_t random_input[4];
    uint16_t selected_index[4];
    uint16_t values[4];
    uint8_t gates[4];
    uint8_t used_candidate[4];
    uint8_t returned_default[4];
    uint8_t sanitized[4];
    uint8_t requested_random_17e7[4];
    uint8_t requested_wall_random_decoration_count;
    uint8_t requested_wall_ornate_alcove_type;
    uint8_t out_of_map;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_output;
    uint8_t blocked_missing_candidates;
    uint8_t blocked_zero_range;
    uint32_t candidate_hash;
    uint32_t receipt_hash;
} DM2_V1_SkprojectMap0CEEWallDecorationReceipt;

typedef struct {
    int valid;
    int blocked_missing_rect;
    int blocked_missing_overlay;
    uint16_t width;
    uint16_t height;
    uint16_t xend;
    uint16_t initial_srcofs;
    uint16_t pixperline;
    int16_t alpha_mask;
    uint8_t palette_update_requested;
    uint8_t used_plain_path;
    uint8_t used_overlay_path;
    uint8_t used_alpha_blit;
    uint16_t blit_runs;
    uint16_t skipped_prefix_pixels;
    uint16_t skipped_suffix_pixels;
    uint16_t odd_width_source_advances;
    uint32_t dest_start_offset;
    uint32_t dest_final_offset;
    uint32_t source_cursor_hash;
} DM2_V1_SkprojectBlitSpecialEffectsReceipt;

typedef struct {
    int valid;
    int blocked_missing_rect;
    uint16_t rect_x;
    uint16_t rect_y;
    uint16_t rect_w;
    uint16_t rect_h;
    int16_t src_x;
    int16_t src_y;
    int16_t color_key;
    int16_t flip_mirror;
    uint16_t source_width;
    uint16_t source_height;
    uint16_t dest_stride;
    uint8_t bpp;
    uint8_t requested_offset_rect;
    uint8_t requested_fire_blit_picture;
    uint8_t requested_dirty_rect;
    uint8_t requested_local_palette;
} DM2_V1_SkprojectDrawIconPictBuffReceipt;

typedef struct {
    int valid;
    int blocked_missing_button_group;
    uint8_t category;
    uint8_t cls2;
    uint8_t entry;
    uint16_t button_id;
    int16_t alpha_mask;
    int16_t src_x;
    int16_t src_y;
    uint8_t requested_image_entry;
    uint8_t requested_blit_rect;
    uint8_t requested_local_palette;
    uint8_t requested_icon_pict_buff;
} DM2_V1_SkprojectDrawIconPictEntryReceipt;

typedef struct {
    int16_t blit_x;
    int16_t blit_y;
    int16_t inflate;
} DM2_V1_Skproject2405RectState;

typedef struct {
    int valid;
    uint16_t rectno;
    DM2_V1_SkprojectRect source_rect;
    DM2_V1_SkprojectRect rect;
    int16_t blit_x;
    int16_t blit_y;
    int16_t inflate;
    uint8_t requested_query_blit_rect;
    uint8_t requested_inflate_rect;
    uint8_t blocked_missing_state;
    uint8_t blocked_missing_rects;
    uint8_t blocked_missing_output;
    uint8_t blocked_rect_out_of_bounds;
    uint32_t receipt_hash;
} DM2_V1_Skproject2405RectReceipt;

typedef struct {
    uint16_t object_id;
    uint16_t dbspec_word6;
    uint16_t item_w2;
    uint16_t game_tick;
    uint16_t player_dir;
    uint16_t random16;
    uint8_t fit_for_equip;
    uint8_t champion_index;
    uint8_t selected_hand_action;
    const uint16_t *selected_hand_items;
    uint16_t selected_hand_item_count;
} DM2_V1_Skproject2405ItemState;

typedef struct {
    int valid;
    uint16_t object_id;
    uint16_t dbspec_word6;
    uint16_t frame_count;
    uint16_t animation_mode;
    uint16_t selected_entry;
    uint16_t charge;
    uint16_t max_charge;
    uint16_t bucket_width;
    uint16_t tick_input;
    uint16_t random_input;
    uint16_t player_dir;
    uint8_t base_entry;
    uint8_t used_equip_variant;
    uint8_t used_tick_mode;
    uint8_t used_random_mode;
    uint8_t used_direction_mode;
    uint8_t used_charge_mode;
    uint8_t used_charge_tick_mode;
    uint8_t blocked_missing_state;
    uint8_t blocked_not_drawn;
    uint8_t blocked_not_fit_for_equip;
    uint8_t blocked_selected_hand;
    uint8_t blocked_zero_bucket_width;
    uint32_t receipt_hash;
} DM2_V1_Skproject2405ItemEntryReceipt;

typedef struct {
    int valid;
    int blocked_missing_picture;
    int blocked_invalid_dimensions;
    int blocked_missing_blit_rect;
    uint16_t rect_no;
    uint16_t width;
    uint16_t height;
    int16_t src_x;
    int16_t src_y;
    int16_t dst_x;
    int16_t dst_y;
    int16_t color_key;
    uint8_t rect_no_used_direct_xy;
    uint8_t rect_no_forced_blit_flag;
    uint8_t requested_query_pict_bits;
    uint8_t requested_query_blit_rect;
    uint8_t requested_blit;
} DM2_V1_SkprojectDrawDefPictReceipt;

typedef struct {
    int valid;
    int blocked_missing_rect;
    uint16_t cache_index;
    uint16_t dest_stride;
    uint16_t overlay_pattern;
    uint8_t requested_cache_buffer;
    uint8_t requested_offset_rect;
    uint8_t requested_gray_blit;
    uint8_t requested_dirty_rect;
} DM2_V1_SkprojectDrawGrayOverlayReceipt;

typedef struct {
    int valid;
    int blocked_inactive;
    uint16_t progress_per_mille;
    uint16_t expanded_rect_no;
    uint16_t computed_width;
    uint16_t previous_width;
    uint8_t requested_fill_backbuff_rect;
    uint8_t requested_dialogue_to_screen;
} DM2_V1_SkprojectDialogueProgressReceipt;

typedef struct {
    int valid;
    int blocked_missing_bitmap;
    int blocked_missing_rect;
    int dest_is_screen;
    uint16_t src_width;
    uint16_t dest_width;
    int16_t src_x;
    int16_t src_y;
    int16_t alpha_mask;
    uint8_t source_bpp;
    uint8_t dest_bpp;
    uint8_t requested_blit;
    uint8_t requested_palette;
} DM2_V1_SkprojectDialoguePictReceipt;

typedef struct {
    int valid;
    uint8_t requested_fill_entire_pict;
    uint8_t gdat_text_category;
    uint8_t gdat_text_cls2;
    uint8_t gdat_text_entry;
    uint16_t text_rect_no;
    uint16_t foreground_color;
    uint8_t requested_vp_rc_str;
} DM2_V1_SkprojectWakeUpTextReceipt;

#define DM2_V1_SKPROJECT_MAP_RECORD_END 0xfffeu
#define DM2_V1_SKPROJECT_OBJECT_NULL 0xffffu
#define DM2_V1_SKPROJECT_OBJECT_EFFECT_FIREBALL 0xff80u

typedef struct {
    uint16_t next;
    uint16_t w2;
    uint8_t record_type;
} DM2_V1_SkprojectMapRecord;

typedef struct {
    int valid;
    uint16_t record_link;
    uint8_t db_type;
    uint8_t real_db_type;
    uint16_t db_index;
    uint16_t record_count;
    uint16_t record_size;
    uint32_t byte_offset;
    uint8_t typed_accessor;
    uint8_t requested_type;
    uint8_t used_detached_record_route;
    uint8_t null_accessor;
    uint8_t generic_container_accessor;
    uint8_t actuator_accessor;
    int blocked_missing_counts;
    int blocked_missing_sizes;
    int blocked_end_marker;
    int blocked_object_null;
    int blocked_effect_record;
    int blocked_db_type_out_of_range;
    int blocked_index_out_of_range;
    int blocked_type_mismatch;
} DM2_V1_SkprojectRecordAddressReceipt;

typedef struct {
    int valid;
    uint16_t scanned_records;
    uint16_t matched_records;
    uint16_t updated_records;
    uint16_t counter_increment;
    uint8_t blocked_missing_records;
} DM2_V1_SkprojectMap20661F37Receipt;

typedef struct {
    int valid;
    uint16_t returned_head;
    uint16_t rewired_records;
    uint16_t appended_tail;
    uint8_t blocked_missing_records;
} DM2_V1_SkprojectMap20661EC9Receipt;

typedef struct {
    int valid;
    uint16_t offset;
    uint8_t previous_value;
    uint8_t new_value;
    uint8_t blocked_missing_tmpmap;
    uint8_t blocked_out_of_bounds;
} DM2_V1_SkprojectTmpmapFlagReceipt;

typedef struct {
    uint8_t map_id;
    int16_t world_x;
    int16_t world_y;
    int16_t width;
    int16_t height;
    uint8_t tile_type_at_local;
    uint8_t teleporter_record_active;
} DM2_V1_SkprojectMapDescriptor;

typedef struct {
    uint16_t category;
    uint16_t cls2;
    uint16_t type;
    uint16_t cls4;
    uint16_t raw_index;
    uint32_t raw_length;
} DM2_V1_SkprojectGdatDescriptor;

typedef struct {
    int valid;
    uint16_t inspected_entries;
    uint16_t unique_raw_indexes;
    uint32_t largest_raw_length;
    uint8_t sound_category_available;
    uint8_t sound_unique_count;
    uint8_t blocked_missing_entries;
    uint8_t blocked_missing_output;
    uint32_t receipt_hash;
} DM2_V1_SkprojectGdatSoundAllocationReceipt;

typedef struct {
    int valid;
    uint16_t raw_index;
    uint16_t entry_type_2;
    uint16_t entry_type_5;
    uint8_t upper_nibble;
    uint8_t current_zone;
    uint8_t accepted_zero_gate;
    uint8_t accepted_current_zone;
    uint8_t rejected_other_zone;
} DM2_V1_SkprojectGdatZoneReceipt;

typedef struct {
    int valid;
    uint16_t script_count;
    uint16_t visited_entries;
    uint16_t incremented_entries;
    uint16_t decremented_entries;
    uint16_t skipped_b_or_c_entries;
    uint16_t skipped_highbit_entries;
    uint16_t sound_gate_skips;
    uint8_t sound_table_active;
    uint8_t blocked_missing_scripts;
    uint8_t blocked_missing_marks;
    uint8_t blocked_mark_capacity;
    uint32_t mark_hash;
} DM2_V1_SkprojectLoadDyn4Receipt;

typedef struct {
    int valid;
    int found;
    uint8_t blocked_missing_descriptors;
    uint8_t blocked_missing_output;
    uint8_t used_resume_cursor;
    uint8_t scanned_candidates;
    int16_t source_map;
    int16_t locate_delta;
    int16_t source_world_x;
    int16_t source_world_y;
    int16_t selected_map;
    int16_t selected_x;
    int16_t selected_y;
    uint8_t selected_tile_type;
    uint8_t rejected_teleporter;
} DM2_V1_SkprojectLocateOtherLevelReceipt;

typedef struct {
    int valid;
    uint8_t in_bounds;
    uint8_t target_differs_from_current;
    uint8_t requested_change_to_target;
    uint8_t requested_restore_current;
    uint8_t requested_load_newmap;
    uint8_t requested_party_rotate;
    int16_t current_map;
    int16_t target_map;
    int16_t x;
    int16_t y;
    int16_t rotation;
    int16_t move_from_x;
    int16_t move_from_y;
    int16_t move_to_x;
    int16_t move_to_y;
} DM2_V1_SkprojectMap3BF83Receipt;

typedef struct {
    int valid;
    uint16_t active_v1e0534;
    uint16_t arrow_panel;
    int16_t highlight_param;
    uint8_t requested_highlight;
} DM2_V1_SkprojectArrowHighlightReceipt;

typedef struct {
    int valid;
    uint8_t requested_drop_record;
    int16_t source_map;
    int16_t source_x;
    int16_t source_y;
    int16_t locate_delta;
    int16_t located_map;
    int16_t located_x;
    int16_t located_y;
    int16_t query_rotation;
    int16_t final_party_dir;
    uint8_t requested_restore_source_map;
} DM2_V1_SkprojectOtherLevelReceipt;

typedef struct {
    int valid;
    uint8_t candidate_count;
    uint8_t wound_attempts;
    uint8_t wound_successes;
    uint8_t noise_requests;
    int16_t x;
    int16_t y;
    int16_t arg0;
    int16_t arg1;
    uint8_t first_direction;
    uint8_t second_direction;
    int16_t first_candidate;
    int16_t second_candidate;
    int16_t wounded_champions[2];
    uint8_t noise_hero_types[2];
} DM2_V1_SkprojectMove12B4023FReceipt;

typedef struct {
    int valid;
    uint8_t admitted;
    uint8_t blocked_door_closed_flag;
    uint8_t blocked_attack_power;
    uint8_t blocked_tile_type;
    uint8_t queued_timer;
    uint8_t changed_tile_type;
    uint8_t rebirth_altar;
    uint8_t test_byte_offset;
    uint8_t tested_flag_mask;
    uint8_t tile_type_before;
    uint8_t tile_type_after;
    uint16_t attack_power;
    uint16_t required_power;
    uint16_t timer_ticks;
    int16_t x;
    int16_t y;
} DM2_V1_SkprojectAttackDoorReceipt;

#define DM2_V1_SKPROJECT_PARTY_HERO_LIMIT 4u

typedef struct {
    uint8_t alive;
    uint16_t strength;
    uint16_t stamina_adjusted_strength;
    uint16_t max_stamina;
    uint16_t cur_stamina;
} DM2_V1_SkprojectLiftHero;

typedef struct {
    uint16_t creature_weight;
    uint16_t event_hero_index;
    uint16_t hero_count;
    DM2_V1_SkprojectLiftHero heroes[DM2_V1_SKPROJECT_PARTY_HERO_LIMIT];
    uint16_t rand16_values[DM2_V1_SKPROJECT_PARTY_HERO_LIMIT];
} DM2_V1_SkprojectLiftRequest;

typedef struct {
    int valid;
    uint8_t blocked_overweight_creature;
    uint8_t can_lift;
    uint16_t checked_heroes;
    uint16_t stamina_adjustments[DM2_V1_SKPROJECT_PARTY_HERO_LIMIT];
} DM2_V1_SkprojectLiftReceipt;

typedef struct {
    int valid;
    uint8_t ornate_alcove;
    uint8_t cls2_missing;
    uint8_t cls2;
    uint16_t data_index;
} DM2_V1_SkprojectWallAlcoveReceipt;

typedef struct {
    uint8_t present;
    uint8_t detail_b4;
} DM2_V1_SkprojectTeleporterProbe;

typedef struct {
    int valid;
    int16_t x;
    int16_t y;
    uint8_t direct_present;
    uint8_t adjacent_present;
    uint8_t checked_adjacent_count;
    uint8_t selected_direction;
    int16_t selected_x;
    int16_t selected_y;
    uint8_t teleporter_b4;
    uint8_t blocked_missing_adjacent_probes;
} DM2_V1_SkprojectTeleporterSearchReceipt;

typedef struct {
    int valid;
    uint16_t object_record;
    uint8_t base_direction;
    uint8_t terminal_direction;
    uint8_t kept_direction_for_ff89;
    uint8_t rotated_for_other_records;
    uint8_t requested_creature_push;
} DM2_V1_SkprojectThrownObjectTerminalReceipt;

typedef struct {
    int valid;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint16_t creature_weight;
    uint16_t force_threshold;
    uint16_t random_range;
    uint16_t random_value;
    uint8_t creature_movable;
    uint8_t blocked_unmovable;
    uint8_t lifted_by_force;
    uint8_t lifted_by_random_zero;
    uint8_t requested_lift_handoff;
} DM2_V1_SkprojectCreaturePushReceipt;

typedef struct {
    uint16_t link_word;
    uint8_t record_type;
    uint8_t actuator_class;
    uint16_t required_item_type;
    uint8_t target_flag;
    uint8_t activation_bits;
    uint8_t consume_projectile;
    uint8_t tile_type_at_destination;
    int16_t destination_x;
    int16_t destination_y;
    uint8_t side_when_tile_nonzero;
    uint8_t side_when_tile_zero;
    uint16_t alcove_data_index;
} DM2_V1_SkprojectWallAttackRecord;

typedef struct {
    int valid;
    uint8_t found_effect;
    uint8_t projectile_cut;
    uint8_t projectile_side_after;
    uint8_t invoked_actuator;
    uint8_t activated_record_index;
    uint8_t used_alcove_relocation;
    uint8_t used_teleport_relocation;
    uint8_t blocked_no_records;
    uint8_t checked_records;
    uint8_t matching_side_records;
    uint8_t requested_side;
    uint8_t wall_side;
    uint16_t projectile_item_type;
    int16_t source_x;
    int16_t source_y;
    int16_t target_x;
    int16_t target_y;
} DM2_V1_SkprojectAttackWallReceipt;

#define DM2_V1_SKPROJECT_CACHE_LIMIT 128
#define DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT 256
#define DM2_V1_SKPROJECT_MEMENT_BUFFER_BYTES 32
#define DM2_V1_SKPROJECT_MEMENT_NONE 0xffffu

typedef struct {
    uint32_t hashes[DM2_V1_SKPROJECT_CACHE_LIMIT];
    uint16_t sorted_cache_indices[DM2_V1_SKPROJECT_CACHE_LIMIT];
    uint16_t cache_to_mement[DM2_V1_SKPROJECT_CACHE_LIMIT];
    uint16_t raw_to_mement[DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT];
    uint8_t mement_buffers[DM2_V1_SKPROJECT_CACHE_LIMIT]
                          [DM2_V1_SKPROJECT_MEMENT_BUFFER_BYTES];
    uint16_t cache_count;
    uint16_t cache_capacity;
    uint16_t raw_count;
    uint16_t mement_count;
    uint16_t temp_hash_counter;
    uint16_t next_free_mementi;
    uint16_t mement_allocation_count;
    uint16_t lowest_free_cache_index;
} DM2_V1_SkprojectCacheState;

typedef struct {
    uint16_t index;
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
    uint32_t payload_bytes;
    uint16_t header_width;
    uint16_t header_height;
    uint16_t header_bpp;
} DM2_V1_SkprojectNewPictReceipt;

typedef struct {
    uint16_t w4;
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls4;
    uint16_t w12;
} DM2_V1_SkprojectPictureRef;

typedef struct {
    uint16_t w6;
    uint16_t w52;
    uint16_t w54;
} DM2_V1_SkprojectExtendedPictureRef;

typedef struct {
    uint16_t cache_index;
    uint16_t width;
    uint16_t height;
    uint16_t word22;
    uint8_t payload_available;
} DM2_V1_Skproject0B36CachePicture;

typedef struct {
    uint16_t w4;
    uint16_t w12;
    uint16_t w14;
    uint16_t w16;
    uint16_t w22;
    uint16_t width;
    uint16_t height;
    uint16_t rect_no;
    int16_t color_key_passthrough;
    uint8_t has_bits;
} DM2_V1_Skproject0B36Picture;

typedef struct {
    uint16_t dbidx;
    DM2_V1_SkprojectRect rect;
    uint16_t group_size;
    DM2_V1_SkprojectRect dirty_rects[5];
} DM2_V1_Skproject0B36ButtonGroup;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint16_t width;
    uint16_t height;
    uint16_t word22;
    uint8_t assigned_picture;
    uint8_t requested_mement_buffer;
    uint8_t blocked_missing_cache_picture;
    uint8_t blocked_missing_payload;
    uint8_t blocked_cache_index_mismatch;
    uint32_t receipt_hash;
} DM2_V1_Skproject0B36CachePictureReceipt;

typedef struct {
    int valid;
    uint16_t previous_group_size;
    uint16_t new_group_size;
    uint8_t reused_covering_rect;
    uint8_t replaced_contained_rect;
    uint8_t requested_compaction;
    uint8_t clipped_to_group;
    uint8_t dropped_empty_clip;
    uint8_t blocked_missing_group;
    uint8_t blocked_missing_rect;
    DM2_V1_SkprojectRect input_rect;
    DM2_V1_SkprojectRect stored_rect;
    uint32_t dirty_rect_hash;
    uint32_t receipt_hash;
} DM2_V1_Skproject0B36DirtyRectReceipt;

typedef struct {
    int valid;
    uint16_t rectno;
    uint16_t allocated_cache_index;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t requested_query_expanded_rect;
    uint8_t requested_alloc_temp_cache_index;
    uint8_t requested_alloc_new_pict;
    uint8_t requested_initial_dirty_rect;
    uint8_t blocked_missing_group;
    uint8_t blocked_missing_rects;
    uint8_t blocked_rect_out_of_bounds;
    DM2_V1_Skproject0B36DirtyRectReceipt dirty_receipt;
    uint32_t receipt_hash;
} DM2_V1_Skproject0B36ButtonGroupInitReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint16_t rectno;
    int16_t color_key;
    uint16_t width_before;
    uint16_t height_before;
    uint16_t width_after;
    uint16_t height_after;
    uint8_t requested_group_cache_bits;
    uint8_t requested_query_pict_bits;
    uint8_t requested_query_blit_rect;
    uint8_t requested_offset_rect;
    uint8_t requested_draw_def_pict;
    uint8_t requested_dirty_rect;
    uint8_t blocked_missing_picture;
    uint8_t blocked_missing_group;
    uint8_t blocked_missing_blit_rects;
    uint8_t blocked_rect_out_of_bounds;
    DM2_V1_SkprojectRect blit_rect;
    DM2_V1_SkprojectRect picture_rect;
    DM2_V1_Skproject0B36DirtyRectReceipt dirty_receipt;
    uint32_t receipt_hash;
} DM2_V1_Skproject0B36DrawCachedPictureReceipt;

typedef struct {
    uint16_t word30;
} DM2_V1_SkprojectCreatureAISpec;

typedef struct {
    int valid;
    uint16_t record_link;
    uint16_t word30;
    uint8_t blocked_object_null;
    uint8_t blocked_missing_ai_spec;
} DM2_V1_SkprojectCreatureAIWord30Receipt;

typedef struct {
    int valid;
    int16_t input_x;
    int16_t input_y;
    uint16_t current_map;
    int16_t direction; /* -1 = down, +1 = up */
    uint16_t flags;
    uint8_t tile_value;
    uint8_t tile_type;
    int16_t selected_map;
    int16_t selected_x;
    int16_t selected_y;
    uint8_t blocked_stairs_gate;
    uint8_t blocked_stairs_direction;
    uint8_t blocked_pit_ladder_gate;
    uint8_t blocked_no_ladder;
    uint8_t blocked_missing_tile;
    uint8_t requested_locate_other_level;
    uint8_t requested_change_to_selected;
    uint8_t requested_change_back;
    uint8_t rejected_target_pit_impassable;
    uint8_t requested_target_tile_check;
    uint8_t ladder_down_flag;
} DM2_V1_SkprojectLevelTransitionReceipt;

typedef struct {
    int valid;
    int16_t input_x;
    int16_t input_y;
    uint16_t current_map;
    DM2_V1_SkprojectLevelTransitionReceipt down_transition;
    DM2_V1_SkprojectLevelTransitionReceipt up_transition;
} DM2_V1_SkprojectLevelTransitionPairReceipt;

typedef struct {
    int valid;
    uint16_t rectno;
    uint8_t fill_black_requested;
    uint8_t group_already_initialized;
    uint8_t blocked_missing_group;
    DM2_V1_Skproject0B36ButtonGroupInitReceipt init_receipt;
} DM2_V1_SkprojectButtonGroupBlackFillReceipt;

typedef struct {
    int valid;
    uint16_t slot_count;
    uint16_t drawn_slots;
    uint8_t requested_draw_cmd_slot[16];
    uint8_t requested_draw_player_attack_dir;
} DM2_V1_SkprojectCommandSlotLoopReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint16_t dirty_rect_count;
    uint8_t requested_hide_mouse;
    uint8_t requested_show_mouse;
    uint8_t requested_blit_picture;
    uint8_t requested_free_temp_cache_index;
    uint8_t cache_index_cleared;
    uint8_t blocked_missing_group;
} DM2_V1_Skproject0B36BlitDirtyRectsReceipt;

typedef struct {
    int valid;
    int blocked_missing_text;
    uint16_t text_len;
    int16_t width;
    int16_t height;
} DM2_V1_SkprojectTextMetricsReceipt;

typedef struct {
    int valid;
    int16_t x;
    int16_t y;
    uint8_t clr1;
    uint8_t clr2;
    const char *text;
    uint8_t blocked_missing_text;
    uint8_t blocked_empty_text;
    uint8_t requested_draw_string;
    uint8_t requested_dirty_rect;
    DM2_V1_SkprojectTextMetricsReceipt metrics;
    DM2_V1_Skproject0B36DirtyRectReceipt dirty_receipt;
} DM2_V1_Skproject0B36DrawStringReceipt;

typedef struct {
    int valid;
    uint16_t active_v1e0534;
    uint16_t arrow_panel;
    uint8_t requested_highlight;
    DM2_V1_SkprojectHighlightArrowPanelReceipt highlight_receipt;
} DM2_V1_SkprojectSkWin12B40092Receipt;

typedef struct {
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls4;
    uint16_t data_index;
    uint16_t fallback_data_index;
    int data_absent;
    int fallback_absent;
    int16_t y_offset;
    uint8_t bits_pixel;
    uint16_t existing_mementi;
} DM2_V1_SkprojectImageMementRequest;

typedef enum {
    DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY = 0,
    DM2_V1_SKPROJECT_IMAGE_MEMENT_ABSENT = 1,
    DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET = 2,
    DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP = 3,
    DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY = 4,
    DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING = 5
} DM2_V1_SkprojectImageMementStatus;

typedef struct {
    DM2_V1_SkprojectImageMementStatus status;
    uint16_t selected_data_index;
    uint16_t touched_mementi;
    uint16_t pinned_entry_index;
} DM2_V1_SkprojectImageMementReceipt;

typedef struct {
    int valid;
    int recycled_to_free_list;
    uint16_t mementi;
    uint16_t previous_w4;
    uint16_t yy;
} DM2_V1_SkprojectRecycleMementReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint32_t cache_hash;
    uint16_t ici;
    uint16_t previous_cache_count;
    uint16_t new_cache_count;
    uint16_t previous_lowest_free_cache_index;
    uint16_t new_lowest_free_cache_index;
    uint8_t cleared_hash;
    uint8_t removed_sorted_entry;
    uint8_t blocked_missing_state;
    uint8_t blocked_out_of_range;
    uint8_t blocked_hash_not_found;
} DM2_V1_SkprojectDeallocFreeCacheIndexReceipt;

typedef struct {
    int valid;
    uint16_t index;
    uint16_t plain_index;
    uint16_t resolved_mementi;
    uint8_t used_cache_route;
    uint8_t cleared_raw_slot;
    uint8_t cleared_cache_slot;
    uint8_t requested_free_cache_index;
    uint8_t requested_recycle_mementi;
    uint8_t cleared_current_mementi;
    uint8_t blocked_missing_state;
    uint8_t blocked_no_mement;
    DM2_V1_SkprojectDeallocFreeCacheIndexReceipt free_cache;
    DM2_V1_SkprojectRecycleMementReceipt recycle;
} DM2_V1_SkprojectFreeIndexedMementReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint8_t requested_temp_pin_clear;
    uint8_t requested_free_indexed_mement;
    DM2_V1_SkprojectFreeIndexedMementReceipt indexed;
} DM2_V1_SkprojectFreeTempCacheIndexReceipt;

typedef struct {
    int valid;
    int recycled_fallback;
    int exhausted_after_allocation;
    uint16_t returned_mementi;
    uint16_t previous_next_free_mementi;
    uint16_t next_free_mementi;
    uint16_t fallback_mementi;
    uint16_t allocation_count;
} DM2_V1_SkprojectFindFreeMementiReceipt;

typedef struct {
    int valid;
    int cleared_pinned_entry;
    int recycled_existing;
    uint16_t selected_data_index;
    uint16_t recycled_mementi;
} DM2_V1_SkprojectFreeImageMementReceipt;

typedef enum {
    DM2_V1_SKPROJECT_PICT_MEMENT_NONE = 0,
    DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE = 1,
    DM2_V1_SKPROJECT_PICT_MEMENT_CACHE = 2
} DM2_V1_SkprojectPictMementRoute;

typedef struct {
    DM2_V1_SkprojectPictMementRoute route;
    DM2_V1_SkprojectImageMementReceipt image;
    uint16_t cache_index;
} DM2_V1_SkprojectPictMementReceipt;

typedef struct {
    int valid;
    uint8_t global_free_gate;
    uint8_t allocation_flag;
    uint8_t requested_dealloc_upper;
    uint8_t requested_dealloc_lower;
    uint8_t requested_draw_icon_entry;
    uint32_t allocation_handle;
} DM2_V1_SkprojectFreePict6Receipt;

typedef struct {
    int valid;
    uint16_t object_id;
    int db_type;
    int delta;
    uint16_t previous_w2;
    uint16_t new_w2;
    uint16_t previous_charge;
    uint16_t new_charge;
    uint16_t max_charge;
    int blocked_null_object;
    int blocked_unsupported_db_type;
} DM2_V1_SkprojectItemChargeReceipt;

#define DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT 32
#define DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS 30
#define DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS 8
#define DM2_V1_SKPROJECT_MONEY_ITEM_MAX 10

typedef struct {
    uint16_t object_id;
    uint16_t w2;
    uint16_t next_object_id;
    uint16_t contained_object_id;
    uint16_t gdat_word_values[0x36];
    uint16_t distinctive_item_type;
    uint8_t gdat_cls1;
    uint8_t gdat_cls2;
    uint8_t container_type;
    uint8_t is_moneybox;
    uint8_t is_currency;
    uint8_t champion_bones_owner;
} DM2_V1_SkprojectItemValueRecord;

typedef struct {
    const DM2_V1_SkprojectItemValueRecord *records;
    uint16_t record_count;
} DM2_V1_SkprojectItemValueWorld;

typedef struct {
    int valid;
    int blocked_null_object;
    int blocked_missing_record;
    int blocked_recursion_limit;
    uint16_t object_id;
    uint8_t cls4;
    int db_type;
    int32_t base_value;
    uint16_t charge;
    uint16_t charge_multiplier_cls4;
    int32_t charge_value_added;
    int32_t potion_value_before_scale;
    int32_t potion_value_after_scale;
    int32_t contained_recursive_value;
    int32_t moneybox_contained_value;
    int32_t moneybox_rounding_value;
    int32_t final_value;
} DM2_V1_SkprojectItemValueReceipt;

typedef struct {
    uint16_t inventory[DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS];
    uint16_t current_container_items[DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS];
    uint16_t selected_hand_items[2];
    uint8_t selected_hand_action;
    uint16_t selected_player_plus_one;
} DM2_V1_SkprojectPlayerWeightRequest;

typedef struct {
    int valid;
    uint16_t player;
    int included_open_chest_overlay;
    int blocked_missing_request;
    int blocked_player_not_selected;
    int blocked_selected_hand_action;
    int blocked_selected_hand_not_chest;
    uint32_t inventory_weight;
    uint32_t open_chest_weight;
    uint32_t final_weight;
    uint16_t hero_flag_or;
} DM2_V1_SkprojectPlayerWeightReceipt;

typedef struct {
    int valid;
    int blocked_null_object;
    int blocked_missing_request;
    int blocked_inventory_slot_range;
    int equipped_to_container_overlay;
    int process_item_bonus_requested;
    uint16_t player;
    uint16_t raw_object_id;
    uint16_t cleared_object_id;
    uint16_t inventory_slot;
    uint16_t container_slot;
    uint16_t previous_object_id;
} DM2_V1_SkprojectEquipItemReceipt;

typedef struct {
    int valid;
    int blocked_null_object;
    int blocked_missing_record;
    uint16_t object_id;
    uint8_t db_type;
    uint8_t container_type;
    uint8_t gdat_cls1;
    uint8_t gdat_cls2;
    uint8_t has_moneybox_item_list;
    uint8_t is_moneybox;
    uint8_t is_chest;
    uint8_t is_currency;
    uint16_t gdat_flags;
} DM2_V1_SkprojectItemClassifyReceipt;

typedef struct {
    int valid;
    int blocked_null_object;
    int blocked_missing_record;
    uint16_t object_id;
    uint8_t gdat_cls1;
    uint8_t gdat_cls2;
    uint8_t champion_bones_item_id;
    uint8_t champion_bones_owner;
    uint8_t champion_count;
    uint8_t requested_gdat_item_name;
    uint16_t champion_bones_index;
} DM2_V1_SkprojectItemNameReceipt;

typedef struct {
    int valid;
    int blocked_missing_text;
    uint16_t object_id;
    uint8_t container_cls2;
    uint16_t requested_order;
    uint16_t expanded_item_id;
    uint16_t parsed_slot_count;
    int16_t returned_money_index;
} DM2_V1_SkprojectItemOrderReceipt;

typedef struct {
    int valid;
    uint16_t value;
    uint16_t clean;
    uint16_t keta;
    uint8_t returned_offset;
    char buffer[5];
    char returned_text[5];
} DM2_V1_SkprojectFmtNumReceipt;

typedef struct {
    int valid;
    int blocked_missing_text;
    uint16_t length;
} DM2_V1_SkprojectStrLenReceipt;

typedef struct {
    int valid;
    int blocked_missing_haystack;
    int blocked_missing_needle;
    int needle_empty_returns_null;
    int found;
    uint16_t match_offset;
} DM2_V1_SkprojectStrStrReceipt;

typedef struct {
    int valid;
    int blocked_missing_output;
    int blocked_missing_input;
    int blocked_capacity;
    uint16_t copied_length;
    uint16_t result_length;
    uint32_t output_hash;
} DM2_V1_SkprojectStrCopyCatReceipt;

typedef struct {
    int valid;
    int blocked_missing_output;
    int blocked_capacity;
    int32_t value;
    uint16_t written_length;
    char text[16];
} DM2_V1_SkprojectLtoa10Receipt;

typedef struct {
    int valid;
    uint8_t input;
    uint8_t output;
} DM2_V1_SkprojectScriptChrReceipt;

typedef struct {
    int valid;
    int blocked_missing_output;
    int blocked_missing_record;
    int blocked_recursion_limit;
    uint16_t moneybox_object_id;
    uint16_t visited_records;
    uint16_t currency_records;
    uint16_t matched_currency_records;
    uint16_t money_item_count;
    int16_t counts[DM2_V1_SKPROJECT_MONEY_ITEM_MAX];
} DM2_V1_SkprojectCountByCoinTypesReceipt;

typedef struct {
    uint8_t current;
    uint8_t maximum;
} DM2_V1_SkprojectChampionAttribute;

typedef struct {
    int valid;
    int16_t value;
    uint8_t result;
} DM2_V1_SkprojectIsNegativeReceipt;

typedef struct {
    int valid;
    uint16_t record_link;
    uint8_t db_type;
    uint8_t container_type;
    uint8_t result;
    int blocked_object_null;
    int blocked_non_container_db;
} DM2_V1_SkprojectContainerMapReceipt;

typedef struct {
    int valid;
    int blocked_missing_inventory;
    uint8_t requested_kind;
    int16_t selected_slot;
    uint16_t selected_object;
    uint8_t checked_pouch1;
    uint8_t checked_pouch2;
    uint8_t checked_scabbard1;
    uint8_t checked_scabbard_tail;
} DM2_V1_SkprojectPossessionSlotReceipt;

typedef struct {
    int valid;
    int blocked_missing_attribute;
    uint8_t attribute_index;
    uint8_t previous_current;
    uint8_t maximum;
    int16_t delta_input;
    int16_t reduced_delta;
    int16_t source_si;
    uint8_t final_current;
} DM2_V1_SkprojectBoostAttributeReceipt;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} DM2_V1_SkprojectUiRect;

typedef struct {
    uint16_t event;
    int16_t x;
    int16_t y;
    DM2_V1_SkprojectUiRect rect;
} DM2_V1_SkprojectUiEvent;

typedef struct {
    uint8_t present;
    uint8_t hand_cooldown[3];
    uint8_t hand_activable[2];
} DM2_V1_SkprojectUiChampionState;

typedef struct {
    int valid;
    int untouched_non_adjustable_event;
    uint16_t player_dir;
    uint16_t input_event;
    uint16_t output_event;
    int16_t mapped_player;
    uint8_t mapped_hand;
    int16_t diagonal_w2;
    int16_t diagonal_w3;
    int selected_spell_triangle;
    int blocked_missing_event;
    int blocked_missing_party;
    int blocked_no_player;
    int blocked_hand_cooldown;
    int blocked_hand_not_activable;
    int blocked_leader_hand_cooldown;
} DM2_V1_SkprojectAdjustUiEventReceipt;

typedef struct {
    int valid;
    uint8_t cls4_input;
    uint16_t rect_no;
    uint16_t option_mask;
    uint16_t active_mask;
    uint8_t cls4_drawn;
    uint8_t incremented_for_active_option;
    uint8_t gdat_category;
    uint8_t gdat_cls2;
    int16_t alpha;
} DM2_V1_SkprojectCharsheetOptionIconReceipt;

typedef struct {
    uint8_t category;
    uint8_t index;
    uint8_t entry;
} DM2_V1_SkprojectCommandSlotItem;

typedef struct {
    int valid;
    uint16_t slot;
    uint8_t ww;
    uint8_t magical_map_flags;
    uint8_t used_container_icon;
    uint8_t used_interface_icon;
    uint8_t icon_category;
    uint8_t icon_index;
    uint8_t icon_entry;
    uint16_t icon_button_id;
    uint16_t name_button_id;
    uint8_t requested_name_string;
    uint16_t foreground_color;
    uint16_t background_color;
    uint8_t blocked_missing_item;
} DM2_V1_SkprojectDrawCmdSlotReceipt;

typedef struct {
    uint8_t category;
    uint8_t cls2;
    uint8_t entry;
    uint16_t button_id;
} DM2_V1_SkprojectGdatIconPlan;

typedef struct {
    int valid;
    uint16_t moneybox_object_id;
    uint8_t container_cls2;
    DM2_V1_SkprojectGdatIconPlan box_icon;
    uint8_t inspected_slots;
    uint8_t drawn_coin_slots;
    uint16_t first_coin_button_id;
    uint16_t last_coin_button_id;
    uint8_t first_coin_item_db;
    uint8_t first_coin_item_type;
    uint8_t first_coin_stack_count;
    int blocked_missing_coin_tables;
} DM2_V1_SkprojectDrawMoneyboxReceipt;

typedef struct {
    int valid;
    uint16_t rect_no;
    int16_t current_value;
    int16_t max_value;
    uint8_t rune;
    uint16_t color;
    int16_t scaled_value;
    uint8_t drew_power_bar;
    uint8_t drew_rune_label;
    uint8_t drew_low_marker;
    uint8_t drew_high_marker;
    int blocked_missing_rect;
    int blocked_invalid_max;
} DM2_V1_SkprojectDrawItemStatsBarReceipt;

typedef struct {
    int valid;
    uint16_t container_object_id;
    uint8_t container_cls2;
    uint8_t right_panel;
    DM2_V1_SkprojectGdatIconPlan background_icon;
    DM2_V1_SkprojectGdatIconPlan opened_lid_icon;
    uint8_t slot_count;
    uint8_t drawn_slots;
    uint16_t first_slot_button_id;
    uint16_t last_slot_button_id;
    uint8_t uses_inventory_relative_blit;
    int blocked_missing_items;
} DM2_V1_SkprojectDrawContainerPanelReceipt;

typedef struct {
    int valid;
    uint16_t object_id;
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls4;
    uint16_t rect_no;
    uint8_t slot_index;
    uint8_t item_icon_entry;
    uint8_t background_entry;
    uint8_t highlight_entry;
    uint8_t requested_background_dialogue;
    uint8_t requested_highlight_overlay;
    uint8_t requested_icon_entry;
    int blocked_null_object;
} DM2_V1_SkprojectDrawItemIconReceipt;

typedef struct {
    int valid;
    uint8_t traversed_records;
    uint8_t drawn_items;
    uint16_t first_button_id;
    uint16_t last_button_id;
    uint16_t terminal_record_id;
    int stopped_at_limit;
    int blocked_missing_chain;
} DM2_V1_SkprojectDrawContainerSurveyReceipt;

typedef struct {
    int valid;
    uint16_t object_id;
    uint8_t cls1;
    uint8_t cls2;
    uint8_t item_icon_entry;
    uint16_t width;
    uint16_t height;
    uint8_t requested_image_entry;
    uint8_t requested_local_palette;
    uint8_t requested_4bpp_blit;
    int blocked_missing_item_record;
} DM2_V1_SkprojectDrawItemInHandReceipt;

typedef struct {
    int valid;
    uint16_t object_id;
    uint8_t show_details;
    uint8_t used_scroll_text;
    uint8_t used_item_icon;
    uint16_t item_icon_rect;
    int blocked_null_object;
} DM2_V1_SkprojectDrawItemSurveyReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t object_id;
    uint8_t primary_action_icon;
    uint8_t secondary_action_icon;
    uint8_t selected_hand;
    uint16_t action_button_id;
    uint8_t requested_dialogue_pict;
    uint8_t requested_icon_entry;
    int blocked_missing_object;
} DM2_V1_SkprojectDrawHandActionIconsReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t possession_index;
    uint16_t object_id;
    uint16_t temp_cache_index;
    uint16_t picture_width;
    uint16_t picture_height;
    uint8_t bpp;
    uint8_t requested_hand_activable_probe;
    uint8_t requested_alloc_temp_cache_index;
    uint8_t requested_alloc_new_pict;
    int blocked_not_hand_activable;
    int blocked_invalid_dimensions;
} DM2_V1_SkprojectDrawItemOnWoodPanelReceipt;

typedef struct {
    int valid;
    uint16_t rect_no;
    int16_t current_value;
    int16_t max_value;
    char text[10];
    uint16_t foreground_color;
    uint16_t background_color;
    int blocked_invalid_range;
} DM2_V1_SkprojectDrawCurMaxHmsReceipt;

typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    int16_t cur_stamina;
    int16_t max_stamina;
    int16_t cur_mana;
    int16_t max_mana;
} DM2_V1_SkprojectChampion3StatValues;

typedef struct {
    int valid;
    DM2_V1_SkprojectDrawCurMaxHmsReceipt hp;
    DM2_V1_SkprojectDrawCurMaxHmsReceipt stamina;
    DM2_V1_SkprojectDrawCurMaxHmsReceipt mana;
    int blocked_missing_stats;
} DM2_V1_SkprojectDrawPlayer3StatTextReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint8_t panel_variant_cls4;
    uint16_t panel_button_id;
    uint8_t gdat_category;
    uint8_t gdat_cls2;
    uint8_t reset_group_size;
    int blocked_button_group_busy;
} DM2_V1_SkprojectDrawPlayer3StatPaneReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t rect_no;
    uint8_t queried_expanded_rect;
    uint8_t drew_health;
    uint8_t drew_stamina;
    uint8_t drew_mana;
    uint16_t hms_rect_base;
    int blocked_missing_rect;
} DM2_V1_SkprojectDrawPlayer3StatHealthBarReceipt;

typedef struct {
    int valid;
    DM2_V1_SkprojectGdatIconPlan left_name_icon;
    DM2_V1_SkprojectGdatIconPlan right_name_icon;
    uint16_t name_button_id;
    uint16_t foreground_color;
    uint16_t background_color;
    uint8_t used_event_hero_color;
    uint8_t requested_name_string;
} DM2_V1_SkprojectDrawPlayerNameAtCmdSlotReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t damage_value;
    DM2_V1_SkprojectGdatIconPlan damage_icon;
    uint16_t text_button_id;
    uint16_t foreground_color;
    uint16_t background_color;
    char damage_text[4];
} DM2_V1_SkprojectDrawPlayerDamageReceipt;

typedef struct {
    int valid;
    uint8_t draw_frame_icon;
    uint8_t rune_count;
    uint16_t first_rune_button_id;
    uint16_t last_rune_button_id;
    uint8_t requested_clear_spell_area;
    DM2_V1_SkprojectGdatIconPlan frame_icon;
    uint16_t foreground_color;
    uint16_t background_color;
} DM2_V1_SkprojectDrawSpellToBeCastReceipt;

typedef struct {
    int valid;
    uint8_t nrunes;
    DM2_V1_SkprojectGdatIconPlan panel_icon;
    uint8_t drew_rune_choice_buttons;
    uint16_t first_choice_button_id;
    uint16_t last_choice_button_id;
    uint8_t requested_spell_to_be_cast;
    uint8_t requested_player_attack_dir;
} DM2_V1_SkprojectDrawSpellPanelReceipt;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t yy;
    uint8_t relative_pos;
    uint8_t mirror_flip;
    uint8_t leader_icon_entry;
    uint8_t spell_icon_entry;
    uint16_t leader_rect_no;
    uint16_t spell_rect_no;
    uint8_t requested_fill_rect_summary;
    uint8_t requested_leader_summary_image;
    uint8_t requested_spell_summary_image;
    uint8_t requested_gray_overlay;
    int blocked_dead_champion;
} DM2_V1_SkprojectDrawSquadSpellLeaderReceipt;

typedef struct {
    int valid;
    uint8_t squad_gfx_set;
    uint8_t champion_count;
    uint8_t drawn_champions;
    DM2_V1_SkprojectGdatIconPlan base_icon;
    uint8_t requested_alloc_pict_buff;
    uint8_t requested_free_pict_buff;
    uint8_t requested_squad_icon_palette;
    uint8_t requested_aura_summary_image;
} DM2_V1_SkprojectDrawSquadPosInterfaceReceipt;

typedef struct {
    int valid;
    uint16_t champion_index;
    uint8_t aura_of_speed;
    uint8_t aura_rand_y;
    uint8_t aura_rand_x;
    int16_t jitter_x;
    int16_t jitter_y;
    DM2_V1_SkprojectGdatIconPlan base_icon;
    uint16_t squad_icon_rect;
    uint16_t left_arrow_button;
    uint16_t right_arrow_button;
    uint8_t requested_alloc_pict_buff;
    uint8_t requested_squad_palette;
    uint8_t requested_icon_blit;
    uint8_t requested_free_pict_buff;
    uint8_t drew_enchantment_aura;
} DM2_V1_SkprojectDrawPlayerAttackDirReceipt;

typedef struct {
    int valid;
    uint16_t record_id;
    uint8_t container_cls2;
    uint8_t container_mode;
    uint16_t flags_before;
    uint16_t flags_after;
    uint8_t requested_container_panel_init;
    uint8_t requested_command_slots;
    uint8_t requested_map_draw;
    uint8_t requested_gray_overlay;
    uint16_t target_x;
    uint16_t target_y;
    uint16_t target_map;
    int16_t shift_x;
    int16_t shift_y;
} DM2_V1_SkprojectDrawMajicMapReceipt;

typedef struct {
    int valid;
    int16_t food;
    int16_t water;
    int16_t poison;
    DM2_V1_SkprojectGdatIconPlan panel_icon;
    DM2_V1_SkprojectGdatIconPlan food_text_icon;
    DM2_V1_SkprojectGdatIconPlan water_text_icon;
    DM2_V1_SkprojectGdatIconPlan poison_text_icon;
    uint16_t food_bar_rect;
    uint16_t water_bar_rect;
    uint16_t poison_bar_rect;
    uint8_t drew_poison;
    uint8_t inventory_subpanel;
} DM2_V1_SkprojectDrawFoodWaterPoisonPanelReceipt;

typedef struct {
    int valid;
    int16_t current_value;
    int16_t floor_value;
    uint16_t rect_no;
    uint16_t color;
    uint16_t tail_color;
    int16_t scaled_value;
    uint16_t selected_color;
    uint8_t requested_scale_rect;
    uint8_t requested_black_background_fill;
    uint8_t requested_value_fill;
    uint8_t requested_tail_fill;
    int blocked_missing_rect;
    int blocked_invalid_range;
} DM2_V1_SkprojectDrawPowerStatBarReceipt;

typedef struct {
    int valid;
    uint16_t object_id;
    uint16_t line_count;
    uint16_t first_text_rect;
    uint8_t inventory_subpanel;
    uint8_t requested_message_text;
    uint8_t requested_scroll_background;
    uint8_t requested_scroll_overlay;
    uint8_t requested_centered_lines;
    int blocked_not_scroll;
} DM2_V1_SkprojectDrawScrollTextReceipt;

typedef struct {
    int valid;
    uint16_t rect_no;
    uint16_t foreground_color;
    uint16_t background_color;
    int16_t draw_x;
    int16_t draw_y;
    uint16_t text_len;
    uint8_t requested_query_str_metrics;
    uint8_t requested_query_blit_rect;
    uint8_t requested_draw_string;
    uint8_t requested_dirty_rect;
    int blocked_missing_text;
    int blocked_missing_rect;
} DM2_V1_SkprojectDrawSimpleStrReceipt;

typedef struct {
    int valid;
    uint16_t champion_index;
    uint8_t skill_lines;
    uint8_t attribute_lines;
    uint8_t inventory_subpanel;
    uint16_t skill_text_rect;
    uint16_t attribute_text_rect;
    uint8_t requested_blank_panel;
    uint8_t requested_skill_names;
    uint8_t requested_attribute_names;
    uint8_t requested_ability_values;
} DM2_V1_SkprojectDrawSkillPanelReceipt;

typedef struct {
    int valid;
    uint8_t lever_is_on;
    DM2_V1_SkprojectGdatIconPlan lever_icon;
    uint8_t requested_drawings_completed;
    uint8_t requested_open_sound;
    uint8_t inventory_subpanel;
} DM2_V1_SkprojectDrawCryocellLeverReceipt;

typedef struct {
    int valid;
    uint8_t cls4;
    uint16_t rect_no;
    uint8_t gdat_category;
    uint8_t gdat_cls2;
    uint8_t blit_mode;
    uint8_t requested_inflated_rect;
    uint8_t requested_local_palette;
} DM2_V1_SkprojectDrawEyeMouthRectangleReceipt;

#define DM2_V1_SKPROJECT_FONT_PLANE_BYTES (6u * 128u)
#define DM2_V1_SKPROJECT_FONT_PIXELS 24u
#define DM2_V1_SKPROJECT_TEXT_LIMIT 127u

typedef struct {
    uint8_t glyph;
    uint8_t foreground;
    uint8_t background;
    uint8_t pixels[DM2_V1_SKPROJECT_FONT_PIXELS];
    uint8_t written_pixels;
    int valid;
    int blocked_missing_font_plane;
} DM2_V1_SkprojectFontReceipt;

typedef enum {
    DM2_V1_SKPROJECT_TEXT_ROUTE_STRING = 0,
    DM2_V1_SKPROJECT_TEXT_ROUTE_STRONG = 1,
    DM2_V1_SKPROJECT_TEXT_ROUTE_BUTTON = 2,
    DM2_V1_SKPROJECT_TEXT_ROUTE_NAME = 3,
    DM2_V1_SKPROJECT_TEXT_ROUTE_VP = 4,
    DM2_V1_SKPROJECT_TEXT_ROUTE_VP_RC = 5,
    DM2_V1_SKPROJECT_TEXT_ROUTE_LOCAL = 6,
    DM2_V1_SKPROJECT_TEXT_ROUTE_BACKBUFF = 7
} DM2_V1_SkprojectTextRoute;

typedef struct {
    DM2_V1_SkprojectTextRoute route;
    int valid;
    int blocked_missing_text;
    int blocked_empty_text;
    int strong_shadow_passes;
    int fill_background;
    int uses_alpha_mask;
    int adjusts_button_rect;
    int centered_by_metrics;
    int dest_is_screen;
    int16_t dest_width;
    int16_t input_x;
    int16_t input_baseline_y;
    int16_t draw_x;
    int16_t draw_y;
    int16_t fill_x;
    int16_t fill_y;
    int16_t fill_w;
    int16_t fill_h;
    int16_t char_w;
    int16_t char_h;
    int16_t text_w;
    int16_t text_h;
    int16_t first_char_x;
    int16_t last_char_x;
    uint16_t char_count;
    uint16_t foreground;
    uint16_t background;
} DM2_V1_SkprojectTextDrawReceipt;

typedef struct {
    int valid;
    int blocked_missing_text;
    int blocked_missing_output;
    int blocked_unimplemented_substitution;
    uint16_t consumed_bytes;
    uint16_t written_bytes;
} DM2_V1_SkprojectFormatTextReceipt;

typedef struct {
    int valid;
    int blocked_missing_text;
    int blocked_missing_output;
    uint16_t start_offset;
    uint16_t next_offset;
    int16_t consumed_width;
    uint16_t copied_bytes;
    int split_at_space;
    int stopped_at_newline;
    int stopped_at_nul;
} DM2_V1_SkprojectHintLineReceipt;

/* skproject SKWINSPX v4 SkWinCore::BETWEEN_VALUE clamps newv to
 * [minv,maxv]. SKWINSPX v5 exposes the same behavior as DM2_BETWEEN_VALUE. */
int16_t dm2_v1_skproject_between_value(int16_t minv,
                                       int16_t newv,
                                       int16_t maxv);
int16_t dm2_v1_skproject_dm2_between_value(int16_t minv,
                                           int16_t maxv,
                                           int16_t value);
int16_t dm2_v1_skproject_abs(int16_t value);
int16_t dm2_v1_skproject_calc_square_distance(int16_t from_x,
                                               int16_t from_y,
                                               int16_t to_x,
                                               int16_t to_y);
int dm2_v1_skproject_calc_vector_dir(
    DM2_V1_SkprojectRandomData *randdat,
    int16_t from_x,
    int16_t from_y,
    int16_t to_x,
    int16_t to_y,
    DM2_V1_SkprojectVectorDirReceipt *out_receipt);

void dm2_v1_skproject_temp_rect_ring_init(
    DM2_V1_SkprojectTempRectRing *ring);
int dm2_v1_skproject_alloc_temp_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt);
int dm2_v1_skproject_alloc_temp_origin_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt);

void dm2_v1_skproject_random_init(DM2_V1_SkprojectRandomData *randdat);
uint32_t dm2_v1_skproject_rand(DM2_V1_SkprojectRandomData *randdat);
uint16_t dm2_v1_skproject_rand16(DM2_V1_SkprojectRandomData *randdat,
                                 uint16_t max_value);
int dm2_v1_skproject_randbit(DM2_V1_SkprojectRandomData *randdat);
uint8_t dm2_v1_skproject_randdir(DM2_V1_SkprojectRandomData *randdat);
int dm2_v1_skproject_calc_vector_w_dir(
    int16_t dir,
    int16_t xx,
    int16_t yy,
    int16_t *x,
    int16_t *y,
    DM2_V1_SkprojectVectorWDirReceipt *out_receipt);
int32_t dm2_v1_skproject_compute_power_4_within(int16_t mask,
                                                int16_t ordinal);
int dm2_v1_skproject_fill_i16table(
    int16_t *table,
    int16_t value,
    uint16_t entries,
    DM2_V1_SkprojectFillI16TableReceipt *out_receipt);
int dm2_v1_skproject_pt_in_rect(
    const DM2_V1_SkprojectRect *rect,
    int16_t point_x,
    int16_t point_y,
    DM2_V1_SkprojectPtInRectReceipt *out_receipt);
int dm2_v1_skproject_offset_rect(
    const DM2_V1_SkprojectRect *origin,
    const DM2_V1_SkprojectRect *source,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_SkprojectOffsetRectReceipt *out_receipt);
int dm2_v1_skproject_ptr_advance(
    uint32_t initial_offset,
    int32_t delta,
    uint32_t capacity,
    uint32_t *out_offset,
    DM2_V1_SkprojectPtrAdvanceReceipt *out_receipt);
int dm2_v1_skproject_write_byte(
    uint8_t *buffer,
    uint32_t capacity,
    uint32_t offset,
    uint8_t value,
    DM2_V1_SkprojectCursorAccessReceipt *out_receipt);
int dm2_v1_skproject_write_word(
    uint8_t *buffer,
    uint32_t capacity,
    uint32_t offset,
    uint16_t value,
    DM2_V1_SkprojectCursorAccessReceipt *out_receipt);
int dm2_v1_skproject_read_byte(
    const uint8_t *buffer,
    uint32_t capacity,
    uint32_t offset,
    uint8_t *out_value,
    DM2_V1_SkprojectCursorAccessReceipt *out_receipt);
int dm2_v1_skproject_read_sbyte(
    const uint8_t *buffer,
    uint32_t capacity,
    uint32_t offset,
    int8_t *out_value,
    DM2_V1_SkprojectCursorAccessReceipt *out_receipt);
int dm2_v1_skproject_read_word(
    const uint8_t *buffer,
    uint32_t capacity,
    uint32_t offset,
    uint16_t *out_value,
    DM2_V1_SkprojectCursorAccessReceipt *out_receipt);
uint8_t dm2_v1_skproject_compressed_rect_row_size(uint8_t mask);
int dm2_v1_skproject_compress_rects(
    const int16_t *data_words,
    uint32_t word_count,
    DM2_V1_SkprojectRectTable *out_table,
    DM2_V1_SkprojectCompressRectsReceipt *out_receipt);
int dm2_v1_skproject_query_rect(
    const DM2_V1_SkprojectRectTable *table,
    uint16_t rectno,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_SkprojectQueryRectReceipt *out_receipt);
int dm2_v1_skproject_palettecolor_from_color(uint8_t color,
                                             uint8_t *out_palette);
int dm2_v1_skproject_palettecolor_from_ui8(uint8_t color,
                                           uint8_t *out_palette);
int dm2_v1_skproject_palettecolor_to_ui8(uint8_t palette,
                                         uint8_t *out_color);
int dm2_v1_skproject_palettecolor_to_pixel(uint8_t palette,
                                           uint8_t *out_pixel);
int dm2_v1_skproject_convert_driverpalette(
    const uint8_t *alpha_rgb8_palette,
    int immediate_colors_before,
    DM2_V1_SkprojectDriverPaletteReceipt *out_receipt);
int dm2_v1_skproject_select_palette_set(
    int16_t set,
    DM2_V1_SkprojectPaletteSetReceipt *out_receipt);
int dm2_v1_skproject_update_blit_palette(
    const uint8_t *palette,
    uint16_t colors,
    const uint8_t **out_palette_ptr);
int dm2_v1_skproject_xlat_palette(
    const uint8_t *palette,
    uint16_t colors,
    const uint8_t *conversion_table,
    DM2_V1_SkprojectXlatPaletteReceipt *out_receipt);
void dm2_v1_skproject_ibmio_palette_state_init(
    DM2_V1_SkprojectIbmioPaletteState *state);
int dm2_v1_skproject_00eb_04bc_palette_set(
    DM2_V1_SkprojectIbmioPaletteState *state,
    const uint8_t rgb888[16][3],
    uint16_t set,
    DM2_V1_SkprojectIbmioPaletteReceipt *out_receipt);
int dm2_v1_skproject_0759_0688_palette_set(
    DM2_V1_SkprojectIbmioPaletteState *state,
    const uint8_t rgb888[16][3],
    uint16_t set,
    DM2_V1_SkprojectIbmioPaletteReceipt *out_receipt);
int dm2_v1_skproject_0759_06a1_select_palette_set(
    DM2_V1_SkprojectIbmioPaletteState *state,
    uint8_t set,
    DM2_V1_SkprojectPaletteSetReceipt *out_receipt);
int dm2_v1_skproject_00eb_070c_blit_4to8(
    const uint8_t *src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t *dst8,
    size_t dst8_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    DM2_V1_SkprojectIbmioBlit4To8Receipt *out_receipt);
int dm2_v1_skproject_0759_0310_blit_4to8_self(
    const uint8_t *src4,
    size_t src4_size,
    uint16_t off_pixels,
    uint8_t *dst8,
    size_t dst8_size,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    DM2_V1_SkprojectIbmioBlit4To8Receipt *out_receipt);
int dm2_v1_skproject_0759_02c6_copy_4bpp_sequence(
    uint8_t *buffer4,
    size_t buffer4_size,
    uint16_t off_dst_pixels,
    uint16_t off_src_pixels,
    uint16_t width_pixels,
    DM2_V1_SkprojectAnimCopy4BppReceipt *out_receipt);
void dm2_v1_skproject_mouse_state_init(
    DM2_V1_SkprojectMouseState *state);
int dm2_v1_skproject_01b0_0adb_hide_mouse(
    DM2_V1_SkprojectMouseState *state,
    DM2_V1_SkprojectMouseHideReceipt *out_receipt);
int dm2_v1_skproject_01b0_0c70_set_cursor_shape(
    DM2_V1_SkprojectMouseState *state,
    uint16_t shape,
    DM2_V1_SkprojectMouseShapeReceipt *out_receipt);
int dm2_v1_skproject_01b0_0ca4_set_cursor_bounds(
    DM2_V1_SkprojectMouseState *state,
    const uint16_t bounds[4],
    uint16_t mode,
    DM2_V1_SkprojectMouseBoundsReceipt *out_receipt);
void dm2_v1_skproject_anim_runtime_state_init(
    DM2_V1_SkprojectAnimRuntimeState *state);
int dm2_v1_skproject_anim_runtime_push_event(
    DM2_V1_SkprojectAnimRuntimeState *state,
    uint16_t event_word);
int dm2_v1_skproject_0759_0126_capture_int_ff(
    DM2_V1_SkprojectAnimRuntimeState *state,
    uint32_t host_vector,
    DM2_V1_SkprojectAnimVectorReceipt *out_receipt);
int dm2_v1_skproject_0759_06db_install_timer(
    DM2_V1_SkprojectAnimRuntimeState *state,
    uint32_t host_vector,
    uint16_t timer_reload_ticks,
    DM2_V1_SkprojectAnimTimerInstallReceipt *out_receipt);
int dm2_v1_skproject_0759_06c2_timer_tick(
    DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectAnimTimerTickReceipt *out_receipt);
int dm2_v1_skproject_0759_072c_poll_ibmio(
    DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectIbmioPollReceipt *out_receipt);
int dm2_v1_skproject_0759_071b_wait_ibmio_event(
    DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectIbmioWaitEventReceipt *out_receipt);
int dm2_v1_skproject_0759_065f_fill_screen_rect(
    DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectScreenRectFillReceipt *out_receipt);
int dm2_v1_skproject_0759_06b5_clear_screen(
    DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectScreenClearReceipt *out_receipt);
int dm2_v1_skproject_01b0_1ed2_sound_available(
    const DM2_V1_SkprojectAnimRuntimeState *state,
    DM2_V1_SkprojectSoundAvailableReceipt *out_receipt);
void dm2_v1_skproject_ui_predicate_state_init(
    DM2_V1_SkprojectUiPredicateState *state);
int dm2_v1_skproject_return_1(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_is_game_ended(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_0023(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_003e(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_007b(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_009e(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_00c5(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_00f3(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_012d(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_014f(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_0184(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_01ba(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_dispatch_predicate(
    uint8_t predicate_index,
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_1031_023b_child_list(
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiChildListReceipt *out_receipt);
int dm2_v1_skproject_1031_01d5_resolve_rect(
    uint16_t rectno,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    const DM2_V1_SkprojectRect *topleft_rects,
    uint16_t topleft_rect_count,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_SkprojectUiResolveRectReceipt *out_receipt);
int dm2_v1_skproject_1031_027e_traverse(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *root,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiTraverseReceipt *out_receipt);
int dm2_v1_skproject_1031_024c_action_list(
    const DM2_V1_SkprojectUiNodeRef *ref,
    const DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiActionListReceipt *out_receipt);
void dm2_v1_skproject_ui_runtime_state_init(
    DM2_V1_SkprojectUiRuntimeState *state);
int dm2_v1_skproject_1031_0a88_action_hit(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    uint16_t action_index,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    const DM2_V1_SkprojectRect *topleft_rects,
    uint16_t topleft_rect_count,
    int16_t point_x,
    int16_t point_y,
    uint16_t action_mask,
    DM2_V1_SkprojectUiActionResolveReceipt *out_receipt);
int dm2_v1_skproject_1031_0c58_select_event(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    uint16_t event_code,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    uint16_t action_index,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    const DM2_V1_SkprojectRect *topleft_rects,
    uint16_t topleft_rect_count,
    DM2_V1_SkprojectUiActionResolveReceipt *out_receipt);
int dm2_v1_skproject_1031_0b7e_flush_pending_mouse(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    DM2_V1_SkprojectUiEventResetReceipt *out_receipt);
int dm2_v1_skproject_1031_10c8_center_button(
    DM2_V1_SkprojectUiButtonGroup *group,
    const DM2_V1_SkprojectRect *container_rect,
    const DM2_V1_SkprojectRect *mouse_rect,
    uint16_t width,
    uint16_t height,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_SkprojectUiCenteredButtonReceipt *out_receipt);
int dm2_v1_skproject_1031_030a_hit_test(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *root,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    const DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    const DM2_V1_SkprojectRect *topleft_rects,
    uint16_t topleft_rect_count,
    int16_t point_x,
    int16_t point_y,
    uint16_t action_mask,
    DM2_V1_SkprojectUiHitTestReceipt *out_receipt);
int dm2_v1_skproject_1031_03f2_find_action(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *root,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    const DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    uint16_t action_code,
    DM2_V1_SkprojectUiActionSearchReceipt *out_receipt);
int dm2_v1_skproject_1031_04f5_clear_pending_redraw(
    DM2_V1_SkprojectUiRuntimeState *state,
    DM2_V1_SkprojectUiPendingRedrawReceipt *out_receipt);
int dm2_v1_skproject_1031_050c_release_item_capture(
    DM2_V1_SkprojectUiRuntimeState *state,
    DM2_V1_SkprojectUiMouseCaptureReceipt *out_receipt);
int dm2_v1_skproject_1031_098e_reset_events(
    DM2_V1_SkprojectUiRuntimeState *state,
    DM2_V1_SkprojectUiEventResetReceipt *out_receipt);
int dm2_v1_skproject_1031_0541_select_tree(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    uint16_t tree_index,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiSelectTreeReceipt *out_receipt);
int dm2_v1_skproject_1031_0667_restore_active_tree(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiSelectTreeReceipt *out_receipt);
int dm2_v1_skproject_1031_0675_reset_and_select_tree(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    uint16_t tree_index,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiSelectTreeReceipt *out_receipt);
int dm2_v1_skproject_gate_1031(
    uint8_t predicate_index,
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *ref,
    DM2_V1_SkprojectUiPredicateReceipt *out_receipt);
int dm2_v1_skproject_10777_reset_capture(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    int16_t *capture_count,
    DM2_V1_SkprojectUiResetCaptureReceipt *out_receipt);
int dm2_v1_skproject_107b0_select_active_tree(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiSelectTreeReceipt *out_receipt);
int dm2_v1_skproject_1031_06a5_select_saved_tree(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiSelectTreeReceipt *out_receipt);
int dm2_v1_skproject_1031_06b3_search_action(
    const DM2_V1_SkprojectUiPredicateState *state,
    const DM2_V1_SkprojectUiNodeRef *root,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    const DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    uint16_t action_code,
    DM2_V1_SkprojectUiSearchActionReceipt *out_receipt);
int dm2_v1_skproject_1031_0781_queue_event_by_code(
    DM2_V1_SkprojectUiRuntimeState *runtime,
    const DM2_V1_SkprojectUiPredicateState *predicate_state,
    uint16_t event_code,
    const DM2_V1_SkprojectUiNodeRef *roots,
    uint16_t root_count,
    const DM2_V1_SkprojectUiNodeRef *nodes,
    uint16_t node_count,
    const uint8_t *child_bytes,
    size_t child_bytes_size,
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    const DM2_V1_SkprojectUiAction *actions,
    uint16_t action_count,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    const DM2_V1_SkprojectRect *topleft_rects,
    uint16_t topleft_rect_count,
    DM2_V1_SkprojectUiQueueEventReceipt *out_receipt);
int dm2_v1_skproject_1031_07d6_remap_ui_tables(
    DM2_V1_SkprojectUiLeafMeta *leaf_meta,
    uint16_t leaf_meta_count,
    DM2_V1_SkprojectUiClickRectNode *clickrects,
    uint16_t clickrect_count,
    DM2_V1_SkprojectUiAction *v1d338c,
    uint16_t v1d338c_count,
    DM2_V1_SkprojectUiAction *v1d39bc,
    uint16_t v1d39bc_count,
    uint8_t *table1d3cd0,
    uint16_t table1d3cd0_count,
    DM2_V1_SkprojectUiNodeRef *table1d3ba0,
    uint16_t table1d3ba0_count,
    DM2_V1_SkprojectUiNodeRef *table1d3ed5,
    uint16_t table1d3ed5_count,
    DM2_V1_SkprojectUiTableRemapReceipt *out_receipt);
int dm2_v1_skproject_click_magical_map_at(
    int16_t click_x,
    int16_t click_y,
    uint16_t ui_code,
    uint8_t current_hero,
    uint8_t current_actmode,
    uint16_t item_in_hand,
    const uint8_t *item_record,
    size_t item_record_size,
    const uint8_t *minion_record,
    size_t minion_record_size,
    int16_t map_origin_x,
    int16_t map_origin_y,
    int16_t cell_stride_x,
    int16_t cell_stride_y,
    int16_t map_offset_x,
    int16_t map_offset_y,
    int16_t current_map,
    int16_t party_x,
    int16_t party_y,
    int16_t party_map,
    int16_t teleport_map,
    int16_t teleport_x,
    int16_t teleport_y,
    const uint8_t *tiles,
    int16_t map_width,
    int16_t map_height,
    const uint8_t *passage,
    DM2_V1_SkprojectUiMagicalMapClickReceipt *out_receipt);
int dm2_v1_skproject_sub_blit_specialeffects_receipt(
    const DM2_V1_SkprojectRect *rect,
    uint16_t xend,
    uint16_t srcofs,
    uint16_t overlay_origin,
    uint16_t overlay_stride,
    uint16_t pixperline,
    int16_t alpha_mask,
    const uint8_t *overlay_mask,
    DM2_V1_SkprojectBlitSpecialEffectsReceipt *out_receipt);
int dm2_v1_skproject_draw_icon_pict_buff(
    int has_rect,
    uint16_t rect_x,
    uint16_t rect_y,
    uint16_t rect_w,
    uint16_t rect_h,
    int16_t src_x,
    int16_t src_y,
    int16_t color_key,
    int16_t flip_mirror,
    uint16_t source_width,
    uint16_t source_height,
    uint16_t dest_stride,
    const uint8_t *local_palette,
    DM2_V1_SkprojectDrawIconPictBuffReceipt *out_receipt);
int dm2_v1_skproject_draw_icon_pict_entry(
    uint8_t category,
    uint8_t cls2,
    uint8_t entry,
    int has_button_group,
    uint16_t button_id,
    int16_t alpha_mask,
    DM2_V1_SkprojectDrawIconPictEntryReceipt *out_receipt);
int dm2_v1_skproject_2405_00ec_query_blit_rect(
    const DM2_V1_Skproject2405RectState *state,
    uint16_t rectno,
    const DM2_V1_SkprojectRect *blit_rects,
    uint16_t blit_rect_count,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_Skproject2405RectReceipt *out_receipt);
int dm2_v1_skproject_2405_011f_query_inflated_rect(
    const DM2_V1_Skproject2405RectState *state,
    uint16_t rectno,
    const DM2_V1_SkprojectRect *blit_rects,
    uint16_t blit_rect_count,
    DM2_V1_SkprojectRect *out_rect,
    DM2_V1_Skproject2405RectReceipt *out_receipt);
uint8_t dm2_v1_skproject_2405_014a_item_entry(
    const DM2_V1_Skproject2405ItemState *state,
    uint16_t equip_slot,
    uint16_t tick_modulus,
    DM2_V1_Skproject2405ItemEntryReceipt *out_receipt);
int dm2_v1_skproject_draw_def_pict(
    const DM2_V1_SkprojectExtendedPictureRef *picture,
    uint16_t rect_no,
    uint16_t width,
    uint16_t height,
    int16_t src_x,
    int16_t src_y,
    int16_t dst_x,
    int16_t dst_y,
    int16_t color_key,
    int blit_rect_exists,
    DM2_V1_SkprojectDrawDefPictReceipt *out_receipt);
int dm2_v1_skproject_draw_gray_overlay(
    int has_rect,
    uint16_t cache_index,
    uint16_t dest_stride,
    uint16_t overlay_pattern,
    DM2_V1_SkprojectDrawGrayOverlayReceipt *out_receipt);
int dm2_v1_skproject_draw_dialogue_progress(
    int dballoc_active,
    uint16_t progress_per_mille,
    uint16_t expanded_rect_width,
    uint16_t previous_width,
    DM2_V1_SkprojectDialogueProgressReceipt *out_receipt);
int dm2_v1_skproject_draw_dialogue_pict(
    int has_src_bitmap,
    int has_dest_bitmap,
    int has_rect,
    uint16_t src_width,
    uint16_t dest_bitmap_width,
    int dest_is_screen,
    int16_t src_x,
    int16_t src_y,
    int16_t alpha_mask,
    uint8_t source_bpp,
    uint8_t dest_bpp,
    const uint8_t *palette,
    DM2_V1_SkprojectDialoguePictReceipt *out_receipt);
int dm2_v1_skproject_draw_wake_up_text(
    DM2_V1_SkprojectWakeUpTextReceipt *out_receipt);
int dm2_v1_skproject_move_side_offset(
    uint16_t record_word_e,
    int16_t direction_delta,
    uint8_t creature_5x5_pos,
    DM2_V1_SkprojectMoveSideOffsetReceipt *out_receipt);
int dm2_v1_skproject_move_admission(
    const DM2_V1_SkprojectMoveAdmissionRequest *request,
    DM2_V1_SkprojectMoveAdmissionReceipt *out_receipt);
int dm2_v1_skproject_set_destination_of_minion_map(
    uint16_t previous_record_w6,
    int16_t current_map,
    int16_t destination_x,
    int16_t destination_y,
    int16_t selected_map,
    int16_t map_width,
    int16_t map_height,
    DM2_V1_SkprojectMinionDestinationReceipt *out_receipt);
int dm2_v1_skproject_map_0cee_17e7(
    uint16_t random_input,
    uint16_t divisor,
    uint16_t range_input,
    uint16_t savegame_seed,
    DM2_V1_SkprojectMapRandomReceipt *out_receipt);
int dm2_v1_skproject_map_0cee_04e5(
    const uint8_t *tiles,
    int16_t width,
    int16_t height,
    int16_t dir,
    int16_t forward,
    int16_t side,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectMapTileVectorReceipt *out_receipt);
int dm2_v1_skproject_get_tile_value(
    const uint8_t *tiles,
    const uint8_t *passage,
    int16_t width,
    int16_t height,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectGetTileValueReceipt *out_receipt);
int dm2_v1_skproject_get_address_of_tile_record(
    int16_t x,
    int16_t y,
    uint16_t tile_record_link,
    const uint16_t record_counts[16],
    const uint16_t record_sizes[16],
    DM2_V1_SkprojectRecordAddressReceipt *out_receipt);
int dm2_v1_skproject_fill_entire_pict(
    uint16_t width,
    uint16_t height,
    uint8_t bpp,
    uint8_t fill,
    DM2_V1_SkprojectFillReceipt *out_receipt);
int dm2_v1_skproject_fill_rect_summary(
    uint16_t rect_width,
    uint16_t rect_height,
    uint8_t fill,
    int has_buffer,
    int has_rect,
    DM2_V1_SkprojectFillReceipt *out_receipt);
int dm2_v1_skproject_fill_str(
    uint8_t *buffer,
    uint16_t buffer_capacity,
    uint16_t count,
    uint8_t value,
    uint16_t delta,
    DM2_V1_SkprojectFillStrReceipt *out_receipt);
int dm2_v1_skproject_fill_halftone_rectv(
    uint8_t *pixels,
    uint16_t pixel_capacity,
    uint16_t stride,
    const DM2_V1_SkprojectRect *rect,
    DM2_V1_SkprojectHalftoneRectReceipt *out_receipt);
int dm2_v1_skproject_fill_halftone_recti(
    uint8_t *pixels,
    uint16_t pixel_capacity,
    uint16_t stride,
    uint16_t rectno,
    const DM2_V1_SkprojectRect *expanded_rect,
    DM2_V1_SkprojectHalftoneRectReceipt *out_receipt);
int dm2_v1_skproject_mouse_release_capture(
    int16_t *capture_count,
    DM2_V1_SkprojectMouseReleaseCaptureReceipt *out_receipt);
int dm2_v1_skproject_highlight_arrow_panel(
    uint8_t cls4,
    uint16_t rectno,
    uint8_t bright,
    DM2_V1_SkprojectHighlightArrowPanelReceipt *out_receipt);
int dm2_v1_skproject_map_3b001(
    int16_t current_map,
    int16_t value_0270,
    int16_t value_0272,
    DM2_V1_SkprojectMap3B001Receipt *out_receipt);
int dm2_v1_skproject_map_0cee_1815(
    int gate,
    int16_t map_width,
    int16_t map_height,
    int16_t v1d3248,
    int16_t x,
    int16_t y,
    int16_t selector,
    uint16_t savegame_seed,
    const uint8_t *candidate_table,
    uint16_t candidate_count,
    DM2_V1_SkprojectMap1815Receipt *out_receipt);
int dm2_v1_skproject_map_0cee_185a(
    uint16_t *out_words4,
    int16_t map_width,
    int16_t map_height,
    int16_t v1d3248,
    int16_t gate0,
    int16_t gate1,
    int16_t gate2,
    int16_t gate3,
    int16_t x,
    int16_t y,
    int16_t rotation,
    int16_t step,
    uint16_t savegame_seed,
    const uint8_t *candidate_table,
    uint16_t candidate_count,
    const uint8_t *ornate_alcove_flags,
    DM2_V1_SkprojectMap185AReceipt *out_receipt);
int dm2_v1_skproject_map_0cee_wall_decoration_chain(
    const DM2_V1_SkprojectMap0CEEWallDecorationState *state,
    uint16_t out_words4[4],
    DM2_V1_SkprojectMap0CEEWallDecorationReceipt *out_receipt);
int dm2_v1_skproject_map_2066_1f37(
    DM2_V1_SkprojectMapRecord *records,
    uint16_t record_count,
    uint16_t head,
    uint16_t value,
    int16_t *counter,
    DM2_V1_SkprojectMap20661F37Receipt *out_receipt);
uint16_t dm2_v1_skproject_map_2066_1ec9(
    DM2_V1_SkprojectMapRecord *records,
    uint16_t record_count,
    uint16_t head,
    uint16_t append,
    DM2_V1_SkprojectMap20661EC9Receipt *out_receipt);
int dm2_v1_skproject_tmpmap_or_flag(
    uint8_t *tmpmap,
    uint16_t tmpmap_size,
    int16_t y,
    int16_t x,
    int16_t offset,
    DM2_V1_SkprojectTmpmapFlagReceipt *out_receipt);
int dm2_v1_skproject_get_address_of_record(
    uint16_t record_link,
    const uint16_t record_counts[16],
    const uint16_t record_sizes[16],
    DM2_V1_SkprojectRecordAddressReceipt *out_receipt);
int dm2_v1_skproject_get_typed_address_of_record(
    uint16_t record_link,
    uint8_t requested_type,
    const uint16_t record_counts[16],
    const uint16_t record_sizes[16],
    int detached_route,
    DM2_V1_SkprojectRecordAddressReceipt *out_receipt);
int dm2_v1_skproject_locate_other_level(
    const DM2_V1_SkprojectMapDescriptor *maps,
    uint16_t map_count,
    int16_t source_map,
    int16_t locate_delta,
    int16_t *x,
    int16_t *y,
    const uint8_t *candidate_cursor,
    uint16_t candidate_count,
    uint16_t resume_offset,
    uint16_t *out_resume_offset,
    DM2_V1_SkprojectLocateOtherLevelReceipt *out_receipt);
int dm2_v1_skproject_map_3bf83(
    int16_t x,
    int16_t y,
    int16_t target_map,
    int16_t rotation,
    int16_t current_map,
    int16_t current_x,
    int16_t current_y,
    int16_t target_width,
    int16_t target_height,
    DM2_V1_SkprojectMap3BF83Receipt *out_receipt);
int dm2_v1_skproject_move_12b4_0092(
    uint16_t active_v1e0534,
    uint16_t arrow_panel,
    int16_t highlight_param,
    DM2_V1_SkprojectArrowHighlightReceipt *out_receipt);
int dm2_v1_skproject_move_12b4_00af(
    int enter_forward,
    int16_t source_map,
    int16_t source_x,
    int16_t source_y,
    int16_t located_map,
    int16_t located_x,
    int16_t located_y,
    int16_t query_rotation,
    DM2_V1_SkprojectOtherLevelReceipt *out_receipt);
int dm2_v1_skproject_move_12b4_023f(
    int16_t x,
    int16_t y,
    int16_t arg0,
    int16_t arg1,
    const int16_t direction_champions[4],
    const uint8_t champion_hero_types[4],
    const uint8_t wound_results[4],
    DM2_V1_SkprojectMove12B4023FReceipt *out_receipt);
int dm2_v1_skproject_attack_door(
    uint8_t tile_type,
    uint8_t record_byte2,
    uint8_t record_byte3,
    uint16_t attack_power,
    uint16_t required_power,
    int use_byte2_gate,
    int rebirth_altar,
    uint16_t timer_delay,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectAttackDoorReceipt *out_receipt);
int dm2_v1_skproject_attack_wall(
    const DM2_V1_SkprojectWallAttackRecord *records,
    uint16_t record_count,
    uint16_t projectile_record_word,
    uint16_t projectile_item_type,
    int16_t attack_dir,
    uint8_t randdir,
    int16_t source_x,
    int16_t source_y,
    DM2_V1_SkprojectAttackWallReceipt *out_receipt);
int dm2_v1_skproject_move_12b4_099e(
    const DM2_V1_SkprojectLiftRequest *request,
    DM2_V1_SkprojectLiftReceipt *out_receipt);
int dm2_v1_skproject_wall_ornate_alcove_data_index(
    int ornate_alcove_from_record,
    int16_t cls2,
    uint16_t gdat_data_index,
    DM2_V1_SkprojectWallAlcoveReceipt *out_receipt);
int dm2_v1_skproject_move_2fcf_0b8b(
    int16_t x,
    int16_t y,
    const DM2_V1_SkprojectTeleporterProbe *adjacent_probes,
    DM2_V1_SkprojectTeleporterSearchReceipt *out_receipt);
int dm2_v1_skproject_move_075f_0af9(
    uint16_t object_record,
    uint8_t base_direction,
    DM2_V1_SkprojectThrownObjectTerminalReceipt *out_receipt);
int dm2_v1_skproject_move_12b4_0d75(
    int16_t x,
    int16_t y,
    uint8_t direction,
    int creature_movable,
    uint16_t creature_weight,
    uint16_t force_threshold,
    uint16_t random_value,
    DM2_V1_SkprojectCreaturePushReceipt *out_receipt);
int32_t dm2_v1_skproject_atimesb_rshiftc(int16_t a,
                                         int8_t c,
                                         int16_t b);
int dm2_v1_skproject_is_negative(
    int16_t value,
    DM2_V1_SkprojectIsNegativeReceipt *out_receipt);
int dm2_v1_skproject_is_container_map(
    uint16_t record_link,
    uint8_t container_type,
    DM2_V1_SkprojectContainerMapReceipt *out_receipt);
int16_t dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
    const uint16_t inventory[30],
    uint8_t requested_kind,
    DM2_V1_SkprojectPossessionSlotReceipt *out_receipt);

void dm2_v1_skproject_cache_state_init(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_capacity,
    uint16_t raw_count,
    uint16_t mement_count);
int dm2_v1_skproject_find_ici_from_cache_hash(
    const DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_ici);
uint16_t dm2_v1_skproject_insert_cache_hash_at(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t ici);
uint16_t dm2_v1_skproject_query_mementi_from(
    const DM2_V1_SkprojectCacheState *state,
    uint16_t index);
int dm2_v1_skproject_add_cache_hash(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_cache_index);
uint8_t *dm2_v1_skproject_query_mement_buff_from_cache_index(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_index);
uint32_t dm2_v1_skproject_get_temp_cache_hash(
    const DM2_V1_SkprojectCacheState *state);
uint16_t dm2_v1_skproject_alloc_temp_cache_index(
    DM2_V1_SkprojectCacheState *state);
int dm2_v1_skproject_test_mement(int32_t dw0, int32_t stored_len);
int dm2_v1_skproject_recycle_mementi(
    DM2_V1_SkprojectCacheState *state,
    uint16_t mementi,
    uint16_t previous_w4,
    uint16_t yy,
    DM2_V1_SkprojectRecycleMementReceipt *out_receipt);
int dm2_v1_skproject_free_cache_index(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_index,
    DM2_V1_SkprojectDeallocFreeCacheIndexReceipt *out_receipt);
int dm2_v1_skproject_free_indexed_mement(
    DM2_V1_SkprojectCacheState *state,
    uint16_t index,
    int free_cache_immediately,
    uint16_t *current_mementi,
    DM2_V1_SkprojectFreeIndexedMementReceipt *out_receipt);
int dm2_v1_skproject_free_temp_cache_index(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_index,
    uint16_t *current_mementi,
    DM2_V1_SkprojectFreeTempCacheIndexReceipt *out_receipt);
uint16_t dm2_v1_skproject_find_free_mementi(
    DM2_V1_SkprojectCacheState *state,
    uint16_t fallback_mementi,
    DM2_V1_SkprojectFindFreeMementiReceipt *out_receipt);
int dm2_v1_skproject_alloc_new_pict(
    uint16_t index,
    uint16_t width,
    uint16_t height,
    uint16_t bpp,
    DM2_V1_SkprojectNewPictReceipt *out_receipt);
uint32_t dm2_v1_skproject_calc_pict_ent_hash(
    const DM2_V1_SkprojectExtendedPictureRef *ref);
int dm2_v1_skproject_0b36_00c3_cache_picture(
    const DM2_V1_Skproject0B36CachePicture *cache_picture,
    DM2_V1_Skproject0B36Picture *picture,
    DM2_V1_Skproject0B36CachePictureReceipt *out_receipt);
int dm2_v1_skproject_0b36_0d67_adjust_dirty_rects(
    DM2_V1_Skproject0B36ButtonGroup *group,
    const DM2_V1_SkprojectRect *rect,
    DM2_V1_Skproject0B36DirtyRectReceipt *out_receipt);
int dm2_v1_skproject_0b36_0c52_init_button_group(
    DM2_V1_Skproject0B36ButtonGroup *group,
    uint16_t rectno,
    uint16_t add_initial_dirty_rect,
    uint16_t allocated_cache_index,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    DM2_V1_Skproject0B36ButtonGroupInitReceipt *out_receipt);
int dm2_v1_skproject_0b36_11c0_draw_cached_picture(
    DM2_V1_Skproject0B36Picture *picture,
    DM2_V1_Skproject0B36ButtonGroup *group,
    uint16_t rectno,
    int16_t color_key,
    const DM2_V1_SkprojectRect *blit_rects,
    uint16_t blit_rect_count,
    DM2_V1_Skproject0B36DrawCachedPictureReceipt *out_receipt);
int dm2_v1_skproject_alloc_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectImageMementReceipt *out_receipt);
int dm2_v1_skproject_alloc_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectPictMementReceipt *out_receipt);
int dm2_v1_skproject_free_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt);
int dm2_v1_skproject_free_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt);
int dm2_v1_skproject_free_pict6(
    uint8_t global_free_gate,
    uint8_t allocation_flag,
    uint32_t allocation_handle,
    DM2_V1_SkprojectFreePict6Receipt *out_receipt);
uint16_t dm2_v1_skproject_add_item_charge(
    uint16_t object_id,
    uint16_t *record_w2,
    int16_t delta,
    DM2_V1_SkprojectItemChargeReceipt *out_receipt);
uint16_t dm2_v1_skproject_get_max_charge(uint16_t object_id);
int dm2_v1_skproject_is_container_moneybox(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    int has_moneybox_item_list,
    DM2_V1_SkprojectItemClassifyReceipt *out_receipt);
int dm2_v1_skproject_is_container_chest(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    int has_moneybox_item_list,
    DM2_V1_SkprojectItemClassifyReceipt *out_receipt);
int dm2_v1_skproject_is_miscitem_currency(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    DM2_V1_SkprojectItemClassifyReceipt *out_receipt);
int dm2_v1_skproject_get_item_name(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t champion_bones_item_id,
    uint8_t champion_count,
    DM2_V1_SkprojectItemNameReceipt *out_receipt);
int dm2_v1_skproject_get_item_order_in_container(
    uint16_t object_id,
    uint8_t container_cls2,
    const char *order_text,
    const uint16_t *money_item_ids,
    uint16_t money_item_count,
    uint16_t order,
    DM2_V1_SkprojectItemOrderReceipt *out_receipt);
int dm2_v1_skproject_fmt_num(
    uint16_t value,
    uint16_t clean,
    uint16_t keta,
    DM2_V1_SkprojectFmtNumReceipt *out_receipt);
int dm2_v1_skproject_sk_strlen(
    const char *text,
    DM2_V1_SkprojectStrLenReceipt *out_receipt);
int dm2_v1_skproject_sk_strstr(
    const char *haystack,
    const char *needle,
    DM2_V1_SkprojectStrStrReceipt *out_receipt);
int dm2_v1_skproject_sk_strcpy(
    char *dest,
    uint16_t dest_capacity,
    const char *source,
    DM2_V1_SkprojectStrCopyCatReceipt *out_receipt);
int dm2_v1_skproject_sk_strcat(
    char *dest,
    uint16_t dest_capacity,
    const char *source,
    DM2_V1_SkprojectStrCopyCatReceipt *out_receipt);
int dm2_v1_skproject_ltoa10(
    int32_t value,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectLtoa10Receipt *out_receipt);
int dm2_v1_skproject_skchr_to_scriptchr(
    uint8_t value,
    DM2_V1_SkprojectScriptChrReceipt *out_receipt);
int32_t dm2_v1_skproject_query_item_value(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t cls4,
    DM2_V1_SkprojectItemValueReceipt *out_receipt);
int32_t dm2_v1_skproject_query_item_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    DM2_V1_SkprojectItemValueReceipt *out_receipt);
int dm2_v1_skproject_calc_player_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t player,
    const DM2_V1_SkprojectPlayerWeightRequest *request,
    DM2_V1_SkprojectPlayerWeightReceipt *out_receipt);
int dm2_v1_skproject_equip_item_to_inventory(
    DM2_V1_SkprojectPlayerWeightRequest *request,
    uint16_t player,
    uint16_t object_id,
    uint16_t inventory_slot,
    DM2_V1_SkprojectEquipItemReceipt *out_receipt);
int dm2_v1_skproject_count_by_coin_types(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t moneybox_object_id,
    const uint16_t *money_item_ids,
    uint16_t money_item_count,
    int16_t *out_counts,
    DM2_V1_SkprojectCountByCoinTypesReceipt *out_receipt);
int dm2_v1_skproject_boost_attribute(
    DM2_V1_SkprojectChampionAttribute *attributes,
    uint8_t attribute_index,
    int16_t delta,
    DM2_V1_SkprojectBoostAttributeReceipt *out_receipt);
int dm2_v1_skproject_adjust_ui_event(
    DM2_V1_SkprojectUiEvent *event,
    uint16_t player_dir,
    const int16_t player_at_position[4],
    const DM2_V1_SkprojectUiChampionState *champions,
    uint16_t champion_count,
    DM2_V1_SkprojectAdjustUiEventReceipt *out_receipt);
int dm2_v1_skproject_draw_charsheet_option_icon(
    uint8_t cls4,
    uint16_t rect_no,
    uint16_t option_mask,
    uint16_t active_mask,
    DM2_V1_SkprojectCharsheetOptionIconReceipt *out_receipt);
int dm2_v1_skproject_draw_cmd_slot(
    uint16_t slot,
    uint8_t ww,
    uint8_t magical_map_flags,
    uint8_t held_container_type,
    const DM2_V1_SkprojectCommandSlotItem *item,
    DM2_V1_SkprojectDrawCmdSlotReceipt *out_receipt);
int dm2_v1_skproject_draw_moneybox(
    uint16_t moneybox_object_id,
    uint8_t container_cls2,
    const int16_t coin_order[10],
    const int16_t coin_counts[10],
    const uint16_t money_item_ids[10],
    DM2_V1_SkprojectDrawMoneyboxReceipt *out_receipt);
int dm2_v1_skproject_draw_item_stats_bar(
    uint16_t rect_no,
    int16_t current_value,
    int16_t max_value,
    uint8_t rune,
    uint16_t color,
    int rect_exists,
    DM2_V1_SkprojectDrawItemStatsBarReceipt *out_receipt);
int dm2_v1_skproject_draw_container_panel(
    uint16_t container_object_id,
    uint8_t container_cls2,
    uint8_t right_panel,
    const uint16_t items[8],
    DM2_V1_SkprojectDrawContainerPanelReceipt *out_receipt);
int dm2_v1_skproject_draw_item_icon(
    uint16_t object_id,
    uint8_t cls1,
    uint8_t cls2,
    uint8_t cls4,
    uint16_t rect_no,
    uint8_t slot_index,
    int selected,
    DM2_V1_SkprojectDrawItemIconReceipt *out_receipt);
int dm2_v1_skproject_draw_container_survey(
    const uint16_t *record_chain,
    uint16_t record_count,
    DM2_V1_SkprojectDrawContainerSurveyReceipt *out_receipt);
int dm2_v1_skproject_draw_item_in_hand(
    uint16_t object_id,
    uint8_t cls1,
    uint8_t cls2,
    uint8_t cls4,
    uint16_t width,
    uint16_t height,
    DM2_V1_SkprojectDrawItemInHandReceipt *out_receipt);
int dm2_v1_skproject_draw_item_survey(
    uint16_t object_id,
    uint8_t show_details,
    DM2_V1_SkprojectDrawItemSurveyReceipt *out_receipt);
int dm2_v1_skproject_draw_hand_action_icons(
    uint16_t player,
    uint16_t object_id,
    uint8_t primary_action_icon,
    uint8_t secondary_action_icon,
    uint8_t selected_hand,
    DM2_V1_SkprojectDrawHandActionIconsReceipt *out_receipt);
int dm2_v1_skproject_draw_item_on_wood_panel(
    uint16_t player,
    uint16_t possession_index,
    uint16_t object_id,
    int hand_activable,
    uint16_t base_width,
    uint16_t base_height,
    uint16_t extra_width,
    uint16_t extra_height,
    uint16_t temp_cache_index,
    DM2_V1_SkprojectDrawItemOnWoodPanelReceipt *out_receipt);
int dm2_v1_skproject_draw_cur_max_hms(
    uint16_t rect_no,
    int16_t current_value,
    int16_t max_value,
    DM2_V1_SkprojectDrawCurMaxHmsReceipt *out_receipt);
int dm2_v1_skproject_draw_player_3stat_text(
    const DM2_V1_SkprojectChampion3StatValues *stats,
    DM2_V1_SkprojectDrawPlayer3StatTextReceipt *out_receipt);
int dm2_v1_skproject_draw_player_3stat_pane(
    uint16_t player,
    int cur_hp,
    uint16_t inventory_player_plus_one,
    uint8_t button_group_busy,
    uint8_t clear_group_size,
    DM2_V1_SkprojectDrawPlayer3StatPaneReceipt *out_receipt);
int dm2_v1_skproject_draw_player_3stat_health_bar(
    uint16_t player,
    int rect_exists,
    DM2_V1_SkprojectDrawPlayer3StatHealthBarReceipt *out_receipt);
int dm2_v1_skproject_draw_player_name_at_cmdslot(
    uint16_t curacthero,
    uint16_t event_heroidx,
    DM2_V1_SkprojectDrawPlayerNameAtCmdSlotReceipt *out_receipt);
int dm2_v1_skproject_draw_player_damage(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_SkprojectDrawPlayerDamageReceipt *out_receipt);
int dm2_v1_skproject_draw_spell_to_be_cast(
    const char *runes,
    int draw_frame_icon,
    DM2_V1_SkprojectDrawSpellToBeCastReceipt *out_receipt);
int dm2_v1_skproject_draw_spell_panel(
    uint8_t nrunes,
    DM2_V1_SkprojectDrawSpellPanelReceipt *out_receipt);
int dm2_v1_skproject_draw_squad_spell_and_leader_icon(
    uint16_t player,
    uint16_t yy,
    uint8_t player_pos,
    uint8_t player_dir,
    int cur_hp,
    uint16_t champion_leader,
    uint8_t sleeping,
    uint8_t hero_b44,
    DM2_V1_SkprojectDrawSquadSpellLeaderReceipt *out_receipt);
int dm2_v1_skproject_draw_squad_pos_interface(
    uint8_t squad_gfx_set,
    const uint8_t *champion_pos,
    const uint8_t *champion_alive,
    const uint8_t *champion_enchanted,
    uint8_t champion_count,
    uint8_t player_dir,
    uint8_t selected_pos_plus_one,
    DM2_V1_SkprojectDrawSquadPosInterfaceReceipt *out_receipt);
int dm2_v1_skproject_draw_player_attack_dir(
    uint16_t champion_index,
    uint8_t squad_gfx_set,
    uint8_t aura_of_speed,
    uint8_t aura_rand_y,
    uint8_t aura_rand_x,
    uint8_t enchantment_power,
    DM2_V1_SkprojectDrawPlayerAttackDirReceipt *out_receipt);
int dm2_v1_skproject_draw_majic_map(
    uint16_t record_id,
    uint8_t container_cls2,
    uint8_t container_mode,
    uint16_t flags_before,
    uint16_t command_slot_count,
    uint16_t player_x,
    uint16_t player_y,
    uint16_t player_map,
    uint8_t gray_overlay_condition,
    DM2_V1_SkprojectDrawMajicMapReceipt *out_receipt);
int dm2_v1_skproject_draw_food_water_poison_panel(
    int16_t food,
    int16_t water,
    int16_t poison,
    DM2_V1_SkprojectDrawFoodWaterPoisonPanelReceipt *out_receipt);
int dm2_v1_skproject_draw_power_stat_bar(
    int16_t current_value,
    uint16_t rect_no,
    uint16_t color,
    int16_t floor_value,
    uint16_t tail_color,
    int rect_exists,
    DM2_V1_SkprojectDrawPowerStatBarReceipt *out_receipt);
int dm2_v1_skproject_draw_scroll_text(
    uint16_t object_id,
    int object_is_scroll,
    const char *message_text,
    DM2_V1_SkprojectDrawScrollTextReceipt *out_receipt);
int dm2_v1_skproject_draw_simple_str(
    uint16_t rect_no,
    uint16_t foreground_color,
    uint16_t background_color,
    const char *text,
    int rect_exists,
    DM2_V1_SkprojectDrawSimpleStrReceipt *out_receipt);
int dm2_v1_skproject_draw_skill_panel(
    uint16_t champion_index,
    uint8_t visible_skill_lines,
    uint8_t visible_attribute_lines,
    DM2_V1_SkprojectDrawSkillPanelReceipt *out_receipt);
int dm2_v1_skproject_draw_cryocell_lever(
    uint8_t lever_is_on,
    DM2_V1_SkprojectDrawCryocellLeverReceipt *out_receipt);
int dm2_v1_skproject_draw_eye_mouth_colored_rectangle(
    uint8_t cls4,
    uint16_t rect_no,
    DM2_V1_SkprojectDrawEyeMouthRectangleReceipt *out_receipt);
int dm2_v1_skproject_query_font(
    const uint8_t *font_plane,
    uint8_t glyph,
    uint8_t foreground,
    uint8_t background,
    DM2_V1_SkprojectFontReceipt *out_receipt);
int dm2_v1_skproject_query_str_metrics(
    const char *text,
    DM2_V1_SkprojectTextMetricsReceipt *out_receipt);
int dm2_v1_skproject_plan_draw_string(
    DM2_V1_SkprojectTextRoute route,
    int16_t dest_width,
    int16_t x,
    int16_t baseline_y,
    uint16_t foreground,
    uint16_t background,
    const char *text,
    DM2_V1_SkprojectTextDrawReceipt *out_receipt);
int dm2_v1_skproject_format_skstr_literal(
    const char *source,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectFormatTextReceipt *out_receipt);
int dm2_v1_skproject_decode_gdat_text_literal(
    const uint8_t *source,
    uint16_t source_len,
    int encrypted,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectFormatTextReceipt *out_receipt);
int dm2_v1_skproject_split_hint_line(
    const char *source,
    uint16_t start_offset,
    int16_t max_width,
    char *dest,
    uint16_t dest_capacity,
    DM2_V1_SkprojectHintLineReceipt *out_receipt);
int dm2_v1_skproject_gdat_sound_allocation_scan(
    const DM2_V1_SkprojectGdatDescriptor *entries,
    uint16_t entry_count,
    DM2_V1_SkprojectGdatSoundAllocationReceipt *out_receipt);
int dm2_v1_skproject_gdat_accepts_current_zone(
    uint16_t raw_index,
    uint16_t entry_type_2,
    uint16_t entry_type_5,
    uint8_t current_zone,
    DM2_V1_SkprojectGdatZoneReceipt *out_receipt);
int dm2_v1_skproject_load_dyn4_receipt(
    const DM2_V1_SkprojectGdatDescriptor *scripts,
    uint16_t script_count,
    const DM2_V1_SkprojectGdatDescriptor *entries,
    uint16_t entry_count,
    uint8_t *marks,
    uint16_t mark_capacity,
    int sound_table_active,
    DM2_V1_SkprojectLoadDyn4Receipt *out_receipt);

int dm2_v1_skproject_0cee_2df4_creature_ai_word30(
    uint16_t record_link,
    const DM2_V1_SkprojectCreatureAISpec *ai_spec,
    DM2_V1_SkprojectCreatureAIWord30Receipt *out_receipt);
int dm2_v1_skproject_19f0_124b_level_transition(
    int16_t *x,
    int16_t *y,
    uint16_t current_map,
    int16_t direction,
    uint16_t flags,
    const DM2_V1_SkprojectMapDescriptor *maps,
    uint16_t map_count,
    const uint8_t *tile_values,
    int16_t tile_width,
    int16_t tile_height,
    const uint8_t *ladder_around_dirs,
    uint16_t ladder_around_count,
    const uint8_t *target_tile_value,
    DM2_V1_SkprojectLevelTransitionReceipt *out_receipt);
int dm2_v1_skproject_29ee_18eb_level_transition_pair(
    int16_t x,
    int16_t y,
    uint16_t current_map,
    const DM2_V1_SkprojectMapDescriptor *maps,
    uint16_t map_count,
    const uint8_t *tile_values,
    int16_t tile_width,
    int16_t tile_height,
    const uint8_t *ladder_around_dirs,
    uint16_t ladder_around_count,
    const uint8_t *target_tile_value,
    DM2_V1_SkprojectLevelTransitionPairReceipt *out_receipt);
int dm2_v1_skproject_29ee_00a3_init_button_group_black(
    DM2_V1_Skproject0B36ButtonGroup *group,
    uint16_t rectno,
    const DM2_V1_SkprojectRect *expanded_rects,
    uint16_t expanded_rect_count,
    uint16_t allocated_cache_index,
    DM2_V1_SkprojectButtonGroupBlackFillReceipt *out_receipt);
int dm2_v1_skproject_29ee_0b2b_draw_command_slots(
    uint16_t slot_count,
    DM2_V1_SkprojectCommandSlotLoopReceipt *out_receipt);
int dm2_v1_skproject_0b36_0cbe_blit_dirty_rects(
    DM2_V1_Skproject0B36ButtonGroup *group,
    uint16_t free_cache_index,
    DM2_V1_Skproject0B36BlitDirtyRectsReceipt *out_receipt);
int dm2_v1_skproject_0b36_129a_draw_string_to_cache(
    DM2_V1_Skproject0B36ButtonGroup *group,
    int16_t x,
    int16_t y,
    uint8_t clr1,
    uint8_t clr2,
    const char *text,
    DM2_V1_Skproject0B36DrawStringReceipt *out_receipt);
int dm2_v1_skproject_12b4_0092_skwin_arrow_panel(
    uint16_t active_v1e0534,
    uint16_t arrow_panel,
    DM2_V1_SkprojectSkWin12B40092Receipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^443C mouse-event lock / UI tracking receipts */
void dm2_v1_skproject_ui_tracking_state_init(
    DM2_V1_SkprojectUiTrackingState *state);
int dm2_v1_skproject_443c_087c_lock_mouse_event(
    DM2_V1_SkprojectUiTrackingState *state,
    DM2_V1_SkprojectMouseEventLockReceipt *out_receipt);
int dm2_v1_skproject_443c_0889_unlock_mouse_event(
    DM2_V1_SkprojectUiTrackingState *state,
    DM2_V1_SkprojectMouseEventUnlockReceipt *out_receipt);
int dm2_v1_skproject_443c_040e_reset_mouse_tracking(
    DM2_V1_SkprojectUiTrackingState *state,
    DM2_V1_SkprojectMouseTrackingResetReceipt *out_receipt);
int dm2_v1_skproject_443c_00a9_set_tracking_context(
    DM2_V1_SkprojectUiTrackingState *state,
    uint16_t ref,
    int16_t x,
    int16_t cx,
    int16_t y,
    int16_t cy,
    DM2_V1_SkprojectMouseTrackingContextReceipt *out_receipt);
int dm2_v1_skproject_443c_06b4_insert_tracking_object(
    DM2_V1_SkprojectUiTrackingState *state,
    DM2_V1_SkprojectUiTrackingObject *obj,
    DM2_V1_SkprojectUiTrackingInsertReceipt *out_receipt);
int dm2_v1_skproject_443c_07d5_remove_tracking_object(
    DM2_V1_SkprojectUiTrackingState *state,
    DM2_V1_SkprojectUiTrackingObject *obj,
    DM2_V1_SkprojectUiTrackingRemoveReceipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^3E74 mement/cache management family.
   Models the source tlbMementsPointers table, LRU/MRU list (w4/w6/w8),
   free-block list (pv4/pv8), cache-index table, and tick-based usage
   reset used by ALLOC_LOWER_CPXHEAP, ALLOC_CPXHEAP_MEM, and the
   GDAT picture cache. */
#define DM2_V1_SKPROJECT_MEMENT_MAX 32u

typedef struct {
    uint16_t index;
    int32_t size;          /* negative = allocated, positive = free */
    uint16_t usage;        /* w4: 0 = cold, 0xffff = detached, 0xfffe = locked */
    int16_t lru_prev;      /* w6: previous mement index in LRU list */
    int16_t lru_next;      /* w8: next mement index in LRU list */
    uint16_t cache_index;  /* w10 | 0x8000, or 0xffff if none */
    uint16_t raw_index;    /* data index when not cache-backed */
    uint8_t in_free_list;
    uint8_t in_lru_list;
} DM2_V1_SkprojectMement;

typedef struct {
    DM2_V1_SkprojectMement mements[DM2_V1_SKPROJECT_MEMENT_MAX];
    uint16_t mement_count;
    int16_t lru_head;      /* _4976_5d90 MRU head */
    int16_t lru_tail;      /* _4976_5c8c LRU tail */
    int16_t lru_recent;    /* _4976_5d70 most-recently touched */
    int16_t free_head;     /* _4976_5d94 free-block list head */
    int16_t free_tail;     /* _4976_5d5e free-block list tail */
    int16_t next_free_ci;  /* _4976_5d36 next free cache index */
    uint16_t ci_count;     /* _4976_5d24 / _4976_5c92 cache-index capacity */
    uint16_t cache_to_mement[DM2_V1_SKPROJECT_MEMENT_MAX];
    uint16_t data_to_mement[DM2_V1_SKPROJECT_MEMENT_MAX];
    uint32_t last_tick;
    uint32_t heap_size;
    uint32_t free_heap_size;
} DM2_V1_SkprojectMementState;

typedef struct {
    int valid;
    uint16_t mementi;
    int32_t size_before;
    int32_t size_after;
    uint16_t usage_before;
    uint16_t usage_after;
    int16_t lru_head_after;
    int16_t lru_recent_after;
    uint8_t touched;
    uint8_t recycled;
    uint32_t receipt_hash;
} DM2_V1_SkprojectTouchMementReceipt;

typedef struct {
    int valid;
    uint16_t mementi;
    int16_t lru_prev_after;
    int16_t lru_next_after;
    uint8_t removed_from_lru;
    uint8_t cleared_links;
    uint32_t receipt_hash;
} DM2_V1_SkprojectRemoveMementReceipt;

typedef struct {
    int valid;
    uint16_t mementi;
    int32_t size;
    int16_t free_head_after;
    int16_t free_tail_after;
    uint8_t unlinked;
    uint8_t list_emptied;
    uint32_t receipt_hash;
} DM2_V1_SkprojectUnlinkFreeBlockReceipt;

typedef struct {
    int valid;
    uint16_t mementi;
    int32_t size;
    int16_t free_head_after;
    int16_t inserted_after;
    uint8_t inserted;
    uint8_t became_head;
    uint8_t became_tail;
    uint32_t receipt_hash;
} DM2_V1_SkprojectInsertFreeBlockReceipt;

typedef struct {
    int valid;
    uint16_t moved_blocks;
    uint16_t skipped_blocks;
    uint32_t free_heap_after;
    int16_t free_tail_after;
    uint32_t receipt_hash;
} DM2_V1_SkprojectCompactHeapReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint16_t mementi;
    uint8_t found_mementi;
    uint8_t removed_from_lru;
    uint8_t cleared_links;
    uint32_t receipt_hash;
} DM2_V1_Skproject3e74FreeCacheIndexReceipt;

typedef struct {
    int valid;
    uint16_t cache_index;
    uint16_t mementi;
    uint16_t recycle_yy;
    uint8_t found_mementi;
    uint8_t recycled;
    uint8_t freed_cache_index;
    uint32_t receipt_hash;
} DM2_V1_SkprojectRecycleOrFreeCacheReceipt;

typedef struct {
    int valid;
    int16_t cache_index;
    int16_t next_free_ci_after;
    uint16_t ci_count_before;
    uint16_t ci_count_after;
    uint8_t exhausted;
    uint32_t receipt_hash;
} DM2_V1_SkprojectFindFreeCacheIndexReceipt;

typedef struct {
    int valid;
    uint32_t tick;
    uint16_t reset_mements;
    uint16_t skipped_mements;
    int16_t lru_head_after;
    uint32_t receipt_hash;
} DM2_V1_SkprojectResetUsageCountersReceipt;

void dm2_v1_skproject_mement_state_init(DM2_V1_SkprojectMementState *state);
void dm2_v1_skproject_mement_lru_push_front(
    DM2_V1_SkprojectMementState *state,
    uint16_t mementi);
int dm2_v1_skproject_3e74_48c9_touch_mement(
    DM2_V1_SkprojectMementState *state,
    uint16_t mementi,
    DM2_V1_SkprojectTouchMementReceipt *out_receipt);
int dm2_v1_skproject_3e74_4549_remove_mement_from_list(
    DM2_V1_SkprojectMementState *state,
    uint16_t mementi,
    DM2_V1_SkprojectRemoveMementReceipt *out_receipt);
int dm2_v1_skproject_3e74_0c8c_unlink_free_block(
    DM2_V1_SkprojectMementState *state,
    uint16_t mementi,
    DM2_V1_SkprojectUnlinkFreeBlockReceipt *out_receipt);
int dm2_v1_skproject_3e74_0d32_insert_free_block(
    DM2_V1_SkprojectMementState *state,
    uint16_t mementi,
    DM2_V1_SkprojectInsertFreeBlockReceipt *out_receipt);
int dm2_v1_skproject_3e74_2b30_compact_heap(
    DM2_V1_SkprojectMementState *state,
    DM2_V1_SkprojectCompactHeapReceipt *out_receipt);
int dm2_v1_skproject_3e74_583a_free_cache_index(
    DM2_V1_SkprojectMementState *state,
    uint16_t cache_index,
    DM2_V1_Skproject3e74FreeCacheIndexReceipt *out_receipt);
int dm2_v1_skproject_3e74_585a_recycle_or_free_cache(
    DM2_V1_SkprojectMementState *state,
    uint16_t cache_index,
    uint16_t yy,
    DM2_V1_SkprojectRecycleOrFreeCacheReceipt *out_receipt);
int dm2_v1_skproject_3e74_4471_find_free_cache_index(
    DM2_V1_SkprojectMementState *state,
    DM2_V1_SkprojectFindFreeCacheIndexReceipt *out_receipt);
int dm2_v1_skproject_3e74_44ad_reset_usage_counters(
    DM2_V1_SkprojectMementState *state,
    uint32_t tick,
    DM2_V1_SkprojectResetUsageCountersReceipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^1C9A creature AI pointer / animation family (cycle 6) */
typedef struct {
    int valid;
    uint8_t is_static_object;
    uint16_t creature_index;
    uint16_t offset;
    uint8_t static_branch;
    uint8_t table_branch;
    uint32_t receipt_hash;
} DM2_V1_Skproject1C9A02C3Receipt;

int dm2_v1_skproject_1c9a_02c3_creature_ai_pointer(
    uint8_t is_static_object,
    uint16_t creature_index,
    DM2_V1_Skproject1C9A02C3Receipt *out_receipt);

typedef struct {
    uint16_t w0;
    uint16_t w2;
    uint8_t b4;
} DM2_V1_SkprojectAnimFrame;

typedef struct {
    int valid;
    uint16_t input_si;
    uint16_t output_si;
    uint8_t result_di;
    uint8_t has_content;
    uint8_t blocked_missing_frames;
    uint8_t blocked_missing_random;
    uint8_t blocked_out_of_range;
    uint16_t frame_index_used;
    uint16_t frames_consumed;
} DM2_V1_Skproject4937_01a9Receipt;

int dm2_v1_skproject_4937_01a9_select_frame(
    uint16_t xx,
    uint16_t *yy,
    const DM2_V1_SkprojectAnimFrame *frames,
    uint16_t frame_count,
    DM2_V1_SkprojectRandomData *randdat,
    DM2_V1_Skproject4937_01a9Receipt *out_receipt);

typedef struct {
    int valid;
    uint16_t sequence_w0;
    uint16_t result;
} DM2_V1_Skproject4937_000fReceipt;

int dm2_v1_skproject_4937_000f_animation_w0(
    uint16_t sequence_w0,
    DM2_V1_Skproject4937_000fReceipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^2759 command/hand helpers (cycle 6) */
typedef struct {
    int valid;
    uint8_t object_null;
    uint8_t cls1;
    uint8_t cls2;
    uint8_t found;
    uint8_t checked_count;
} DM2_V1_Skproject2759_0155Receipt;

int dm2_v1_skproject_2759_0155_query_object_commands(
    uint8_t object_null,
    uint8_t cls1,
    uint8_t cls2,
    const uint8_t *gdat_loadable,
    const uint8_t *cmdstr_cncm,
    const uint8_t *cmdstr_cnnc,
    DM2_V1_Skproject2759_0155Receipt *out_receipt);

typedef struct {
    int valid;
    uint8_t result;
    uint8_t object_null;
    uint8_t is_container;
    uint8_t container_type;
    uint8_t container_subtype;
    uint8_t has_missile_ref;
    uint8_t minion_type;
    uint16_t container_w6;
    uint8_t command;
} DM2_V1_Skproject2759_01feReceipt;

int dm2_v1_skproject_2759_01fe_command_valid(
    uint8_t command,
    uint8_t is_container,
    uint8_t container_type,
    uint8_t container_subtype,
    uint8_t has_missile_ref,
    uint8_t minion_type,
    uint16_t container_w6,
    DM2_V1_Skproject2759_01feReceipt *out_receipt);

typedef struct {
    int valid;
    int16_t hand;
    uint8_t hand_activable;
    uint8_t result;
    uint8_t side_effect_requested;
} DM2_V1_Skproject2759_0e93Receipt;

int dm2_v1_skproject_2759_0e93_hand_activation(
    int16_t hand,
    uint8_t hand_activable,
    const int16_t *item_selected_hands,
    uint16_t item_selected_count,
    DM2_V1_Skproject2759_0e93Receipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^24A5 centered viewport string helper (cycle 6) */
#define DM2_V1_SKPROJECT_24A5_0732_MAX_CONVERTED 200u

typedef struct {
    int valid;
    int16_t xx;
    int16_t yy;
    int16_t str_width;
    int16_t draw_x;
    uint8_t mbcs_present;
    uint8_t empty_string;
    uint8_t converted[DM2_V1_SKPROJECT_24A5_0732_MAX_CONVERTED];
    uint16_t converted_len;
    uint8_t requested_draw_vp_str;
} DM2_V1_Skproject24A5_0732Receipt;

int dm2_v1_skproject_24a5_0732_draw_centered_vp_str(
    int16_t xx,
    int16_t yy,
    const char *str,
    int16_t str_width,
    uint8_t mbcs_present,
    DM2_V1_Skproject24A5_0732Receipt *out_receipt);

/* SKWIN/SkWinCore.cpp:^2E62 item icon update helper (cycle 6) */
typedef struct {
    uint8_t b3;
    uint8_t b4;
    uint8_t b5;
    uint16_t w6;
} DM2_V1_Skproject2E62SlotState;

typedef struct {
    int valid;
    uint16_t player;
    uint16_t item_no;
    uint16_t si;
    uint16_t champion_inventory;
    uint16_t next_champion_number;
    uint16_t champion_index;
    uint16_t selected_hand_action;
    uint8_t early_return;
    uint8_t is_inventory_player;
    uint8_t is_next_champion;
    uint8_t item_in_hand;
    uint8_t body_flag;
    uint8_t item_cls2;
    uint8_t item_dbspec_word0_high;
    uint8_t dbspec_variant;
    uint8_t requested_draw_item_icon;
    uint8_t requested_draw_3stat_pane;
    uint8_t state_changed;
    DM2_V1_Skproject2E62SlotState state_before;
    DM2_V1_Skproject2E62SlotState state_after;
} DM2_V1_Skproject2E62_03B5Receipt;

int dm2_v1_skproject_2e62_03b5_item_icon_update(
    uint16_t player,
    uint16_t item_no,
    uint16_t champion_inventory,
    uint16_t next_champion_number,
    uint16_t champion_index,
    uint16_t selected_hand_action,
    uint8_t body_flag,
    uint16_t item_object,
    uint8_t item_cls2,
    uint8_t item_dbspec_word0_high,
    uint8_t dbspec_variant,
    DM2_V1_Skproject2E62SlotState *in_out_state,
    DM2_V1_Skproject2E62_03B5Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:23 DM2_query_098d_000f — 5x5 position to
   coarse grid coordinate conversion: w1 = ebxw % 5 + 4*eaxw,
   w2 = ebxw / 5 + 4*edxw. */
typedef struct {
    int valid;
    int16_t eaxw;
    int16_t edxw;
    int16_t ebxw;
    int16_t w1;
    int16_t w2;
} DM2_V1_Skproject098d000fReceipt;

int dm2_v1_skproject_098d_000f(
    int16_t eaxw,
    int16_t edxw,
    int16_t ebxw,
    int16_t *out_w1,
    int16_t *out_w2,
    DM2_V1_Skproject098d000fReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:29 DM2_IS_CLS1_CRITICAL_FOR_LOAD — returns true
   when the GDAT cls1 byte is 0x1b, 0x06 or 0x05. */
typedef struct {
    int valid;
    uint8_t cls1;
    uint8_t critical;
} DM2_V1_SkprojectCls1CriticalForLoadReceipt;

int dm2_v1_skproject_is_cls1_critical_for_load(
    uint8_t cls1,
    DM2_V1_SkprojectCls1CriticalForLoadReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:36 DM2_QUERY_GDAT_DYN_BUFF — source-locked
   allocation-path receipt.  The caller supplies the allocator facts
   (gfxalloc_done, cache hit, high/low pool) and the receipt records the
   source branch without performing real heap allocation. */
typedef struct {
    int valid;
    uint32_t dbidx_in;
    uint8_t gfxalloc_done;
    uint8_t cache_hit;
    uint8_t pool_hi;
    uint32_t raw_data_length;
    uint32_t dbidx_out;
    uint8_t path_taken;
#define DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_INITIAL 0u
#define DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_CACHE   1u
#define DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_CPX     2u
    uint32_t requested_size;
    uint8_t loaded_raw_data;
    uint8_t allocated_gfx256;
    uint8_t allocation1_called;
} DM2_V1_SkprojectGdatDynBuffReceipt;

int dm2_v1_skproject_query_gdat_dyn_buff(
    uint32_t dbidx_in,
    int gfxalloc_done,
    int cache_hit,
    int pool_hi,
    uint32_t raw_data_length,
    uint32_t *out_dbidx_out,
    DM2_V1_SkprojectGdatDynBuffReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:635 DM2_IS_WALL_ORNATE_ALCOVE — after the source
   resolves the wall-ornate cls2, it queries GDAT category
   DM2_GDAT_CATEGORY_WALL_GFX (9), index cls2, type
   DM2_GDAT_ENTRY_TYPE_WORD_VALUE (11), field 10, and returns the predicate
   (data_index != 0).  This receipt takes the already-resolved data_index so
   the helper stays independent of the asset loader. */
typedef struct {
    int valid;
    int blocked_invalid_cls2;
    uint8_t cls2;
    uint16_t data_index;
    uint8_t alcove_flag;
    uint32_t gdat_receipt_hash;
} DM2_V1_SkprojectWallOrnateAlcoveReceipt;

int dm2_v1_skproject_is_wall_ornate_alcove(
    uint8_t cls2,
    uint16_t data_index,
    DM2_V1_SkprojectWallOrnateAlcoveReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:646 DM2_IS_TILE_BLOCKED — source tile-type bit
   predicate used by movement and viewport code. */
typedef struct {
    int valid;
    uint8_t tile_type;
    uint8_t blocked;
    uint8_t branch;
} DM2_V1_SkprojectTileBlockedReceipt;

int dm2_v1_skproject_is_tile_blocked(
    uint8_t tile_type,
    DM2_V1_SkprojectTileBlockedReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:682 DM2_IS_REBIRTH_ALTAR — source-locked receipt
   over the map-header / record-byte predicate.  The caller supplies the
   record byte at offset 2 and the relevant bytes from ddat.v1e03c0. */
typedef struct {
    int valid;
    uint8_t record_byte2;
    uint8_t map_header_byte2;
    uint8_t map_header_byte3;
    uint16_t map_header_word_e;
    int32_t altar_value;
    uint8_t used_map_header_path;
} DM2_V1_SkprojectRebirthAltarReceipt;

int dm2_v1_skproject_is_rebirth_altar(
    uint8_t record_byte2,
    uint8_t map_header_byte2,
    uint8_t map_header_byte3,
    uint16_t map_header_word_e,
    DM2_V1_SkprojectRebirthAltarReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:793 DM2_IS_WALL_ORNATE_SPRING — after the source
   resolves the wall-ornate record to a cls2 via DM2_QUERY_CLS2_FROM_RECORD,
   it queries GDAT category DM2_GDAT_CATEGORY_WALL_GFX (9), index cls2,
   type DM2_GDAT_ENTRY_TYPE_WORD_VALUE (11), field 12, and returns the
   predicate (data_index != 0).  This receipt takes the already-resolved
   data_index so the helper stays independent of the asset loader. */
typedef struct {
    int valid;
    int blocked_invalid_cls2;
    uint8_t cls2;
    uint16_t data_index;
    uint8_t spring_flag;
    uint32_t gdat_receipt_hash;
} DM2_V1_SkprojectWallOrnateSpringReceipt;

int dm2_v1_skproject_is_wall_ornate_spring(
    uint8_t cls2,
    uint16_t data_index,
    DM2_V1_SkprojectWallOrnateSpringReceipt *out_receipt);

const char *dm2_v1_skproject_core_source_evidence(void);


#endif
