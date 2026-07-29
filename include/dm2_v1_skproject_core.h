#ifndef FIRESTAFF_DM2_V1_SKPROJECT_CORE_H
#define FIRESTAFF_DM2_V1_SKPROJECT_CORE_H

#include <stddef.h>
#include <stdint.h>

/* Forward declarations: the implementation includes the full headers, but
 * including them here would pull in dm2_v1_dungeon_loader.h and create
 * conflicting declarations for dm2_v1_skproject_get_tile_value and
 * dm2_v1_skproject_get_address_of_tile_record. */
struct DM2_V1_RecordPoolSet;
struct DM2_V1_DungeonData;

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
    uint16_t word32;
    uint16_t word34; /* byte offset 0x22; byte_at(spec, 0x23) is its high byte */
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
int dm2_v1_skproject_core_get_tile_value(
    const uint8_t *tiles,
    const uint8_t *passage,
    int16_t width,
    int16_t height,
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectGetTileValueReceipt *out_receipt);
int dm2_v1_skproject_core_get_address_of_tile_record(
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

/* SKULLWIN/c_querydb.cpp:1486 DM2_GET_CREATURE_AT — source-locked wrapper
   around the existing cell-chain resolver; returns the first DB4 record
   chained on (map,x,y) or DM2_V1_RECORD_HANDLE_NULL. */
typedef struct {
    int valid;
    int blocked_missing_pool_set;
    int blocked_missing_dungeon;
    int16_t creature_record;
} DM2_V1_SkprojectGetCreatureAtReceipt;

int dm2_v1_skproject_get_creature_at(
    const struct DM2_V1_RecordPoolSet *pool_set,
    const struct DM2_V1_DungeonData *dungeon,
    int map,
    int x,
    int y,
    int16_t *out_creature,
    DM2_V1_SkprojectGetCreatureAtReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1645 DM2_FIND_LADDAR_AROUND — source-locked wrapper
   around the existing ladder search; converts its receipt into the core
   querydb receipt shape. */
typedef struct {
    int valid;
    int found;
    int level;
    int origin_x;
    int origin_y;
    int ladder_x;
    int ladder_y;
    int kind;
    int vertical_delta;
    uint32_t search_hash;
} DM2_V1_SkprojectFindLadderAroundReceipt;

int dm2_v1_skproject_find_ladder_around(
    const struct DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectFindLadderAroundReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1828 DM2_GET_PLAYER_AT_POSITION — party-position
   array lookup over caller-owned glbPlayerAtPosition[4]. */
typedef struct {
    int valid;
    uint8_t position;
    int8_t player_index;
} DM2_V1_SkprojectGetPlayerAtPositionReceipt;

int dm2_v1_skproject_get_player_at_position(
    uint8_t position,
    const int8_t player_at_position[4],
    int8_t *out_player,
    DM2_V1_SkprojectGetPlayerAtPositionReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1900 DM2_DIR_FROM_5x5_POS — dominant-axis
   direction extraction from a view-relative 5x5 cell index. */
typedef struct {
    int valid;
    uint8_t pos5x5;
    int8_t rel_x;
    int8_t rel_y;
    uint8_t dir;
    int blocked_center;
} DM2_V1_SkprojectDirFrom5x5PosReceipt;

int dm2_v1_skproject_dir_from_5x5_pos(
    uint8_t pos5x5,
    uint8_t *out_dir,
    DM2_V1_SkprojectDirFrom5x5PosReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1926 DM2_GET_GLOB_VAR — word-global lookup over
   caller-owned global-words table (source size is DM2_GLOBAL_WORDS_SIZE). */
typedef struct {
    int valid;
    uint16_t index;
    uint16_t value;
    int blocked_out_of_range;
} DM2_V1_SkprojectGetGlobVarReceipt;

int dm2_v1_skproject_get_glob_var(
    uint16_t index,
    const uint16_t *global_words,
    uint16_t global_word_count,
    uint16_t *out_value,
    DM2_V1_SkprojectGetGlobVarReceipt *out_receipt);

/* SKULLWIN/dm2global.cpp:20 DM2_UPDATE_GLOB_VAR — source-locked receipt over
   a caller-owned global-words table.  Operation modes (op):
     0=set to 1, 1=set to 0, 2=toggle (0->1, nonzero->0),
     3=add value, 4=subtract value, 5=no-op, 6=assign value.
   Returns the new value written. */
typedef struct {
    int valid;
    uint16_t index;
    uint16_t old_value;
    uint16_t new_value;
    uint16_t op;
    int16_t operand;
    int blocked_out_of_range;
    int blocked_bad_op;
} DM2_V1_SkprojectUpdateGlobVarReceipt;

int dm2_v1_skproject_update_glob_var(
    uint16_t index,
    uint16_t op,
    int16_t operand,
    uint16_t *global_words,
    uint16_t global_word_count,
    DM2_V1_SkprojectUpdateGlobVarReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:982 DM2_GET_CREATURE_WEIGHT — source-locked receipt
   over a caller-resolved creature weight; >0xfd is recorded as overweight. */
typedef struct {
    int valid;
    uint16_t weight;
    int overweight;
} DM2_V1_SkprojectGetCreatureWeightReceipt;

int dm2_v1_skproject_get_creature_weight(
    uint16_t weight_in,
    uint16_t *out_weight,
    DM2_V1_SkprojectGetCreatureWeightReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2499 DM2_CONVERT_PALETTE256 — convert a 256-entry
   caller-owned RGB888 palette to an RGB666 destination, with optional
   per-index translation table. */
typedef struct {
    int valid;
    uint32_t palette_hash;
} DM2_V1_SkprojectConvertPalette256Receipt;

int dm2_v1_skproject_convert_palette256(
    const uint8_t *src_rgb8,
    const uint8_t *translation_table,
    uint8_t dst_rgb6[256][3],
    DM2_V1_SkprojectConvertPalette256Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:880 DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR —
   source-locked receipt over a tile-chain search for a distinctive item type,
   optionally descending into containers.  The distinctive type is resolved
   through a caller-provided callback because DM2_GET_DISTINCTIVE_ITEMTYPE
   record semantics are not yet proven in Firestaff. */
typedef uint16_t (*dm2_v1_skproject_distinctive_type_fn)(
    uint16_t object_id,
    void *user);

typedef struct {
    int valid;
    int blocked_missing_dungeon;
    int blocked_missing_callback;
    int16_t input_x;
    int16_t input_y;
    uint16_t distinctive_type;
    uint8_t search_items;
    uint8_t found;
    uint16_t matched_object_id;
    uint16_t container_count;
    uint16_t item_count;
} DM2_V1_SkprojectDistinctiveItemOnActuatorReceipt;

int dm2_v1_skproject_is_distinctive_item_on_actuator(
    const struct DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    uint16_t distinctive_type,
    int search_items,
    dm2_v1_skproject_distinctive_type_fn type_fn,
    void *type_user,
    DM2_V1_SkprojectDistinctiveItemOnActuatorReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1509 DM2_FIND_HAND_WITH_EMPTY_FLASK — source-locked
   receipt that scans the two hand slots for a type-8 item whose cls2 is 0x14.
   The cls2 resolver is a caller callback because full record semantics are
   not yet proven. */
typedef uint8_t (*dm2_v1_skproject_cls2_from_object_fn)(
    uint16_t object_id,
    void *user);

typedef struct {
    int valid;
    int blocked_missing_callback;
    int16_t hand;
    uint16_t hand_object_ids[2];
    uint8_t hand_types[2];
    uint8_t hand_cls2[2];
    uint8_t found;
} DM2_V1_SkprojectFindHandWithEmptyFlaskReceipt;

int dm2_v1_skproject_find_hand_with_empty_flask(
    uint16_t hand_object_ids[2],
    dm2_v1_skproject_cls2_from_object_fn cls2_fn,
    void *cls2_user,
    int16_t *out_hand,
    DM2_V1_SkprojectFindHandWithEmptyFlaskReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1540 DM2_FIND_DISTINCTIVE_ITEM_ON_TILE —
   source-locked receipt that walks a tile chain looking for an item whose
   distinctive type matches, optionally constrained by subtype (bits 14-15 of
   the ObjectID). */
typedef struct {
    int valid;
    int blocked_missing_dungeon;
    int blocked_missing_callback;
    int16_t input_x;
    int16_t input_y;
    uint16_t distinctive_type;
    int16_t subtype;
    uint16_t found_object_id;
    uint8_t found;
    uint16_t visited_items;
} DM2_V1_SkprojectFindDistinctiveItemOnTileReceipt;

int dm2_v1_skproject_find_distinctive_item_on_tile(
    const struct DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    uint16_t distinctive_type,
    int16_t subtype,
    dm2_v1_skproject_distinctive_type_fn type_fn,
    void *type_user,
    DM2_V1_SkprojectFindDistinctiveItemOnTileReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:1576 DM2_FIND_TILE_ACTUATOR — source-locked receipt
   that walks a tile chain looking for the first DB3 actuator whose ordinal
   matches, optionally constrained by side (ObjectID bits 14-15). */
typedef struct {
    int valid;
    int blocked_missing_dungeon;
    int16_t input_x;
    int16_t input_y;
    uint8_t actuator_ordinal;
    int16_t side;
    uint16_t found_object_id;
    uint8_t found;
    uint8_t actuator_count;
    uint8_t skipped_non_actuator;
} DM2_V1_SkprojectFindTileActuatorReceipt;

int dm2_v1_skproject_find_tile_actuator(
    const struct DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    uint8_t actuator_ordinal,
    int16_t side,
    uint16_t *out_object_id,
    DM2_V1_SkprojectFindTileActuatorReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2175 DM2_CALC_PLAYER_WALK_DELAY — source-locked
   receipt implementing the encumbrance / bodyflag / walkspeed formula.  All
   hero fields are caller-resolved so the helper does not depend on the
   unproven c_hero layout. */
typedef struct {
    int valid;
    uint16_t max_load;
    uint16_t player_weight;
    uint8_t bodyflag;
    int8_t walkspeed;
    uint8_t savegames1_b_04;
    uint8_t overburdened;
    uint8_t heavy_load;
    uint8_t bodyflag_slow;
    int32_t base_delay;
    int32_t final_delay;
} DM2_V1_SkprojectCalcPlayerWalkDelayReceipt;

int dm2_v1_skproject_calc_player_walk_delay(
    uint16_t max_load,
    uint16_t player_weight,
    uint8_t bodyflag,
    int8_t walkspeed,
    uint8_t savegames1_b_04,
    int32_t *out_delay,
    DM2_V1_SkprojectCalcPlayerWalkDelayReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2237 DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH —
   source-locked receipt implementing the attack/throw strength formula.  All
   hero/item/skill fields are caller-resolved. */
typedef struct {
    int valid;
    uint8_t ability;
    uint16_t max_load;
    uint16_t item_weight;
    uint8_t skill_level;
    int16_t skill_kind;
    uint16_t dbspec_word5;
    uint16_t dbspec_word8;
    uint16_t dbspec_word9;
    uint8_t bodyflag;
    uint8_t hand_index;
    int16_t stamina_adj;
    int32_t pre_strength;
    int32_t final_strength;
    uint8_t bodyflag_halved;
} DM2_V1_SkprojectComputePlayerAttackOrThrowStrengthReceipt;

int dm2_v1_skproject_compute_player_attack_or_throw_strength(
    uint8_t ability,
    uint16_t max_load,
    uint16_t item_weight,
    uint8_t skill_level,
    int16_t skill_kind,
    uint16_t dbspec_word5,
    uint16_t dbspec_word8,
    uint16_t dbspec_word9,
    uint8_t bodyflag,
    uint8_t hand_index,
    int16_t stamina_adj,
    int16_t *out_strength,
    DM2_V1_SkprojectComputePlayerAttackOrThrowStrengthReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2431 DM2_query_32cb_0804 — palette/alpha query
   dispatcher.  The full GDAT palette-cache path is caller-owned; this
   receipted helper records the route decision and fails closed when the
   GDAT-dependent consumers are not supplied. */
typedef struct {
    int valid;
    int blocked_missing_palette;
    int blocked_missing_colors_out;
    int blocked_missing_gdat_path;
    int32_t edxl;
    int32_t ebxl;
    int32_t ecxl;
    int16_t colors_before;
    int16_t colors_after;
    uint8_t routed_to_0b36;
    uint8_t routed_to_b073;
} DM2_V1_SkprojectQuery32cb0804Receipt;

int dm2_v1_skproject_query_32cb_0804(
    uint8_t palette[256][3],
    int32_t edxl,
    int32_t ebxl,
    int32_t ecxl,
    int16_t *colors_io,
    DM2_V1_SkprojectQuery32cb0804Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2477 DM2_query_0b36_037e — cached-picture palette
   path.  Requires the CPX/dballoc layer; without it the helper is
   receipted and fail-closed. */
typedef struct {
    int valid;
    int blocked_missing_palette;
    int blocked_missing_colors_out;
    int blocked_missing_dballoc_path;
    uint8_t edxb;
    uint8_t ebxb;
    uint8_t ecxb;
    uint8_t argb0;
    int16_t argw1;
    int16_t argw2;
    int16_t colors_before;
} DM2_V1_SkprojectQuery0b36037eReceipt;

int dm2_v1_skproject_query_0b36_037e(
    uint8_t palette[256][3],
    uint8_t edxb,
    uint8_t ebxb,
    uint8_t ecxb,
    uint8_t argb0,
    int16_t argw1,
    int16_t argw2,
    int16_t *colors_io,
    DM2_V1_SkprojectQuery0b36037eReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2674 DM2_query_1c9a_08bd — creature airborne/levitate
   predicate from the runtime creature table.  The caller owns both the
   object record bytes and the 34-byte-per-creature runtime array. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_creatures;
    int blocked_out_of_range;
    uint8_t creature_index;
    uint8_t byte_1a;
    uint8_t byte_1f;
    uint8_t result;
} DM2_V1_SkprojectQuery1c9a08bdReceipt;

int dm2_v1_skproject_query_1c9a_08bd(
    const uint8_t *object_record,
    const uint8_t *creatures,
    uint16_t creature_count,
    uint8_t *out_result,
    DM2_V1_SkprojectQuery1c9a08bdReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2699 DM2_IS_CREATURE_FLOATING — checks the creature
   AI spec word@0xa bit 2, falling back to DM2_query_1c9a_08bd. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_ai_spec;
    int blocked_missing_creatures;
    uint16_t object_handle;
    uint8_t creature_type;
    uint16_t ai_word10;
    uint8_t ai_spec_floating_bit;
    uint8_t used_fallback;
    uint8_t fallback_result;
    DM2_V1_SkprojectQuery1c9a08bdReceipt fallback_receipt;
    uint8_t floating;
} DM2_V1_SkprojectIsCreatureFloatingReceipt;

int dm2_v1_skproject_is_creature_floating(
    uint16_t object_handle,
    const struct DM2_V1_RecordPoolSet *pools,
    const uint8_t *creatures,
    uint16_t creature_count,
    const uint16_t *ai_word10,
    uint16_t ai_word10_count,
    uint8_t *out_floating,
    DM2_V1_SkprojectIsCreatureFloatingReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2718 DM2_IS_OBJECT_FLOATING — classifies floating by
   record pool type, delegating type-4 creatures to IS_CREATURE_FLOATING. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_ai_spec;
    int blocked_missing_creatures;
    uint16_t object_handle;
    uint8_t object_type;
    uint8_t floating;
    uint8_t delegated_to_creature;
    DM2_V1_SkprojectIsCreatureFloatingReceipt creature_receipt;
} DM2_V1_SkprojectIsObjectFloatingReceipt;

int dm2_v1_skproject_is_object_floating(
    uint16_t object_handle,
    const struct DM2_V1_RecordPoolSet *pools,
    const uint8_t *creatures,
    uint16_t creature_count,
    const uint16_t *ai_word10,
    uint16_t ai_word10_count,
    uint8_t *out_floating,
    DM2_V1_SkprojectIsObjectFloatingReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2738 DM2_QUERY_OBJECT_5x5_POS — view-cell position
   for an object handle, rotating a base 5x5 cell by the view direction.
   Type-4 creatures need the GDAT-backed DM2_QUERY_CREATURE_5x5_POS and are
   fail-closed here; other types use a caller-owned subtype table. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_creature_pos;
    int blocked_missing_pos_table;
    int blocked_bad_pos;
    uint16_t object_handle;
    uint8_t object_type;
    uint8_t subtype;
    uint8_t direction;
    uint8_t base_pos;
    uint8_t rotated_pos;
    uint8_t used_creature_path;
    uint8_t used_object_table;
    uint8_t used_default_pos;
} DM2_V1_SkprojectQueryObject5x5PosReceipt;

int dm2_v1_skproject_query_object_5x5_pos(
    uint16_t object_handle,
    uint8_t direction,
    const struct DM2_V1_RecordPoolSet *pools,
    const uint8_t *object_pos_table, /* 4 entries, indexed by subtype */
    uint8_t *out_pos,
    DM2_V1_SkprojectQueryObject5x5PosReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2801 DM2_query_48ae_05ae — item-versus-creature
   damage/value formula.  Full evaluation needs GDAT creature-word and item
   DBSPEC lookups; the helper records inputs and fails closed without them. */
typedef struct {
    int valid;
    int blocked_missing_item_handle;
    int blocked_missing_gdat_path;
    uint16_t item_handle;
    uint8_t creature_type;
    uint16_t item_word10;
    int32_t argl0;
    int32_t argl1_in;
    int32_t argl1_out;
    int32_t result;
} DM2_V1_SkprojectQuery48ae05aeReceipt;

int dm2_v1_skproject_query_48ae_05ae(
    uint16_t item_handle,
    uint8_t creature_type,
    uint16_t item_word10,
    int32_t argl0,
    int32_t argl1,
    int32_t *out_result,
    DM2_V1_SkprojectQuery48ae05aeReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2936 DM2_query_4E26 — timer-word tick calculation.
   Fully source-locked: bit 0x4000 yields 0, bit 0x8000 adds a shifted
   interval to the game tick modulo the period, otherwise returns the period. */
typedef struct {
    int valid;
    int blocked_missing_timer_word;
    int blocked_zero_divisor;
    int cleared_timer_bits;
    uint16_t timer_word_before;
    uint16_t timer_word_after;
    uint32_t game_tick;
    uint16_t result;
    uint8_t bit_4000;
    uint8_t bit_8000;
    uint8_t bit_1000;
} DM2_V1_SkprojectQuery4e26Receipt;

int dm2_v1_skproject_query_4e26(
    uint16_t *timer_word,
    uint32_t game_tick,
    uint16_t *out_value,
    DM2_V1_SkprojectQuery4e26Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:2990 DM2_query_4DA3 — GDAT (0xf,cls2,0x7,0xfd) eight-byte
   blob fetch.  The source multiplies the timer-word interval (via DM2_query_4E26)
   by eight, adds the caller's addend, masks to 16 bits, and copies eight bytes.
   The helper is receipted and fail-closed when the GDAT blob is unavailable or
   the computed offset overruns it. */
typedef struct {
    int valid;
    int blocked_missing_gdat;
    int blocked_missing_timer_word;
    int blocked_zero_divisor;
    int blocked_out_of_bounds;
    uint8_t cls2;
    uint32_t addend;
    uint16_t timer_word_before;
    uint16_t timer_word_after;
    uint16_t interval;
    uint32_t offset;
    uint8_t copied[8];
} DM2_V1_SkprojectQuery4da3Receipt;

int dm2_v1_skproject_query_4da3(
    uint8_t cls2,
    uint32_t addend,
    uint16_t *timer_word,
    const uint8_t *gdat_data,
    uint32_t gdat_size,
    uint8_t out_bytes[8],
    DM2_V1_SkprojectQuery4da3Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3012 DM2_QUERY_CREATURE_5x5_POS — creature view-cell
   position.  The source checks GDAT loadability, resolves the creature AI spec
   pointer, calls DM2_query_4DA3, and rotates the fifth byte of the returned
   eight-byte blob.  Non-creature callers should use dm2_v1_skproject_query_object_5x5_pos.
   The helper is receipted and fail-closed when any dependency is missing. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_ai_spec;
    int blocked_missing_gdat;
    int blocked_4da3_failed;
    int blocked_bad_pos;
    uint8_t creature_type;
    uint8_t direction;
    uint8_t base_pos;
    uint8_t rotated_pos;
} DM2_V1_SkprojectQueryCreature5x5PosReceipt;

int dm2_v1_skproject_query_creature_5x5_pos(
    const uint8_t *creature_record,
    uint8_t direction,
    const DM2_V1_SkprojectCreatureAISpec *ai_spec,
    uint16_t addend_from_1c9a_02c3,
    uint16_t timer_word_from_1c9a_02c3,
    const uint8_t *gdat_4da3_data,
    uint32_t gdat_size,
    uint8_t *out_pos,
    DM2_V1_SkprojectQueryCreature5x5PosReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3061 DM2_query_0cee_0897 — teleporter-sensor probe on a
   tile.  The source accepts only tile type 5, walks the square's record list for
   a type-3 actuator with word2 & 0x7f == 0x27, and derives a direction/detail
   value from the first matching record.  The helper is receipted and fail-closed
   when the tile map or record pool is unavailable. */
typedef struct {
    int valid;
    int blocked_missing_tiles;
    int blocked_missing_record_pool;
    int blocked_out_of_bounds;
    int blocked_not_tile_type_5;
    int blocked_no_teleporter;
    int16_t x;
    int16_t y;
    uint8_t tile_value;
    uint8_t tile_type;
    uint16_t first_record_link;
    uint16_t found_record_link;
    uint8_t detail;
} DM2_V1_SkprojectQuery0cee0897Receipt;

int dm2_v1_skproject_query_0cee_0897(
    int16_t x,
    int16_t y,
    const uint8_t *tile_values,
    int16_t width,
    int16_t height,
    const struct DM2_V1_RecordPoolSet *pools,
    uint16_t *out_first_record_link,
    uint8_t *out_detail,
    DM2_V1_SkprojectQuery0cee0897Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3111 DM2_GET_TELEPORTER_DETAIL — resolves a teleporter
   sensor into its destination square/map.  The source calls DM2_query_0cee_0897
   at the origin, temporarily changes map, calls it again at the destination,
   and packs the result into a five-byte detail structure.  The helper is
   receipted and fail-closed when the origin/destination state is unavailable. */
typedef struct {
    uint8_t b_00; /* direction + 2 mod 4 */
    uint8_t b_01; /* direction + 1 mod 4 */
    uint8_t b_02; /* destination x */
    uint8_t b_03; /* destination y */
    uint8_t b_04; /* destination map */
} DM2_V1_SkprojectTeleporterDetail;

typedef struct {
    int valid;
    int blocked_missing_origin;
    int blocked_missing_destination;
    int blocked_missing_map_state;
    int blocked_tile_not_teleporter;
    int16_t origin_x;
    int16_t origin_y;
    int16_t dest_x;
    int16_t dest_y;
    uint8_t dest_map;
} DM2_V1_SkprojectGetTeleporterDetailReceipt;

int dm2_v1_skproject_get_teleporter_detail(
    int16_t x,
    int16_t y,
    const uint8_t *tile_values,
    int16_t width,
    int16_t height,
    const struct DM2_V1_RecordPoolSet *pools,
    uint8_t current_map,
    const uint8_t *dest_tile_values,
    int16_t dest_width,
    int16_t dest_height,
    DM2_V1_SkprojectTeleporterDetail *out_detail,
    DM2_V1_SkprojectGetTeleporterDetailReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3173 DM2_IS_CREATURE_MOVABLE_THERE — runtime movement
   predicate for a creature at (x,y) moving in direction.  The source fetches the
   creature at the square, checks weight, resolves teleporters, and validates the
   destination tile.  The helper is receipted and fail-closed when the runtime
   state is unavailable. */
typedef struct {
    int valid;
    int blocked_missing_creature;
    int blocked_overweight;
    int blocked_teleporter_forbidden;
    int blocked_target_blocked;
    int blocked_target_occupied;
    int blocked_missing_tile;
    int blocked_missing_record_pool;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint16_t creature_handle;
    uint16_t creature_weight;
    uint8_t movable;
} DM2_V1_SkprojectIsCreatureMovableThereReceipt;

int dm2_v1_skproject_is_creature_movable_there(
    int16_t x,
    int16_t y,
    uint8_t direction,
    uint16_t creature_handle,
    uint16_t creature_weight,
    const uint8_t *tile_values,
    int16_t width,
    int16_t height,
    const struct DM2_V1_RecordPoolSet *pools,
    uint8_t current_map,
    const uint8_t *dest_tile_values,
    int16_t dest_width,
    int16_t dest_height,
    uint16_t *out_creature_handle,
    DM2_V1_SkprojectIsCreatureMovableThereReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3296 DM2_query_0cee_1a46 — wall-decoration/actuator
   record walk for side-indexed wall gfx.  The source walks the square's record
   list, handling text records (type 2) and actuators (type 3) with ornate
   animation frames.  The helper delegates the static text-record path to the
   dungeon-loader wall-gfx walker and is receipted/fail-closed on the dynamic
   actuator/animation paths. */
typedef struct {
    int valid;
    int blocked_missing_record_pool;
    int blocked_missing_output;
    int blocked_missing_dungeon;
    int blocked_no_wall_gfx;
    int blocked_actuator_animation_path;
    uint16_t first_thing;
    int16_t view_dir;
    int16_t side_index;
    uint16_t wall_gfx_index;
    uint16_t wall_gfx_field;
    uint8_t found_static_text;
} DM2_V1_SkprojectQuery0cee1a46Receipt;

int dm2_v1_skproject_query_0cee_1a46(
    const struct DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int16_t view_dir,
    int16_t side_index,
    int16_t *out_wall_gfx_index,
    int16_t *out_wall_gfx_field,
    DM2_V1_SkprojectQuery0cee1a46Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3735 DM2_query_48ae_011a — object frame-class query
   from GDAT loadability.  The source reads cls1/cls2 from the object record and
   returns a frame class based on whether GDAT entries 0x8/0x9/0xa/0xc are
   loadable.  The helper is receipted and fail-closed when the record or GDAT
   loadability callback is unavailable. */
typedef int (*DM2_V1_SkprojectGdatLoadableFn)(
    uint8_t cls1,
    uint8_t cls2,
    uint8_t entry_index,
    uint8_t entry_id,
    void *user);

typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_missing_loadable_fn;
    int blocked_missing_gdat_path;
    uint16_t object_handle;
    uint8_t object_type;
    uint8_t cls1;
    uint8_t cls2;
    int32_t frame_class;
    uint8_t entry_8_loadable;
    uint8_t entry_9_loadable;
    uint8_t entry_a_loadable;
    uint8_t entry_c_loadable;
} DM2_V1_SkprojectQuery48ae011aReceipt;

int dm2_v1_skproject_query_48ae_011a(
    uint16_t object_handle,
    const struct DM2_V1_RecordPoolSet *pools,
    DM2_V1_SkprojectGdatLoadableFn loadable_fn,
    void *loadable_user,
    int32_t *out_frame_class,
    DM2_V1_SkprojectQuery48ae011aReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3760 DM2_query_0cee_2e09 — creature AI spec word at
   byte offset 0x20 (word 16).  The source resolves the AI spec from the creature
   type and returns word_at(spec,0x20).  The helper is receipted and fail-closed
   when the AI spec is unavailable. */
typedef struct {
    int valid;
    int blocked_object_null;
    int blocked_missing_ai_spec;
    uint16_t record_link;
    uint16_t word32;
} DM2_V1_SkprojectQuery0cee2e09Receipt;

int dm2_v1_skproject_query_0cee_2e09(
    uint16_t record_link,
    const DM2_V1_SkprojectCreatureAISpec *ai_spec,
    uint16_t *out_word32,
    DM2_V1_SkprojectQuery0cee2e09Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp cycle-14 query batch — shared caller-supplied
   callback types.  The DM2 runtime spatial index, record pools, GDAT
   creature tables, and data-segment tables are not yet owned by Firestaff,
   so the source-locked helpers below take them as callbacks/tables and fail
   closed when they are unavailable. */
typedef uint16_t (*DM2_V1_SkprojectGdatCreatureWordFn)(
    uint8_t creature_type,
    uint8_t word_index,
    void *user);

typedef uint8_t (*DM2_V1_SkprojectTileValueFn)(
    int16_t x,
    int16_t y,
    void *user);

typedef const uint8_t *(*DM2_V1_SkprojectRecordAccessorFn)(
    uint16_t handle,
    uint16_t *out_size,
    void *user);

typedef int32_t (*DM2_V1_SkprojectCls2FromRecordFn)(
    uint16_t handle,
    void *user); /* -1 when the record is unavailable; else cls2 */

typedef uint16_t (*DM2_V1_SkprojectDistinctiveTypeFn)(
    uint16_t handle,
    void *user);

typedef int32_t (*DM2_V1_SkprojectNextRecordFn)(
    uint16_t handle,
    void *user); /* -1 on a bad link; else the next record handle */

typedef int32_t (*DM2_V1_SkprojectCreatureAtFn)(
    int16_t x,
    int16_t y,
    void *user); /* -1 when no creature occupies the cell */

typedef const DM2_V1_SkprojectCreatureAISpec *(
    *DM2_V1_SkprojectAISpecFromRecordFn)(
    uint8_t creature_type,
    void *user);

typedef uint16_t (*DM2_V1_SkprojectCreature5x5PosValueFn)(
    const uint8_t *record,
    uint16_t record_size,
    uint8_t rotation_param,
    void *user);

typedef int (*DM2_V1_SkprojectQuery098d000fFn)(
    int16_t x,
    int16_t y,
    int16_t value,
    int16_t *out_x,
    int16_t *out_y,
    void *user);

/* SKULLWIN/c_querydb.cpp:3769 DM2_query_1c9a_03cf — nearest creature query
   over a five-cell scan.  Receipted and fail-closed on missing callbacks,
   tables, records, AI specs, or out-of-bounds table indices. */
typedef struct {
    int valid;
    int found;
    int blocked_missing_output;
    int blocked_missing_callback;
    int blocked_missing_table;
    int blocked_table_bounds;
    int blocked_missing_record;
    int blocked_missing_ai_spec;
    int blocked_query_098d;
    int16_t input_x;
    int16_t input_y;
    uint16_t direction;
    int16_t range;
    int16_t adj_x;
    int16_t adj_y;
    int16_t output_x;
    int16_t output_y;
    uint32_t result_handle;
    int32_t distance2;
    int16_t threshold;
    int ai_spec_byte23;
    int32_t checked_handles[5];
    uint16_t steps;
} DM2_V1_SkprojectQuery1c9a03cfReceipt;

int dm2_v1_skproject_query_1c9a_03cf(
    int16_t *x,
    int16_t *y,
    uint16_t direction,
    DM2_V1_SkprojectCreatureAtFn creature_at_fn,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectAISpecFromRecordFn ai_spec_fn,
    DM2_V1_SkprojectCreature5x5PosValueFn pos_fn,
    DM2_V1_SkprojectQuery098d000fFn q098d_fn,
    void *user,
    const int16_t *table1d2752,
    uint16_t table1d2752_size,
    const int16_t (*table1d62b0)[2],
    uint16_t table1d62b0_rows,
    const int16_t (*table1d62d0)[2],
    uint16_t table1d62d0_rows,
    const int16_t *table1d62e0,
    uint16_t table1d62e0_size,
    const int8_t *table1d62e8,
    uint16_t table1d62e8_size,
    uint32_t *out_handle,
    DM2_V1_SkprojectQuery1c9a03cfReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3892 DM2_query_48ae_01af — bit-gated byte table
   lookup over caller-supplied table1d2660 data. */
typedef struct {
    int valid;
    int blocked_missing_table;
    int blocked_table_bounds;
    uint16_t object_word;
    uint16_t offset;
    uint8_t cls2;
    uint8_t result;
} DM2_V1_SkprojectQuery48ae01afReceipt;

int dm2_v1_skproject_query_48ae_01af(
    uint16_t object_word,
    uint16_t offset,
    const int8_t *table1d2660,
    uint16_t table1d2660_size,
    uint8_t *out_value,
    DM2_V1_SkprojectQuery48ae01afReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3927 DM2_query_0cee_2e35 — GDAT creature word 4
   with a zero-to-4 substitution. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint8_t creature_type;
    uint16_t gdat_value;
    uint16_t result;
} DM2_V1_SkprojectQuery0cee2e35Receipt;

int dm2_v1_skproject_query_0cee_2e35(
    uint8_t creature_type,
    DM2_V1_SkprojectGdatCreatureWordFn word_fn,
    void *word_user,
    uint16_t *out_value,
    DM2_V1_SkprojectQuery0cee2e35Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:3938 DM2_QUERY_CREATURE_PICST — narrow receipt
   that decodes the creature/palette inputs and fails closed on the picture
   blit path, which Firestaff does not yet own. */
typedef struct {
    int valid;
    int blocked_missing_record;
    int blocked_picture_query;
    int16_t input_x;
    int16_t input_y;
    uint16_t argw0;
    uint8_t creature_type;
    uint16_t creature_word_e;
    uint8_t palette_state_present;
    uint8_t palette_byte7;
} DM2_V1_SkprojectQueryCreaturePicstReceipt;

int dm2_v1_skproject_query_creature_picst(
    int16_t x,
    int16_t y,
    const uint8_t *creature_record,
    uint16_t creature_record_size,
    const uint8_t *palette_state,
    uint16_t argw0,
    DM2_V1_SkprojectQueryCreaturePicstReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:4259 DM2_query_2fcf_164e — recursive container
   search for a distinctive item type over caller-supplied record access. */
typedef struct {
    int valid;
    int found;
    int blocked_missing_callback;
    int blocked_not_container;
    int blocked_cls2_range;
    int blocked_missing_record;
    int blocked_bad_link;
    uint16_t container_handle;
    uint16_t distinctive_type;
    uint16_t matched_handle;
    uint16_t first_child;
    uint16_t steps;
    uint8_t cls2;
} DM2_V1_SkprojectQuery2fcf164eReceipt;

int dm2_v1_skproject_query_2fcf_164e(
    uint16_t container_handle,
    uint16_t distinctive_type,
    DM2_V1_SkprojectRecordAccessorFn accessor_fn,
    DM2_V1_SkprojectCls2FromRecordFn cls2_fn,
    DM2_V1_SkprojectDistinctiveTypeFn type_fn,
    DM2_V1_SkprojectNextRecordFn next_fn,
    void *user,
    DM2_V1_SkprojectQuery2fcf164eReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:4297 DM2_query_2fcf_16ff — party possession search
   over hero inventories, hand containers, and the wielded object. */
typedef struct {
    uint16_t cur_hp;
    uint16_t inventory[30];
} DM2_V1_SkprojectHeroState;

typedef struct {
    uint16_t hero_count;
    DM2_V1_SkprojectHeroState heroes[4];
    int16_t hand_container_mode; /* ddat.v1d67bc; hand scan only when == 5 */
    uint16_t hand_containers[8];
    uint16_t wielded; /* ddat.savegamewpc.w_00 */
} DM2_V1_SkprojectPartyState;

typedef struct {
    int valid;
    int found;
    int blocked_missing_party;
    int blocked_missing_callback;
    int from_hand_container;
    int from_wielded;
    uint16_t distinctive_type;
    uint16_t matched_handle;
    uint8_t hero_index;
} DM2_V1_SkprojectQuery2fcf16ffReceipt;

int dm2_v1_skproject_query_2fcf_16ff(
    uint16_t distinctive_type,
    const DM2_V1_SkprojectPartyState *party_state,
    DM2_V1_SkprojectRecordAccessorFn accessor_fn,
    DM2_V1_SkprojectCls2FromRecordFn cls2_fn,
    DM2_V1_SkprojectDistinctiveTypeFn type_fn,
    DM2_V1_SkprojectNextRecordFn next_fn,
    void *user,
    DM2_V1_SkprojectQuery2fcf16ffReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:4400 DM2_query_48ae_0767 — inventory weight
   packing over caller-supplied item weights. */
typedef struct {
    int valid;
    int blocked_missing_output;
    int blocked_missing_weights;
    int16_t capacity;
    uint16_t out_count;
    uint16_t item_count;
    uint16_t written;
    int32_t packed_weight;
} DM2_V1_SkprojectQuery48ae0767Receipt;

int dm2_v1_skproject_query_48ae_0767(
    int16_t capacity,
    uint16_t out_count,
    uint8_t *out_indices,
    uint16_t *out_written,
    uint16_t item_count,
    const int16_t *item_weights,
    int32_t *out_total_weight,
    DM2_V1_SkprojectQuery48ae0767Receipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:4765 DM2_query_0cee_06dc — adjacent-tile door/wall
   predicate over caller-supplied tile access and direction tables. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int blocked_missing_table;
    int16_t input_x;
    int16_t input_y;
    uint8_t tile_value;
    uint8_t bit;
    int16_t neighbour_x;
    int16_t neighbour_y;
    uint8_t neighbour_type;
    uint8_t result;
} DM2_V1_SkprojectQuery0cee06dcReceipt;

int dm2_v1_skproject_query_0cee_06dc(
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectTileValueFn tile_fn,
    void *tile_user,
    const int16_t *table1d27fc,
    const int16_t *table1d2804,
    uint8_t *out_result,
    DM2_V1_SkprojectQuery0cee06dcReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp + c_1c9a.cpp cycle-15 query batch — additional
   caller-supplied callback types.  Map switching, ladder/level location,
   level cls2 allowance lists, GDAT entry data indices, tile records, and
   the creature AI flag word stay caller-owned; the helpers fail closed when
   they are unavailable. */
typedef int32_t (*DM2_V1_SkprojectChangeMapFn)(int16_t map, void *user);

typedef int32_t (*DM2_V1_SkprojectFindLadderAroundFn)(
    int16_t x,
    int16_t y,
    int16_t direction,
    void *user);

typedef int32_t (*DM2_V1_SkprojectLocateOtherLevelFn)(
    int16_t map,
    int16_t direction,
    int16_t *x,
    int16_t *y,
    void *user);

typedef uint16_t (*DM2_V1_SkprojectAISpecFlagsFn)(
    uint16_t handle,
    void *user);

typedef const uint8_t *(*DM2_V1_SkprojectLevelCls2ListFn)(
    int16_t level,
    uint16_t *out_count,
    void *user);

typedef uint16_t (*DM2_V1_SkprojectGdatEntryDataIndexFn)(
    uint8_t cls1,
    uint8_t cls2,
    uint8_t entry_index,
    uint8_t data_index,
    void *user);

typedef int32_t (*DM2_V1_SkprojectTileRecordFn)(
    int16_t x,
    int16_t y,
    void *user);

typedef int32_t (*DM2_V1_SkprojectRebirthAltarFn)(
    int32_t record,
    void *user);

typedef int32_t (*DM2_V1_SkprojectDoorGdatFn)(
    uint8_t value,
    void *user);

typedef int (*DM2_V1_SkprojectRandbitFn)(void *user);

typedef int32_t (*DM2_V1_SkprojectWallItemRecordFn)(
    int16_t x,
    int16_t y,
    void *user);

typedef int (*DM2_V1_SkprojectLineCellFn)(
    int16_t x,
    int16_t y,
    void *user);

/* SKULLWIN/c_querydb.cpp:4807 DM2_query_19f0_124b — source-locked stairs/
   pit transition query.  Changes to the requested map, classifies the tile
   at (*x, *y), admits open pits (type 2, flags 0x8, direction 1, tile bit 3
   set, bit 0 clear), ladder targets (flags 0x100 plus
   DM2_FIND_LADDAR_AROUND), directionless falls (flags 0x10, direction -1),
   and stairs (type 3, flags 0x100, direction by tile bit 2), then delegates
   to DM_LOCATE_OTHER_LEVEL.  Directionless falls re-validate the target
   tile on the located map and restore the original map. */
typedef struct {
    int valid;
    int blocked_missing_output;
    int blocked_missing_callback;
    int admitted_pit;
    int admitted_stairs;
    int ladder_found;
    int fallthrough;
    int16_t map;
    int16_t direction;
    uint16_t flags;
    uint8_t tile_value;
    uint8_t tile_type;
    int32_t locate_result;
    uint8_t target_tile_value;
    uint8_t target_tile_type;
    int target_admitted;
    int32_t result;
} DM2_V1_SkprojectQuery19f0124bReceipt;

int dm2_v1_skproject_query_19f0_124b(
    int16_t *x,
    int16_t *y,
    int16_t map,
    int16_t direction,
    uint16_t flags,
    DM2_V1_SkprojectChangeMapFn change_map_fn,
    DM2_V1_SkprojectTileValueFn tile_fn,
    DM2_V1_SkprojectFindLadderAroundFn ladder_fn,
    DM2_V1_SkprojectLocateOtherLevelFn locate_fn,
    void *user,
    int32_t *out_result,
    DM2_V1_SkprojectQuery19f0124bReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:4967 DM2_query_29ee_18eb — source-locked ladder
   transition pair: stores the inputs into the caller state and runs
   DM2_query_19f0_124b downwards (direction -1, flags 0x110) and upwards
   (direction 1, flags 0x108), keeping the resulting map words. */
typedef struct {
    uint16_t v1e0b68; /* down query x (in/out) */
    uint16_t v1e0b6a; /* down query y (in/out) */
    uint16_t v1e0b60; /* down result map word */
    uint16_t v1e0b5e; /* up query x (in/out) */
    uint16_t v1e0b5c; /* up query y (in/out) */
    uint16_t v1e0b66; /* up result map word */
    uint16_t v1e0b6e; /* stored input x */
    uint16_t v1e0b70; /* stored input y */
    uint16_t v1e0b64; /* stored input map */
} DM2_V1_Skproject29ee18ebState;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    int32_t down_result;
    int32_t up_result;
} DM2_V1_SkprojectQuery29ee18ebReceipt;

int dm2_v1_skproject_query_29ee_18eb(
    uint16_t x,
    uint16_t y,
    uint16_t map,
    DM2_V1_Skproject29ee18ebState *state,
    DM2_V1_SkprojectChangeMapFn change_map_fn,
    DM2_V1_SkprojectTileValueFn tile_fn,
    DM2_V1_SkprojectFindLadderAroundFn ladder_fn,
    DM2_V1_SkprojectLocateOtherLevelFn locate_fn,
    void *user,
    DM2_V1_SkprojectQuery29ee18ebReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:5025 DM2_IS_CREATURE_ALLOWED_ON_LEVEL — source
   predicate: creatures whose AI spec flags carry bit 0x40 in the high byte
   are always allowed; otherwise the record cls2 must appear in the
   level's allowance list (ddat.v1e03c8/mapdat.tmpmap pointer chain, which
   the caller resolves). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int blocked_missing_list;
    int ai_flag_override;
    int allowed;
    uint16_t handle;
    int16_t level;
    uint8_t cls2;
    uint16_t ai_flags;
    uint16_t list_count;
    uint16_t checked;
} DM2_V1_SkprojectIsCreatureAllowedOnLevelReceipt;

int dm2_v1_skproject_is_creature_allowed_on_level(
    uint16_t handle,
    int16_t level,
    DM2_V1_SkprojectAISpecFlagsFn ai_flags_fn,
    DM2_V1_SkprojectCls2FromRecordFn cls2_fn,
    DM2_V1_SkprojectLevelCls2ListFn list_fn,
    void *user,
    DM2_V1_SkprojectIsCreatureAllowedOnLevelReceipt *out_receipt);

/* SKULLWIN/c_querydb.cpp:5073 DM2_query_0cee_319e — GDAT entry 9 data
   index 11 query keyed by record cls2; returns 0 when the cls2 is the
   source 0xff none value. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t handle;
    uint8_t cls2;
    uint16_t result;
} DM2_V1_SkprojectQuery0cee319eReceipt;

int dm2_v1_skproject_query_0cee_319e(
    uint16_t handle,
    DM2_V1_SkprojectCls2FromRecordFn cls2_fn,
    DM2_V1_SkprojectGdatEntryDataIndexFn gdat_fn,
    void *user,
    uint16_t *out_value,
    DM2_V1_SkprojectQuery0cee319eReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:23 DM2_1BAAD — source-locked tile passability
   predicate.  Type-0 tiles pass; type-4 doors in variants 3/4 pass unless
   the rebirth-altar GDAT door query is nonzero and DM2_RANDBIT wins; type-6
   tiles with bit 2 clear pass; tiles without bit 0x10 block.  Otherwise the
   wall-tile record chain decides: type-0xf records with (word@2 & 0x7f) ==
   0xe pass, and type-4 creature records pass when the creature located by
   DM2_query_1c9a_03cf is not material/not non-solid per the AI spec flags. */
typedef struct {
    void *user;
    DM2_V1_SkprojectTileValueFn tile_fn;
    DM2_V1_SkprojectTileRecordFn tile_record_fn;
    DM2_V1_SkprojectRebirthAltarFn rebirth_fn;
    DM2_V1_SkprojectDoorGdatFn door_gdat_fn;
    DM2_V1_SkprojectRandbitFn randbit_fn;
    DM2_V1_SkprojectWallItemRecordFn wall_record_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectAISpecFlagsFn ai_flags_fn;
    /* creature branch wiring for the cycle-14 DM2_query_1c9a_03cf helper */
    DM2_V1_SkprojectCreatureAtFn creature_at_fn;
    DM2_V1_SkprojectAISpecFromRecordFn ai_spec_fn;
    DM2_V1_SkprojectCreature5x5PosValueFn pos5x5_fn;
    DM2_V1_SkprojectQuery098d000fFn q098d_fn;
    const int16_t *table1d2752;
    uint16_t table1d2752_size;
    const int16_t (*table1d62b0)[2];
    uint16_t table1d62b0_rows;
    const int16_t (*table1d62d0)[2];
    uint16_t table1d62d0_rows;
    const int16_t *table1d62e0;
    uint16_t table1d62e0_size;
    const int8_t *table1d62e8;
    uint16_t table1d62e8_size;
} DM2_V1_Skproject1baadContext;

typedef struct {
    int valid;
    int blocked_missing_callback;
    int passable;
    int via_door;
    int via_type6;
    int via_actuator;
    int via_creature;
    uint8_t tile_value;
    uint8_t tile_type;
    uint8_t door_variant;
    uint8_t rebirth_value;
    uint16_t door_gdat_value;
    int randbit;
    uint16_t records_checked;
    uint32_t creature_handle;
    uint16_t creature_flags;
} DM2_V1_Skproject1baadReceipt;

int dm2_v1_skproject_1baad(
    int16_t x,
    int16_t y,
    const DM2_V1_Skproject1baadContext *ctx,
    DM2_V1_Skproject1baadReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:152 DM2_1BC29 — cache wrapper around DM2_1BAAD:
   passes immediately when the current map and coordinates match the cached
   transition, otherwise delegates. */
typedef struct {
    uint16_t v1d3248; /* current map */
    uint16_t v1e08d6; /* cached map */
    uint16_t v1e08d8; /* cached x */
    uint16_t v1e08d4; /* cached y */
} DM2_V1_Skproject1bc29Cache;

typedef struct {
    int valid;
    int blocked_missing_cache;
    int cache_hit;
    int passable;
    DM2_V1_Skproject1baadReceipt nested;
} DM2_V1_Skproject1bc29Receipt;

int dm2_v1_skproject_1bc29(
    uint16_t x,
    uint16_t y,
    const DM2_V1_Skproject1bc29Cache *cache,
    const DM2_V1_Skproject1baadContext *ctx,
    DM2_V1_Skproject1bc29Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:163 DM2_19f0_0207 — source-locked line walk.
   Adjacent endpoints return 1 immediately; otherwise the walk steps cells
   from (x2, y2) back toward (x1, y1), choosing each step by comparing the
   fixed-point (<<6) slope error against the initial slope, calling the
   caller callback for every visited cell.  A nonzero callback aborts with
   0; reaching the start returns DM2_CALC_SQUARE_DISTANCE between the
   endpoints.  Diagonal lines step both axes per iteration. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int aborted;
    uint16_t steps;
    int16_t last_x;
    int16_t last_y;
    int32_t result;
} DM2_V1_Skproject19f00207Receipt;

int32_t dm2_v1_skproject_19f0_0207(
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    DM2_V1_SkprojectLineCellFn cell_fn,
    void *user,
    DM2_V1_Skproject19f00207Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:470 DM2_19f0_045a — source-locked tile-state cache
   refresh.  A cache hit (same map word and coordinates) returns the input x
   unchanged; a miss updates the cache, reads the tile, and seeds the
   downstream state words: 0xffff when tile bit 0x10 is set else 0xfffe,
   zeroed flags, -1 marker, and selector 1. */
typedef struct {
    uint16_t v1d3248; /* current map word (input) */
    uint16_t v1e08a8; /* cached x */
    uint16_t v1e08aa; /* cached y */
    uint16_t v1e08ac; /* cached map */
    uint16_t v1e08ae; /* tile value */
    uint16_t v1e08b0;
    uint16_t v1e08b2;
    uint16_t v1e08b4;
    uint8_t v1e08b6;
    uint8_t v1e08b7;
    int32_t v1e08be;
    uint16_t v1e08c4;
} DM2_V1_Skproject19f0045aState;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    int cache_hit;
    uint8_t tile_value;
    int32_t result;
} DM2_V1_Skproject19f0045aReceipt;

int dm2_v1_skproject_19f0_045a(
    uint16_t x,
    uint16_t y,
    DM2_V1_Skproject19f0045aState *state,
    DM2_V1_SkprojectTileValueFn tile_fn,
    void *user,
    DM2_V1_Skproject19f0045aReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp + c_ai.cpp cycle-16 symbol batch — additional
   caller-supplied callback types.  Runtime commands (CREATURE_GO_THERE,
   DM2_19f0_2165, record cut/append), hero/party access, and the creature
   record itself stay caller-owned; the helpers fail closed when they are
   unavailable. */
typedef int (*DM2_V1_SkprojectGoThereFn)(
    uint16_t mode,
    int16_t x,
    int16_t y,
    int16_t dir_x,
    int16_t arg_y,
    uint16_t direction,
    void *user);

typedef int32_t (*DM2_V1_SkprojectHeroAtPosFn)(
    int16_t x,
    int16_t y,
    uint16_t pos,
    void *user); /* -1 when no hero matches */

typedef uint16_t (*DM2_V1_SkprojectHeroItemFn)(
    uint16_t hero,
    uint16_t slot,
    void *user);

typedef uint16_t (*DM2_V1_SkprojectHeroPartyPosFn)(
    uint16_t hero,
    void *user);

typedef int16_t (*DM2_V1_SkprojectPlayerAtPosFn)(
    uint16_t pos,
    void *user); /* -1 when the position is empty */

typedef int32_t (*DM2_V1_SkprojectCanHandleItFn)(
    uint16_t item,
    int16_t handle,
    void *user);

typedef int32_t (*DM2_V1_SkprojectCanHandleItemInFn)(
    uint16_t item_type,
    uint16_t possession,
    uint16_t slot,
    void *user);

typedef int16_t (*DM2_V1_SkprojectAddItemChargeFn)(
    uint16_t item,
    int16_t delta,
    void *user);

typedef int32_t (*DM2_V1_SkprojectTileRecordLinkFn)(
    int16_t x,
    int16_t y,
    void *user);

typedef void (*DM2_V1_SkprojectCmd2165Fn)(
    uint16_t mode,
    int16_t x,
    int16_t y,
    int16_t target_x,
    int16_t target_y,
    int16_t arg5,
    int16_t arg6,
    void *user);

typedef int32_t (*DM2_V1_SkprojectFindTileActuatorFn)(
    int16_t x,
    int16_t y,
    uint8_t cls,
    uint8_t type,
    void *user); /* -1 when absent */

typedef void (*DM2_V1_SkprojectCutAppendRecordFn)(
    uint16_t record,
    int16_t x,
    int16_t y,
    void *user);

typedef int32_t (*DM2_V1_SkprojectCmd06bdFn)(
    uint16_t creature,
    int16_t type,
    uint16_t direction,
    void *user); /* 0 when the source would return NULL */

typedef uint16_t (*DM2_V1_SkprojectTimerDirFn)(
    uint16_t timer_index,
    void *user); /* (timdat.timerarray[i].getB() << 4) >> 14 */

typedef void (*DM2_V1_SkprojectOverseeRecordFn)(
    uint16_t record,
    uint8_t mode,
    int16_t *state_words,
    void *user);

/* SKULLWIN/c_1c9a.cpp:503 DM2_19f0_04bf — source-locked cached tile-record
   chain walk.  Returns ddat.v1e08b2 when already resolved; otherwise seeds
   ddat.v1e08b0 from DM2_GET_TILE_RECORD_LINK when needed and walks forward
   while the record type ((handle & 0x3c00) >> 10) is at most 3, caching the
   first record beyond that range. */
typedef struct {
    uint16_t v1e08a8; /* cached tile x */
    uint16_t v1e08aa; /* cached tile y */
    uint16_t v1e08b0; /* chain head */
    uint16_t v1e08b2; /* cached result */
} DM2_V1_Skproject19f004bfState;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    uint16_t chain_head;
    uint16_t records_walked;
    uint16_t result;
} DM2_V1_Skproject19f004bfReceipt;

int dm2_v1_skproject_19f0_04bf(
    DM2_V1_Skproject19f004bfState *state,
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn,
    DM2_V1_SkprojectNextRecordFn next_fn,
    void *user,
    DM2_V1_Skproject19f004bfReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:542 DM2_19f0_050f — source-locked cached creature
   record lookup: returns ddat.v1e08b4 when resolved, otherwise walks from
   DM2_19f0_04bf while the record type is not 4 and caches the first type-4
   record. */
typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    uint16_t records_walked;
    uint16_t result;
} DM2_V1_Skproject19f0050fReceipt;

int dm2_v1_skproject_19f0_050f(
    uint16_t *v1e08b4,
    DM2_V1_Skproject19f004bfState *state04bf,
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn,
    DM2_V1_SkprojectNextRecordFn next_fn,
    void *user,
    DM2_V1_Skproject19f0050fReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:576 DM2_19f0_0547 — source one-liner delegating to
   DM2_CREATURE_CAN_HANDLE_IT(item, handle). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t item;
    int16_t handle;
    int32_t result;
} DM2_V1_Skproject19f00547Receipt;

int dm2_v1_skproject_19f0_0547(
    uint16_t item,
    int16_t handle,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    void *user,
    DM2_V1_Skproject19f00547Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:584 DM2_19f0_0559 — source-locked creature turn
   decision.  The facing is ((word@0xe << 6) >> 14) of the creature record.
   Facing the requested direction clears byte 0x1a and returns 0; otherwise
   the turn is +1/-1 by the shorter arc (random on a 180 turn), byte 0x1d
   becomes (facing + turn) & 3, byte 0x1a becomes 7 for a right turn and 6
   for a left turn, and the result word is set to -4. */
typedef struct {
    uint8_t b1a; /* creature byte 0x1a (output) */
    uint8_t b1d; /* creature byte 0x1d (output) */
    int16_t v1e056f; /* result word (output) */
} DM2_V1_Skproject19f00559State;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    int already_facing;
    int16_t direction;
    uint8_t facing;
    int16_t turn;
} DM2_V1_Skproject19f00559Receipt;

int dm2_v1_skproject_19f0_0559(
    int16_t direction,
    uint16_t creature_word_e,
    DM2_V1_SkprojectRandomData *randdat,
    DM2_V1_Skproject19f00559State *state,
    DM2_V1_Skproject19f00559Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:933 DM2_1c9a_0598 — source popcount over the low 32
   bits, bounded at 32 iterations. */
typedef struct {
    int valid;
    uint32_t value;
    uint32_t count;
} DM2_V1_Skproject1c9a0598Receipt;

uint32_t dm2_v1_skproject_1c9a_0598(
    uint32_t value,
    DM2_V1_Skproject1c9a0598Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:960 DM2_19f0_0891 — source-locked creature move
   decision.  Caller-owned shadow of the creature record bytes the source
   commits on success. */
typedef struct {
    uint16_t w18; /* packed x | y << 5 | map << 10 */
    uint8_t b1a;  /* action */
    uint8_t b1b;  /* direction toward target */
    uint8_t b1c;  /* secondary position / hero partypos */
    uint8_t b1d;
    uint8_t b1e;  /* action parameter */
    uint8_t b20;  /* mode */
} DM2_V1_SkprojectCreatureShadow;

typedef struct {
    /* scalar runtime inputs */
    uint16_t v1e0578;       /* movement/AI flags */
    uint16_t sight_range;   /* (word@0x14 of v1e0552) >> 12 */
    uint16_t current_map;   /* ddat.v1d3248 */
    uint16_t transition_map;/* ddat.v1e08d6 */
    uint16_t transition_x;  /* ddat.v1e08d8 */
    uint16_t transition_y;  /* ddat.v1e08d4 */
    uint16_t v1e0258;       /* transition direction seed */
    uint8_t v1e0584_flags;  /* table1d607e[v1e0584].uc[1] */
    uint8_t v1e0552_flags;  /* byte@0 of v1e0552 */
    int16_t creature_x;     /* s350.v1e0562.getxA */
    int16_t creature_y;     /* s350.v1e0562.getyA */
    uint8_t creature_v1e0571;
    uint16_t creature_word_e;
    int16_t map_width;
    int16_t map_height;
    DM2_V1_SkprojectRandomData *randdat;
    void *user;
    /* command / query callbacks */
    DM2_V1_SkprojectGoThereFn go_there_fn;
    DM2_V1_SkprojectHeroAtPosFn hero_at_fn;
    DM2_V1_SkprojectHeroItemFn hero_item_fn;
    DM2_V1_SkprojectHeroPartyPosFn hero_pos_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_SkprojectPlayerAtPosFn player_at_fn;
    const DM2_V1_Skproject1baadContext *ctx1baad;
    DM2_V1_Skproject19f0045aState *state045a;
    DM2_V1_Skproject19f004bfState *state04bf;
    uint16_t *v1e08b4;      /* ddat.v1e08b4 cache word (in/out) */
    DM2_V1_SkprojectTileValueFn tile_fn;
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_Skproject19f00559State *state0559;
    DM2_V1_SkprojectCreatureShadow *creature;
    int16_t *v1e056f;
} DM2_V1_Skproject0891Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int blocked_missing_callback;
    int rejected;
    uint8_t mode;
    int16_t target_x;
    int16_t target_y;
    int16_t cell_x;
    int16_t cell_y;
    int16_t distance;
    int16_t direction;
    int hero_target;
    int go_there_ok;
    int line_of_sight_ok;
    int committed;
    int16_t result_word;
} DM2_V1_Skproject19f00891Receipt;

int dm2_v1_skproject_19f0_0891(
    uint16_t mode,
    int16_t target_x,
    int16_t target_y,
    int16_t dir_x,
    int16_t arg_y,
    int16_t direction,
    const DM2_V1_Skproject0891Context *ctx,
    DM2_V1_Skproject19f00891Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:648 DM2_19f0_05e8 — source-locked creature target
   scan.  Steps from the start cell along the direction (all four when -1),
   looking for a creature the target can handle or a matching item actuator,
   then walks back over the visibility grid and delegates the final move to
   DM2_19f0_0891.  The visibility grid uses a 128-byte row stride with four
   bytes per cell. */
typedef struct {
    int16_t creature_x;
    int16_t creature_y;
    uint16_t v1e0578;
    uint16_t sight_range;
    uint16_t current_map;
    int16_t map_width;
    int16_t map_height;
    const uint8_t *vis_grid;
    void *user;
    DM2_V1_SkprojectTileValueFn tile_fn;
    DM2_V1_SkprojectCreatureAtFn creature_at_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn;
    DM2_V1_SkprojectCls2FromRecordFn cls2_fn;
    DM2_V1_SkprojectGdatEntryDataIndexFn gdat_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    const DM2_V1_Skproject1bc29Cache *cache1bc29;
    const DM2_V1_Skproject1baadContext *ctx1baad;
    const DM2_V1_Skproject0891Context *ctx0891;
    DM2_V1_Skproject19f0045aState *state045a;
} DM2_V1_Skproject05e8Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int blocked_missing_callback;
    int found;
    int rejected;
    uint8_t found_via; /* 1 = creature, 2 = item actuator */
    int16_t found_x;
    int16_t found_y;
    int16_t direction;
    uint16_t range;
    uint16_t steps;
    uint32_t creature_handle;
    uint16_t packed_target;
    int32_t result;
} DM2_V1_Skproject19f005e8Receipt;

int dm2_v1_skproject_19f0_05e8(
    uint16_t target_type,
    uint16_t *out_packed,
    int16_t start_x,
    int16_t start_y,
    int16_t direction,
    int item_search,
    const DM2_V1_Skproject05e8Context *ctx,
    DM2_V1_Skproject19f005e8Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:1663 DM2_19f0_0d10 — source-locked door-target move
   decision.  Requires the destination to be a door tile, walks the record
   chain for door state, and either delegates to DM2_19f0_0891 (mode 0x84)
   or commits the move into the caller-owned creature shadow. */
typedef struct {
    /* scalar runtime inputs */
    uint16_t v1e057a;       /* door-move capability mask */
    uint16_t v1e0578;       /* movement/AI flags */
    uint16_t sight_range;   /* (word@0x14 of v1e0552) >> 12 */
    uint16_t current_map;   /* ddat.v1d3248 */
    uint16_t *v1e08b0;      /* door record cache (in/out) */
    uint16_t v1e08ae;       /* cached door tile value */
    uint16_t creature_word_e;/* s350.v1e054e->w_0e (for the 0559 turn) */
    int16_t map_width;
    int16_t map_height;
    DM2_V1_SkprojectRandomData *randdat;
    void *user;
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectWallItemRecordFn wall_record_fn;
    DM2_V1_SkprojectTimerDirFn timer_dir_fn; /* timdat timer direction */
    const DM2_V1_Skproject1baadContext *ctx1baad;
    DM2_V1_Skproject19f0045aState *state045a;
    const DM2_V1_Skproject0891Context *ctx0891;
    DM2_V1_Skproject19f00559State *state0559;
    DM2_V1_SkprojectCreatureShadow *creature;
    int16_t *v1e056f;
} DM2_V1_Skproject0d10Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int blocked_missing_callback;
    int rejected;
    uint8_t mode;
    uint8_t door_variant;
    uint8_t door_flags; /* door record byte 3 */
    uint8_t outcome;    /* 0 = rejected, 1 = vw_04 1, 2 = vw_04 2, 3 = 0891 delegate, 4 = commit */
    int16_t cell_x;
    int16_t cell_y;
    int16_t distance;
    int16_t direction;
    uint16_t capability; /* vo_14 after gating */
    int requested_door_flag_10;
    int32_t delegate_result;
    int16_t result_word;
} DM2_V1_Skproject19f00d10Receipt;

int dm2_v1_skproject_19f0_0d10(
    uint16_t mode,
    int16_t from_x,
    int16_t from_y,
    int16_t dir_x,
    int16_t arg_y,
    int16_t direction,
    const DM2_V1_Skproject0d10Context *ctx,
    DM2_V1_Skproject19f00d10Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp cycle-16 batch — shared caller-owned creature AI state
   derived from the s350/ddat runtime words. */
typedef struct {
    int16_t creature_x;      /* s350.v1e0562.getxA */
    int16_t creature_y;      /* s350.v1e0562.getyA */
    uint16_t creature_word_e;/* s350.v1e054e->w_0e */
    uint16_t possession;     /* s350.v1e054e->possession.w_00 */
    uint8_t creature_type;   /* byte@0x4 of the creature record */
    uint16_t creature_word8; /* word@0x8 of the creature record */
    int16_t target_x;        /* s350.creatures->w_18 GetX */
    int16_t target_y;        /* s350.creatures->w_18 GetY */
    int16_t v1e0572;
    uint16_t v1e0574;
    uint16_t v1e057c;
    uint16_t v1e0576;        /* movement flags (XACT_80) */
    uint16_t v1e0578;        /* movement/AI flags */
    uint16_t v1e07d8_w04;
    uint16_t v1e07d8_w06;
    uint16_t creature_word_a; /* s350.v1e054e->w_0a */
    uint16_t v1e054c;         /* creature record link */
    int16_t v1e056f;         /* result word the source returns */
    uint16_t creature_w0e;   /* in/out shadow (XACT_62) */
    uint8_t creature_b1a;    /* in/out shadow (XACT_62) */
    void *user;
    DM2_V1_SkprojectGoThereFn go_there_fn;
    DM2_V1_SkprojectCanHandleItemInFn can_handle_item_in_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_SkprojectAddItemChargeFn add_charge_fn;
    DM2_V1_SkprojectDistinctiveTypeFn type_fn;
    DM2_V1_SkprojectCmd2165Fn cmd2165_fn;
    DM2_V1_SkprojectCreatureAtFn creature_at_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectFindTileActuatorFn find_actuator_fn;
    DM2_V1_SkprojectWallItemRecordFn wall_record_fn;
    DM2_V1_SkprojectCutAppendRecordFn cut_record_fn;
    DM2_V1_SkprojectCutAppendRecordFn append_record_fn;
    DM2_V1_SkprojectCmd06bdFn cmd06bd_fn;
    DM2_V1_SkprojectRandomData *randdat;
    DM2_V1_Skproject19f00559State *state0559;
} DM2_V1_SkprojectXactContext;

/* SKULLWIN/c_ai.cpp:21 DM2_14cd_2807 — source-locked oversee-record item
   callback: admits items the creature can handle, clears the accumulator on
   the first admitted item, and adds the DM2_query_48ae_05ae damage value
   (charge -1 when the state word 8 is zero). */
typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    int admitted;
    int blocked_gdat_path;
    uint16_t item_handle;
    uint16_t distinctive_type;
    int16_t charge;
    int32_t damage;
    int16_t accumulated;
} DM2_V1_Skproject14cd2807Receipt;

int dm2_v1_skproject_14cd_2807(
    uint16_t item_handle,
    int16_t *state_words,
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_Skproject14cd2807Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:56 DM2_14cd_2886 — source-locked oversee driver: builds
   the five-word state array {0xffff, w1, w2, w3, w4} and delegates the
   DM2_OVERSEE_RECORD iteration to the caller, returning state word 0. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t record;
    int16_t state_words[5];
    int16_t result;
} DM2_V1_Skproject14cd2886Receipt;

int16_t dm2_v1_skproject_14cd_2886(
    uint16_t record,
    uint16_t w1,
    uint16_t w2,
    uint16_t w3,
    uint16_t w4,
    DM2_V1_SkprojectOverseeRecordFn oversee_fn,
    void *user,
    DM2_V1_Skproject14cd2886Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:78 DM2_PROCEED_XACT_56 — one-step random-direction move:
   returns -4 when DM2_CREATURE_GO_THERE accepts, -2 otherwise. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int go_there_ok;
    uint16_t facing;
    int8_t result;
} DM2_V1_SkprojectXact56Receipt;

int dm2_v1_skproject_proceed_xact_56(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact56Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:86 DM2_PROCEED_XACT_57 — random-turn move: tries the
   random side step first, then the opposite arc, and falls back to the
   DM2_19f0_0559 turn when both fail. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int16_t turn;
    uint16_t facing;
    uint16_t first_direction;
    uint16_t second_direction;
    int first_ok;
    int second_ok;
    int turned;
} DM2_V1_SkprojectXact57Receipt;

int dm2_v1_skproject_proceed_xact_57(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact57Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:106 DM2_PROCEED_XACT_59_76 — possessed-item throw/use:
   returns -2 when the item cannot be handled, otherwise issues the
   DM2_19f0_2165 command and returns the v1e056f result word. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int rejected;
    uint16_t item_type;
    int command_issued;
    int8_t result;
} DM2_V1_SkprojectXact5976Receipt;

int dm2_v1_skproject_proceed_xact_59_76(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact5976Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:119 DM2_PROCEED_XACT_62 — fountain/item sorting
   behaviour.  Returns -2 when the creature already handles the item, -3
   when the behaviour is disabled, otherwise sorts matching records toward
   the chain head and issues the DM2_19f0_2165 command, returning v1e056f. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t target_x;
    uint16_t target_y;
    uint16_t wanted_type;
    int actuator_found;
    int records_moved;
    int command_issued;
    int alternate_path;
    int8_t result;
} DM2_V1_SkprojectXact62Receipt;

int dm2_v1_skproject_proceed_xact_62(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact62Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:295 DM2_PROCEED_XACT_63 — pass-item-to-creature check:
   returns -2 when the creature ahead can handle the item, -3 otherwise. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t item_type;
    uint8_t slot;
    uint32_t creature_handle;
    int8_t result;
} DM2_V1_SkprojectXact63Receipt;

int dm2_v1_skproject_proceed_xact_63(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact63Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:333 DM2_PROCEED_XACT_64 — throw possessed item forward:
   returns v1e056f after issuing the DM2_19f0_2165 command, -3 when the
   behaviour does not apply. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t item_type;
    uint16_t facing;
    int command_issued;
    int8_t result;
} DM2_V1_SkprojectXact64Receipt;

int dm2_v1_skproject_proceed_xact_64(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact64Receipt *out_receipt);

/* SKULLWIN/c_0aaf.cpp + c_1c9a.cpp cycle-16 batch-17 — additional
   caller-supplied callback types.  GDAT text/image access, teleporter
   details, dballoc, and the creature runtime commands stay caller-owned;
   the helpers fail closed when they are unavailable. */
typedef int (*DM2_V1_SkprojectGdatTextFn)(
    uint8_t cls1,
    uint8_t cls2,
    uint8_t index,
    char *out_text,
    void *user); /* nonzero when the entry exists */

typedef const uint8_t *(*DM2_V1_SkprojectGdatDataPtrFn)(
    uint8_t cls1,
    uint8_t cls2,
    uint8_t entry,
    uint8_t data,
    void *user);

typedef int (*DM2_V1_SkprojectTeleporterDetailFn)(
    DM2_V1_SkprojectTeleporterDetail *out_detail,
    int16_t x,
    int16_t y,
    void *user);

typedef int32_t (*DM2_V1_SkprojectMove075f06bdFn)(
    const uint8_t *record,
    uint16_t word2,
    void *user);

typedef int (*DM2_V1_SkprojectAllocation11Fn)(
    uint32_t key,
    uint16_t *out_index,
    void *user);

typedef void (*DM2_V1_SkprojectDballocFreeFn)(
    uint16_t index,
    void *user);

typedef int (*DM2_V1_SkprojectIsChestFn)(
    uint16_t handle,
    void *user);

/* SKULLWIN/c_0aaf.cpp:22 DM2_0aaf_0067 — source-locked GDAT 0x1a text-list
   builder for the source menu/dialog path.  The helper builds the entry
   list (each entry: low byte from DM2_QUERY_GDAT_ENTRY_DATA_INDEX(0x1a, id,
   0xb, index) with the source zero-to-index substitution, high byte marking
   the terminator entry), records the ddat.v1e0204 update, and fails closed
   on the UI event loop Firestaff does not own. */
typedef struct {
    uint8_t count;      /* entries built (ddat.v1e0204) */
    int16_t last_index; /* vw_54: entry marked by a nonzero high byte */
    uint8_t v1e0204;    /* value written to ddat.v1e0204 */
    uint8_t low[20];    /* low bytes per entry */
    uint8_t high[20];   /* high bytes per entry */
    uint16_t value;     /* vl_50 of the terminator entry */
} DM2_V1_Skproject0aaf0067List;

typedef struct {
    int valid;
    int blocked_missing_callback;
    int blocked_missing_output;
    int blocked_ui_loop;
    uint8_t mode;
    uint8_t texts_scanned;
} DM2_V1_Skproject0aaf0067Receipt;

int dm2_v1_skproject_0aaf_0067(
    uint8_t mode,
    DM2_V1_SkprojectGdatTextFn text_fn,
    DM2_V1_SkprojectGdatEntryDataIndexFn gdat_fn,
    void *user,
    DM2_V1_Skproject0aaf0067List *out_list,
    DM2_V1_Skproject0aaf0067Receipt *out_receipt);

/* SKULLWIN/c_0aaf.cpp:174 DM2_0aaf_01db — source-locked dialogue background
   route.  Without the v1e0a88 image path the source fills the expanded rect
   with a palette colour (E_COL01/E_COL00 by the flag); with the image path
   it centers the GDAT image entry in dm2rect4, offsets by the event rect,
   and draws with the image-local or GDAT palette.  The helper records the
   decoded route and fails closed on the draw path. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int blocked_draw_path;
    int route_fill;
    int route_draw;
    int skipped;
    uint16_t image_width;
    uint16_t image_height;
    int16_t rect_x;
    int16_t rect_y;
    uint8_t palette_is_local;
} DM2_V1_Skproject0aaf01dbReceipt;

int dm2_v1_skproject_0aaf_01db(
    uint16_t rect_id,
    int fill_flag,
    uint8_t v1e0a88,
    uint8_t v1e0206,
    uint8_t v1e0207,
    uint8_t v1e0208,
    int16_t event_rect_x,
    int16_t event_rect_y,
    int16_t host_x,
    int16_t host_y,
    int16_t host_w,
    int16_t host_h,
    DM2_V1_SkprojectGdatDataPtrFn gdat_fn,
    void *user,
    DM2_V1_Skproject0aaf01dbReceipt *out_receipt);

/* SKULLWIN/c_0aaf.cpp:251 DM2_0aaf_02f8 — source-locked narrow receipt for
   the recursive master dialog.  The helper decodes the mode/flag gates
   (mode 0xe/0x87 with a zero flag skips the fade, mode 7/0x13 remaps to the
   0x59 text when loadable, mode-0x0e recursion through DM2_0aaf_0067) and
   fails closed on the dialog UI path Firestaff does not own. */
typedef struct {
    int valid;
    int blocked_dialog_path;
    uint8_t mode;
    uint8_t flag;
    int skip_fade;
    int remap_59;
    int recursion_requested;
    uint8_t recursion_mode;
} DM2_V1_Skproject0aaf02f8Receipt;

int dm2_v1_skproject_0aaf_02f8(
    uint8_t mode,
    uint8_t flag,
    uint8_t dialog2_active,
    DM2_V1_SkprojectGdatEntryDataIndexFn gdat_fn,
    void *user,
    DM2_V1_Skproject0aaf02f8Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:2259 DM2_19f0_13aa — source-locked teleporter-side
   scan.  For each of the four directions the source steps up to three cells
   looking for a type-0xe record whose timer direction opposes the scan
   direction, calling DM2_move_075f_06bd on its word@2.  The scan is gated
   per direction by the AI flags, the creature's own position/facing, or a
   RAND&7 roll. */
typedef struct {
    uint8_t v1e0584_flags;
    uint16_t creature_word_a;
    uint8_t v1e0552_flags;
    int16_t creature_x;
    int16_t creature_y;
    uint16_t creature_word_e;
    int16_t map_width;
    int16_t map_height;
    DM2_V1_SkprojectRandomData *randdat;
    void *user;
    DM2_V1_SkprojectWallItemRecordFn wall_record_fn;
    DM2_V1_SkprojectTimerDirFn timer_dir_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectMove075f06bdFn move075f_fn;
    const DM2_V1_Skproject1baadContext *ctx1baad;
} DM2_V1_Skproject19f013aaContext;

typedef struct {
    int valid;
    int blocked_missing_context;
    int found;
    uint8_t direction;
    uint8_t found_step;
    uint16_t found_handle;
    uint16_t found_word2;
} DM2_V1_Skproject19f013aaReceipt;

int dm2_v1_skproject_19f0_13aa(
    int16_t x,
    int16_t y,
    const DM2_V1_Skproject19f013aaContext *ctx,
    DM2_V1_Skproject19f013aaReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:2430 DM2_19f0_1511 — source one-liner delegating to
   DM2_CREATURE_CAN_HANDLE_IT(item, 9). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t item;
    int32_t result;
} DM2_V1_Skproject19f01511Receipt;

int dm2_v1_skproject_19f0_1511(
    uint16_t item,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    void *user,
    DM2_V1_Skproject19f01511Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:2438 DM2_D283 — source-locked teleporter detail
   probe.  Requires a type-5 tile with bit 3 set, a teleporter detail from
   one of the four adjacent cells, a matching destination map byte, and a
   destination square distance of exactly 1.  Returns the tile record handle
   or -1. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int found;
    uint8_t tile_value;
    uint8_t probe_side;
    uint8_t detail_b04;
    uint16_t record_word2;
    uint16_t record_word4;
    int16_t distance;
    int32_t result;
} DM2_V1_SkprojectD283Receipt;

int dm2_v1_skproject_d283(
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectTileValueFn tile_fn,
    DM2_V1_SkprojectTeleporterDetailFn detail_fn,
    DM2_V1_SkprojectTileRecordFn tile_record_fn,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    void *user,
    DM2_V1_SkprojectD283Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:2514 DM2_CREATURE_GO_THERE — source-locked narrow
   receipt for the 32-mode creature move dispatcher.  The helper decodes the
   mode word (low 5 bits selector, bit 0x20/0x40/0x80 gates), applies the
   table1d6290 capability gate (caller-supplied table) and the v1e0576 gate,
   resolves the step target for modes below 4, and fails closed on the
   runtime dispatch Firestaff does not own. */
typedef struct {
    int valid;
    int blocked_missing_table;
    int blocked_runtime_dispatch;
    uint8_t mode;
    uint8_t capability;
    uint8_t gate_open;
    uint8_t table_entry;
    int16_t cell_x;
    int16_t cell_y;
    int16_t direction;
    int at_target;
} DM2_V1_SkprojectCreatureGoThereReceipt;

int dm2_v1_skproject_creature_go_there(
    uint16_t mode,
    int16_t x,
    int16_t y,
    int16_t dir_x,
    int16_t arg_y,
    int16_t direction,
    const int8_t *table1d6290,
    uint16_t table1d6290_size,
    uint16_t v1e0576,
    DM2_V1_SkprojectCreatureGoThereReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:3986 DM2_19f0_2024 — source-locked chest/creature
   item scan.  With the 0x10 flag and a chest it scans the chest contents;
   with a type-4 record and the 0x28 mask it scans the creature possession
   chain using the AI spec word@0 bit and the DM2_query_48ae_01af GDAT byte
   as the side mask.  Returns the accumulated side value or -1. */
typedef struct {
    uint16_t v1e057c;
    const int8_t *table1d2660; /* DM2_query_48ae_01af table */
    uint16_t table1d2660_size;
    void *user;
    DM2_V1_SkprojectIsChestFn is_chest_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_SkprojectAISpecFromRecordFn ai_spec_fn;
    DM2_V1_SkprojectAISpecFlagsFn ai_flags_fn;
} DM2_V1_Skproject19f02024Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int is_chest_scan;
    uint8_t side_mask;
    uint16_t start_handle;
    uint16_t records_walked;
    int32_t result;
} DM2_V1_Skproject19f02024Receipt;

int dm2_v1_skproject_19f0_2024(
    uint16_t handle,
    int16_t arg_item,
    int16_t arg_dir,
    const DM2_V1_Skproject19f02024Context *ctx,
    DM2_V1_Skproject19f02024Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:4123 DM2_19f0_2165 — source-locked creature action
   dispatcher.  Caller-owned ddat transition/action state words. */
typedef struct {
    uint16_t v1d3248;
    uint16_t v1e08d6;
    uint16_t v1e08d8;
    uint16_t v1e08d4;
    uint16_t v1e08ae;
    uint16_t v1e08a8;
    uint16_t v1e08aa;
    uint16_t v1e08ac;
    int16_t v1e08be;
    uint8_t v1e08bf;
    uint8_t v1e08c0[4];
    uint16_t v1e08b0;
    uint16_t v1e08b2;
    uint16_t v1e057c;
    uint8_t v1e0584_flags;
    uint16_t creature_word_e;
    uint8_t creature_b1d;
} DM2_V1_Skproject19f02165State;

typedef struct {
    DM2_V1_Skproject19f02165State *state;
    DM2_V1_Skproject19f0045aState *state045a;
    DM2_V1_Skproject19f004bfState *state04bf;
    uint16_t *v1e08b4;
    const int8_t *table1d2660;
    uint16_t table1d2660_size;
    const int8_t *table1d6299;
    uint16_t table1d6299_size;
    DM2_V1_SkprojectRandomData *randdat;
    void *user;
    DM2_V1_SkprojectTileValueFn tile_fn;
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_SkprojectIsChestFn is_chest_fn;
    DM2_V1_SkprojectAISpecFromRecordFn ai_spec_fn;
    DM2_V1_SkprojectAISpecFlagsFn ai_flags_fn;
    DM2_V1_Skproject19f00559State *state0559;
    DM2_V1_SkprojectCreatureShadow *creature;
    int16_t *v1e056f;
} DM2_V1_Skproject19f02165Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int rejected;
    uint8_t mode;
    uint8_t at_target;
    uint8_t action;      /* vw_10 low byte */
    uint8_t secondary;   /* vo_08 low byte */
    int16_t direction;
    int committed;
    int16_t result_word;
} DM2_V1_Skproject19f02165Receipt;

int dm2_v1_skproject_19f0_2165(
    uint16_t mode,
    int16_t from_x,
    int16_t from_y,
    int16_t dir_x,
    int16_t arg_y,
    int16_t argw1,
    int16_t argw2,
    const DM2_V1_Skproject19f02165Context *ctx,
    DM2_V1_Skproject19f02165Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:4640 DM2_19f0_266c — source-locked chain walk for a
   side-matching actuator record.  Records type-3 records on the requested
   side with a nonzero non-0x26 word@2&0x7f, returns early when a 0x1a
   record's flag and handle checks pass, and otherwise returns the last
   recorded handle or 0xffff. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t records_walked;
    uint16_t result;
} DM2_V1_Skproject19f0266cReceipt;

int dm2_v1_skproject_19f0_266c(
    uint16_t handle,
    uint16_t side,
    uint16_t arg_type,
    int16_t arg_item,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectNextRecordFn next_fn,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    void *user,
    DM2_V1_Skproject19f0266cReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:4720 DM2_19f0_2723 — source-locked item admission
   predicate by record word@2 & 0x7f over the source class table. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint8_t record_class;
    int32_t result;
} DM2_V1_Skproject19f02723Receipt;

int dm2_v1_skproject_19f0_2723(
    uint16_t handle,
    int16_t arg1,
    int16_t arg2,
    int16_t arg3,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    void *user,
    DM2_V1_Skproject19f02723Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:4840 DM2_19f0_2813 — source-locked door interaction
   decision.  Requires the v1e057e bit, a row/column-aligned door tile with
   bit 0x10, a 0x26 record on the opposing side matching the creature type,
   and the 266c/2723 admission chain; commits the move into the creature
   shadow when the commit bit is set. */
typedef struct {
    uint16_t v1e057e;
    uint16_t current_map;
    uint16_t v1e08ae;
    uint16_t v1e08a8;
    uint16_t v1e08aa;
    uint16_t *v1e08b0;
    uint8_t creature_type;
    uint16_t creature_word_e;
    int16_t map_width;
    int16_t map_height;
    DM2_V1_SkprojectRandomData *randdat;
    void *user;
    DM2_V1_SkprojectTileRecordLinkFn tile_link_fn;
    DM2_V1_SkprojectRecordAccessorFn record_fn;
    DM2_V1_SkprojectNextRecordFn next_fn;
    DM2_V1_SkprojectCanHandleItFn can_handle_fn;
    DM2_V1_Skproject19f0045aState *state045a;
    DM2_V1_Skproject19f00559State *state0559;
    DM2_V1_SkprojectCreatureShadow *creature;
    int16_t *v1e056f;
} DM2_V1_Skproject19f02813Context;

typedef struct {
    int valid;
    int blocked_missing_context;
    int rejected;
    uint8_t mode;
    int16_t cell_x;
    int16_t cell_y;
    int16_t direction;
    uint16_t door_record;
    uint16_t admitted_handle;
    int committed;
    int16_t result_word;
} DM2_V1_Skproject19f02813Receipt;

int dm2_v1_skproject_19f0_2813(
    uint16_t mode,
    int16_t from_x,
    int16_t from_y,
    int16_t dir_x,
    int16_t arg_y,
    int16_t argw1,
    int16_t argw2,
    const DM2_V1_Skproject19f02813Context *ctx,
    DM2_V1_Skproject19f02813Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5083 DM2_4DEA — source-locked GDAT (0xf, cls, 0x7,
   0xfc) four-byte fetch at index ((DM2_query_4E26(timer) + offset) &
   0xffff). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint8_t cls;
    uint16_t index;
    uint32_t value;
} DM2_V1_Skproject4deaReceipt;

int dm2_v1_skproject_4dea(
    uint8_t cls,
    uint16_t offset,
    uint16_t *timer_word,
    uint32_t game_tick,
    DM2_V1_SkprojectGdatDataPtrFn gdat_fn,
    void *user,
    uint32_t *out_value,
    DM2_V1_Skproject4deaReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5089 DM2_1BA1B — source-locked door/portal
   passability predicate.  Type-4 doors pass only for variant 4 with a zero
   door-graphics query; types 0 and 7 pass; type 6 passes with bit 2 clear;
   everything else blocks. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int passable;
    uint8_t tile_value;
    uint8_t tile_type;
    uint8_t door_variant;
    uint8_t rebirth_value;
    int32_t door_gfx_value;
} DM2_V1_Skproject1ba1bReceipt;

int dm2_v1_skproject_1ba1b(
    int16_t x,
    int16_t y,
    DM2_V1_SkprojectTileValueFn tile_fn,
    DM2_V1_SkprojectTileRecordFn tile_record_fn,
    DM2_V1_SkprojectRebirthAltarFn rebirth_fn,
    DM2_V1_SkprojectDoorGdatFn door_gfx_fn,
    void *user,
    DM2_V1_Skproject1ba1bReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5134 DM2_1c9a_0247 — source-locked dballoc flush:
   runs DM2_ALLOCATION11 with the (map & 0x300) | (v1e054c & 0x300) key
   under the 0x20000000 and 0x30000000 selectors and frees the returned
   indices. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t key_low;
    uint16_t freed_2;
    uint16_t freed_3;
} DM2_V1_Skproject1c9a0247Receipt;

int dm2_v1_skproject_1c9a_0247(
    uint16_t map_word,
    uint16_t v1e054c,
    DM2_V1_SkprojectAllocation11Fn alloc_fn,
    DM2_V1_SkprojectDballocFreeFn free_fn,
    void *user,
    DM2_V1_Skproject1c9a0247Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5161 DM2_1c9a_0648 — source-locked transition cache
   refresh on a map change.  When the new map differs from v1e027c the
   source copies the v1e0258/v1e0270/v1e0272/v1e0266 transition words,
   otherwise the party absolute direction and v1e0260/v1e0262 words. */
typedef struct {
    uint16_t v1d3248;
    uint16_t v1e027c;
    uint16_t v1e0258;
    uint16_t v1e0270;
    uint16_t v1e0272;
    uint16_t v1e0266;
    uint16_t v1e0260;
    uint16_t v1e0262;
    uint16_t party_absdir;
    uint16_t v1e08da;
    uint16_t v1e08d8;
    uint16_t v1e08d4;
    uint16_t v1e08d6;
} DM2_V1_Skproject1c9a0648State;

typedef struct {
    int valid;
    int blocked_missing_state;
    int blocked_missing_callback;
    int map_changed;
    int from_party;
    uint16_t result;
} DM2_V1_Skproject1c9a0648Receipt;

int dm2_v1_skproject_1c9a_0648(
    uint16_t map,
    DM2_V1_Skproject1c9a0648State *state,
    DM2_V1_SkprojectChangeMapFn change_map_fn,
    void *user,
    DM2_V1_Skproject1c9a0648Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp cycle-18 batch — additional caller-supplied callback
   types for the DM2_OVERSEE_RECORD plugin family, record-chain container
   redistribution, and the creature/minion allocation helpers.  All of
   these stay caller-owned; the helpers below fail closed when a required
   callback is unavailable. */
typedef int32_t (*DM2_V1_SkprojectOverseeSearchFn)(
    uint16_t start_record,
    uint16_t creature,
    int32_t filter,
    void *user); /* 0xfffffffe sentinel on no match */

typedef int32_t (*DM2_V1_SkprojectIsMoneyboxFn)(
    uint16_t record,
    void *user);

typedef void (*DM2_V1_SkprojectCutRecordFromFn)(
    uint16_t record,
    uint16_t container,
    int16_t x,
    int16_t y,
    void *user);

typedef void (*DM2_V1_SkprojectAppendRecordToFn)(
    uint16_t record,
    uint16_t container,
    int16_t x,
    int16_t y,
    void *user);

typedef void (*DM2_V1_SkprojectDeallocRecordFn)(
    uint16_t record,
    void *user);

typedef int32_t (*DM2_V1_SkprojectContentsHeadFn)(
    uint16_t container_record,
    void *user); /* 0xfffe sentinel for an empty container */

typedef int32_t (*DM2_V1_SkprojectBlend4deaFn)(
    uint8_t creature_type,
    uint16_t base,
    const int16_t *table,
    void *user);

typedef void (*DM2_V1_SkprojectAnimationFrameFn)(
    uint8_t creature_type,
    uint16_t mode,
    uint16_t ai_pointer,
    uint16_t ai_addend,
    uint16_t v1e055e_word0,
    void *user);

typedef int32_t (*DM2_V1_SkprojectCreatureAtSlotFn)(
    uint16_t x,
    uint16_t y,
    void *user); /* -1 when no creature occupies the cell */

typedef void (*DM2_V1_SkprojectQueueTimerFn)(
    uint16_t creature_slot,
    uint8_t type,
    uint8_t actor,
    uint8_t x,
    uint8_t y,
    uint16_t tick,
    int32_t *out_timer,
    void *user);

typedef void (*DM2_V1_SkprojectDeleteTimerFn)(
    uint16_t timer,
    void *user);

typedef int (*DM2_V1_SkprojectSlotOccupiedFn)(
    uint16_t slot,
    void *user); /* nonzero when creatures[slot] word0 >= 0 (occupied) */

typedef int32_t (*DM2_V1_SkprojectRecycleRecordFn)(
    uint8_t cls,
    uint8_t priority,
    void *user); /* -1 on failure */

typedef void (*DM2_V1_SkprojectDeleteCreatureRecordFn)(
    uint16_t x,
    uint16_t y,
    uint16_t arg2,
    uint16_t arg3,
    void *user);

typedef int32_t (*DM2_V1_SkprojectMissileRefOfMinionFn)(
    uint16_t creature,
    uint16_t default_map,
    void *user); /* 0 when the source would return NULL */

typedef void (*DM2_V1_SkprojectAi13e40360Fn)(
    uint16_t creature,
    int16_t x,
    int16_t y,
    uint16_t reason,
    uint16_t arg4,
    void *user);

typedef void (*DM2_V1_SkprojectAttackCreatureFn)(
    uint16_t creature,
    int16_t x,
    int16_t y,
    uint16_t dir,
    uint16_t power,
    uint16_t arg5,
    void *user);

/* SKULLWIN/c_1c9a.cpp:5198 DM2_1c9a_0694 — DM2_OVERSEE_RECORD plugin
   predicate.  When the filter word is not the 0xfffffffe wildcard the
   item's distinctive type (DM2_GET_DISTINCTIVE_ITEMTYPE) must equal the
   filter for a match. */
int dm2_v1_skproject_1c9a_0694(
    uint16_t record,
    int32_t filter,
    DM2_V1_SkprojectDistinctiveTypeFn distinctive_type_fn,
    void *user);

/* SKULLWIN/c_1c9a.cpp:5217 DM2_1c9a_06bd — thin DM2_OVERSEE_RECORD search
   wrapper using the DM2_1c9a_0694 predicate above.  Returns 0 (NULL) when
   start_record is the -1 sentinel or the search reports no match
   (0xfffffffe). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int no_match;
} DM2_V1_Skproject06bdReceipt;

int32_t dm2_v1_skproject_1c9a_06bd(
    int32_t start_record,
    uint16_t creature,
    int16_t filter,
    DM2_V1_SkprojectOverseeSearchFn oversee_fn,
    void *user,
    DM2_V1_Skproject06bdReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5248 DM2_1c9a_078b — recursive record-chain walk
   belonging to DM2_PROCEED_XACT_71.  Visits container-chain nodes whose
   type nibble (bits 10-13 of the raw record value) is 5..13 or 9,
   optionally filtered by a direction nibble (bits 14-15); for a moneybox
   (type 9) that the creature cannot directly handle, its contents are
   redistributed into the outer container and the moneybox itself is cut
   and deallocated. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint32_t visited;
    uint32_t redistributed;
} DM2_V1_Skproject078bReceipt;

int32_t dm2_v1_skproject_1c9a_078b(
    uint16_t container,
    int16_t creature_type,
    uint8_t direction_filter,
    uint16_t start_record,
    DM2_V1_SkprojectNextRecordFn next_fn,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    DM2_V1_SkprojectIsMoneyboxFn moneybox_fn,
    DM2_V1_SkprojectCutRecordFromFn cut_fn,
    DM2_V1_SkprojectAppendRecordToFn append_fn,
    DM2_V1_SkprojectDeallocRecordFn dealloc_fn,
    DM2_V1_SkprojectContentsHeadFn contents_head_fn,
    void *user,
    DM2_V1_Skproject078bReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5376 DM2_1c9a_0958 — creature AI-spec blend via
   DM2_4DEA; result is masked to a multiple of 0x80 and shifted right by 7
   (i.e. the blended byte divided by 128). */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int32_t blended;
} DM2_V1_Skproject0958Receipt;

int32_t dm2_v1_skproject_1c9a_0958(
    uint8_t creature_type,
    uint16_t ai_pointer,
    const int16_t *table,
    DM2_V1_SkprojectBlend4deaFn blend_fn,
    void *user,
    DM2_V1_Skproject0958Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5403 DM2_1c9a_09b9 — belongs to
   DM2_ACTIVATE_CREATURE_KILLER; compares the creature record's word at
   offset 0x8 against the supplied creature index. */
int dm2_v1_skproject_1c9a_09b9(
    uint16_t creature_record,
    uint16_t creature_index,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    void *user);

/* SKULLWIN/c_1c9a.cpp:5415 DM2_1c9a_09db — belongs to
   DM2_FILL_CAII_CUR_MAP; resolves the creature's AI pointer and forwards
   it to DM2_GET_CREATURE_ANIMATION_FRAME. */
typedef struct {
    int valid;
    int blocked_missing_callback;
} DM2_V1_Skproject09dbReceipt;

int dm2_v1_skproject_1c9a_09db(
    uint8_t creature_type,
    uint16_t ai_pointer,
    uint16_t v1e055e_word0,
    DM2_V1_SkprojectAnimationFrameFn animation_fn,
    void *user,
    DM2_V1_Skproject09dbReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5433 DM2_CREATURE_SOMETHING_1c9a_0a48 (was
   SKW_1c9a_0a48) — creature retaliation/idle-delay scheduler over the
   `s350` sequencer scratch state (v1e0552/v1e054e/v1e055a/v1e055e caches,
   the active creature record, ddat sound-zone tables).  Firestaff does
   not yet model that scratch state, so this helper stays fail-closed: it
   documents the required inputs and reports blocked_missing_state until a
   caller supplies a full state bridge. */
typedef struct {
    int valid;
    int blocked_missing_state;
} DM2_V1_Skproject0a48Receipt;

int32_t dm2_v1_skproject_creature_something_1c9a_0a48(
    void *state,
    DM2_V1_Skproject0a48Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5694 DM2_1c9a_0cf7 — queues a "creature moved away"
   timer for the creature at (x,y) on the given map and stores the timer
   handle back into the creature array slot; cancels any prior pending
   timer for that slot first via DM2_1c9a_0db0. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int32_t creature_slot;
    int32_t timer;
} DM2_V1_Skproject0cf7Receipt;

int dm2_v1_skproject_1c9a_0cf7(
    uint16_t map,
    uint8_t x,
    uint8_t y,
    uint16_t gametick,
    DM2_V1_SkprojectCreatureAtSlotFn creature_at_fn,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectQueueTimerFn queue_timer_fn,
    DM2_V1_SkprojectDeleteTimerFn delete_timer_fn,
    void *user,
    DM2_V1_Skproject0cf7Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5733 DM2_1c9a_0db0 — cancels the pending
   "creature moved away" timer for a creature record when its type nibble
   (bits 10-13 of record^byte0, masked and shifted) equals 4. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int cancelled;
} DM2_V1_Skproject0db0Receipt;

int dm2_v1_skproject_1c9a_0db0(
    uint16_t record,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectDeleteTimerFn delete_timer_fn,
    void *user,
    DM2_V1_Skproject0db0Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5765 DM2_14cd_0802 — belongs to
   DM2_ALLOC_CAII_TO_CREATURE; resets the newly allocated creature's
   caii-index (offset 0x12 = -1) and caii-flags (offset 0x13 = 0). */
typedef struct {
    uint8_t caii_index;
    uint8_t caii_flags;
} DM2_V1_Skproject14cd0802Slot;

void dm2_v1_skproject_14cd_0802(DM2_V1_Skproject14cd0802Slot *slot);

/* SKULLWIN/c_1c9a.cpp:5771 DM2_ALLOC_CAII_TO_CREATURE — allocates a free
   creature array slot for a record not yet materialized (byte 0x5 ==
   0xff), recycling a world record when the array is full, and finishes
   by calling DM2_14cd_0802 and DM2_1c9a_0cf7 for the new slot.  Firestaff
   models the creature array occupancy check and slot count through
   caller callbacks and fails closed without them. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int already_allocated;
    int32_t slot;
    int recycled;
} DM2_V1_SkprojectAllocCaiiReceipt;

int dm2_v1_skproject_alloc_caii_to_creature(
    uint16_t record,
    uint8_t record_byte5,
    uint16_t slot_count,
    DM2_V1_SkprojectSlotOccupiedFn slot_occupied_fn,
    DM2_V1_SkprojectRecycleRecordFn recycle_fn,
    void *user,
    DM2_V1_SkprojectAllocCaiiReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5895 DM2_1c9a_0fcb — releases a creature array slot:
   clears its type field, cancels its pending timer via DM2_1c9a_0db0,
   decrements the world record counter, marks the backing record's byte
   0x5 as unallocated, and (when the slot lacked the AI-spec flag 0x1 and
   was of creature type 0x13) deletes the creature record at its last
   known timer position. */
typedef struct {
    int valid;
    int blocked_out_of_range;
    int blocked_missing_callback;
    int deleted_creature_record;
} DM2_V1_Skproject0fcbReceipt;

int dm2_v1_skproject_1c9a_0fcb(
    uint16_t slot,
    uint16_t max_slot,
    uint16_t record_word0,
    uint8_t creature_type,
    uint16_t ai_spec_flags,
    int32_t timer_index,
    uint16_t timer_x,
    uint16_t timer_y,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectDeleteTimerFn delete_timer_fn,
    DM2_V1_SkprojectDeleteCreatureRecordFn delete_creature_fn,
    void *user,
    DM2_V1_Skproject0fcbReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:5960 DM2_CREATE_MINION — searches around a source
   position (via table1d27fc/table1d2804 direction offset tables) for a
   free tile to spawn a minion creature, allocating it through
   dm2_dballochandler.DM2_ALLOC_NEW_CREATURE.  The tile/placement search
   and creature allocator are caller-owned runtime state that Firestaff
   does not yet bridge, so this helper stays fail-closed. */
typedef struct {
    int valid;
    int blocked_missing_state;
} DM2_V1_SkprojectCreateMinionReceipt;

int16_t dm2_v1_skproject_create_minion(
    void *state,
    DM2_V1_SkprojectCreateMinionReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:6148 DM2_RELEASE_MINION — belongs to
   DM2_ENGAGE_COMMAND; when the creature has a live missile reference,
   switches to the missile's map, computes its direction/distance words,
   dispatches DM2_ai_13e4_0360 with reason 0x13, then restores the
   previous current map. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    int had_missile_ref;
} DM2_V1_SkprojectReleaseMinionReceipt;

void dm2_v1_skproject_release_minion(
    uint16_t creature,
    uint16_t current_map,
    DM2_V1_SkprojectMissileRefOfMinionFn missile_ref_fn,
    DM2_V1_SkprojectChangeMapFn change_map_fn,
    DM2_V1_SkprojectAi13e40360Fn ai_13e4_0360_fn,
    void *user,
    DM2_V1_SkprojectReleaseMinionReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:6181 DM2_1c9a_17c7 — belongs to DM2_WOUND_CREATURE;
   true only on the current map (ddat.v1e08d6), outside cutscene/network
   gating (v1e0238==0, v1e0976==0), when the target's coarse direction
   from the reference position matches ddat.v1e08da and both axis deltas
   from (x,y) are within 2 tiles. */
typedef struct {
    uint16_t v1e08d6;
    uint16_t v1e0238;
    uint16_t v1e0976;
    uint16_t v1e08d8;
    uint16_t v1e08d4;
    uint16_t v1e08da;
    uint16_t map;
} DM2_V1_Skproject17c7State;

int dm2_v1_skproject_1c9a_17c7(
    int16_t x,
    int16_t y,
    const DM2_V1_Skproject17c7State *state,
    int32_t (*calc_vector_dir_fn)(uint16_t ref_y, int16_t dy, int16_t ref_x,
                                   int16_t dx, void *user),
    void *user);

/* SKULLWIN/c_1c9a.cpp:6240 DM2_1c9a_19d4 — dispatch gate that only calls
   DM2_ATTACK_CREATURE when the command word is in [6,0x15]; bit 0x8000 of
   the command word sign-extends into the high byte of the direction
   argument before dispatch. */
typedef struct {
    int valid;
    int out_of_range;
} DM2_V1_Skproject19d4Receipt;

void dm2_v1_skproject_1c9a_19d4(
    uint16_t creature,
    int16_t x,
    uint16_t cmd,
    int16_t y,
    DM2_V1_SkprojectAttackCreatureFn attack_fn,
    void *user,
    DM2_V1_Skproject19d4Receipt *out_receipt);

const char *dm2_v1_skproject_core_source_evidence(void);

/* --- Batch 19a: c_1c9a.cpp walk path / CAII symbols --- */

typedef const uint8_t *(*DM2_V1_SkprojectGetAddressOfRecordFn)(
    uint16_t record, void *user);
typedef uint16_t (*DM2_V1_SkprojectGetNextRecordLinkFn)(
    uint16_t record, void *user);

/* SKULLWIN/c_1c9a.cpp:6273 DM2_1c9a_1a48 */
typedef struct {
    int valid;
    int matched;
    uint16_t result_bits;
    int records_walked;
} DM2_V1_Skproject1a48Receipt;

int32_t dm2_v1_skproject_1c9a_1a48(
    int16_t direction,
    int16_t test_mask,
    int16_t tile_record_link,
    DM2_V1_SkprojectGetAddressOfRecordFn get_address_fn,
    DM2_V1_SkprojectGetNextRecordLinkFn get_next_fn,
    void *user,
    DM2_V1_Skproject1a48Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:6354 DM2_1c9a_1b16 */
typedef struct {
    int valid;
    int matched;
    uint16_t result_bits;
    int records_walked;
} DM2_V1_Skproject1b16Receipt;

int32_t dm2_v1_skproject_1c9a_1b16(
    int16_t sensor_dir_wanted,
    int16_t slot_wanted,
    int16_t tile_record_link,
    DM2_V1_SkprojectGetAddressOfRecordFn get_address_fn,
    DM2_V1_SkprojectGetNextRecordLinkFn get_next_fn,
    void *user,
    DM2_V1_Skproject1b16Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:6420 DM2_1c9a_1bae */
typedef int32_t (*DM2_V1_Skproject1baeFallbackFn)(int32_t x, int32_t y,
                                                   void *user);
typedef struct {
    int valid;
    int matched_creature_pos;
    int used_fallback;
    int32_t result;
} DM2_V1_Skproject1baeReceipt;

int32_t dm2_v1_skproject_1c9a_1bae(
    int16_t x,
    int16_t y,
    int16_t creature_x,
    int16_t creature_y,
    DM2_V1_Skproject1baeFallbackFn fallback_fn,
    void *user,
    DM2_V1_Skproject1baeReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:6438 DM2_FIND_WALK_PATH */
typedef struct {
    int valid;
    int path_found;
    int steps;
} DM2_V1_SkprojectFindWalkPathReceipt;

void dm2_v1_skproject_find_walk_path_receipt_init(
    DM2_V1_SkprojectFindWalkPathReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:9670 DM2___SET_CURRENT_THINKING_CREATURE_WALK_PATH */
typedef struct {
    void *creatures;
    uint8_t walk_path_b00;
    uint8_t walk_path_b01;
    uint16_t v1e054c;
    int16_t *v1e07e6;
} DM2_V1_SkprojectWalkPathState;

typedef int16_t *(*DM2_V1_SkprojectGetBmpFn)(int16_t id, void *user);

typedef struct {
    int valid;
    int early_exit_no_creatures;
    int early_exit_b00_zero;
    int alloc_failed;
    int alloc_succeeded;
} DM2_V1_SkprojectSetWalkPathReceipt;

void dm2_v1_skproject_set_current_thinking_creature_walk_path(
    DM2_V1_SkprojectWalkPathState *state,
    DM2_V1_SkprojectAllocation11Fn alloc_fn,
    DM2_V1_SkprojectGetBmpFn get_bmp_fn,
    void *user,
    DM2_V1_SkprojectSetWalkPathReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:9696 DM2_1c9a_381c */
typedef struct {
    int valid;
    int used_walk_path;
    int used_fallback;
    uint8_t direction;
    uint8_t step_count;
} DM2_V1_Skproject381cReceipt;

int32_t dm2_v1_skproject_1c9a_381c(
    DM2_V1_SkprojectWalkPathState *state,
    uint8_t *creature_base,
    DM2_V1_SkprojectAllocation11Fn alloc_fn,
    DM2_V1_SkprojectGetBmpFn get_bmp_fn,
    void *user,
    DM2_V1_Skproject381cReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:9748 DM2_1c9a_38a8 */
typedef struct {
    int placeholder;
} DM2_V1_Skproject38a8State;

typedef struct {
    int valid;
    int searched_action_list;
} DM2_V1_Skproject38a8Receipt;

int32_t dm2_v1_skproject_1c9a_38a8(
    const DM2_V1_Skproject38a8State *state,
    DM2_V1_Skproject38a8Receipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:9895 DM2_FILL_CAII_CUR_MAP */
typedef const uint8_t *(*DM2_V1_SkprojectQueryCreatureAISpecFn)(
    uint8_t creature_type, void *user);
typedef void (*DM2_V1_SkprojectAllocCaiiToCreatureFn)(
    uint16_t record, uint16_t col, uint16_t row, void *user);

typedef struct {
    int16_t map_width;
    int16_t map_height;
    const uint8_t *tile_data;
    const uint16_t *record_links;
} DM2_V1_SkprojectFillCaiiState;

typedef struct {
    int valid;
    int tiles_with_things;
    int creatures_found;
    int caii_allocated;
} DM2_V1_SkprojectFillCaiiReceipt;

int32_t dm2_v1_skproject_fill_caii_cur_map(
    const DM2_V1_SkprojectFillCaiiState *map_state,
    DM2_V1_SkprojectGetAddressOfRecordFn get_address_fn,
    DM2_V1_SkprojectGetNextRecordLinkFn get_next_fn,
    DM2_V1_SkprojectQueryCreatureAISpecFn query_ai_fn,
    DM2_V1_SkprojectAllocCaiiToCreatureFn alloc_caii_fn,
    void *user,
    DM2_V1_SkprojectFillCaiiReceipt *out_receipt);

/* SKULLWIN/c_1c9a.cpp:9995 DM2_FILL_ORPHAN_CAII — iterates all maps,
   calling DM2_FILL_CAII_CUR_MAP on each, then restores the original map. */
typedef int32_t (*DM2_V1_SkprojectFillCaiiCurMapFn)(void *user);

typedef struct {
    int valid;
    int blocked_missing_callback;
    uint16_t original_map;
    uint16_t num_maps;
    uint16_t maps_iterated;
} DM2_V1_SkprojectFillOrphanCaiiReceipt;

void dm2_v1_skproject_fill_orphan_caii(
    uint16_t current_map,
    uint16_t num_maps,
    DM2_V1_SkprojectChangeMapFn change_map_fn,
    DM2_V1_SkprojectFillCaiiCurMapFn fill_caii_fn,
    void *user,
    DM2_V1_SkprojectFillOrphanCaiiReceipt *out_receipt);

/* SKULLWIN/c_addon.cpp:30 event_loop_T1 — platform event loop timing
   logic.  The receipt captures the 25Hz sub-tick counter state and whether
   a vsync blit should occur this iteration. */
typedef struct {
    int valid;
    int tick_count;
    int blit_due;
    int vsync_triggered;
} DM2_V1_SkprojectEventLoopT1Receipt;

void dm2_v1_skproject_event_loop_t1(
    int timer_events,
    int vsync_counter_in,
    int *vsync_counter_out,
    int *tick_out,
    DM2_V1_SkprojectEventLoopT1Receipt *out_receipt);

/* SKULLWIN/c_addon.cpp:122 wait_for_vsync */
void dm2_v1_skproject_wait_for_vsync(int *vsync_counter);

/* SKULLWIN/c_addon.cpp:127 wft (wait-for-tick) */
typedef struct {
    int valid;
    int would_block;
} DM2_V1_SkprojectWftReceipt;

void dm2_v1_skproject_wft(
    int tick_in,
    int *tick_out,
    DM2_V1_SkprojectWftReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:364 DM2_PROCEED_XACT_65 */
typedef uint16_t (*DM2_V1_SkprojectQueryCreatureAiSpecFlagsFn)(
    int32_t creature_handle,
    void *user);

typedef struct {
    uint16_t v1e08d6;
    uint16_t v1e08d8;
    uint16_t v1e08d4;
    uint16_t current_map;
} DM2_V1_SkprojectXact65State;

typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t ahead_x;
    uint16_t ahead_y;
    int creature_found;
    int ai_spec_allows;
    int party_at_target;
    int8_t result;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact65Receipt;

int dm2_v1_skproject_proceed_xact_65(
    const DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_SkprojectXact65State *state,
    DM2_V1_SkprojectQueryCreatureAiSpecFlagsFn ai_spec_flags_fn,
    DM2_V1_SkprojectXact65Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:400 DM2_14cd_2662 */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t target_x;
    uint16_t target_y;
    int creature_at_target;
    int found_handler;
} DM2_V1_Skproject14cd2662Receipt;

int dm2_v1_skproject_14cd_2662(
    uint8_t adjust,
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_Skproject14cd2662Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:519 DM2_PROCEED_XACT_66 */
typedef struct {
    int valid;
    int blocked_missing_context;
    int handler_ahead;
    int xact63_accepted;
    int8_t result;
    uint16_t out_w0e;
    uint8_t out_b1a;
    uint16_t out_v1e0572;
} DM2_V1_SkprojectXact66Receipt;

int dm2_v1_skproject_proceed_xact_66(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact66Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:571 DM2_PROCEED_XACT_67 */
typedef int16_t (*DM2_V1_SkprojectAi14cd2886Fn)(
    const uint16_t *record_chain,
    uint16_t item_type,
    int32_t adjust,
    int flag1,
    int flag2,
    int flag3,
    void *user);

typedef int32_t (*DM2_V1_SkprojectQuery48ae0767Fn)(
    int32_t value,
    int32_t param,
    int16_t *out_array,
    int16_t *out_count,
    void *user);

typedef uint16_t (*DM2_V1_SkprojectRand16Fn)(uint16_t max, void *user);
typedef uint16_t (*DM2_V1_SkprojectRandDirFn)(void *user);

typedef struct {
    int valid;
    int blocked_missing_context;
    int handler_ahead;
    int creature_at_facing;
    int16_t damage_ratio;
    int16_t defense_value;
    int8_t result;
    uint16_t out_w0e;
    uint16_t out_w10;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact67Receipt;

int dm2_v1_skproject_proceed_xact_67(
    DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_SkprojectXact65State *state,
    DM2_V1_SkprojectAi14cd2886Fn ai2886_fn,
    DM2_V1_SkprojectQuery48ae0767Fn query0767_fn,
    DM2_V1_SkprojectRand16Fn rand16_fn,
    DM2_V1_SkprojectRandDirFn randdir_fn,
    DM2_V1_SkprojectXact67Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:745 DM2_PROCEED_XACT_68 — creature AI: evaluate facing
   creature's combat stats via DM2_14cd_2886 and DM2_query_48ae_0767, compare
   attack vs defense totals, set b1a=28 on match or b1a=27 otherwise. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int creature_found;
    int16_t facing_dir;
    int32_t damage_total;
    int32_t defense_total;
    int8_t result;
    uint16_t out_w0c;
    uint16_t out_w10;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact68Receipt;

int dm2_v1_skproject_proceed_xact_68(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectAi14cd2886Fn ai2886_fn,
    DM2_V1_SkprojectQuery48ae0767Fn query0767_fn,
    DM2_V1_SkprojectXact68Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:828 DM2_PROCEED_XACT_69 — creature AI: set creature
   target position from facing direction offset and set b1a to 21 or 22
   based on v1e0572. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint16_t out_w18;
    uint8_t out_b1a;
    uint8_t out_b1d;
} DM2_V1_SkprojectXact69Receipt;

int dm2_v1_skproject_proceed_xact_69(
    DM2_V1_SkprojectXactContext *ctx,
    const int16_t *table1d27fc,
    const int16_t *table1d2804,
    DM2_V1_SkprojectXact69Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:843 DM2_PROCEED_XACT_70 — creature AI: set creature
   target from raw x/y/direction, check for creature at the target, and
   test DM2_CREATURE_CAN_HANDLE_ITEM_IN.  Sets b1a=24 on rejection. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int creature_found;
    int can_handle;
    int8_t result;
    uint16_t out_w18;
    uint8_t out_b1a;
    uint8_t out_b1c;
    uint8_t out_b1e;
} DM2_V1_SkprojectXact70Receipt;

int dm2_v1_skproject_proceed_xact_70(
    DM2_V1_SkprojectXactContext *ctx,
    const int16_t *table1d27fc,
    const int16_t *table1d2804,
    DM2_V1_SkprojectXact70Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:897 DM2_PROCEED_XACT_71 — creature AI: possession
   redistribution via DM2_1c9a_078b, then test CREATURE_CAN_HANDLE_ITEM_IN
   and dispatch DM2_19f0_2165 on success. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int redistributed;
    int dispatched;
    int8_t result;
} DM2_V1_SkprojectXact71Receipt;

int dm2_v1_skproject_proceed_xact_71(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact71Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:949 DM2_PROCEED_XACT_72_87_88 — creature AI: simple
   b1a assignment from v1e0572, falling back to v1e07d8_w04. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact72Receipt;

int dm2_v1_skproject_proceed_xact_72_87_88(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact72Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:958 DM2_PROCEED_XACT_73 — creature AI: flag-word
   manipulation on v1e054e->w_0a based on v1e0574 selector (0-2=bit ops,
   3-4=hexe table scan, 16-18=bit ops).  Sets b1a=51 on change. */
typedef struct {
    uint16_t creature_word_a;  /* s350.v1e054e->w_0a */
    const uint8_t *hexe_table; /* s350.v1e07d8.xp_0a entries, each 14 bytes */
    uint16_t hexe_count;       /* number of entries */
} DM2_V1_SkprojectXact73State;

typedef struct {
    int valid;
    int blocked_missing_context;
    int flag_changed;
    int8_t result;
    uint16_t out_word_a;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact73Receipt;

int dm2_v1_skproject_proceed_xact_73(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact73State *state73,
    DM2_V1_SkprojectXact73Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1067 DM2_PROCEED_XACT_74 — creature AI: flee-or-chase
   based on random attack chance vs DM2_1c9a_381c walk path evaluation,
   dispatches DM2_CREATURE_GO_THERE or DM2_19f0_0559. */
typedef int32_t (*DM2_V1_Skproject381cSimpleFn)(void *user);
typedef int32_t (*DM2_V1_SkprojectRandBitFn)(void *user);

typedef struct {
    uint16_t v1e0552_w16;     /* word@0x16 of v1e0552 */
    uint16_t creature_word_a; /* s350.v1e054e->w_0a */
    uint16_t ddat_v1d3248;    /* current map */
} DM2_V1_SkprojectXact74State;

typedef struct {
    int valid;
    int blocked_missing_context;
    int attack_roll;
    int walk_path_available;
    int8_t result;
    uint8_t out_b1a;
} DM2_V1_SkprojectXact74Receipt;

int dm2_v1_skproject_proceed_xact_74(
    DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_SkprojectXact74State *state74,
    DM2_V1_Skproject381cSimpleFn walk381c_fn,
    DM2_V1_SkprojectRandBitFn randbit_fn,
    const int16_t *table1d27fc,
    const int16_t *table1d2804,
    DM2_V1_SkprojectXact74Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1179 DM2_14cd_102e — recursive record-chain item
   counter belonging to DM2_ai_14cd_10d2.  Walks a record chain, counts
   items the creature can handle (via DM2_CREATURE_CAN_HANDLE_IT), and
   recurses into container/chest sub-chains. */
typedef struct {
    int valid;
    int blocked_missing_callback;
    uint32_t visited;
    int32_t count;
} DM2_V1_Skproject102eReceipt;

int32_t dm2_v1_skproject_14cd_102e(
    int16_t creature_type,
    uint16_t start_record,
    uint8_t direction_filter,
    int recurse_type4,
    int recurse_chests,
    DM2_V1_SkprojectNextRecordFn next_fn,
    DM2_V1_SkprojectRecordAccessorFn record_fn,
    DM2_V1_SkprojectCanHandleItFn can_handle_fn,
    DM2_V1_SkprojectIsChestFn is_chest_fn,
    void *user,
    DM2_V1_Skproject102eReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1285 DM2_ai_14cd_10d2 — 4-slot cache search/claim. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int found_existing;
    int claimed_new;
    int cache_full;
    int slot_index;
    uint8_t header[8];
} DM2_V1_Skproject14cd10d2Receipt;

int dm2_v1_skproject_14cd_10d2(
    const uint8_t *record_ptr,
    int32_t record_type,
    uint8_t cache[4][0x20],
    int *cache_dirty,
    DM2_V1_Skproject14cd10d2Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1470 DM2_PROCEED_XACT_75 */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t mode_code;
    uint8_t out_creature_b1e;
    uint16_t saved_v1e0578;
    uint16_t masked_v1e0578;
    int flag8_cleared;
    int8_t result;
} DM2_V1_SkprojectXact75Receipt;

typedef struct {
    const uint8_t *hexe_xp_0a;
    uint8_t b_02;
    uint8_t b_03;
    uint16_t w_04;
    uint16_t w_06;
} DM2_V1_SkprojectXact75Input;

int dm2_v1_skproject_proceed_xact_75(
    const DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_SkprojectXact75Input *input,
    uint8_t cache[4][0x20],
    int *cache_dirty,
    DM2_V1_SkprojectXact75Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1498 DM2_ai_14cd_0f3c — attack plan entry builder. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int skipped_null;
    int skipped_full;
    int skipped_negative;
    int entry_added;
    int16_t adjusted_strength;
    uint8_t entry[22];
    int entry_index;
} DM2_V1_Skproject14cd0f3cReceipt;

typedef struct {
    int32_t eaxl;
    const uint8_t *record_ptr;
    const uint8_t *hexe_ptr;
    int32_t ecxl;
    int8_t argb0;
    int32_t argl1;
    int8_t argb2;
    int8_t argb3;
} DM2_V1_Skproject14cd0f3cInput;

typedef struct {
    uint16_t v1e0571;
    uint16_t v1e08d6;
    uint8_t v1e0552_byte1;
    uint16_t v1e0580;
} DM2_V1_Skproject14cd0f3cState;

int dm2_v1_skproject_14cd_0f3c(
    const DM2_V1_Skproject14cd0f3cInput *input,
    const DM2_V1_Skproject14cd0f3cState *state,
    uint8_t plan_entries[][22],
    int *plan_count,
    int max_plan_entries,
    DM2_V1_Skproject14cd0f3cReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1569 DM2_PROCEED_XACT_77 */
typedef struct {
    int valid;
    int blocked_missing_context;
    int hexe_entries_scanned;
    int plan_entries_added;
    int walk_path_found;
    int8_t walk_path_result;
    int8_t result;
} DM2_V1_SkprojectXact77Receipt;

int dm2_v1_skproject_proceed_xact_77(
    const DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_Skproject14cd0f3cState *plan_state,
    int8_t max_strength,
    DM2_V1_SkprojectXact77Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1611 DM2_PROCEED_XACT_78 */
typedef struct {
    int valid;
    int blocked_missing_context;
    int map_matches;
    int tile_passable;
    int16_t computed_dir;
    int8_t result;
} DM2_V1_SkprojectXact78Receipt;

int dm2_v1_skproject_proceed_xact_78(
    const DM2_V1_SkprojectXactContext *ctx,
    const DM2_V1_SkprojectXact65State *state,
    DM2_V1_SkprojectXact78Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1629 DM2_PROCEED_XACT_79 */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t out_b1e;
    uint8_t out_b1a;
    uint8_t out_b1b;
    uint8_t out_b1c;
    uint8_t out_b20;
} DM2_V1_SkprojectXact79Receipt;

int dm2_v1_skproject_proceed_xact_79(
    DM2_V1_SkprojectRandDirFn randdir_fn,
    DM2_V1_SkprojectRand16Fn rand16_fn,
    void *user,
    DM2_V1_SkprojectXact79Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1645 DM2_PROCEED_XACT_80 */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t go_mode;
    uint16_t adjusted_dir;
    uint16_t saved_v1e0576;
    uint16_t modified_v1e0576;
    int8_t result;
} DM2_V1_SkprojectXact80Receipt;

int dm2_v1_skproject_proceed_xact_80(
    DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact80Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1672 DM2_PROCEED_XACT_81 */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t command_byte;
    uint16_t v1e07d8_w04;
    int8_t result;
} DM2_V1_SkprojectXact81Receipt;

int dm2_v1_skproject_proceed_xact_81(
    const DM2_V1_SkprojectXactContext *ctx,
    uint8_t w06_low,
    uint16_t w04,
    DM2_V1_SkprojectXact81Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1679 DM2_14cd_3582 — coin wallet rebalance helper for xact_82. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int32_t total_value;
    int needs_rebalance;
    int8_t comparison_mode; /* peek16 of first arg */
} DM2_V1_Skproject14cd3582Receipt;

int dm2_v1_skproject_14cd_3582(
    int32_t mode,
    uint16_t wallet_handle,
    const uint16_t *coin_values,
    uint16_t coin_type_count,
    DM2_V1_Skproject14cd3582Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1805 DM2_PROCEED_XACT_82 — creature buy/sell interaction. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t out_b1a;
    int has_creature;
    int16_t creature_handle;
    int16_t wallet_handle;
    int used_sell_path;
    int8_t result;
} DM2_V1_SkprojectXact82Receipt;

int dm2_v1_skproject_proceed_xact_82(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact82Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1939 DM2_PROCEED_XACT_83 — creature action 0x23-0x25. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t out_b1a;
    int has_w0a_bit7;
    int8_t result;
} DM2_V1_SkprojectXact83Receipt;

int dm2_v1_skproject_proceed_xact_83(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact83Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:1967 DM2_PROCEED_XACT_84 — creature item consume/drop. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int has_possession;
    int16_t possession_handle;
    int16_t item_category;
    int item_consumable;
    int item_deallocated;
    int8_t result;
} DM2_V1_SkprojectXact84Receipt;

int dm2_v1_skproject_proceed_xact_84(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectRand16Fn rand_fn,
    void *user,
    DM2_V1_SkprojectXact84Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2078 DM2_PROCEED_XACT_85 — search tile for drinkable text. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int found_text;
    uint8_t out_b1e;
    uint8_t out_b1a;
    int8_t result;
} DM2_V1_SkprojectXact85Receipt;

int dm2_v1_skproject_proceed_xact_85(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact85Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2120 DM2_PROCEED_XACT_86 — set creature b20/b1e/b1a. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t out_b20;
    uint8_t out_b1e;
    uint8_t out_b1a;
    int8_t result;
} DM2_V1_SkprojectXact86Receipt;

int dm2_v1_skproject_proceed_xact_86(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact86Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2129 DM2_PROCEED_XACT_89 — projectile via 19f0_0d10. */
typedef struct {
    int valid;
    int blocked_missing_context;
    uint8_t command_byte;
    int8_t result;
} DM2_V1_SkprojectXact89Receipt;

int dm2_v1_skproject_proceed_xact_89(
    const DM2_V1_SkprojectXactContext *ctx,
    uint8_t w06_low,
    DM2_V1_SkprojectXact89Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2136 DM2_PROCEED_XACT_90 — random chance check. */
typedef struct {
    int valid;
    int blocked_missing_context;
    int16_t threshold;
    int8_t result;
} DM2_V1_SkprojectXact90Receipt;

int dm2_v1_skproject_proceed_xact_90(
    int16_t v1e0572,
    DM2_V1_SkprojectRand16Fn rand16_fn,
    void *user,
    DM2_V1_SkprojectXact90Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2142 DM2_PROCEED_XACT_91 */
typedef struct {
    int valid;
    int blocked_missing_context;
    int8_t result;
} DM2_V1_SkprojectXact91Receipt;

int dm2_v1_skproject_proceed_xact_91(
    const DM2_V1_SkprojectXactContext *ctx,
    DM2_V1_SkprojectXact91Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2152 DM2_PROCEED_XACT — dispatch classifier */
typedef struct {
    int valid;
    int dispatched;
    int8_t input_eaxb;
    int8_t opt;
    int8_t result;
} DM2_V1_SkprojectProceedXactReceipt;

int dm2_v1_skproject_proceed_xact_classify(
    int8_t eaxb,
    DM2_V1_SkprojectProceedXactReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2340 DM2_13e4_01a3 */
typedef struct {
    int valid;
    int blocked_already_init;
    uint16_t v1e0576;
    uint16_t v1e0578;
    uint16_t v1e057a;
    uint16_t v1e057c;
    uint16_t v1e057e;
    uint16_t v1e0582;
    uint8_t v1e07ec;
    uint8_t v1e058d;
    int alloc_attempted;
    int alloc_failed;
} DM2_V1_Skproject13e401a3Receipt;

int dm2_v1_skproject_13e4_01a3_classify(
    uint8_t v1e07eb,
    const uint8_t *v1e0552_ptr,
    uint16_t v1e0584_in,
    DM2_V1_Skproject13e401a3Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2414 DM2_14cd_062e */
typedef struct {
    int valid;
    uint8_t creature_b12;
    uint8_t creature_b13;
    int has_table_entry;
    uint8_t raw_byte5;
    uint8_t mask_e0;
    uint8_t mask_60;
    int map_mismatch;
    uint8_t result;
} DM2_V1_Skproject14cd062eReceipt;

int dm2_v1_skproject_14cd_062e_classify(
    const uint8_t *creature_ptr,
    uint16_t v1e0571,
    uint16_t v1e08d6,
    DM2_V1_Skproject14cd062eReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2446 DM2_14cd_18cc */
typedef struct {
    int valid;
    int8_t parb02;
    int8_t parb03;
} DM2_V1_Skproject14cd18ccReceipt;

int dm2_v1_skproject_14cd_18cc_classify(
    int32_t eaxl,
    int32_t edxl,
    DM2_V1_Skproject14cd18ccReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2466 DM2_2c1d_09d9 */
typedef struct {
    int valid;
    uint32_t skill_sum;
    uint16_t result;
} DM2_V1_Skproject2c1d09d9Receipt;

int dm2_v1_skproject_2c1d_09d9_compute(
    uint16_t heros_in_party,
    const uint16_t skills[][4],
    uint16_t max_heroes,
    DM2_V1_Skproject2c1d09d9Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:2516 DM2_14cd_1316 */
typedef struct {
    int valid;
    uint8_t raw_byte;
    uint8_t condition;
    int inverted;
    int has_0x40_gate;
    int gate_matched;
    int32_t result;
} DM2_V1_Skproject14cd1316Receipt;

int dm2_v1_skproject_14cd_1316_classify(
    uint8_t condition_byte,
    int16_t edxw,
    uint8_t ebxb,
    uint8_t creature_b12,
    DM2_V1_Skproject14cd1316Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3135 DM2_14cd_18f2 */
typedef struct {
    int valid;
    int blocked_null_ptr;
    uint8_t action_byte;
    int negated;
    int entries_visited;
    int entries_matched;
    int entries_delegated;
} DM2_V1_Skproject14cd18f2Receipt;

int dm2_v1_skproject_14cd_18f2_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    int8_t ecxb,
    uint16_t argw0,
    DM2_V1_Skproject14cd18f2Receipt *out_receipt);


/* SKULLWIN/c_ai.cpp:3210 DM2_14cd_19a4 — sign-extend, delegate to 18f2 */
typedef struct {
    int valid;
    int8_t eaxb_extended;
    int8_t edxb_extended;
    /* delegates with ecxb=0, argw0=0xffff */
} DM2_V1_Skproject14cd19a4Receipt;

int dm2_v1_skproject_14cd_19a4_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    DM2_V1_Skproject14cd19a4Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3222 DM2_14cd_19c2 — guarded delegate to 18f2 */
typedef struct {
    int valid;
    int blocked_null_ptr;
    int blocked_no_readiness;   /* v1e058d == 0 */
    int blocked_no_v1e0578;
    int byte5_lte_zero;         /* byte@5 of 10d2 result <= 0 */
    int negation_flag;          /* vb_04 != 0 => negate ecxb */
    int8_t ecxb_delegated;
    int8_t edxb_delegated;
} DM2_V1_Skproject14cd19c2Receipt;

int dm2_v1_skproject_14cd_19c2_classify(
    int8_t eaxb,
    const uint8_t *table_ptr,
    int8_t edxb,
    int8_t ecxb,
    int8_t argb0,
    uint8_t v1e058d,
    uint16_t v1e0578,
    const uint8_t *lookup_result,
    DM2_V1_Skproject14cd19c2Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3250 DM2_14cd_1a3c — sign-extend, delegate to 19c2(ecxl=2, argb0=1) */
typedef struct {
    int valid;
    int8_t eaxb_extended;
    int8_t edxb_extended;
} DM2_V1_Skproject14cd1a3cReceipt;

int dm2_v1_skproject_14cd_1a3c_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    DM2_V1_Skproject14cd1a3cReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3262 DM2_14cd_1a5a — sign-extend, delegate to 19c2(ecxl=4, argb0=3) */
typedef struct {
    int valid;
    int8_t eaxb_extended;
    int8_t edxb_extended;
} DM2_V1_Skproject14cd1a5aReceipt;

int dm2_v1_skproject_14cd_1a5a_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    DM2_V1_Skproject14cd1a5aReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3273 DM2_14cd_1a78 — table walker with 10d2/1316/0f3c */
typedef struct {
    int valid;
    int blocked_null_ptr;
    int blocked_byte7_zero;     /* byte@7 of 10d2 result == 0 */
    int entries_visited;
    int entries_matched;        /* 14cd_1316 passed */
    int entries_delegated;      /* via 14cd_0f3c */
} DM2_V1_Skproject14cd1a78Receipt;

int dm2_v1_skproject_14cd_1a78_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    int8_t ecxb,
    const uint8_t *lookup_result,
    DM2_V1_Skproject14cd1a78Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3356 DM2_14cd_1b74 — sign-extend, delegate to 1a78(ecxl=1) */
typedef struct {
    int valid;
    int8_t eaxb_extended;
    int8_t edxb_extended;
} DM2_V1_Skproject14cd1b74Receipt;

int dm2_v1_skproject_14cd_1b74_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    const uint8_t *lookup_result,
    DM2_V1_Skproject14cd1b74Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3368 DM2_14cd_1b90 — sign-extend, delegate to 1a78(ecxl=3) */
typedef struct {
    int valid;
    int8_t eaxb_extended;
    int8_t edxb_extended;
} DM2_V1_Skproject14cd1b90Receipt;

int dm2_v1_skproject_14cd_1b90_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    const uint8_t *lookup_result,
    DM2_V1_Skproject14cd1b90Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3380 DM2_14cd_1bac — like 19c2 but checks v1e0578&8 */
typedef struct {
    int valid;
    int blocked_null_ptr;
    int blocked_no_readiness;
    int blocked_no_v1e0578;
    int v1e0578_bit3_set;       /* v1e0578 & 0x8 != 0 */
    int byte5_lte_zero;
    int negation_flag;
    int8_t ecxb_delegated;
    int8_t edxb_delegated;
} DM2_V1_Skproject14cd1bacReceipt;

int dm2_v1_skproject_14cd_1bac_classify(
    int8_t eaxb,
    int8_t edxb,
    const uint8_t *table_ptr,
    int8_t ecxb,
    int8_t argb0,
    uint8_t v1e058d,
    uint16_t v1e0578,
    const uint8_t *lookup_result,
    DM2_V1_Skproject14cd1bacReceipt *out_receipt);

/* ---- batch 22b: SKULLWIN/c_ai.cpp DM2_14cd_1c27,
   DM2_14cd_1c45, DM2_14cd_1c63, DM2_14cd_1c8d, DM2_14cd_1cec,
   DM2_14cd_1d42, DM2_14cd_1d6c, DM2_14cd_1e36 ---- */

/* SKULLWIN/c_ai.cpp:3414 DM2_14cd_1c27 — sign-extends eaxl/edxl
   low bytes, delegates to 1bac with ecxl=2, argb0=1. */
typedef struct {
    int valid;
    int32_t sign_ext_eax;
    int32_t sign_ext_edx;
    int ecxl;   /* always 2 */
    int argb0;  /* always 1 */
} DM2_V1_Skproject14cd1c27Receipt;

int dm2_v1_skproject_14cd_1c27_classify(
    int32_t eaxl, int32_t edxl,
    DM2_V1_Skproject14cd1c27Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3426 DM2_14cd_1c45 — sign-extends eaxl/edxl
   low bytes, delegates to 1bac with ecxl=4, argb0=3. */
typedef struct {
    int valid;
    int32_t sign_ext_eax;
    int32_t sign_ext_edx;
    int ecxl;   /* always 4 */
    int argb0;  /* always 3 */
} DM2_V1_Skproject14cd1c45Receipt;

int dm2_v1_skproject_14cd_1c45_classify(
    int32_t eaxl, int32_t edxl,
    DM2_V1_Skproject14cd1c45Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3438 DM2_14cd_1c63 — checks v1e07d8.b_03 == 0xd;
   if so, argw0 = v1e07d8.w_08, else 0xffff.  Delegates to 14cd_18f2
   with eaxb=5, ecxb=0. */
typedef struct {
    int valid;
    int b03_is_0d;
    uint16_t argw0;
    int32_t sign_ext_edx;
    int eaxb;            /* always 5 */
} DM2_V1_Skproject14cd1c63Receipt;

int dm2_v1_skproject_14cd_1c63_classify(
    int32_t edxl,
    uint8_t v1e07d8_b03, uint16_t v1e07d8_w08,
    DM2_V1_Skproject14cd1c63Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3459 DM2_14cd_1c8d — creature position match check. */
typedef struct {
    int valid;
    int blocked_eax_zero;
    int x_match;
    int y_match;
    int map_match;
    int skipped;
    int32_t sign_ext_edx;
    int eaxb;               /* always 6 when delegating */
} DM2_V1_Skproject14cd1c8dReceipt;

int dm2_v1_skproject_14cd_1c8d_classify(
    int32_t eaxl, int32_t edxl,
    uint16_t creature_word_0c,
    uint16_t v1e0562_xa, uint16_t v1e0562_ya,
    uint8_t v1e0571,
    DM2_V1_Skproject14cd1c8dReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3495 DM2_14cd_1cec — missile ref lookup. */
typedef struct {
    int valid;
    int blocked_no_missile;
    int blocked_wrong_type;
    uint8_t missile_type;
    uint16_t argw0;
    int32_t sign_ext_edx;
    int eaxb;                   /* always 7 when delegating */
} DM2_V1_Skproject14cd1cecReceipt;

typedef const uint8_t *(*DM2_V1_GetMissileRefFn)(
    uint16_t v1e054c, uint16_t arg1, void *user);

typedef const uint8_t *(*DM2_V1_GetRecordAddressFn)(
    uint16_t record_ref, void *user);

int dm2_v1_skproject_14cd_1cec_classify(
    int32_t edxl,
    uint16_t v1e054c,
    DM2_V1_GetMissileRefFn get_missile_fn,
    DM2_V1_GetRecordAddressFn get_record_fn,
    void *user,
    DM2_V1_Skproject14cd1cecReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3522 DM2_14cd_1d42 — checks v1e07d8.b_03 == 5. */
typedef struct {
    int valid;
    int b03_is_05;
    uint16_t argw0;
    int32_t sign_ext_edx;
    int eaxb;            /* always 0x12 */
} DM2_V1_Skproject14cd1d42Receipt;

int dm2_v1_skproject_14cd_1d42_classify(
    int32_t edxl,
    uint8_t v1e07d8_b03, uint16_t v1e07d8_w08,
    DM2_V1_Skproject14cd1d42Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3541 DM2_14cd_1d6c — table walker. */
typedef struct {
    int valid;
    int blocked_null_ptr;
    int entries_visited;
    int entries_matched;
    int entries_delegated;
} DM2_V1_Skproject14cd1d6cReceipt;

typedef int16_t (*DM2_V1_CanHandleItemFn)(
    int16_t item, uint32_t creature_w2, uint16_t arg2, void *user);

typedef int32_t (*DM2_V1_1316CheckFn)(
    uint16_t byte1_ext, int32_t word2_ext, int32_t vb10_ext, void *user);

typedef void (*DM2_V1_0f3cDelegateFn)(
    int32_t byte0_ext, const uint8_t *entry_ptr,
    const uint8_t *hexe_ptr, int32_t vb14_ext,
    int zero, uint16_t ffff, int8_t vb10, int8_t vb18,
    void *user);

int dm2_v1_skproject_14cd_1d6c_classify(
    int32_t eaxl, int32_t edxl, const uint8_t *xebxp, int32_t ecxl,
    uint16_t creature_w2,
    DM2_V1_CanHandleItemFn can_handle_fn,
    DM2_V1_1316CheckFn check_1316_fn,
    DM2_V1_0f3cDelegateFn delegate_fn,
    void *user,
    DM2_V1_Skproject14cd1d6cReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3632 DM2_14cd_1e36 — sign-extends, delegates
   to 1d6c with ecxl=0xf. */
typedef struct {
    int valid;
    int32_t sign_ext_eax;
    int32_t sign_ext_edx;
    int ecxl;   /* always 0xf */
} DM2_V1_Skproject14cd1e36Receipt;

int dm2_v1_skproject_14cd_1e36_classify(
    int32_t eaxl, int32_t edxl,
    DM2_V1_Skproject14cd1e36Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3644 DM2_14cd_1e52 — sign-extends eaxl/edxl,
   delegates to 1d6c with ecxl=0x10. */
typedef struct {
    int valid;
    int32_t sign_ext_eax;
    int32_t sign_ext_edx;
    int ecxl;   /* always 0x10 */
} DM2_V1_Skproject14cd1e52Receipt;

int dm2_v1_skproject_14cd_1e52_classify(
    int32_t eaxl, int32_t edxl,
    DM2_V1_Skproject14cd1e52Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3656 DM2_3DC4C — creature type GDAT bit5 check. */
typedef struct {
    int valid;
    int32_t map_index;
    uint16_t table_word;
    uint8_t creature_type;
    uint16_t gdat_result;
    int bit5_set;
    int return_value;   /* 1 if bit5 NOT set, 0 if set */
} DM2_V1_Skproject3DC4CReceipt;

typedef uint16_t (*DM2_V1_QueryGdatEntryFn)(
    uint8_t cls, uint8_t type, uint8_t idx, uint8_t sub, void *user);

typedef uint16_t (*DM2_V1_ReadTableWordFn)(
    int32_t offset, void *user);

int dm2_v1_skproject_3dc4c_classify(
    int32_t eaxl,
    uint16_t v1e0571,
    DM2_V1_ReadTableWordFn read_table_fn,
    DM2_V1_QueryGdatEntryFn query_gdat_fn,
    void *user,
    DM2_V1_Skproject3DC4CReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3679 DM2_14cd_1e6e — random-based creature
   word@0xa bit7 manipulation with 0f3c delegation. */
typedef struct {
    int valid;
    int dc4c_result;        /* result of DM2_3DC4C call */
    int eaxl_nonzero_path;  /* 1 if eaxl (RG3Blo) != 0 */
    int rand_check;         /* 1 if rand condition met */
    int bit7_state;         /* creature word@0xa bit7 before */
    int delegated;          /* 1 if delegated to 0f3c */
    int clear_bit7;         /* 1 if bit7 was cleared */
} DM2_V1_Skproject14cd1e6eReceipt;

typedef int32_t (*DM2_V1_RandFn)(void *user);

int dm2_v1_skproject_14cd_1e6e_classify(
    int32_t eaxl, int32_t edxl,
    uint16_t v1e0571,
    uint16_t creature_word_0a,
    DM2_V1_ReadTableWordFn read_table_fn,
    DM2_V1_QueryGdatEntryFn query_gdat_fn,
    DM2_V1_RandFn rand_fn,
    void *user,
    DM2_V1_Skproject14cd1e6eReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3739 DM2_14cd_1eec — table walker, matches
   byte@0xc == ecxl, delegates to 0f3c. */
typedef struct {
    int valid;
    int blocked_null_ptr;
    int entries_visited;
    int entries_matched;
    int entries_delegated;
} DM2_V1_Skproject14cd1eecReceipt;

int dm2_v1_skproject_14cd_1eec_classify(
    int32_t eaxl, int32_t edxl, const uint8_t *xebxp, int32_t ecxl,
    uint16_t creature_w8,
    DM2_V1_1316CheckFn check_1316_fn,
    DM2_V1_0f3cDelegateFn delegate_fn,
    void *user,
    DM2_V1_Skproject14cd1eecReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3804 DM2_14cd_1f8b — sign-extends, delegates
   to 1eec with ecxl=0x15. */
typedef struct {
    int valid;
    int32_t sign_ext_eax;
    int32_t sign_ext_edx;
    int ecxl;   /* always 0x15 */
} DM2_V1_Skproject14cd1f8bReceipt;

int dm2_v1_skproject_14cd_1f8b_classify(
    int32_t eaxl, int32_t edxl,
    DM2_V1_Skproject14cd1f8bReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3816 DM2_14cd_1fa7 — packs v1e08d8/d4/d6
   into argw0, delegates to 18f2 with eaxb=0x16. */
typedef struct {
    int valid;
    uint16_t packed_word;
    int32_t sign_ext_edx;
} DM2_V1_Skproject14cd1fa7Receipt;

int dm2_v1_skproject_14cd_1fa7_classify(
    int32_t edxl,
    uint16_t v1e08d8, uint16_t v1e08d4, uint16_t v1e08d6,
    DM2_V1_Skproject14cd1fa7Receipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3841 DM2_14cd_0f0a — 17-way switch dispatcher. */
typedef struct {
    int valid;
    uint8_t sub_index;
    int dispatched;     /* 1 if a case was taken */
    int case_taken;     /* 0-16 or -1 if default */
} DM2_V1_Skproject14cd0f0aReceipt;

int dm2_v1_skproject_14cd_0f0a_classify(
    int32_t eaxl, int32_t edxl, int32_t ebxl,
    DM2_V1_Skproject14cd0f0aReceipt *out_receipt);

/* SKULLWIN/c_ai.cpp:3931 DM2_14cd_0389 — v1e07d8 validity check,
   creature b12/b13 table lookup, dispatches to 0f0a. */
typedef struct {
    int valid;
    int blocked_b00;
    int blocked_b01;
    int blocked_b03;
    int blocked_b12_ff;
    uint8_t creature_b12;
    uint8_t creature_b13;
    uint8_t table_byte5;
    uint8_t table_byte6;
} DM2_V1_Skproject14cd0389Receipt;

typedef const uint8_t *(*DM2_V1_TableLookupFn)(
    int32_t b12_idx, void *user);

int dm2_v1_skproject_14cd_0389_classify(
    uint8_t v1e07d8_b00, uint8_t v1e07d8_b01, int32_t v1e07d8_b03,
    const uint8_t *creature_ptr,
    DM2_V1_TableLookupFn table_fn,
    void *user,
    DM2_V1_Skproject14cd0389Receipt *out_receipt);

/* ---- batch 23b: SKULLWIN/c_ai.cpp DM2_14cd_0457, DM2_14cd_0550,
   DM2_14cd_0276, DM2_14cd_0684, DM2_14cd_08f5, DM2_DECIDE_NEXT_XACT,
   DM2_14cd_0067, DM2_SELECT_CREATURE_37FC ---- */

typedef int16_t (*DM2V1_MinCallback)(int16_t a, int16_t b);
typedef int16_t (*DM2V1_MaxCallback)(int16_t a, int16_t b);
typedef void (*DM2V1_CopyMemoryCallback)(void *dst, const void *src, int32_t len);
typedef int16_t (*DM2V1_Rand16Callback)(int16_t range);
typedef int32_t (*DM2V1_RandCallback)(void);
typedef int16_t (*DM2V1_RandDirCallback)(void);
typedef int8_t (*DM2V1_Call0389Callback)(void);
typedef int32_t (*DM2V1_Call062eCallback)(void);
typedef void (*DM2V1_Call0f0aCallback)(int32_t a, int32_t b, int32_t c, void *d);
typedef int32_t (*DM2V1_QueryGdatCreatureCallback)(int32_t type, int32_t param);
typedef int32_t (*DM2V1_GetGlobVarCallback)(int32_t idx);

typedef struct DM2_V1_Skproject0457Receipt {
    int32_t entries_processed;
    int32_t entries_removed;
    int32_t final_count;
    int32_t initial_count;
    int8_t  b00_value;
} DM2_V1_Skproject0457Receipt;

int dm2_v1_skproject_0457_classify(
    int8_t *plan_entries,
    int32_t *entry_count,
    int8_t b00_value,
    DM2V1_MinCallback min_cb,
    DM2V1_CopyMemoryCallback copy_cb,
    DM2_V1_Skproject0457Receipt *out_receipt);

typedef struct DM2_V1_Skproject0550Receipt {
    int32_t entries_visited;
    int32_t exact_match;
    int32_t random_skipped;
    int32_t entries_dispatched;
    int8_t  match_key;
    int8_t  terminator_hit;
} DM2_V1_Skproject0550Receipt;

int dm2_v1_skproject_0550_classify(
    const int8_t *entry_table,
    int8_t match_key,
    int8_t secondary_key,
    int32_t exact_flag,
    int32_t v1e07ec,
    const int8_t *table1d5f82,
    DM2V1_Rand16Callback rand16_cb,
    DM2V1_Call0f0aCallback call_0f0a_cb,
    DM2_V1_Skproject0550Receipt *out_receipt);

typedef struct DM2_V1_Skproject0276Receipt {
    int8_t  b00_assigned;
    int8_t  b01_assigned;
    int8_t  b02_assigned;
    int8_t  b03_assigned;
    int16_t w04_assigned;
    int16_t w06_assigned;
    int16_t w08_assigned;
    int32_t memory_allocated;
    int32_t alloc_size;
    int32_t xp_0a_set;
} DM2_V1_Skproject0276Receipt;

int dm2_v1_skproject_0276_classify(
    const int8_t *input_struct,
    int16_t v1e054c,
    DM2V1_MaxCallback max_cb,
    DM2_V1_Skproject0276Receipt *out_receipt);

typedef struct DM2_V1_Skproject0684Receipt {
    int32_t called_0389;
    int32_t result_0389;
    int32_t called_062e;
    int32_t called_0550;
    int32_t called_0457;
    int32_t called_find_walk;
    int32_t called_0276;
    int32_t v1e0674_count;
    int8_t  final_xact;
    int32_t table_flag_skip;
    int32_t rand_dir_taken;
    int32_t walk_path_result;
} DM2_V1_Skproject0684Receipt;

int dm2_v1_skproject_0684_classify(
    const int8_t *creatures,
    int16_t v1e0584,
    const int8_t *table1d607e,
    DM2V1_Call0389Callback call_0389_cb,
    DM2V1_RandDirCallback rand_dir_cb,
    DM2V1_Call062eCallback call_062e_cb,
    DM2_V1_Skproject0684Receipt *out_receipt);

typedef struct DM2_V1_Skproject08f5Receipt {
    int8_t  input_eaxl;
    int8_t  vb_00_table_idx;
    int8_t  entry_index;
    int8_t  looked_up_byte;
    int32_t reset_to_ff;
    int32_t advance_result;
    int8_t  new_entry_index;
} DM2_V1_Skproject08f5Receipt;

int dm2_v1_skproject_08f5_classify(
    int32_t eaxl,
    const int8_t *creatures,
    const int8_t *table1d5f82,
    DM2_V1_Skproject08f5Receipt *out_receipt);

typedef struct DM2_V1_SkprojectDecideNextXactReceipt {
    int8_t  table_index;
    int8_t  initial_entry;
    int8_t  final_entry;
    int32_t f6_commands_seen;
    int8_t  chosen_xact;
    int16_t v1e0572_assigned;
    int16_t v1e0574_assigned;
} DM2_V1_SkprojectDecideNextXactReceipt;

int dm2_v1_skproject_decide_next_xact_classify(
    int32_t eaxl,
    const int8_t *creatures,
    const int8_t *table1d5f82,
    DM2_V1_SkprojectDecideNextXactReceipt *out_receipt);

typedef struct DM2_V1_Skproject0067Receipt {
    int16_t creature_flags;
    int32_t rand_value;
    int32_t same_type_as_v1e08d6;
    int32_t flags_modified;
    int16_t final_flags;
    int32_t entries_scanned;
    int16_t selected_behavior;
    int32_t exact_match_found;
    int32_t partial_match_found;
    int32_t glob_var_match;
    int8_t  prev_behavior;
    int32_t behavior_changed;
} DM2_V1_Skproject0067Receipt;

int dm2_v1_skproject_0067_classify(
    const int8_t *behavior_table,
    const int8_t *creatures,
    const int8_t *spx_creature,
    const int8_t *v1e0552,
    int16_t v1e0571,
    int16_t v1e08d6,
    int16_t v1e0584,
    int16_t v1e054c,
    const int8_t *table1d607e,
    DM2V1_RandCallback rand_cb,
    DM2V1_Rand16Callback rand16_cb,
    DM2V1_GetGlobVarCallback glob_var_cb,
    DM2_V1_Skproject0067Receipt *out_receipt);

typedef struct DM2_V1_SkprojectSelectCreature37FCReceipt {
    int32_t queried_gdat;
    int16_t v1e0584_resolved;
    int16_t v1e0586_result;
    int32_t v1e0588_offset;
} DM2_V1_SkprojectSelectCreature37FCReceipt;

int dm2_v1_skproject_select_creature_37fc_classify(
    int16_t v1e0584,
    const int8_t *spx_creature,
    const int8_t **table1d6190,
    DM2V1_QueryGdatCreatureCallback query_gdat_cb,
    DM2_V1_SkprojectSelectCreature37FCReceipt *out_receipt);

/* ---- batch 24a: SKULLWIN/c_ai.cpp DM2_14cd_09e2, DM2_50CB,
   DM2_13e4_0982, DM2_4EA8, DM2_PREPARE_LOCAL_CREATURE_VAR,
   DM2_UNPREPARE_LOCAL_CREATURE_VAR, DM2_ai_13e4_0360, DM2_ai_13e4_071b ---- */

typedef struct {
    int8_t direction;
    int8_t alloc_pool;
    int8_t rand_dir_zero;
    int8_t has_0x40_flag;
    int8_t creature_go_result;
    int8_t direction_is_5;
    int8_t v1e058d_nonzero;
    int8_t finalized;
} DM2_V1_Skproject14cd09e2Receipt;

int32_t dm2_v1_skproject_14cd_09e2_classify(
    int8_t table_byte0, int8_t v1e07ed_initial,
    int8_t rand_dir_result, int8_t creature_go_result,
    int8_t v1e058d,
    DM2_V1_Skproject14cd09e2Receipt *out_receipt);

typedef struct {
    int16_t offset_result;
    int8_t entry_mask_3f;
    int8_t high_nibble;
    int8_t return_code;
    int8_t offset_was_ffff;
} DM2_V1_Skproject50CBReceipt;

int32_t dm2_v1_skproject_50cb_classify(
    int8_t creature_type, int16_t initial_offset,
    int16_t edx_index, const uint8_t *gdat_data,
    int32_t gdat_data_len,
    DM2_V1_Skproject50CBReceipt *out_receipt);

typedef struct {
    int8_t savegame_b03_zero;
    int8_t has_0x10_flag;
    int8_t behavior_0x1a;
    int8_t behavior_0x17;
    int8_t is_type_0x22;
    int8_t action_is_0xff;
    int8_t queue_timer;
    int8_t skip_to_dispatch;
} DM2_V1_Skproject13e40982Receipt;

int32_t dm2_v1_skproject_13e4_0982_classify(
    int8_t savegame_b03, int8_t ai_spec_byte1,
    int8_t creature_byte_0x1a, int8_t creature_byte_0x17,
    int8_t timer_type,
    DM2_V1_Skproject13e40982Receipt *out_receipt);

typedef struct {
    int32_t tick_count;
    int8_t creature_type;
    int16_t start_index;
} DM2_V1_Skproject4EA8Receipt;

int32_t dm2_v1_skproject_4ea8_classify(
    int8_t creature_type, int16_t start_index,
    const uint8_t *gdat_data, int32_t gdat_data_len,
    DM2_V1_Skproject4EA8Receipt *out_receipt);

typedef struct {
    int8_t had_prior_context;
    int8_t creature_index_ff;
    int8_t timer_type_is_0x22;
    int8_t behavior_0x1a_was_ff;
    int16_t record_word;
    int16_t saved_map_index;
} DM2_V1_SkprojectPrepareLocalCreatureVarReceipt;

int32_t dm2_v1_skproject_prepare_local_creature_var_classify(
    int16_t record_word, int8_t edx_byte, int8_t ebx_byte,
    int16_t ecx_word, int16_t prior_map_index,
    int8_t v1e07ea, int8_t creature_record_byte5,
    int8_t creature_byte_0x1a,
    DM2_V1_SkprojectPrepareLocalCreatureVarReceipt *out_receipt);

typedef struct {
    int8_t had_saved_context;
    int8_t restored;
    int8_t cleared_v1e07ea;
} DM2_V1_SkprojectUnprepareLocalCreatureVarReceipt;

int32_t dm2_v1_skproject_unprepare_local_creature_var_classify(
    void *saved_context,
    DM2_V1_SkprojectUnprepareLocalCreatureVarReceipt *out_receipt);

typedef struct {
    int8_t record_was_ffff;
    int8_t creature_found;
    int8_t creature_byte5_ff;
    int8_t behavior_0x17_is_0x13;
    int8_t behavior_0x1a_is_0x13;
    int8_t wrote_behavior;
    int8_t table_0x10_set;
    int8_t argl0_nonzero;
} DM2_V1_Skproject13e40360Receipt;

int32_t dm2_v1_skproject_ai_13e4_0360_classify(
    int16_t record_word, int16_t edx_pos, int16_t ebx_pos,
    int8_t ecx_behavior, int32_t argl0,
    int8_t creature_byte5, int8_t creature_byte_0x17,
    int8_t creature_byte_0x1a, int8_t table_entry,
    DM2_V1_Skproject13e40360Receipt *out_receipt);

typedef struct {
    int8_t early_exit_8001;
    int32_t tick_count;
    int16_t fc0_field;
    int8_t modulo_zero;
    int8_t queued_timer;
} DM2_V1_Skproject13e4071bReceipt;

int32_t dm2_v1_skproject_ai_13e4_071b_classify(
    int16_t v1e055e_word0, int16_t v1e055e_word2,
    int8_t creature_type, int32_t gametick,
    int32_t tick_count_from_4ea8,
    DM2_V1_Skproject13e4071bReceipt *out_receipt);

/* ---- batch 24b: SKULLWIN/c_allegro.cpp dtor, set_mouse, vsync,
   stretchblit, start_timer, stop_timer, hide_mouse;
   SKULLWIN/c_ai.cpp DM2_ai_13e4_0806 ---- */

typedef void (*DM2V1_JoinThreadCallback)(void *thread);
typedef void (*DM2V1_DestroyThreadCallback)(void *thread);
typedef void (*DM2V1_DestroyTimerCallback)(void *timer);
typedef void (*DM2V1_DestroyEventQueueCallback)(void *queue);
typedef void (*DM2V1_DestroyDisplayCallback)(void *display);
typedef void (*DM2V1_SetMouseXYCallback)(void *display, int32_t x, int32_t y);
typedef void (*DM2V1_StartTimerCallback)(void *timer);
typedef void (*DM2V1_StopTimerCallback)(void *timer);
typedef void (*DM2V1_HideMouseCursorCallback)(void *display);
typedef int32_t (*DM2V1_StretchblitLockCallback)(void *display, int32_t *pitch);
typedef void (*DM2V1_StretchblitUnlockCallback)(void *display);
typedef void (*DM2V1_FlipDisplayCallback)(void);
typedef int32_t (*DM2V1_Ai4EA8Callback)(int32_t byte4, int32_t word0);
typedef void (*DM2V1_Ai1c9a0db0Callback)(int32_t val);

typedef struct {
    int8_t thread_joined;
    int8_t thread_destroyed;
    int8_t timer_destroyed;
    int8_t event_queue_destroyed;
    int8_t display_destroyed;
    int8_t timer_was_present;
    int8_t event_queue_was_present;
    int8_t display_was_present;
} DM2_V1_SkprojectDtorReceipt;

int32_t dm2_v1_skproject_dtor_classify(
    void *thread, void *timer, void *event_queue, void *display,
    DM2V1_JoinThreadCallback join_thread_cb,
    DM2V1_DestroyThreadCallback destroy_thread_cb,
    DM2V1_DestroyTimerCallback destroy_timer_cb,
    DM2V1_DestroyEventQueueCallback destroy_eq_cb,
    DM2V1_DestroyDisplayCallback destroy_display_cb,
    DM2_V1_SkprojectDtorReceipt *out_receipt);

typedef struct {
    int16_t input_x;
    int16_t input_y;
    int32_t scaled_x;
    int32_t scaled_y;
} DM2_V1_SkprojectSetMouseReceipt;

int32_t dm2_v1_skproject_set_mouse_classify(
    int16_t x, int16_t y, void *display,
    DM2V1_SetMouseXYCallback set_mouse_cb,
    DM2_V1_SkprojectSetMouseReceipt *out_receipt);

typedef struct {
    int8_t called;
} DM2_V1_SkprojectVsyncReceipt;

int32_t dm2_v1_skproject_vsync_classify(
    DM2_V1_SkprojectVsyncReceipt *out_receipt);

typedef struct {
    int8_t lock_succeeded;
    int8_t blit_completed;
    int8_t unlock_called;
    int8_t flip_called;
    int16_t height;
    int16_t width;
} DM2_V1_SkprojectStretchblitReceipt;

int32_t dm2_v1_skproject_stretchblit_classify(
    int8_t *bptr, int16_t width, int16_t height, void *display,
    DM2V1_StretchblitLockCallback lock_cb,
    DM2V1_StretchblitUnlockCallback unlock_cb,
    DM2V1_FlipDisplayCallback flip_cb,
    DM2_V1_SkprojectStretchblitReceipt *out_receipt);

typedef struct {
    int8_t timer_started;
} DM2_V1_SkprojectStartTimerReceipt;

int32_t dm2_v1_skproject_start_timer_classify(
    void *timer, DM2V1_StartTimerCallback start_cb,
    DM2_V1_SkprojectStartTimerReceipt *out_receipt);

typedef struct {
    int8_t timer_stopped;
} DM2_V1_SkprojectStopTimerReceipt;

int32_t dm2_v1_skproject_stop_timer_classify(
    void *timer, DM2V1_StopTimerCallback stop_cb,
    DM2_V1_SkprojectStopTimerReceipt *out_receipt);

typedef struct {
    int8_t cursor_hidden;
} DM2_V1_SkprojectHideMouseReceipt;

int32_t dm2_v1_skproject_hide_mouse_classify(
    void *display, DM2V1_HideMouseCursorCallback hide_cb,
    DM2_V1_SkprojectHideMouseReceipt *out_receipt);

typedef struct {
    int16_t word0;
    int16_t word2;
    int16_t masked_e000;
    int8_t early_return;
    int8_t tick_mod_zero;
    int32_t computed_4ea8;
    int16_t final_word2;
    int8_t timer_queued;
} DM2_V1_SkprojectAi13e40806Receipt;

int32_t dm2_v1_skproject_ai_13e4_0806_classify(
    int16_t word0, int16_t word2,
    int8_t creature_byte4, int32_t gametick,
    DM2V1_Ai4EA8Callback ai_4ea8_cb,
    DM2V1_Ai1c9a0db0Callback ai_1c9a_0db0_cb,
    DM2_V1_SkprojectAi13e40806Receipt *out_receipt);

/* --- Batch 25a: c_alloc.cpp memory allocation classifiers --- */

typedef void (*DM2V1_RaiseSyserrCallback)(int32_t errorCode);
typedef void (*DM2V1_ZeroMemoryCallback)(void *ptr, int32_t size);

typedef enum {
    DM2_V1_ALLOC_TYPE_FREEPOOL  = 0,
    DM2_V1_ALLOC_TYPE_BIGPOOL_LO = 1,
    DM2_V1_ALLOC_TYPE_BIGPOOL_HI = 2
} DM2_V1_AllocType;

typedef struct {
    int32_t amount;
    int32_t available_before;
    int32_t available_after;
    int8_t  did_subtract;
} DM2_V1_SkprojectGetFromFreepoolReceipt;

int32_t dm2_v1_skproject_get_from_freepool_classify(
    int32_t pool_available,
    int32_t amount,
    DM2_V1_SkprojectGetFromFreepoolReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int16_t wmask;
    int32_t pools_checked;
    int32_t smallest_slack;
    int8_t  found;
} DM2_V1_SkprojectFindFreePoolReceipt;

typedef struct {
    int16_t tag;
    int16_t mode;
    int32_t available;
} DM2_V1_FreepoolEntry;

int32_t dm2_v1_skproject_find_free_pool_classify(
    const DM2_V1_FreepoolEntry *pools,
    int32_t pool_count,
    int32_t amount,
    int16_t wmask,
    int32_t *out_best_index,
    DM2_V1_SkprojectFindFreePoolReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int16_t wmask;
    int16_t wtype_raw;
    int8_t  clean_flag;
    int8_t  amount_was_odd;
    int8_t  route_freepool;
    int8_t  route_secondpool;
    int8_t  route_bigpool_hi;
    int8_t  route_bigpool_lo;
    int8_t  route_syserr;
} DM2_V1_SkprojectAllocMemoryRamReceipt;

typedef struct {
    int32_t bigpool;
    int16_t secondpool_mode;
    int32_t secondpool_available;
} DM2_V1_AllocMemoryRamState;

int32_t dm2_v1_skproject_alloc_memory_ram_classify(
    int32_t amount,
    int16_t wmask,
    int16_t wtype,
    const DM2_V1_AllocMemoryRamState *state,
    const DM2_V1_FreepoolEntry *pools,
    int32_t pool_count,
    DM2_V1_SkprojectAllocMemoryRamReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int8_t  amount_was_odd;
    int32_t adjusted_amount;
} DM2_V1_SkprojectDeallocLobigpoolReceipt;

int32_t dm2_v1_skproject_dealloc_lobigpool_classify(
    int32_t amount,
    DM2_V1_SkprojectDeallocLobigpoolReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int8_t  amount_was_odd;
    int32_t adjusted_amount;
} DM2_V1_SkprojectDeallocHibigpoolReceipt;

int32_t dm2_v1_skproject_dealloc_hibigpool_classify(
    int32_t amount,
    DM2_V1_SkprojectDeallocHibigpoolReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int8_t  clean;
    int16_t composed_wtype;
} DM2_V1_SkprojectAllocFreepoolMemoryReceipt;

int32_t dm2_v1_skproject_alloc_freepool_memory_classify(
    int32_t amount,
    int8_t clean,
    DM2_V1_SkprojectAllocFreepoolMemoryReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int8_t  clean;
    int16_t composed_wtype;
} DM2_V1_SkprojectAllocLobigpoolMemoryReceipt;

int32_t dm2_v1_skproject_alloc_lobigpool_memory_classify(
    int32_t amount,
    int8_t clean,
    DM2_V1_SkprojectAllocLobigpoolMemoryReceipt *out_receipt);

typedef struct {
    int32_t amount;
    int8_t  clean;
    int16_t composed_wtype;
} DM2_V1_SkprojectAllocHibigpoolMemoryReceipt;

int32_t dm2_v1_skproject_alloc_hibigpool_memory_classify(
    int32_t amount,
    int8_t clean,
    DM2_V1_SkprojectAllocHibigpoolMemoryReceipt *out_receipt);

/* --- Batch 25b: c_alloc.cpp memory pool management --- */

typedef struct DM2V1_FreepoolNode {
    int32_t tag;
    int16_t mode;
    int32_t amount;
    int32_t available;
    void *endoffree;
    void *eof_bup;
    int32_t ava_bup;
    struct DM2V1_FreepoolNode *fp_prev;
} DM2V1_FreepoolNode;

typedef void (*DM2V1_RaiseSysErrCallback)(int32_t code);
typedef DM2V1_FreepoolNode *(*DM2V1_GetFreepoolListEndCallback)(void *ctx);
typedef void (*DM2V1_SetFreepoolListEndCallback)(void *ctx, DM2V1_FreepoolNode *node);
typedef void *(*DM2V1_AllocRawMemoryCallback)(int32_t size);
typedef void (*DM2V1_FreeRawMemoryCallback)(void *ptr);

typedef struct DM2_V1_SkprojectTagLargestFreePoolReceipt {
    int8_t list_empty;
    int8_t found_match;
    int32_t largest_amount;
    int8_t tagged_result;
} DM2_V1_SkprojectTagLargestFreePoolReceipt;

int32_t dm2_v1_skproject_tag_largest_free_pool_classify(
    DM2V1_FreepoolNode *freepoollist_end,
    int32_t lmask,
    DM2V1_FreepoolNode **out_result,
    DM2_V1_SkprojectTagLargestFreePoolReceipt *out_receipt);

typedef struct DM2_V1_SkprojectAppendFreePoolReceipt {
    int8_t applied;
    int32_t computed_amount;
} DM2_V1_SkprojectAppendFreePoolReceipt;

int32_t dm2_v1_skproject_append_free_pool_classify(
    DM2V1_FreepoolNode *fpp,
    int16_t mode,
    int32_t amount,
    DM2V1_FreepoolNode *freepoollist_end,
    DM2V1_SetFreepoolListEndCallback set_end_cb,
    void *ctx,
    DM2_V1_SkprojectAppendFreePoolReceipt *out_receipt);

typedef struct DM2_V1_SkprojectAddMemToFreePoolReceipt {
    int8_t too_small;
    int8_t aligned_amount;
    int32_t final_amount;
    int8_t appended;
} DM2_V1_SkprojectAddMemToFreePoolReceipt;

int32_t dm2_v1_skproject_add_mem_to_free_pool_classify(
    DM2V1_FreepoolNode *fpp,
    int32_t lmask,
    int32_t amount,
    DM2V1_FreepoolNode *freepoollist_end,
    DM2V1_SetFreepoolListEndCallback set_end_cb,
    void *ctx,
    DM2_V1_SkprojectAddMemToFreePoolReceipt *out_receipt);

typedef struct DM2_V1_SkprojectBupFreepoolReceipt {
    int32_t nodes_visited;
    int32_t nodes_backed_up;
} DM2_V1_SkprojectBupFreepoolReceipt;

int32_t dm2_v1_skproject_bup_freepool_classify(
    DM2V1_FreepoolNode *freepoollist_end,
    DM2_V1_SkprojectBupFreepoolReceipt *out_receipt);

typedef struct DM2_V1_SkprojectRestoreFreepoolReceipt {
    int32_t nodes_visited;
    int32_t nodes_restored;
} DM2_V1_SkprojectRestoreFreepoolReceipt;

int32_t dm2_v1_skproject_restore_freepool_classify(
    DM2V1_FreepoolNode *freepoollist_end,
    DM2_V1_SkprojectRestoreFreepoolReceipt *out_receipt);

typedef struct DM2_V1_SkprojectCompleteAllocationReceipt {
    int8_t is_allocated_set;
    int8_t first_pool_found;
    int8_t first_pool_null_error;
    int8_t second_pool_found;
    int32_t bigpool_amount;
    int16_t bigpool_mode;
    int16_t secondpool_mode;
    int32_t secondpool_available;
} DM2_V1_SkprojectCompleteAllocationReceipt;

typedef struct DM2_V1_SkprojectCompleteAllocationOut {
    void *bigpool_start;
    void *bigpool_endoffree;
    int32_t bigpool;
    int16_t bigpool_mode;
    void *secondpool_endoffree;
    int32_t secondpool_available;
    int16_t secondpool_mode;
} DM2_V1_SkprojectCompleteAllocationOut;

int32_t dm2_v1_skproject_complete_allocation_classify(
    DM2V1_FreepoolNode *freepoollist_end,
    DM2V1_RaiseSysErrCallback raise_err_cb,
    DM2_V1_SkprojectCompleteAllocationOut *out_alloc,
    DM2_V1_SkprojectCompleteAllocationReceipt *out_receipt);

typedef struct DM2_V1_SkprojectSetupMemoryAllocationReceipt {
    int8_t already_allocated;
    int8_t allocation_failed;
    int32_t num_pools;
    int32_t pool_size;
    int8_t complete_called;
} DM2_V1_SkprojectSetupMemoryAllocationReceipt;

int32_t dm2_v1_skproject_setup_memory_allocation_classify(
    void **allocated_memory,
    int32_t num_freepools,
    int32_t sizeof_freepool,
    int16_t default_mask,
    DM2V1_AllocRawMemoryCallback alloc_cb,
    DM2V1_FreepoolNode **freepoollist_end,
    DM2V1_SetFreepoolListEndCallback set_end_cb,
    void *ctx,
    DM2_V1_SkprojectSetupMemoryAllocationReceipt *out_receipt);

/* SKULLWIN/c_record.cpp:454 DM2_QUERY_CLS1_FROM_RECORD — extract record-type
   class-1 from a 16-bit record word.  Bits 10-13 index table1d3298[16].
   Type 14 chains through record word[2]; chain-follow requires record_pool
   access (not performed here — returns 0xff for type 14 without pool). */
typedef struct {
    int valid;
    uint16_t record_word;
    uint8_t record_type;
    uint8_t cls1;
    int blocked_end_marker;
    int blocked_type_14_no_pool;
} DM2_V1_SkprojectQueryCls1Receipt;

int dm2_v1_skproject_query_cls1_from_record(
    uint16_t record_word,
    uint8_t *out_cls1,
    DM2_V1_SkprojectQueryCls1Receipt *out_receipt);

/* SKULLWIN/c_record.cpp:454 — version with record-pool for type-14 chain. */
int dm2_v1_skproject_query_cls1_from_record_ex(
    uint16_t record_word,
    const struct DM2_V1_RecordPoolSet *pools,
    uint8_t *out_cls1,
    DM2_V1_SkprojectQueryCls1Receipt *out_receipt);

/* SKULLWIN/c_record.cpp:175 DM2_GET_DISTINCTIVE_ITEMTYPE — combine record-type
   bits (via table1d3278) and cls2 into a single 16-bit distinctive type.
   The distinctive type is used throughout DM2 for item identity checks. */
typedef struct {
    int valid;
    uint16_t record_word;
    uint16_t distinctive_type;
    uint8_t record_type;
    uint8_t cls2;
    int blocked_end_marker;
} DM2_V1_SkprojectDistinctiveItemtypeReceipt;

int dm2_v1_skproject_get_distinctive_itemtype(
    uint16_t record_word,
    uint8_t cls2,
    uint16_t *out_type,
    DM2_V1_SkprojectDistinctiveItemtypeReceipt *out_receipt);

/* SKULLWIN/c_record.cpp:203 DM2_QUERY_CLS2_FROM_RECORD — extract record class-2
   from the record data bytes.  The cls2 meaning varies by record type:
   type 4: byte at offset 4; types 5/6/10/15: word[1] & 0x7f;
   type 7: always 0; type 8: (word[1]*2) >> 9; type 9: composite from word[2];
   type 14: chain-follow through word[1].
   Types 2 (text) and 3 (actuator) need sub-functions; this implementation
   blocks on those (returns 0xff). */
typedef struct {
    int valid;
    uint16_t record_word;
    uint8_t cls2;
    uint8_t record_type;
    int blocked_end_marker;
    int blocked_type_2_text;
    int blocked_type_3_actuator;
} DM2_V1_SkprojectQueryCls2Receipt;

int dm2_v1_skproject_query_cls2_from_record(
    uint16_t record_word,
    const struct DM2_V1_RecordPoolSet *pools,
    uint8_t *out_cls2,
    DM2_V1_SkprojectQueryCls2Receipt *out_receipt);

/* SKULLWIN/c_record.cpp:367 DM2_GET_ITEMDB_OF_ITEMSPEC_ACTUATOR — maps an
   actuator itemspec (9-bit value) to its DB (record pool) index.
   Returns the DB index (4-10 range) or 0xffff for out-of-range specs. */
typedef struct {
    int valid;
    uint16_t itemspec;
    uint16_t db;
} DM2_V1_SkprojectItemdbOfItemspecReceipt;

int dm2_v1_skproject_get_itemdb_of_itemspec_actuator(
    uint16_t itemspec,
    uint16_t *out_db,
    DM2_V1_SkprojectItemdbOfItemspecReceipt *out_receipt);

/* SKULLWIN/c_record.cpp:403 DM2_GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR — maps an
   actuator itemspec to a type-local offset within its DB. */
typedef struct {
    int valid;
    uint16_t itemspec;
    uint16_t itemtype;
} DM2_V1_SkprojectItemtypeOfItemspecReceipt;

int dm2_v1_skproject_get_itemtype_of_itemspec_actuator(
    uint16_t itemspec,
    uint16_t *out_itemtype,
    DM2_V1_SkprojectItemtypeOfItemspecReceipt *out_receipt);

/* SKULLWIN/c_record.cpp:449 DM2_QUERY_ITEMDB_FROM_DISTINCTIVE_ITEMTYPE —
   chains DM2_GET_ITEMDB_OF_ITEMSPEC_ACTUATOR -> CLS1 lookup. */
typedef struct {
    int valid;
    uint16_t distinctive_type;
    uint8_t itemdb;
} DM2_V1_SkprojectItemdbFromDistinctiveReceipt;

int dm2_v1_skproject_query_itemdb_from_distinctive_itemtype(
    uint16_t distinctive_type,
    uint8_t *out_itemdb,
    DM2_V1_SkprojectItemdbFromDistinctiveReceipt *out_receipt);

typedef struct DM2_V1_SkprojectDtorMemoryAllocationReceipt {
    int8_t was_allocated;
    int8_t freed;
} DM2_V1_SkprojectDtorMemoryAllocationReceipt;

int32_t dm2_v1_skproject_dtor_memory_allocation_classify(
    void **allocated_memory,
    DM2V1_FreeRawMemoryCallback free_cb,
    DM2_V1_SkprojectDtorMemoryAllocationReceipt *out_receipt);


#endif
