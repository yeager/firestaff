#ifndef FIRESTAFF_DM2_V1_SKPROJECT_CORE_H
#define FIRESTAFF_DM2_V1_SKPROJECT_CORE_H

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

typedef struct {
    int valid;
    int blocked_missing_text;
    uint16_t text_len;
    int16_t width;
    int16_t height;
} DM2_V1_SkprojectTextMetricsReceipt;

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
int dm2_v1_skproject_alloc_new_pict(
    uint16_t index,
    uint16_t width,
    uint16_t height,
    uint16_t bpp,
    DM2_V1_SkprojectNewPictReceipt *out_receipt);
uint32_t dm2_v1_skproject_calc_pict_ent_hash(
    const DM2_V1_SkprojectExtendedPictureRef *ref);
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

const char *dm2_v1_skproject_core_source_evidence(void);

#endif
