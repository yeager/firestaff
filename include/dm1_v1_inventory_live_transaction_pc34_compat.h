#ifndef FIRESTAFF_DM1_V1_INVENTORY_LIVE_TRANSACTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_LIVE_TRANSACTION_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_chest_admission_f0333_f0334_pc34_compat.h"
#include "dm1_v1_inventory_consumables_pc34_compat.h"
#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A live inventory panel keeps Thing handles, never copied host item records.
 * C00..C29 are champion slots and C30..C37 are the F0333 chest projection. */
typedef struct DM1_V1_InventoryLiveTransactionPc34 {
    struct DungeonThings_Compat *things;
    uint16_t slots[DM1_PC34_SLOT_COUNT];
    uint16_t mouseThing;
    uint16_t openChestThing;
    uint32_t generation;
} DM1_V1_InventoryLiveTransactionPc34;

typedef enum DM1_V1_InventoryLiveUseKindPc34 {
    DM1_V1_INVENTORY_LIVE_USE_NONE_PC34 = 0,
    DM1_V1_INVENTORY_LIVE_USE_CHEST_PC34,
    DM1_V1_INVENTORY_LIVE_USE_SCROLL_PC34,
    DM1_V1_INVENTORY_LIVE_USE_CONSUMABLE_PC34
} DM1_V1_InventoryLiveUseKindPc34;

typedef struct DM1_V1_InventoryLiveUseReceiptPc34 {
    int valid;
    DM1_V1_InventoryLiveUseKindPc34 kind;
    uint16_t thing;
    int iconIndex;
    int objectInfoIndex;
    int scrollTextIndex;
    DM1ConsumableResultPc34 consumable;
    const char *sourceEvidence;
} DM1_V1_InventoryLiveUseReceiptPc34;

/* Initializes C00..C29 from authenticated raw Things.  Any unknown, stale,
 * or non-inventory Thing rejects the whole panel rather than synthesizing an
 * object record.  Empty slots must be THING_NONE. */
int dm1_v1_inventory_live_begin_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    struct DungeonThings_Compat *things,
    const uint16_t championSlots[DM1_PC34_INVENTORY_SLOT_COUNT]);

/* F0302/F0300/F0301 click transaction.  The source slot and its raw Thing
 * must still agree; moves, swaps, equipment masks, and chest cells either
 * commit together or leave the panel unchanged. */
int dm1_v1_inventory_live_click_slot_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    int pc34Slot);

/* F0333/F0334 projection and rewrite. */
int dm1_v1_inventory_live_open_chest_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state, uint16_t containerThing,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt);
int dm1_v1_inventory_live_close_chest_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt);

/* F0341/F0349 dispatch for the action hand.  Scrolls expose only their raw
 * text-record index; no substitute text/font is produced.  Food, water, and
 * potions consume their real C08/C10 bytes atomically. */
int dm1_v1_inventory_live_use_action_hand_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    DM1ConsumableChampionPc34 *champion,
    const uint16_t *woundRandomMasks, int woundRandomMaskCount,
    DM1_V1_InventoryLiveUseReceiptPc34 *outReceipt);

const char *dm1_v1_inventory_live_transaction_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
