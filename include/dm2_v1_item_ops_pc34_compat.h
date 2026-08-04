#ifndef FIRESTAFF_DM2_V1_ITEM_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ITEM_OPS_PC34_COMPAT_H

/*
 * dm2_v1_item_ops_pc34_compat.h — DM2 V1 item operations from
 * skproject/SKULLWIN/c_item.cpp.
 *
 * Callback-based implementations of:
 *   DM2_F958                         c_item.cpp:1034
 *   DM2_IS_MISCITEM_DRINK_WATER      c_item.cpp:528
 *   DM2_TAKE_OBJECT                  c_item.cpp:1185
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_F958 (c_item.cpp:1034) ----
 * Query item value category 2; clamp to <= -1.
 * Returns the item value if negative, else -1. */
typedef struct {
    int16_t (*query_item_value)(void *ctx, uint16_t record_word, int category);
} DM2_V1_ItemValueCallbacks;

int16_t dm2_v1_f958(uint16_t record_word,
                     const DM2_V1_ItemValueCallbacks *cb, void *ctx);

/* ---- DM2_IS_MISCITEM_DRINK_WATER (c_item.cpp:528) ----
 * Check if item is drinkable water; if so, consume one charge.
 * Returns 1 if water was consumed, 0 otherwise. */
typedef struct {
    int16_t (*query_gdat_dbspec_word)(void *ctx, uint16_t rw, uint8_t idx);
    int16_t (*add_item_charge)(void *ctx, uint16_t rw, int16_t delta);
    uint16_t item_in_hand;
    void (*retake_object)(void *ctx, uint16_t rw);
} DM2_V1_DrinkWaterCallbacks;

int dm2_v1_is_miscitem_drink_water(
    uint16_t record_word,
    const DM2_V1_DrinkWaterCallbacks *cb, void *ctx);

/* ---- DM2_TAKE_OBJECT (c_item.cpp:1185) ----
 * Pick up an item into the player's hand with UI and stat updates.
 * deferred: 0 = process events immediately, nonzero = defer. */
typedef struct {
    int16_t (*query_gdat_dbspec_word)(void *ctx, uint16_t rw, uint8_t idx);
    int16_t (*query_item_weight)(void *ctx, uint16_t rw);
    void (*set_hand_item)(void *ctx, uint16_t rw, int16_t gdat_word, int16_t weight);
    void (*draw_item_in_hand)(void *ctx);
    void (*display_item_name)(void *ctx, uint16_t rw);
    void (*process_events)(void *ctx);
    void (*set_deferred_flag)(void *ctx);
    void (*process_item_bonus)(void *ctx, uint16_t rw);
    void (*moverec_update)(void *ctx);
} DM2_V1_TakeObjectCallbacks;

void dm2_v1_take_object(
    uint16_t record_word, int deferred,
    const DM2_V1_TakeObjectCallbacks *cb, void *ctx);

/* ---- DM2_RETRIEVE_ITEM_BONUS (c_item.cpp:22) ----
 * Query an item bonus value from GDAT; applies sign based on context. */
typedef struct {
    int16_t (*query_gdat_dbspec_word)(void *ctx, uint16_t rw, uint8_t idx);
} DM2_V1_ItemBonusCallbacks;

int16_t dm2_v1_retrieve_item_bonus(
    uint16_t record_word, uint8_t bonus_idx,
    int16_t equipped, int16_t context,
    const DM2_V1_ItemBonusCallbacks *cb, void *ctx);

/* ---- DM2_GET_MAX_CHARGE (c_item.cpp:344) ----
 * Get the maximum charge capacity for an item based on db type. */
int16_t dm2_v1_get_max_charge(uint16_t record_word);

/* ---- DM2_ADD_ITEM_CHARGE (c_item.cpp:251) ----
 * Read or modify the charge count of an item. delta=0 reads current. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
} DM2_V1_ChargeCallbacks;

int16_t dm2_v1_add_item_charge(
    uint16_t record_word, int16_t delta,
    const DM2_V1_ChargeCallbacks *cb, void *ctx);

/* ---- DM2_QUERY_ITEM_WEIGHT (c_item.cpp:496) ----
 * Query the weight of an item (alias for query_item_value category 1). */
int16_t dm2_v1_query_item_weight(
    uint16_t record_word,
    const DM2_V1_ItemValueCallbacks *cb, void *ctx);

/* ---- DM2_GET_ITEM_NAME (c_item.cpp:502) ----
 * Get the name of an item by querying cls1/cls2 and GDAT. */
typedef struct {
    uint8_t (*query_cls1)(void *ctx, uint16_t rw);
    uint8_t (*query_cls2)(void *ctx, uint16_t rw);
    const uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    const char *(*query_gdat_item_name)(void *ctx, uint8_t cls1, uint8_t cls2);
    uint16_t heros_in_party;
} DM2_V1_ItemNameCallbacks;

typedef struct {
    const char *name;
    int16_t hero_index;  /* -1 if not a hero bones item */
} DM2_V1_ItemNameReceipt;

DM2_V1_ItemNameReceipt dm2_v1_get_item_name(
    uint16_t record_word,
    const DM2_V1_ItemNameCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ITEM_OPS_PC34_COMPAT_H */
