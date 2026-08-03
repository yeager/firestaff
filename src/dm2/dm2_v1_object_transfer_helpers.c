#include "dm2_v1_object_transfer_helpers.h"

#include <string.h>

static void dm2_object_transfer_begin(
    DM2_V1_ObjectTransferReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_object_transfer_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static int dm2_find_link(const DM2_V1_ObjectTransferLink *links,
                         size_t link_count,
                         uint16_t ref,
                         uint16_t *out_next_ref)
{
    size_t i;

    if (out_next_ref) {
        *out_next_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    if (!links || ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        link_count > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        return 0;
    }
    for (i = 0u; i < link_count; ++i) {
        if (links[i].ref == ref) {
            if (out_next_ref) {
                *out_next_ref = links[i].next_ref;
            }
            return 1;
        }
    }
    return 0;
}

void dm2_v1_object_transfer_receipt_clear(
    DM2_V1_ObjectTransferReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->object_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->container_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->previous_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->next_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->new_head_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->hand_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->removed_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->slot_index = -1;
    receipt->equip_hand = -1;
}

/* ------------------------------------------------------------------ */
/* DM2_REMOVE_POSSESSION — c_hero.cpp:2485                             */
/* ------------------------------------------------------------------ */

/* Resolves the source slot selection: item[slot] below 30, otherwise
 * hand_container[(2 * slot - 60) / 2] == hand_container[slot - 30]. */
static uint16_t *dm2_possession_slot(DM2_V1_PossessionSlots *slots,
                                     int16_t slot_index)
{
    size_t slot;

    if (!slots || slot_index < 0) {
        return 0;
    }
    slot = (size_t)slot_index;
    if (slot < (size_t)DM2_V1_OBJECT_TRANSFER_HERO_ITEM_SLOTS) {
        if (!slots->hero_items || slot >= slots->hero_item_count) {
            return 0;
        }
        return &slots->hero_items[slot];
    }
    slot -= (size_t)DM2_V1_OBJECT_TRANSFER_HERO_ITEM_SLOTS;
    if (!slots->hand_container || slot >= slots->hand_container_count ||
        slot >= (size_t)DM2_V1_OBJECT_TRANSFER_HAND_CONTAINER_SLOTS) {
        return 0;
    }
    return &slots->hand_container[slot];
}

uint16_t dm2_v1_REMOVE_POSSESSION(
    DM2_V1_PossessionSlots *slots,
    int16_t slot_index,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t *cell;
    uint16_t removed;

    dm2_object_transfer_begin(out_receipt,
                              "REMOVE_POSSESSION",
                              "SKULLWIN/c_hero.cpp:2485");
    cell = dm2_possession_slot(slots, slot_index);
    if (!cell) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->slot_index = slot_index;
        }
        return DM2_V1_OBJECT_TRANSFER_NULL;
    }
    removed = *cell;
    *cell = DM2_V1_OBJECT_TRANSFER_NULL;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->slot_index = slot_index;
        out_receipt->removed_ref = removed;
        out_receipt->object_ref = removed;
    }
    /* `if (RG5W == 0xffff) return RG5L;` — no side effects at all. */
    if (removed == DM2_V1_OBJECT_TRANSFER_NULL) {
        return DM2_V1_OBJECT_TRANSFER_NULL;
    }
    if (out_receipt) {
        out_receipt->mutated = 1;
        /* The hero panel refresh: current active hero, hand slot 0/1,
         * and the slot matching party.curactmode. */
        if (slots->hero_index == (int16_t)(slots->cur_act_hero - 1) &&
            slot_index <= 1 && slot_index == slots->cur_act_mode) {
            out_receipt->ui_refresh_needed = 1;
        }
        /* DM2_PROCESS_ITEM_BONUS(hero, removed, slot, -1) is not
         * modelled here: it is the caller's responsibility, and the
         * receipt carries every argument it needs. */
        out_receipt->item_bonus_pending = 1;
    }
    return removed;
}

/* ------------------------------------------------------------------ */
/* DM2_PUT_OBJECT_INTO_CONTAINER — c_item.cpp:1146                     */
/* ------------------------------------------------------------------ */

int dm2_v1_object_transfer_append_to_chain(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t container_ref,
    uint16_t container_head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_previous_tail_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t current;
    uint16_t next;
    uint16_t tail = DM2_V1_OBJECT_TRANSFER_NULL;
    size_t guard = 0u;

    if (out_new_head_ref) {
        *out_new_head_ref = container_head_ref;
    }
    if (out_previous_tail_ref) {
        *out_previous_tail_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    dm2_object_transfer_begin(out_receipt,
                              "PUT_OBJECT_INTO_CONTAINER",
                              "SKULLWIN/c_item.cpp:1146");
    if (container_ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        object_ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        object_ref == container_ref ||
        link_count > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (container_head_ref == DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_new_head_ref) {
            *out_new_head_ref = object_ref;
        }
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->mutated = 1;
            out_receipt->container_ref = container_ref;
            out_receipt->object_ref = object_ref;
            out_receipt->new_head_ref = object_ref;
        }
        return 1;
    }
    current = container_head_ref;
    while (current != DM2_V1_OBJECT_TRANSFER_NULL &&
           guard++ < DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (current == object_ref ||
            !dm2_find_link(links, link_count, current, &next)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
            }
            return 0;
        }
        tail = current;
        current = next;
    }
    if (guard > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_previous_tail_ref) {
        *out_previous_tail_ref = tail;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->mutated = 1;
        out_receipt->container_ref = container_ref;
        out_receipt->object_ref = object_ref;
        out_receipt->previous_ref = tail;
        out_receipt->new_head_ref = container_head_ref;
    }
    return 1;
}

int dm2_v1_PUT_OBJECT_INTO_CONTAINER(
    uint16_t *pending_target_ref,
    uint16_t *hand_container,
    size_t hand_container_count,
    DM2_V1_ObjectTransferAppend append,
    void *append_context,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t target;
    size_t i;

    dm2_object_transfer_begin(out_receipt,
                              "PUT_OBJECT_INTO_CONTAINER",
                              "SKULLWIN/c_item.cpp:1146");
    if (!pending_target_ref || !hand_container || !append ||
        hand_container_count <
            (size_t)DM2_V1_OBJECT_TRANSFER_HAND_CONTAINER_SLOTS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    /* `RG4W = ddat.v1d6700; if (RG4W == 0xffff) return;` */
    target = *pending_target_ref;
    if (target == DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return 1;
    }
    /* `ddat.v1d6700 = 0xffff;` — the pending target is consumed. */
    *pending_target_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    if (out_receipt) {
        out_receipt->container_ref = target;
    }
    for (i = 0u; i < (size_t)DM2_V1_OBJECT_TRANSFER_HAND_CONTAINER_SLOTS;
         ++i) {
        uint16_t object_ref = hand_container[i];

        if (object_ref == DM2_V1_OBJECT_TRANSFER_NULL) {
            continue;
        }
        hand_container[i] = DM2_V1_OBJECT_TRANSFER_NULL;
        if (!append(append_context, target, object_ref)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->object_ref = object_ref;
            }
            return 0;
        }
        if (out_receipt) {
            out_receipt->mutated = 1;
            out_receipt->object_ref = object_ref;
            out_receipt->moved_count++;
            out_receipt->moved_slot_mask |= 1u << i;
        }
    }
    if (out_receipt) {
        out_receipt->valid = 1;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_LOAD_PROJECTILE_TO_HAND — c_hero.cpp:3643                       */
/* ------------------------------------------------------------------ */

static int dm2_load_projectile_callbacks_ready(
    const DM2_V1_LoadProjectileCallbacks *cb)
{
    return cb && cb->is_missile_valid_to_launcher &&
           cb->is_item_valid_for_command && cb->is_container_chest &&
           cb->chest_chain_head && cb->next_record_link &&
           cb->cut_record_from_chest && cb->equip_item_to_hand;
}

static int dm2_load_projectile_match(
    const DM2_V1_LoadProjectileCallbacks *cb,
    int16_t hero_index,
    int16_t hand_slot,
    int16_t handcmd,
    uint16_t candidate_ref)
{
    if (handcmd == DM2_V1_OBJECT_TRANSFER_HANDCMD_MISSILE) {
        return cb->is_missile_valid_to_launcher(cb->context, hero_index,
                                                hand_slot, candidate_ref);
    }
    return cb->is_item_valid_for_command(cb->context, hero_index,
                                         candidate_ref, handcmd);
}

static int dm2_load_projectile_equip(
    const DM2_V1_LoadProjectileCallbacks *cb,
    int16_t hero_index,
    uint16_t item_ref,
    int16_t target_hand,
    int path,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    int ok = cb->equip_item_to_hand(cb->context, hero_index, item_ref,
                                    target_hand);

    if (out_receipt) {
        out_receipt->path = path;
        out_receipt->object_ref = item_ref;
        out_receipt->hand_ref = item_ref;
        out_receipt->equip_hand = target_hand;
        out_receipt->equipped = ok ? 1 : 0;
        out_receipt->mutated = 1;
        if (ok) {
            out_receipt->valid = 1;
        } else {
            out_receipt->blocked = 1;
        }
    }
    return ok ? 1 : 0;
}

int dm2_v1_LOAD_PROJECTILE_TO_HAND(
    DM2_V1_LoadProjectileToHandInput *input,
    const DM2_V1_LoadProjectileCallbacks *callbacks,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    DM2_V1_ObjectTransferReceipt remove_receipt;
    int16_t hand;
    int16_t other_hand;
    int16_t target_hand;
    int16_t handcmd;
    uint16_t chest_ref;
    uint16_t removed;
    int16_t slot;
    size_t guard;

    dm2_object_transfer_begin(out_receipt,
                              "LOAD_PROJECTILE_TO_HAND",
                              "SKULLWIN/c_hero.cpp:3643");
    if (!input || !dm2_load_projectile_callbacks_ready(callbacks) ||
        !input->handcooldown || !input->handcmd ||
        !input->handdefenseclass || !input->slots.hero_items ||
        input->slots.hero_item_count <=
            (size_t)DM2_V1_OBJECT_TRANSFER_CHEST_SLOT) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    hand = input->hand_slot;
    /* `hero->handcooldown[hand] = 0;` precedes both early returns in
     * the source; the write is bounded to the two real hand slots. */
    if (hand >= 0 && hand < 2) {
        input->handcooldown[hand] = 0;
    }
    if (input->hero_cur_hp == 0) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->path = DM2_V1_LOAD_PROJECTILE_PATH_HERO_DEAD;
        }
        return 0;
    }
    if (hand < 0 || hand >= 2) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->path = DM2_V1_LOAD_PROJECTILE_PATH_BAD_HAND;
        }
        return 0;
    }
    other_hand = (int16_t)(hand ^ 1);
    handcmd = input->handcmd[hand];
    input->handcmd[hand] = -1;
    input->handdefenseclass[hand] = 0;
    if (out_receipt) {
        out_receipt->handcmd = (int)handcmd;
    }
    if (handcmd == DM2_V1_OBJECT_TRANSFER_HANDCMD_MISSILE) {
        /* The missile reload equips into the opposite hand. */
        target_hand = other_hand;
    } else if (handcmd == DM2_V1_OBJECT_TRANSFER_HANDCMD_SPELL) {
        target_hand = hand;
    } else {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->path = DM2_V1_LOAD_PROJECTILE_PATH_UNHANDLED_CMD;
        }
        return 0;
    }
    /* `if (hero->item[target] != -1) return;` */
    if (input->slots.hero_items[target_hand] !=
        DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->path =
                DM2_V1_LOAD_PROJECTILE_PATH_TARGET_HAND_BUSY;
            out_receipt->equip_hand = target_hand;
        }
        return 0;
    }
    chest_ref =
        input->slots.hero_items[DM2_V1_OBJECT_TRANSFER_CHEST_SLOT];
    /* 1. the chest/quiver slot's own item. */
    if (dm2_load_projectile_match(callbacks, input->slots.hero_index,
                                  hand, handcmd, chest_ref)) {
        removed = dm2_v1_REMOVE_POSSESSION(
            &input->slots, (int16_t)DM2_V1_OBJECT_TRANSFER_CHEST_SLOT,
            &remove_receipt);
        return dm2_load_projectile_equip(
            callbacks, input->slots.hero_index, removed, target_hand,
            DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_SLOT, out_receipt);
    }
    /* 2. the chest chain held in that slot. */
    if (callbacks->is_container_chest(callbacks->context, chest_ref)) {
        uint16_t node;

        if (!callbacks->chest_chain_head(callbacks->context, chest_ref,
                                         &node)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
            }
            return 0;
        }
        guard = 0u;
        while (node != DM2_V1_OBJECT_TRANSFER_END_MARKER) {
            if (guard++ >= DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
                if (out_receipt) {
                    out_receipt->blocked = 1;
                }
                return 0;
            }
            if (dm2_load_projectile_match(callbacks,
                                          input->slots.hero_index, hand,
                                          handcmd, node)) {
                if (!callbacks->cut_record_from_chest(callbacks->context,
                                                      chest_ref, node)) {
                    if (out_receipt) {
                        out_receipt->blocked = 1;
                        out_receipt->object_ref = node;
                    }
                    return 0;
                }
                if (out_receipt) {
                    out_receipt->container_ref = chest_ref;
                }
                return dm2_load_projectile_equip(
                    callbacks, input->slots.hero_index, node, target_hand,
                    DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_CHAIN,
                    out_receipt);
            }
            if (!callbacks->next_record_link(callbacks->context, node,
                                             &node)) {
                if (out_receipt) {
                    out_receipt->blocked = 1;
                }
                return 0;
            }
        }
    }
    /* 3. the pouch slots 7..9. */
    for (slot = (int16_t)DM2_V1_OBJECT_TRANSFER_POUCH_FIRST_SLOT;
         slot <= (int16_t)DM2_V1_OBJECT_TRANSFER_POUCH_LAST_SLOT;
         ++slot) {
        if ((size_t)slot >= input->slots.hero_item_count) {
            break;
        }
        if (!dm2_load_projectile_match(callbacks, input->slots.hero_index,
                                       hand, handcmd,
                                       input->slots.hero_items[slot])) {
            continue;
        }
        removed = dm2_v1_REMOVE_POSSESSION(&input->slots, slot,
                                           &remove_receipt);
        return dm2_load_projectile_equip(
            callbacks, input->slots.hero_index, removed, target_hand,
            DM2_V1_LOAD_PROJECTILE_PATH_FROM_POUCH, out_receipt);
    }
    /* 4. the 0x2a path falls back to the chest slot itself; the 0x20
     * path simply returns. */
    if (handcmd == DM2_V1_OBJECT_TRANSFER_HANDCMD_SPELL) {
        removed = dm2_v1_REMOVE_POSSESSION(
            &input->slots, (int16_t)DM2_V1_OBJECT_TRANSFER_CHEST_SLOT,
            &remove_receipt);
        if (removed == DM2_V1_OBJECT_TRANSFER_NULL) {
            if (out_receipt) {
                out_receipt->valid = 1;
                out_receipt->path =
                    DM2_V1_LOAD_PROJECTILE_PATH_EXHAUSTED;
            }
            return 0;
        }
        return dm2_load_projectile_equip(
            callbacks, input->slots.hero_index, removed, target_hand,
            DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_SLOT, out_receipt);
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->path = DM2_V1_LOAD_PROJECTILE_PATH_EXHAUSTED;
    }
    return 0;
}

const char *dm2_v1_object_transfer_helpers_source_evidence(void)
{
    return "skproject SKULLWIN "
           "c_hero.cpp REMOVE_POSSESSION:2485 "
           "c_hero.cpp LOAD_PROJECTILE_TO_HAND:3643 "
           "c_item.cpp PUT_OBJECT_INTO_CONTAINER:1146; "
           "SKWIN mirror SkWinCore.cpp REMOVE_POSSESSION:5430 "
           "LOAD_PROJECTILE_TO_HAND:5543 "
           "PUT_OBJECT_INTO_CONTAINER:7064; "
           "DM2_PROCESS_ITEM_BONUS, DM2_DISPLAY_RIGHT_PANEL_SQUAD_HANDS, "
           "DM2_APPEND_RECORD_TO, DM2_CUT_RECORD_FROM and "
           "DM2_EQUIP_ITEM_TO_HAND stay caller responsibilities and are "
           "receipted rather than synthesised.";
}
