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

#include <stddef.h>
#include <stdint.h>

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_party.h"
#include "dm2_v1_record_pool_pc34_compat.h"

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

/* Source-owned DM2_QUERY_ITEM_WEIGHT over authenticated c_record pools and
 * GDAT. Unlike the callback shim above, this follows a DB9 container's real
 * w2 possession chain and applies the original charge/moneybox rules. It
 * owns no alternate item model and never manufactures a missing record or
 * GDAT value. Source: SKULLWIN/c_item.cpp::DM2_QUERY_ITEM_VALUE (360-498),
 * c_querydb.cpp::DM2_IS_CONTAINER_MONEYBOX (820-845). */
typedef struct {
    int valid;
    int blocked;
    int blocked_record_owner;
    int blocked_chain;
    int blocked_recursion_limit;
    uint16_t root_object_id;
    uint16_t visited_object_count;
    uint16_t contained_object_count;
    uint16_t moneybox_currency_count;
    int32_t final_weight;
    uint32_t source_hash;
} DM2_V1_SourceItemWeightReceipt;

int dm2_v1_query_source_item_weight(
    uint16_t record_word, const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SourceItemWeightReceipt *out_receipt);

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

/* Source-owned DB item -> GDAT name receipt.  This is the authenticated
 * bridge needed by the DM2 leader-hand HUD: c_record.cpp resolves cls1/cls2
 * from the decoded DB record, then c_item.cpp/SkWinCore.cpp queries
 * GDAT dtText/0x18.  It never accepts a bare item index or a host catalog. */
typedef struct {
    uint8_t accepted;
    uint8_t record_type;
    uint8_t cls1;
    uint8_t cls2;
    uint16_t record_word;
    uint8_t blocked_not_item;
    uint8_t blocked_record_owner;
    uint8_t blocked_classification;
    DM2_V1_GdatNameReceipt gdat;
} DM2_V1_SourceItemNameReceipt;

int dm2_v1_query_source_item_name_receipt(
    uint16_t record_word,
    const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SourceItemNameReceipt *out_receipt);

/* ------------------------------------------------------------------ */
/* SKSAVE direct-root item-bonus phase.
 *
 * SKProject SKULLWIN/c_savegame.cpp::DM2_READ_SKSAVE_DUNGEON processes
 * every just-restored c_hero::item root, then the leader hand, with
 * DM2_PROCESS_ITEM_BONUS(..., mode 0).  An OBJECT_END (0xfffe) root is
 * converted to OBJECT_NULL (0xffff) instead.  This bridge uses the decoded
 * source record pool to derive cls1/cls2 and the supplied original GDAT to
 * answer DBSPEC and equipment queries.  It owns no alternate item table.
 *
 * It is deliberately separate from the temporary map preflight because a
 * caller must hold the authenticated GRAPHICS.DAT loader for this original
 * GDAT phase.  Missing record ownership, classification, or required body
 * equipment data rejects the whole operation rather than inventing a bonus.
 */
typedef struct {
    int valid;
    int blocked;
    /* Source-root provenance when PROCESS_ITEM_BONUS rejects a decoded
     * c_hero item or the leader hand.  -2 means that no individual root was
     * reached (for example, a missing pool owner); a slot of -1 identifies
     * the leader hand and still retains its authenticated hero owner. These
     * fields are diagnostic only and never relax the
     * all-or-nothing SKSAVE transaction. */
    int16_t failed_hero_index;
    int16_t failed_item_slot;
    uint16_t failed_record_word;
    uint16_t processed_item_roots;
    uint16_t empty_item_roots;
    uint16_t processed_leader_hand;
    uint16_t empty_leader_hand;
    uint32_t source_hash;
} DM2_V1_SksaveItemBonusReceipt;

int dm2_v1_sksave_process_source_item_bonus_roots(
    DM2_V1_Hero *heroes, size_t hero_capacity, uint16_t hero_count,
    int16_t leader_hero_index,
    uint16_t *leader_hand_root, const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SksaveItemBonusReceipt *out_receipt);

/* c_hero.cpp::DM2_EQUIP_ITEM_TO_HAND invokes PROCESS_ITEM_BONUS with mode 1
 * while SELECT_CHAMPION transfers each original mirror possession. This
 * applies that same source operation to a private New Game c_party. Unlike
 * the SKSAVE mode-0 preflight, it mutates only the supplied RAM heroes. */
int dm2_v1_new_game_apply_source_item_bonuses(
    DM2_V1_Party *party, const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SksaveItemBonusReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ITEM_OPS_PC34_COMPAT_H */
