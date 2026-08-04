/*
 * dm2_v1_creature_combat_assess_pc34_compat.c — DM2 creature combat
 * assessment helpers.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include "dm2_v1_creature_combat_assess_pc34_compat.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* DM2_14cd_2662 — check creature ahead for usable items               */
/* c_ai.cpp:400-516                                                    */
/* ------------------------------------------------------------------ */

int dm2_v1_creature_has_usable_item_ahead(
    const DM2_V1_CreatureHasUsableItemAheadRequest *request,
    DM2_V1_CreatureHasUsableItemAheadReceipt *receipt)
{
    uint8_t dir_byte;
    int16_t facing;
    int16_t target_x, target_y;
    uint16_t creature_handle;
    uint8_t *creature_rec;
    uint16_t link;
    int budget;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* Fail-closed: all callbacks required */
    if (!request->get_creature_at ||
        !request->get_address_of_record ||
        !request->get_next_record_link ||
        !request->creature_can_handle_it ||
        !request->creature_record ||
        !request->dx_table ||
        !request->dy_table) {
        receipt->fail_closed = 1;
        return 0;
    }

    dir_byte = request->direction_byte;

    /* c_ai.cpp:415-428 — if direction_byte != 0xFF, adjust by creature
     * facing extracted from record word at 0xe bits 8-9 (+2, &3) */
    if (dir_byte != 0xFF) {
        uint16_t w0e = (uint16_t)request->creature_record[0x0e] |
                       ((uint16_t)request->creature_record[0x0f] << 8);
        /* c_ai.cpp:419-420 — (w0e << 6) >> 14 = bits 8-9 */
        int16_t creature_facing = (int16_t)((w0e >> 8) & 3);
        /* c_ai.cpp:421-424 — facing + 2 + dir_byte, &3 */
        int32_t adjusted = (int32_t)creature_facing + 2 + (int32_t)dir_byte;
        dir_byte = (uint8_t)(adjusted & 3);
    }

    /* c_ai.cpp:429-433 — compute facing from creature record,
     * then target = creature_pos + delta[facing] */
    {
        uint16_t w0e = (uint16_t)request->creature_record[0x0e] |
                       ((uint16_t)request->creature_record[0x0f] << 8);
        facing = (int16_t)((w0e >> 8) & 3);
    }
    target_x = (int16_t)(request->creature_x + request->dx_table[facing]);
    target_y = (int16_t)(request->creature_y + request->dy_table[facing]);

    /* c_ai.cpp:439 — GET_CREATURE_AT */
    creature_handle = request->get_creature_at(request->ctx,
                                                target_x, target_y);
    if (creature_handle == 0xFFFFu || creature_handle == 0xFFFEu) {
        receipt->result = 0;
        return 1;
    }

    /* c_ai.cpp:442 — GET_ADDRESS_OF_RECORD, read first possession at +2 */
    creature_rec = request->get_address_of_record(request->ctx,
                                                   creature_handle);
    if (!creature_rec) {
        receipt->result = 0;
        return 1;
    }
    link = (uint16_t)creature_rec[2] | ((uint16_t)creature_rec[3] << 8);

    /* c_ai.cpp:445-508 — walk possession chain */
    budget = 256;
    while (link != 0xFFFEu && --budget > 0) {
        int16_t record_type;
        int matched_type;
        int matched_dir;

        /* c_ai.cpp:452-454 — extract record type: (link & 0x3C00) >> 10 */
        record_type = (int16_t)((link & 0x3C00u) >> 10);

        matched_type = 0;
        /* c_ai.cpp:456-463 — type 5..13 or type 9 */
        if (record_type > 4 && record_type < 14)
            matched_type = 1;
        else if (record_type == 9)
            matched_type = 1;

        if (matched_type) {
            /* c_ai.cpp:469-477 — direction filter */
            matched_dir = 0;
            if (dir_byte == 0xFF) {
                matched_dir = 1;
            } else {
                uint16_t item_dir = (link >> 14) & 3;
                if ((uint8_t)dir_byte == (uint8_t)item_dir)
                    matched_dir = 1;
            }

            if (matched_dir) {
                /* c_ai.cpp:482-498 — CREATURE_CAN_HANDLE_IT with type 0x10,
                 * then 0x07. If neither can handle, result=1 (found unusable) */
                uint16_t item_handle = link & 0x3FFFu;
                int can16 = request->creature_can_handle_it(
                    request->ctx, item_handle, 0x10);
                if (can16 == 0) {
                    int can7 = request->creature_can_handle_it(
                        request->ctx, item_handle, 0x07);
                    if (can7 == 0) {
                        /* c_ai.cpp:491-493 — found item creature can't handle */
                        receipt->result = 1;
                        return 1;
                    }
                }
                /* c_ai.cpp:497 — creature can handle it, continue */
            }
        }

        /* c_ai.cpp:506-507 — GET_NEXT_RECORD_LINK */
        link = request->get_next_record_link(request->ctx,
                                              link & 0x3FFFu);
    }

    /* c_ai.cpp:511-514 — return 1 if we broke out (link != 0xFFFE),
     * 0 if we exhausted the chain */
    receipt->result = (link != 0xFFFEu) ? 1 : 0;
    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_2886 — assess combat stat via OVERSEE_RECORD pattern       */
/* c_ai.cpp:55-74 (DM2_14cd_2886)                                     */
/* c_ai.cpp:20-53 (DM2_14cd_2807 callback)                            */
/* ------------------------------------------------------------------ */

/* Internal: the DM2_14cd_2807 callback logic applied to one item */
static int16_t assess_one_item(
    const DM2_V1_CreatureAssessCombatStatRequest *req,
    uint16_t item_handle,
    int16_t *accumulator,
    int16_t check_charges_flag)
{
    int16_t charge_val;
    int16_t distinctive_type;
    int16_t score;
    int32_t creature_byte4;
    int32_t creature_word8;

    /* c_ai.cpp:32 — CREATURE_CAN_HANDLE_IT(item, stat_type) */
    if (req->creature_can_handle_it(req->ctx, item_handle,
                                     (int32_t)req->stat_type) == 0)
        return 0;

    /* c_ai.cpp:34-35 — first call sets accumulator from -1 to 0 */
    if (*accumulator == -1)
        *accumulator = 0;

    /* c_ai.cpp:37-41 — charge query */
    if (check_charges_flag == 0)
        charge_val = -1;
    else
        charge_val = req->add_item_charge(req->ctx, item_handle, 0);

    /* c_ai.cpp:47-50 — distinctive type + query_48ae_05ae */
    distinctive_type = req->get_distinctive_itemtype(req->ctx, item_handle);
    creature_byte4 = (int32_t)req->creature_record[0x04];
    creature_word8 = (int32_t)((int16_t)((uint16_t)req->creature_record[0x08] |
                     ((uint16_t)req->creature_record[0x09] << 8)));
    score = req->query_combat_stat(req->ctx,
                                    (int32_t)distinctive_type,
                                    creature_byte4,
                                    creature_word8,
                                    (int32_t)req->stat_type,
                                    (int32_t)req->param3,
                                    (int32_t)charge_val);

    /* c_ai.cpp:51 — accumulate */
    *accumulator = (int16_t)(*accumulator + score);
    return 0;
}

int dm2_v1_creature_assess_combat_stat(
    const DM2_V1_CreatureAssessCombatStatRequest *request,
    DM2_V1_CreatureAssessCombatStatReceipt *receipt)
{
    uint16_t link;
    int16_t accumulator;
    int budget;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!request->creature_can_handle_it ||
        !request->get_distinctive_itemtype ||
        !request->add_item_charge ||
        !request->query_combat_stat ||
        !request->get_address_of_record ||
        !request->get_next_record_link ||
        !request->creature_record) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:64 — accumulator starts at -1 (0xFFFF) */
    accumulator = -1;

    /* c_ai.cpp:72 — OVERSEE_RECORD walks the possession chain from
     * creature_handle with direction_filter. We inline that walk here. */
    {
        uint8_t *first_rec = request->get_address_of_record(
            request->ctx, (uint16_t)request->creature_handle);
        if (!first_rec) {
            receipt->fail_closed = 1;
            return 0;
        }
        link = (uint16_t)first_rec[2] | ((uint16_t)first_rec[3] << 8);
    }

    budget = 256;
    while (link != 0xFFFEu && --budget > 0) {
        int16_t record_type = (int16_t)((link & 0x3C00u) >> 10);

        /* OVERSEE_RECORD with flag=1 filters to types 5..13 */
        if (record_type > 4 && record_type < 14) {
            /* Direction filter */
            int dir_ok = 0;
            if (request->direction_filter == 0xFF) {
                dir_ok = 1;
            } else {
                uint8_t item_dir = (uint8_t)((link >> 14) & 3);
                if (request->direction_filter == item_dir)
                    dir_ok = 1;
            }
            if (dir_ok) {
                assess_one_item(request, link & 0x3FFFu,
                                &accumulator, request->param4);
            }
        }

        link = request->get_next_record_link(request->ctx,
                                              link & 0x3FFFu);
    }

    /* c_ai.cpp:73 — return accumulator */
    receipt->accumulated_score = accumulator;
    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_102e — recursive item counter                              */
/* c_ai.cpp:1179-1282                                                  */
/* ------------------------------------------------------------------ */

static int32_t count_items_recursive(
    const DM2_V1_CountItemsInChainRequest *req,
    uint16_t handle,
    int depth)
{
    int32_t count = 0;
    int budget = 256;

    if (depth > 16) return 0;  /* safety limit */

    /* c_ai.cpp:1199-1282 — walk chain */
    while (handle != 0xFFFEu && --budget > 0) {
        int16_t record_type = (int16_t)((handle & 0x3C00u) >> 10);
        int did_recurse = 0;

        /* c_ai.cpp:1209-1222 — recurse into type 4 (container) */
        if (req->recurse_containers != 0 && record_type == 4) {
            uint8_t *rec = req->get_address_of_record(req->ctx,
                                                       handle & 0x3FFFu);
            if (rec) {
                uint16_t child = (uint16_t)rec[2] |
                                 ((uint16_t)rec[3] << 8);
                count += count_items_recursive(req, child, depth + 1);
            }
            did_recurse = 1;
        }

        /* c_ai.cpp:1227-1237 — recurse into chest containers */
        if (!did_recurse && req->recurse_chests != 0) {
            if (req->is_container_chest &&
                req->is_container_chest(req->ctx, handle & 0x3FFFu)) {
                uint8_t *rec = req->get_address_of_record(req->ctx,
                                                           handle & 0x3FFFu);
                if (rec) {
                    uint16_t child = (uint16_t)rec[2] |
                                     ((uint16_t)rec[3] << 8);
                    count += count_items_recursive(req, child, depth + 1);
                }
            }
        }

        /* c_ai.cpp:1255-1276 — count items with type 5..13, direction match,
         * and CREATURE_CAN_HANDLE_IT */
        if (record_type > 4 && record_type < 14) {
            int dir_ok = 0;
            if (req->direction_filter == 0xFF) {
                dir_ok = 1;
            } else {
                uint8_t item_dir = (uint8_t)((handle >> 14) & 3);
                if (req->direction_filter == item_dir)
                    dir_ok = 1;
            }
            if (dir_ok) {
                /* c_ai.cpp:1272-1274 */
                if (req->creature_can_handle_it(
                        req->ctx, handle & 0x3FFFu,
                        (int32_t)req->item_type_filter) != 0) {
                    count++;
                }
            }
        }

        /* c_ai.cpp:1279-1280 — next in chain */
        handle = req->get_next_record_link(req->ctx, handle & 0x3FFFu);
    }

    return count;
}

int dm2_v1_count_items_in_chain(
    const DM2_V1_CountItemsInChainRequest *request,
    DM2_V1_CountItemsInChainReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!request->get_address_of_record ||
        !request->get_next_record_link ||
        !request->creature_can_handle_it) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->count = count_items_recursive(request,
                                            request->first_handle, 0);
    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_ai_14cd_10d2 — AI action slot resolver                         */
/* c_ai.cpp:1285-1467                                                  */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_action_slot_resolve(
    const DM2_V1_AiActionSlotResolveRequest *request,
    DM2_V1_AiActionSlotResolveReceipt *receipt)
{
    int i;
    uint8_t *slot;
    uint8_t *free_slot;
    DM2_V1_AiActionSlotTable *table;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!request->slot_table || !request->hexe_entry) {
        receipt->fail_closed = 1;
        return 0;
    }

    table = request->slot_table;

    /* c_ai.cpp:1312-1316 — clear table if needs_clear flag set */
    if (table->needs_clear) {
        memset(table->ai_action_slots, 0, sizeof(table->ai_action_slots));
        table->needs_clear = 0;
    }

    /* c_ai.cpp:1325-1362 — scan existing slots for match or free slot */
    free_slot = NULL;
    for (i = 0; i < DM2_V1_AI_ACTION_SLOT_COUNT; i++) {
        slot = &table->ai_action_slots[i * DM2_V1_AI_ACTION_SLOT_SIZE];

        /* c_ai.cpp:1347 — check pointer at slot[0..3] */
        uint8_t *stored_ptr;
        memcpy(&stored_ptr, slot, sizeof(uint8_t *));

        if (stored_ptr != NULL) {
            /* c_ai.cpp:1350-1354 — match: same hexe_entry and action_type */
            if (stored_ptr == request->hexe_entry &&
                (int8_t)slot[DM2_V1_AI_ACTION_SLOT_TYPE_OFF] ==
                (int8_t)(request->action_type & 0xFF)) {
                receipt->slot_ptr = slot;
                receipt->allocated_new = 0;
                return 1;
            }
        } else {
            /* c_ai.cpp:1358 — remember first free slot */
            if (!free_slot)
                free_slot = slot;
        }
    }

    /* c_ai.cpp:1329-1343 — no match found, allocate in free_slot */
    if (i >= DM2_V1_AI_ACTION_SLOT_COUNT) {
        if (!free_slot) {
            /* All slots occupied, no match — use last scanned position.
             * c_ai.cpp:1330-1343 writes to xp_0c which tracks free slot */
            receipt->fail_closed = 1;
            return 0;
        }

        /* c_ai.cpp:1330-1341 — initialize new slot */
        memcpy(free_slot, &request->hexe_entry, sizeof(uint8_t *));
        free_slot[DM2_V1_AI_ACTION_SLOT_TYPE_OFF] =
            (uint8_t)(request->action_type & 0xFF);
        free_slot[DM2_V1_AI_ACTION_SLOT_TYPE_OFF + 1] = 0;
        free_slot[DM2_V1_AI_ACTION_SLOT_TYPE_OFF + 2] = 0;
        free_slot[DM2_V1_AI_ACTION_SLOT_TYPE_OFF + 3] = 0;

        receipt->slot_ptr = free_slot;
        receipt->allocated_new = 1;
    }

    return 1;
}
