#include "dm2_v1_item_missile_helpers.h"

#include <limits.h>

static void dm2_item_missile_receipt_begin(
    DM2_V1_ItemMissileReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_item_missile_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

void dm2_v1_item_missile_receipt_clear(
    DM2_V1_ItemMissileReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->handled = 0;
    receipt->source_locked = 0;
    receipt->valid = 0;
    receipt->result = 0;
    receipt->blocked = 0;
    receipt->symbol = 0;
    receipt->source_path = 0;
}

int dm2_v1_IS_ITEM_HAND_ACTIVABLE(
    uint16_t item_ref,
    uint16_t dbspec_word,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    int result;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "IS_ITEM_HAND_ACTIVABLE",
                                   "SKWIN/SkWinCore.cpp:8423");
    if (item_ref == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    result = (dbspec_word & DM2_V1_ITEM_HAND_ACTIVABLE_MASK) != 0;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
    }
    return result;
}

int16_t dm2_v1_RETRIEVE_ITEM_BONUS(
    const DM2_V1_ItemBonusFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    int value;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "RETRIEVE_ITEM_BONUS",
                                   "SKWIN/SkWinCore.cpp:5204");
    if (!facts || facts->item_ref == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    value = (int)facts->base_bonus + (int)facts->runtime_delta;
    if (value < INT16_MIN) {
        value = INT16_MIN;
    } else if (value > INT16_MAX) {
        value = INT16_MAX;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = value;
    }
    return (int16_t)value;
}

uint16_t dm2_v1_GET_MISSILE_REF_OF_MINION(
    const uint16_t *missile_refs,
    size_t missile_ref_count,
    size_t slot_index,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    uint16_t result;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "GET_MISSILE_REF_OF_MINION",
                                   "SKWIN/SkWinCore.cpp:8205");
    if (!missile_refs || slot_index >= missile_ref_count) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    result = missile_refs[slot_index];
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = (result == DM2_V1_ITEM_MISSILE_NULL_REF)
                                  ? 0
                                  : (int)result;
        out_receipt->blocked =
            result == DM2_V1_ITEM_MISSILE_NULL_REF ? 1 : 0;
    }
    return result;
}

int dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(
    const DM2_V1_MissileLauncherFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    int result;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "IS_MISSILE_VALID_TO_LAUNCHER",
                                   "SKWIN/SkWinCore.cpp:4973");
    if (!facts ||
        facts->launcher_ref == DM2_V1_ITEM_MISSILE_NULL_REF ||
        facts->missile_ref == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    result = facts->launcher_accepts_missiles &&
             facts->missile_class == facts->required_missile_class;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
        out_receipt->blocked = result ? 0 : 1;
    }
    return result;
}

const char *dm2_v1_item_missile_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp "
           "IS_MISSILE_VALID_TO_LAUNCHER:4973 "
           "RETRIEVE_ITEM_BONUS:5204 "
           "GET_MISSILE_REF_OF_MINION:8205 "
           "IS_ITEM_HAND_ACTIVABLE:8423";
}
