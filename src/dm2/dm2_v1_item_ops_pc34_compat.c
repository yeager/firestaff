/* DM2 V1 item operations — skproject c_item.cpp. */

#include "dm2_v1_item_ops_pc34_compat.h"

#include "dm2_v1_champion_hud_helpers.h"
#include "dm2_v1_skproject_core.h"
#include <stddef.h>
#include <string.h>

#define OBJECT_NULL_WORD 0xFFFFu
#define OBJECT_END_WORD  0xFFFEu

/* ---- DM2_RETRIEVE_ITEM_BONUS (c_item.cpp:22) ---- */
int16_t dm2_v1_retrieve_item_bonus(
    uint16_t record_word, uint8_t bonus_idx,
    int16_t equipped, int16_t context,
    const DM2_V1_ItemBonusCallbacks *cb, void *ctx)
{
    if (!cb) return 0;
    int16_t val = cb->query_gdat_dbspec_word(ctx, record_word, bonus_idx);
    if (val == 0) return 0;
    if ((val & 0x4000) == 0) {
        /* Non-conditional bonus.  SKProject bitem.cpp:31-44 copies the
         * queried word to RG4, clears RG4's low byte by XORing it with
         * RG1Blo, then retains only RG4Bhi's sign bit when the item is not
         * equipped.  The resulting admission test is therefore bit 15 of
         * the original DB/GDAT word; shifting a signed int16 (the former
         * implementation) tests unrelated bits for values such as 0x0100. */
        if (equipped == 0 && (((uint16_t)val & 0x8000u) == 0u))
            return 0;
    } else {
        /* Conditional bonus: only applies in context 0xFFFE, 2, or 3 */
        if (context != (int16_t)0xFFFE && context != 2 && context != 3)
            return 0;
    }
    int8_t signed_val = (int8_t)(val & 0xFF);
    int16_t result = (int16_t)signed_val;
    return (context >= 0) ? result : -result;
}

/* ---- DM2_IS_MISCITEM_DRINK_WATER (c_item.cpp:528) ---- */
int dm2_v1_is_miscitem_drink_water(
    uint16_t record_word,
    const DM2_V1_DrinkWaterCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_dbspec_word || !cb->add_item_charge)
        return 0;
    int16_t drinkable = cb->query_gdat_dbspec_word(ctx, record_word, 0);
    if ((drinkable & 1) == 0)
        return 0;
    int16_t charges = cb->add_item_charge(ctx, record_word, 0);
    if (charges == 0)
        return 0;
    cb->add_item_charge(ctx, record_word, -1);
    if (record_word == cb->item_in_hand && cb->retake_object)
        cb->retake_object(ctx, record_word);
    return 1;
}

/* ---- DM2_F958 (c_item.cpp:1034) ---- */
int16_t dm2_v1_f958(uint16_t record_word,
                     const DM2_V1_ItemValueCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_item_value) return -1;
    int16_t value = cb->query_item_value(ctx, record_word, 2);
    return (value <= -1) ? value : -1;
}

/* ---- DM2_GET_MAX_CHARGE (c_item.cpp:344) ---- */
int16_t dm2_v1_get_max_charge(uint16_t record_word)
{
    if (record_word == OBJECT_NULL_WORD) return 0;
    uint16_t db_type = (record_word >> 10) & 0xF;
    if (db_type < 6) return (db_type == 5) ? 0x0F : 0;
    if (db_type == 6) return 0x0F;
    return (db_type == 0x0A) ? 3 : 0;
}

/* ---- DM2_ADD_ITEM_CHARGE (c_item.cpp:251) ---- */
int16_t dm2_v1_add_item_charge(
    uint16_t record_word, int16_t delta,
    const DM2_V1_ChargeCallbacks *cb, void *ctx)
{
    if (!cb || record_word == OBJECT_NULL_WORD) return 0;
    uint8_t *rec = cb->get_record_address(ctx, record_word);
    if (!rec) return 0;
    uint16_t db_type = (record_word >> 10) & 0xF;
    uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
    int16_t current = 0;
    int16_t max_val = 0;

    if (db_type == 5) {
        current = (int16_t)((w2 * 4) >> 12);
        max_val = 0x0F;
    } else if (db_type == 6) {
        current = (int16_t)((w2 * 8) >> 12);
        max_val = 0x0F;
    } else if (db_type == 0x0A) {
        current = (int16_t)(w2 >> 14);
        max_val = 3;
    } else {
        return 0;
    }

    int16_t new_val = current + delta;
    if (new_val < 0) new_val = 0;
    if (new_val > max_val) new_val = max_val;
    int16_t result = new_val;
    uint16_t masked = (uint16_t)(new_val & 0x0F);

    if (db_type == 5) {
        rec[3] = (rec[3] & 0xC3) | (uint8_t)0;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw = (nw & ~0x3C00) | (masked << 10);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    } else if (db_type == 6) {
        rec[3] = (rec[3] & 0xE1) | (uint8_t)0;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw = (nw & ~0x1E00) | (masked << 9);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    } else { /* 0x0A */
        uint16_t val3 = (uint16_t)(new_val & 0x03);
        rec[3] &= 0x3F;
        uint16_t nw = (uint16_t)(rec[2] | (rec[3] << 8));
        nw |= (val3 << 14);
        rec[2] = (uint8_t)(nw & 0xFF);
        rec[3] = (uint8_t)((nw >> 8) & 0xFF);
    }
    return result;
}

/* ---- DM2_QUERY_ITEM_WEIGHT (c_item.cpp:496) ---- */
int16_t dm2_v1_query_item_weight(
    uint16_t record_word,
    const DM2_V1_ItemValueCallbacks *cb, void *ctx)
{
    if (!cb) return 0;
    return cb->query_item_value(ctx, record_word, 1);
}

/* ---- DM2_GET_ITEM_NAME (c_item.cpp:502) ---- */
DM2_V1_ItemNameReceipt dm2_v1_get_item_name(
    uint16_t record_word,
    const DM2_V1_ItemNameCallbacks *cb, void *ctx)
{
    DM2_V1_ItemNameReceipt receipt;
    receipt.name = NULL;
    receipt.hero_index = -1;
    if (!cb) return receipt;

    uint8_t cls1 = cb->query_cls1(ctx, record_word);
    uint8_t cls2 = cb->query_cls2(ctx, record_word);

    /* Check if this is a hero bones item (cls1=0x15, cls2=0) */
    if (cls1 == 0x15 && cls2 == 0) {
        const uint8_t *rec = cb->get_record_address(ctx, record_word);
        if (rec) {
            uint16_t w2 = (uint16_t)(rec[2] | (rec[3] << 8));
            int16_t hero_idx = (int16_t)(w2 >> 14);
            if (hero_idx >= 0 && (uint16_t)hero_idx < cb->heros_in_party)
                receipt.hero_index = hero_idx;
        }
    }
    receipt.name = cb->query_gdat_item_name(ctx, cls1, cls2);
    return receipt;
}

/* Source: SKULLWIN/c_record.cpp:454, c_record.cpp:203 and
 * c_item.cpp:502; SkWinCore.cpp QUERY_GDAT_ITEM_NAME.  Keep the complete
 * owner chain in one receipt.  In particular, a caller cannot turn a
 * category/index fixture into a name without first proving that the DB5..10
 * record exists in the validated source pool. */
int dm2_v1_query_source_item_name_receipt(
    uint16_t record_word,
    const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SourceItemNameReceipt *out_receipt)
{
    DM2_V1_SourceItemNameReceipt receipt;
    DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
    DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
    uint8_t cls1 = 0xffu;
    uint8_t cls2 = 0xffu;

    memset(&receipt, 0, sizeof(receipt));
    receipt.record_word = record_word;
    receipt.record_type = (uint8_t)((record_word >> 10) & 0x0fu);
    if (receipt.record_type < 5u || receipt.record_type > 10u) {
        receipt.blocked_not_item = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!pools || !loader ||
        !dm2_v1_record_pool_address(pools, (int16_t)record_word)) {
        receipt.blocked_record_owner = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dm2_v1_skproject_query_cls1_from_record_ex(
            record_word, pools, &cls1, &cls1_receipt) ||
        !dm2_v1_skproject_query_cls2_from_record(
            record_word, pools, &cls2, &cls2_receipt) ||
        !cls1_receipt.valid || !cls2_receipt.valid ||
        cls1 == 0xffu || cls2 == 0xffu) {
        receipt.blocked_classification = 1u;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.cls1 = cls1;
    receipt.cls2 = cls2;
    if (!dm2_v1_query_gdat_item_name_receipt(
            loader, (int)cls1, (int)cls2, &receipt.gdat)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.accepted = 1u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

typedef struct {
    const DM2_V1_RecordPoolSet *pools;
    const DM2_V1_AssetLoader *loader;
    int invalid;
} DM2_V1_SksaveItemBonusContext;

static uint32_t dm2_v1_sksave_item_bonus_hash_word(uint32_t hash,
                                                    uint16_t word)
{
    hash ^= (uint8_t)(word & 0xffu);
    hash *= 16777619u;
    hash ^= (uint8_t)(word >> 8);
    return hash * 16777619u;
}

static int dm2_v1_sksave_item_bonus_classify(
    DM2_V1_SksaveItemBonusContext *context, uint16_t record_word,
    uint8_t *out_cls1, uint8_t *out_cls2)
{
    DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
    DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
    uint8_t cls1 = 0xffu;
    uint8_t cls2 = 0xffu;

    if (!context || !context->pools || !context->loader ||
        !dm2_v1_record_pool_address(context->pools, (int16_t)record_word) ||
        !dm2_v1_skproject_query_cls1_from_record_ex(
            record_word, context->pools, &cls1, &cls1_receipt) ||
        !dm2_v1_skproject_query_cls2_from_record(
            record_word, context->pools, &cls2, &cls2_receipt) ||
        !cls1_receipt.valid || !cls2_receipt.valid ||
        cls1 == 0xffu || cls2 == 0xffu) {
        if (context) context->invalid = 1;
        return 0;
    }
    if (out_cls1) *out_cls1 = cls1;
    if (out_cls2) *out_cls2 = cls2;
    return 1;
}

static uint16_t dm2_v1_sksave_item_bonus_query_dbspec(
    void *ctx, uint16_t record_word, int16_t data_index)
{
    DM2_V1_SksaveItemBonusContext *context =
        (DM2_V1_SksaveItemBonusContext *)ctx;
    uint8_t cls1 = 0xffu;
    uint8_t cls2 = 0xffu;
    uint16_t value = 0u;

    if (record_word == OBJECT_NULL_WORD) return 0u;
    if (!dm2_v1_sksave_item_bonus_classify(context, record_word,
                                            &cls1, &cls2)) {
        return 0u;
    }
    /* c_record.cpp::DM2_QUERY_GDAT_DBSPEC_WORD_VALUE asks dtWordValue
     * (11).  A missing source entry is the original query's zero result,
     * not a host-supplied replacement. */
    (void)dm2_v1_query_gdat_entry_data_index(
        context->loader, (int)cls1, (int)cls2, 11, (int)data_index, &value);
    return value;
}

static int32_t dm2_v1_sksave_item_bonus_fit(void *ctx,
                                             uint16_t record_word,
                                             int16_t slot)
{
    DM2_V1_SksaveItemBonusContext *context =
        (DM2_V1_SksaveItemBonusContext *)ctx;
    DM2_V1_ItemFitForEquipReceipt fit;
    uint8_t cls1 = 0xffu;
    uint8_t cls2 = 0xffu;

    if (!dm2_v1_sksave_item_bonus_classify(context, record_word,
                                            &cls1, &cls2) ||
        !dm2_v1_is_item_fit_for_equip_receipt(
            context->loader, (int)cls1, (int)cls2, (int)slot, 1, -1, &fit) ||
        !fit.accepted) {
        if (context) context->invalid = 1;
        return 0;
    }
    return (int32_t)fit.result;
}

static int16_t dm2_v1_sksave_item_bonus_query_dbspec_u8(
    void *ctx, uint16_t record_word, uint8_t data_index)
{
    return (int16_t)dm2_v1_sksave_item_bonus_query_dbspec(
        ctx, record_word, (int16_t)data_index);
}

static int16_t dm2_v1_sksave_item_bonus_retrieve(void *ctx,
                                                   uint16_t record_word,
                                                   uint8_t bonus_index,
                                                   int32_t equipped,
                                                   int16_t mode)
{
    DM2_V1_ItemBonusCallbacks callbacks;

    callbacks.query_gdat_dbspec_word =
        dm2_v1_sksave_item_bonus_query_dbspec_u8;
    return dm2_v1_retrieve_item_bonus(record_word, bonus_index,
                                      (int16_t)equipped, mode,
                                      &callbacks, ctx);
}

static uint8_t dm2_v1_sksave_item_bonus_query_cls2(void *ctx,
                                                    uint16_t record_word)
{
    uint8_t cls2 = 0xffu;
    if (!dm2_v1_sksave_item_bonus_classify(
            (DM2_V1_SksaveItemBonusContext *)ctx, record_word, NULL, &cls2)) {
        return 0u;
    }
    return cls2;
}

static int dm2_v1_sksave_process_one_item_bonus_root(
    DM2_V1_SksaveItemBonusContext *context, uint16_t *root,
    int16_t hero_index, int16_t slot, DM2_V1_SksaveItemBonusReceipt *receipt,
    int leader_hand)
{
    DM2_V1_ProcessItemBonusCallbacks callbacks;
    DM2_V1_ProcessItemBonusInput input;
    DM2_V1_ProcessItemBonusReceipt bonus;

    if (!root || *root == OBJECT_NULL_WORD) return 1;
    if (*root == OBJECT_END_WORD) {
        *root = OBJECT_NULL_WORD;
        if (leader_hand) ++receipt->empty_leader_hand;
        else ++receipt->empty_item_roots;
        return 1;
    }
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&input, 0, sizeof(input));
    memset(&bonus, 0, sizeof(bonus));
    callbacks.query_dbspec_word = dm2_v1_sksave_item_bonus_query_dbspec;
    callbacks.is_item_fit_for_equip = dm2_v1_sksave_item_bonus_fit;
    callbacks.retrieve_item_bonus = dm2_v1_sksave_item_bonus_retrieve;
    callbacks.query_cls2_from_record = dm2_v1_sksave_item_bonus_query_cls2;
    callbacks.ctx = context;
    input.hero_index = hero_index;
    input.item_ref = *root;
    input.slot = slot;
    input.mode = 0;
    dm2_v1_PROCESS_ITEM_BONUS(&input, &callbacks, &bonus);
    if (context->invalid || !bonus.valid || bonus.blocked) return 0;
    if (leader_hand) ++receipt->processed_leader_hand;
    else ++receipt->processed_item_roots;
    return 1;
}

static void dm2_v1_new_game_apply_item_bonus_receipt(DM2_V1_Hero *hero,
    const DM2_V1_ProcessItemBonusReceipt *bonus)
{
    int i;
    if (!hero || !bonus) return;
    hero->maxMP = (int16_t)(hero->maxMP + bonus->max_mp_delta);
    for (i = 0; i < DM2_V1_NUM_ABILITIES; ++i) {
        hero->eability[i] = (int8_t)(hero->eability[i] +
            bonus->eability_delta[i]);
    }
    for (i = 0; i < DM2_V1_NUM_SKILL_SLOTS; ++i) {
        hero->sbonus[i / 4][i % 4] = (int8_t)(hero->sbonus[i / 4][i % 4] +
            bonus->sbonus_delta[i]);
    }
    hero->walkspeed = (int8_t)(hero->walkspeed + bonus->walkspeed_delta);
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag |
        (uint16_t)bonus->heroflag_or);
}

int dm2_v1_new_game_apply_source_item_bonuses(
    DM2_V1_Party *party, const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SksaveItemBonusReceipt *out_receipt)
{
    DM2_V1_SksaveItemBonusReceipt receipt;
    DM2_V1_SksaveItemBonusContext context;
    int hero_index;

    memset(&receipt, 0, sizeof(receipt));
    memset(&context, 0, sizeof(context));
    receipt.source_hash = 2166136261u;
    if (!party || !pools || !pools->valid || !loader ||
        party->heros_in_party <= 0 || party->heros_in_party > DM2_MAX_HEROES) {
        receipt.blocked = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    context.pools = pools;
    context.loader = loader;
    for (hero_index = 0; hero_index < party->heros_in_party; ++hero_index) {
        DM2_V1_Hero *hero = &party->hero[hero_index];
        int slot;
        int32_t total_weight = 0;
        for (slot = 0; slot < DM2_NUM_ITEMS; ++slot) {
            DM2_V1_ProcessItemBonusCallbacks callbacks;
            DM2_V1_ProcessItemBonusInput input;
            DM2_V1_ProcessItemBonusReceipt bonus;
            const uint16_t item = (uint16_t)hero->item[slot];
            receipt.source_hash = dm2_v1_sksave_item_bonus_hash_word(
                receipt.source_hash, item);
            if (item == OBJECT_NULL_WORD) continue;
            if (item == OBJECT_END_WORD ||
                !dm2_v1_sksave_item_bonus_classify(&context, item, NULL, NULL)) {
                receipt.blocked = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            memset(&callbacks, 0, sizeof(callbacks));
            memset(&input, 0, sizeof(input));
            memset(&bonus, 0, sizeof(bonus));
            callbacks.query_dbspec_word = dm2_v1_sksave_item_bonus_query_dbspec;
            callbacks.is_item_fit_for_equip = dm2_v1_sksave_item_bonus_fit;
            callbacks.retrieve_item_bonus = dm2_v1_sksave_item_bonus_retrieve;
            callbacks.query_cls2_from_record = dm2_v1_sksave_item_bonus_query_cls2;
            callbacks.ctx = &context;
            input.hero_index = (int16_t)hero_index;
            input.item_ref = item;
            input.slot = (int16_t)slot;
            input.mode = 1;
            dm2_v1_PROCESS_ITEM_BONUS(&input, &callbacks, &bonus);
            if (context.invalid || !bonus.valid || bonus.blocked) {
                receipt.blocked = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            dm2_v1_new_game_apply_item_bonus_receipt(hero, &bonus);
            total_weight += (int16_t)dm2_v1_sksave_item_bonus_query_dbspec(
                &context, item, 1);
            ++receipt.processed_item_roots;
        }
        hero->weight = (int16_t)total_weight;
        hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x1000u);
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_sksave_process_source_item_bonus_roots(
    DM2_V1_Hero *heroes, size_t hero_capacity, uint16_t hero_count,
    uint16_t *leader_hand_root, const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_SksaveItemBonusReceipt *out_receipt)
{
    DM2_V1_SksaveItemBonusReceipt receipt;
    DM2_V1_SksaveItemBonusContext context;
    uint16_t hero_index;
    uint16_t slot;

    memset(&receipt, 0, sizeof(receipt));
    memset(&context, 0, sizeof(context));
    context.pools = pools;
    context.loader = loader;
    receipt.source_hash = 2166136261u;
    if (!heroes || !leader_hand_root || !pools || !pools->valid || !loader ||
        hero_count > hero_capacity || hero_count > DM2_MAX_HEROES) {
        receipt.blocked = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* SKProject c_savegame.cpp::DM2_READ_SKSAVE_DUNGEON:1206-1224.
     * Retain the c_hero item-major order followed by the leader hand. */
    for (hero_index = 0u; hero_index < hero_count; ++hero_index) {
        for (slot = 0u; slot < DM2_NUM_ITEMS; ++slot) {
            receipt.source_hash = dm2_v1_sksave_item_bonus_hash_word(
                receipt.source_hash, (uint16_t)heroes[hero_index].item[slot]);
            if (!dm2_v1_sksave_process_one_item_bonus_root(
                    &context, (uint16_t *)&heroes[hero_index].item[slot],
                    (int16_t)hero_index, (int16_t)slot, &receipt, 0)) {
                receipt.blocked = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
        }
    }
    receipt.source_hash = dm2_v1_sksave_item_bonus_hash_word(
        receipt.source_hash, *leader_hand_root);
    if (!dm2_v1_sksave_process_one_item_bonus_root(
            &context, leader_hand_root, -1, -1, &receipt, 1)) {
        receipt.blocked = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}


/* ---- DM2_TAKE_OBJECT (c_item.cpp:1185) ---- */
void dm2_v1_take_object(
    uint16_t record_word, int deferred,
    const DM2_V1_TakeObjectCallbacks *cb, void *ctx)
{
    if (!cb || record_word == 0xFFFFu)
        return;

    int16_t gdat_word = cb->query_gdat_dbspec_word
                             ? cb->query_gdat_dbspec_word(ctx, record_word, 0)
                             : 0;
    int16_t weight = cb->query_item_weight
                          ? cb->query_item_weight(ctx, record_word)
                          : 0;
    if (cb->set_hand_item)
        cb->set_hand_item(ctx, record_word, gdat_word, weight);
    if (cb->draw_item_in_hand)
        cb->draw_item_in_hand(ctx);
    if (cb->display_item_name)
        cb->display_item_name(ctx, record_word);

    if (deferred == 0) {
        if (cb->process_events)
            cb->process_events(ctx);
    } else {
        if (cb->set_deferred_flag)
            cb->set_deferred_flag(ctx);
    }

    if (cb->process_item_bonus)
        cb->process_item_bonus(ctx, record_word);
    if (cb->moverec_update)
        cb->moverec_update(ctx);
}
