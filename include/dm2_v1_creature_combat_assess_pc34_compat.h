#ifndef FIRESTAFF_DM2_V1_CREATURE_COMBAT_ASSESS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_COMBAT_ASSESS_PC34_COMPAT_H

/*
 * dm2_v1_creature_combat_assess_pc34_compat.h — DM2 creature combat
 * assessment helpers.
 *
 * Ports five functions from skproject/SKULLWIN/c_ai.cpp that evaluate
 * creature combat capability: item usability checks, possession chain
 * stat accumulation, recursive item counting, and AI action slot
 * resolution.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Callback typedefs                                                   */
/* ------------------------------------------------------------------ */

/* GET_CREATURE_AT(x, y) -> creature record handle, or 0xFFFF if none.
 * Source: c_ai.cpp:439 */
typedef uint16_t (*DM2_V1_GetCreatureAtFn)(void *ctx,
                                            int16_t x, int16_t y);

/* GET_ADDRESS_OF_RECORD(handle) -> pointer to record bytes, or NULL.
 * Source: c_record.cpp:44-52 */
typedef uint8_t *(*DM2_V1_GetAddressOfRecordFn)(void *ctx,
                                                 uint16_t handle);

/* GET_NEXT_RECORD_LINK(handle) -> next handle in chain, 0xFFFE = end.
 * Source: c_record.cpp:54-57 */
typedef uint16_t (*DM2_V1_GetNextRecordLinkFn)(void *ctx,
                                                uint16_t handle);

/* CREATURE_CAN_HANDLE_IT(item_handle, type) -> nonzero if yes.
 * Source: c_ai.cpp:484 */
typedef int (*DM2_V1_CreatureCanHandleItFn)(void *ctx,
                                             uint16_t item_handle,
                                             int32_t type);

/* GET_DISTINCTIVE_ITEMTYPE(handle) -> distinctive type id.
 * Source: c_ai.cpp:50 */
typedef int16_t (*DM2_V1_GetDistinctiveItemTypeFn)(void *ctx,
                                                     uint16_t handle);

/* ADD_ITEM_CHARGE(handle, 0) -> charge value or -1.
 * Source: c_ai.cpp:41 */
typedef int16_t (*DM2_V1_AddItemChargeFn)(void *ctx,
                                            uint16_t handle,
                                            int32_t amount);

/* query_48ae_05ae — score accumulation query.
 * Source: c_ai.cpp:50 */
typedef int16_t (*DM2_V1_QueryCombatStatFn)(void *ctx,
                                              int32_t distinctive_type,
                                              int32_t creature_byte4,
                                              int32_t creature_word8,
                                              int32_t stat_type,
                                              int32_t param4,
                                              int32_t charge);

/* IS_CONTAINER_CHEST(handle) -> nonzero if container.
 * Source: c_ai.cpp:1230 */
typedef int (*DM2_V1_IsContainerChestFn)(void *ctx, uint16_t handle);

/* ------------------------------------------------------------------ */
/* DM2_14cd_2662 — check creature ahead for usable items               */
/* c_ai.cpp:400-516                                                    */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t creature_x;
    int16_t creature_y;
    uint8_t direction_byte;          /* 0xFF = any direction */
    const uint8_t *creature_record;  /* s350.v1e054e, 16+ bytes */
    /* Direction offset tables (4 entries each, indexed by facing) */
    const int16_t *dx_table;         /* table1d27fc */
    const int16_t *dy_table;         /* table1d2804 */
    /* Callbacks */
    DM2_V1_GetCreatureAtFn get_creature_at;
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_GetNextRecordLinkFn get_next_record_link;
    DM2_V1_CreatureCanHandleItFn creature_can_handle_it;
    void *ctx;
} DM2_V1_CreatureHasUsableItemAheadRequest;

typedef struct {
    int valid;
    int fail_closed;
    int result;  /* 1 = found usable item, 0 = not found */
} DM2_V1_CreatureHasUsableItemAheadReceipt;

int dm2_v1_creature_has_usable_item_ahead(
    const DM2_V1_CreatureHasUsableItemAheadRequest *request,
    DM2_V1_CreatureHasUsableItemAheadReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_2886 — assess combat stat via possession chain             */
/* c_ai.cpp:55-74 (with DM2_14cd_2807 callback at lines 20-53)        */
/* ------------------------------------------------------------------ */

/* OVERSEE_RECORD callback context passed as vwa_00 array */
typedef struct {
    int16_t creature_handle;  /* first record in possession chain */
    uint8_t direction_filter; /* RG2Blo from caller */
    /* Stat query parameters */
    int16_t stat_type;        /* vwa_00[2] = ecxl */
    int16_t param3;           /* vwa_00[3] = argl0 */
    int16_t param4;           /* vwa_00[4] = argl1 */
    /* Creature context (s350.v1e054e) */
    const uint8_t *creature_record;
    /* Callbacks */
    DM2_V1_CreatureCanHandleItFn creature_can_handle_it;
    DM2_V1_GetDistinctiveItemTypeFn get_distinctive_itemtype;
    DM2_V1_AddItemChargeFn add_item_charge;
    DM2_V1_QueryCombatStatFn query_combat_stat;
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_GetNextRecordLinkFn get_next_record_link;
    void *ctx;
} DM2_V1_CreatureAssessCombatStatRequest;

typedef struct {
    int valid;
    int fail_closed;
    int16_t accumulated_score;
} DM2_V1_CreatureAssessCombatStatReceipt;

int dm2_v1_creature_assess_combat_stat(
    const DM2_V1_CreatureAssessCombatStatRequest *request,
    DM2_V1_CreatureAssessCombatStatReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_14cd_102e — recursive item counter in possession chain          */
/* c_ai.cpp:1179-1282                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
    int16_t item_type_filter;    /* vql_00 — type to match against */
    uint16_t first_handle;       /* edxl — first record in chain */
    uint8_t direction_filter;    /* ebxl — 0xFF = any direction */
    int32_t recurse_containers;  /* ecxl — nonzero to recurse into type 4 */
    int32_t recurse_chests;      /* argl0 — nonzero to also recurse chests */
    /* Callbacks */
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_GetNextRecordLinkFn get_next_record_link;
    DM2_V1_CreatureCanHandleItFn creature_can_handle_it;
    DM2_V1_IsContainerChestFn is_container_chest;
    void *ctx;
} DM2_V1_CountItemsInChainRequest;

typedef struct {
    int valid;
    int fail_closed;
    int32_t count;
} DM2_V1_CountItemsInChainReceipt;

int dm2_v1_count_items_in_chain(
    const DM2_V1_CountItemsInChainRequest *request,
    DM2_V1_CountItemsInChainReceipt *receipt);

/* ------------------------------------------------------------------ */
/* DM2_ai_14cd_10d2 — AI action slot resolver                         */
/* c_ai.cpp:1285-1467                                                  */
/* ------------------------------------------------------------------ */

/* Each slot is 0x20 (32) bytes in s350.v1e058e; max 4 slots.
 * Slot layout: [pointer (sizeof(void*))][action_type byte][status bytes...]
 * On 32-bit skproject the pointer is 4 bytes; on 64-bit hosts it is 8. */
#define DM2_V1_AI_ACTION_SLOT_SIZE  32
#define DM2_V1_AI_ACTION_SLOT_COUNT  4
#define DM2_V1_AI_ACTION_SLOT_TYPE_OFF ((int)sizeof(void *))

typedef struct {
    uint8_t ai_action_slots[DM2_V1_AI_ACTION_SLOT_COUNT *
                            DM2_V1_AI_ACTION_SLOT_SIZE];
    int needs_clear;  /* s350.v1e058c */
} DM2_V1_AiActionSlotTable;

typedef struct {
    uint8_t *hexe_entry;         /* xeaxp — pointer to hexe table entry */
    int32_t action_type;         /* edxl — action type byte to match */
    DM2_V1_AiActionSlotTable *slot_table;
    /* For DM2_14cd_102e sub-calls */
    const uint8_t *creature_record;  /* s350.v1e054e */
    DM2_V1_GetAddressOfRecordFn get_address_of_record;
    DM2_V1_GetNextRecordLinkFn get_next_record_link;
    DM2_V1_CreatureCanHandleItFn creature_can_handle_it;
    DM2_V1_IsContainerChestFn is_container_chest;
    void *ctx;
} DM2_V1_AiActionSlotResolveRequest;

typedef struct {
    int valid;
    int fail_closed;
    uint8_t *slot_ptr;  /* pointer into slot_table, or NULL */
    int allocated_new;  /* 1 if a new slot was allocated */
} DM2_V1_AiActionSlotResolveReceipt;

int dm2_v1_ai_action_slot_resolve(
    const DM2_V1_AiActionSlotResolveRequest *request,
    DM2_V1_AiActionSlotResolveReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_COMBAT_ASSESS_PC34_COMPAT_H */
