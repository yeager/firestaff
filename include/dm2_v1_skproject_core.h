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

typedef struct {
    uint16_t object_id;
    uint16_t w2;
    uint16_t next_object_id;
    uint16_t contained_object_id;
    uint16_t gdat_word_values[0x36];
    uint8_t container_type;
    uint8_t is_moneybox;
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

/* skproject SKWINSPX v4 SkWinCore::BETWEEN_VALUE clamps newv to
 * [minv,maxv]. SKWINSPX v5 exposes the same behavior as DM2_BETWEEN_VALUE. */
int16_t dm2_v1_skproject_between_value(int16_t minv,
                                       int16_t newv,
                                       int16_t maxv);
int16_t dm2_v1_skproject_dm2_between_value(int16_t minv,
                                           int16_t maxv,
                                           int16_t value);

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

const char *dm2_v1_skproject_core_source_evidence(void);

#endif
