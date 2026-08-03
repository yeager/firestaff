#include "dm2_v1_item_missile_helpers.h"

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
    receipt->steps = 0;
    receipt->action_count = 0;
    receipt->symbol = 0;
    receipt->source_path = 0;
}

/* ------------------------------------------------------------------ */
/* DM2_IS_MISSILE_VALID_TO_LAUNCHER — c_querydb.cpp:949                */
/*                                                                     */
/*   RG3 = DBSPEC5(hero->item[hand]);   launcher word                  */
/*   RG3 & 0x8000 must be set           (item is a launcher)           */
/*   RG1 = DBSPEC5(missile);            candidate word                 */
/*   (RG1 & 0x8000) must be clear       (candidate is not a launcher)  */
/*   return (RG3 & (RG1 & 0x7fff)) != 0 (ammunition classes overlap)   */
/* ------------------------------------------------------------------ */
int dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(
    const DM2_V1_MissileLauncherFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    uint16_t launcher_word;
    uint16_t missile_word;
    int result;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "IS_MISSILE_VALID_TO_LAUNCHER",
                                   "SKULLWIN/c_querydb.cpp:949");
    if (!facts) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    /* Source: `if (RG2W == 0xffff) return 0;` then
     * `RG1W = party.hero[..].item[..]; if (RG1W == -1) return 0;`. */
    if (facts->missile_ref == DM2_V1_ITEM_MISSILE_NULL_REF ||
        facts->launcher_ref == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->blocked = 1;
        }
        return 0;
    }
    launcher_word = facts->launcher_dbspec_word5;
    missile_word = facts->missile_dbspec_word5;
    if ((launcher_word & DM2_V1_ITEM_MISSILE_LAUNCHER_BIT) == 0u) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return 0;
    }
    if ((missile_word & DM2_V1_ITEM_MISSILE_LAUNCHER_BIT) != 0u) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return 0;
    }
    result = (launcher_word &
              (uint16_t)(missile_word & DM2_V1_ITEM_MISSILE_CLASS_MASK)) != 0u;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
    }
    return result;
}

/* ------------------------------------------------------------------ */
/* DM2_RETRIEVE_ITEM_BONUS — c_item.cpp:22                             */
/* ------------------------------------------------------------------ */
int16_t dm2_v1_RETRIEVE_ITEM_BONUS(
    const DM2_V1_ItemBonusFacts *facts,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    uint16_t word;
    int16_t value;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "RETRIEVE_ITEM_BONUS",
                                   "SKULLWIN/c_item.cpp:22");
    if (!facts || facts->item_ref == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    word = facts->dbspec_word;
    /* `if (RG1W == 0) return 0;` */
    if (word == 0u) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return 0;
    }
    if ((word & 0x4000u) == 0u) {
        /* `if (RG2L == 0) { RG4Blo ^= RG1Blo; RG4Bhi &= 0xffffff80; ...`
         * — the low byte cancels, leaving the 0x8000 bit as the gate. */
        if (facts->select_flag == 0) {
            if ((word & 0x8000u) == 0u) {
                if (out_receipt) {
                    out_receipt->valid = 1;
                }
                return 0;
            }
        }
    } else {
        /* `if (RG3W != 0xfffe && RG3W != 2 && RG3W != 3) return 0;` */
        if (facts->mode != -2 && facts->mode != 2 && facts->mode != 3) {
            if (out_receipt) {
                out_receipt->valid = 1;
            }
            return 0;
        }
    }
    /* `RG1Bhi = sgn8(RG1Blo);` — the low byte sign-extended. */
    value = (int16_t)(int8_t)(word & 0x00ffu);
    /* `return RG3W >= 0 ? RG1L : -RG1L;` */
    if (facts->mode < 0) {
        value = (int16_t)(-(int)value);
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = (int)value;
    }
    return value;
}

/* ------------------------------------------------------------------ */
/* DM2_GET_MISSILE_REF_OF_MINION — c_querydb.cpp:1449                  */
/* ------------------------------------------------------------------ */
uint16_t dm2_v1_GET_MISSILE_REF_OF_MINION(
    const DM2_V1_RecordChainAccess *chain,
    uint16_t minion_ref,
    uint16_t filter_word,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    uint16_t node;
    uint16_t word;
    unsigned max_steps;
    unsigned steps = 0u;

    dm2_item_missile_receipt_begin(out_receipt,
                                   "GET_MISSILE_REF_OF_MINION",
                                   "SKULLWIN/c_querydb.cpp:1449");
    if (!chain || !chain->record_word || !chain->next_link) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    max_steps = chain->max_steps ? chain->max_steps
                                 : DM2_V1_ITEM_MISSILE_MAX_CHAIN;
    /* `if (RG1W == 0xffff || RG1W == 0xfffe) return NULL;` */
    if (minion_ref == DM2_V1_ITEM_MISSILE_NULL_REF ||
        minion_ref == DM2_V1_ITEM_MISSILE_END_MARKER) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    /* `if (word_at(RG1P) == 0xffff) return NULL;` */
    if (!chain->record_word(chain->context, minion_ref, 0u, &word)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    if (word == DM2_V1_ITEM_MISSILE_NULL_REF) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    /* `RG4W = word_at(RG1P, 2);` — the minion's possession chain head. */
    if (!chain->record_word(chain->context, minion_ref, 2u, &node)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return DM2_V1_ITEM_MISSILE_NULL_REF;
    }
    for (;;) {
        if (node == DM2_V1_ITEM_MISSILE_END_MARKER) {
            if (out_receipt) {
                out_receipt->valid = 1;
                out_receipt->steps = (int)steps;
            }
            return DM2_V1_ITEM_MISSILE_NULL_REF;
        }
        if (steps++ >= max_steps) {
            /* Corrupt/self-looping chain: fail closed. */
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->steps = (int)steps;
            }
            return DM2_V1_ITEM_MISSILE_NULL_REF;
        }
        /* `RG1Bhi &= 0x3c; RG1L = RG1W >> 0xa; if (RG1L == 0xe)` —
         * handle bits 10-13 select the record's DB index. */
        if ((unsigned)((node & 0x3c00u) >> 10) ==
            DM2_V1_ITEM_MISSILE_RECORD_DB_INDEX) {
            if (!chain->record_word(chain->context, node, 2u, &word)) {
                if (out_receipt) {
                    out_receipt->blocked = 1;
                    out_receipt->steps = (int)steps;
                }
                return DM2_V1_ITEM_MISSILE_NULL_REF;
            }
            if (filter_word == DM2_V1_ITEM_MISSILE_NULL_REF ||
                filter_word == word) {
                if (out_receipt) {
                    out_receipt->valid = 1;
                    out_receipt->result = (int)node;
                    out_receipt->steps = (int)steps;
                }
                return node;
            }
        }
        if (!chain->next_link(chain->context, node, &node)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->steps = (int)steps;
            }
            return DM2_V1_ITEM_MISSILE_NULL_REF;
        }
    }
}

/* ------------------------------------------------------------------ */
/* DM2_IS_ITEM_HAND_ACTIVABLE — c_querydb.cpp:4562                     */
/* ------------------------------------------------------------------ */
static int dm2_hand_activable_callbacks_ready(
    const DM2_V1_HandActivableCallbacks *cb)
{
    return cb && cb->is_container_moneybox && cb->is_container_chest &&
           cb->is_container_map && cb->query_cls1 && cb->query_cls2 &&
           cb->hero_type && cb->gdat_entry_if_loadable &&
           cb->cmdstr_entry && cb->action_applies_to_item &&
           cb->item_charge && cb->find_pouch_or_scabbard_pos &&
           cb->player_skill_level;
}

int dm2_v1_IS_ITEM_HAND_ACTIVABLE(
    const DM2_V1_HandActivableCallbacks *callbacks,
    int16_t hero_index,
    uint16_t item_ref,
    int16_t slot_index,
    DM2_V1_HandActionEntry *out_actions,
    size_t out_action_capacity,
    size_t *out_action_count,
    DM2_V1_ItemMissileReceipt *out_receipt)
{
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cmd;
    size_t count = 0u;
    int hero_mode;
    int result;

    if (out_action_count) {
        *out_action_count = 0u;
    }
    dm2_item_missile_receipt_begin(out_receipt,
                                   "IS_ITEM_HAND_ACTIVABLE",
                                   "SKULLWIN/c_querydb.cpp:4562");
    if (!dm2_hand_activable_callbacks_ready(callbacks)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_actions && out_action_capacity == 0u) {
        out_actions = 0;
    }
    hero_mode = item_ref == DM2_V1_ITEM_MISSILE_NULL_REF;
    if (!hero_mode) {
        /* `if (DM2_IS_CONTAINER_MONEYBOX(..)) return 1;` */
        if (callbacks->is_container_moneybox(callbacks->context, item_ref)) {
            if (out_receipt) {
                out_receipt->valid = 1;
                out_receipt->result = 1;
            }
            return 1;
        }
        if (callbacks->is_container_chest(callbacks->context, item_ref)) {
            if (out_receipt) {
                out_receipt->valid = 1;
                out_receipt->result = 1;
            }
            return 1;
        }
        cls1 = callbacks->query_cls1(callbacks->context, item_ref);
        cls2 = callbacks->query_cls2(callbacks->context, item_ref);
    } else {
        cls1 = (uint8_t)DM2_V1_ITEM_HAND_ACTIVABLE_HERO_CLS1;
        cls2 = callbacks->hero_type(callbacks->context, hero_index);
    }
    for (cmd = (uint8_t)DM2_V1_ITEM_HAND_ACTIVABLE_FIRST_CMD;
         cmd < (uint8_t)DM2_V1_ITEM_HAND_ACTIVABLE_END_CMD;
         ++cmd) {
        int16_t action_word;
        int16_t gate_word;
        int16_t skill;
        int16_t min_level;
        int16_t level;
        int considered = 0;
        int emit = 0;

        if (count >= (size_t)DM2_V1_ITEM_HAND_ACTIVABLE_MAX_ACTIONS) {
            break;
        }
        if (!callbacks->gdat_entry_if_loadable(callbacks->context, cls1,
                                               cls2, 5u, cmd)) {
            continue;
        }
        action_word = callbacks->cmdstr_entry(callbacks->context, cls1,
                                              cls2, cmd, 2u);
        if (action_word == 0) {
            continue;
        }
        /* Field 0x11 either is unset or must name this very slot. */
        gate_word = callbacks->cmdstr_entry(callbacks->context, cls1,
                                            cls2, cmd, 0x11u);
        if (gate_word != 0 && (int16_t)(gate_word - 1) != slot_index) {
            continue;
        }
        if (hero_mode) {
            if (action_word != 0x11) {
                emit = 1;
            } else {
                int16_t pos = callbacks->find_pouch_or_scabbard_pos(
                    callbacks->context, hero_index, slot_index);
                emit = pos >= 0;
            }
            considered = 1;
        } else if (callbacks->action_applies_to_item(callbacks->context,
                                                     action_word,
                                                     item_ref)) {
            int16_t required = callbacks->cmdstr_entry(callbacks->context,
                                                       cls1, cls2, cmd, 8u);
            considered = 1;
            if (required != 0x12) {
                if (required == 0x10 || required == 0x11) {
                    required = 1;
                }
                if (required == 0) {
                    emit = 1;
                } else {
                    int16_t charge = callbacks->item_charge(
                        callbacks->context, item_ref);
                    emit = charge >= required;
                }
            } else {
                int16_t charge = callbacks->item_charge(callbacks->context,
                                                        item_ref);
                emit = charge == 0;
            }
        }
        if (!considered || !emit) {
            continue;
        }
        skill = callbacks->cmdstr_entry(callbacks->context, cls1, cls2,
                                        cmd, 0u);
        min_level = callbacks->cmdstr_entry(callbacks->context, cls1, cls2,
                                            cmd, 1u);
        level = callbacks->player_skill_level(callbacks->context,
                                              hero_index, skill, 1);
        if ((uint16_t)level < (uint16_t)min_level) {
            continue;
        }
        if (out_actions && count < out_action_capacity) {
            out_actions[count].cls1 = cls1;
            out_actions[count].cls2 = cls2;
            out_actions[count].cmd = cmd;
        }
        ++count;
    }
    if (out_action_count) {
        *out_action_count = count;
    }
    /* `if (DM2_IS_CONTAINER_MAP(..)) return 1;` then `count > 0`. */
    if (callbacks->is_container_map(callbacks->context, item_ref)) {
        result = 1;
    } else {
        result = count > 0u;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
        out_receipt->action_count = (int)count;
    }
    return result;
}

const char *dm2_v1_item_missile_helpers_source_evidence(void)
{
    return "skproject SKULLWIN "
           "c_querydb.cpp IS_MISSILE_VALID_TO_LAUNCHER:949 "
           "c_item.cpp RETRIEVE_ITEM_BONUS:22 "
           "c_querydb.cpp GET_MISSILE_REF_OF_MINION:1449 "
           "c_querydb.cpp IS_ITEM_HAND_ACTIVABLE:4562; "
           "SKWIN mirror SkWinCore.cpp "
           "IS_MISSILE_VALID_TO_LAUNCHER:4973 "
           "RETRIEVE_ITEM_BONUS:5204 "
           "GET_MISSILE_REF_OF_MINION:8205 "
           "IS_ITEM_HAND_ACTIVABLE:8423";
}
