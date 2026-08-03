#include "dm2_v1_champion_hud_helpers.h"

#include <string.h>

static void dm2_champion_hud_receipt_begin(
    DM2_V1_ChampionHudReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_champion_hud_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static int16_t dm2_clamp_i16(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return (int16_t)minimum;
    }
    if (value > maximum) {
        return (int16_t)maximum;
    }
    return (int16_t)value;
}

void dm2_v1_champion_hud_receipt_clear(
    DM2_V1_ChampionHudReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

/* DM2_PROCESS_ITEM_BONUS -- c_item.cpp:59 (was SKW_2c1d_03e7).
 *
 * Register mapping:
 *   eax = hero_index (vql_04)
 *   edx = item_ref   (vw_00)
 *   ebx = slot        (RG2 at entry)
 *   ecx = mode        (RG6)
 */
void dm2_v1_PROCESS_ITEM_BONUS(
    const DM2_V1_ProcessItemBonusInput *input,
    const DM2_V1_ProcessItemBonusCallbacks *callbacks,
    DM2_V1_ProcessItemBonusReceipt *out_receipt)
{
    uint16_t dbspec_word0;
    int32_t fit_flag;
    int16_t bonus;
    int16_t mode;
    uint16_t item_ref;
    int walkspeed_changed;
    int i;

    if (!out_receipt) {
        return;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));

    /* Early exits: hero_index < 0 or item_ref == 0xFFFF. */
    if (!input || !callbacks) {
        out_receipt->blocked = 1;
        return;
    }
    if (input->hero_index < 0) {
        out_receipt->blocked = 1;
        return;
    }
    if (input->item_ref == 0xFFFFu) {
        out_receipt->blocked = 1;
        return;
    }

    mode = input->mode;
    item_ref = input->item_ref;

    /* slot >= 0x1E is not an equip slot -- skip to weight recalc. */
    if (input->slot >= 0x1E) {
        goto weight_check;
    }

    /* Query DBSPEC word 0 for this item. */
    if (!callbacks->query_dbspec_word) {
        out_receipt->blocked = 1;
        return;
    }
    dbspec_word0 = callbacks->query_dbspec_word(callbacks->ctx, item_ref, 0);

    /* Bit 0x10: light-level recalc if mode != 0. */
    if (mode != 0 && (dbspec_word0 & 0x10) != 0) {
        out_receipt->light_recalc_count++;
    }

    /* Bit 0x2000: equip bonus flag -- run the full bonus dispatch. */
    if ((dbspec_word0 & 0x2000) == 0) {
        goto weight_check;
    }

    /* IS_ITEM_FIT_FOR_EQUIP(item_ref, slot, 1). */
    if (!callbacks->is_item_fit_for_equip ||
        !callbacks->retrieve_item_bonus) {
        out_receipt->blocked = 1;
        return;
    }
    fit_flag = callbacks->is_item_fit_for_equip(callbacks->ctx,
                                                 item_ref, input->slot);
    walkspeed_changed = 0;

    /* --- MP bonus (DBSPEC index 0x14) --- */
    /* Skipped when mode is 0, 3, or -2. */
    if (mode != 0 && mode != 3 && mode != -2) {
        bonus = callbacks->retrieve_item_bonus(callbacks->ctx,
                    item_ref, 0x14, fit_flag, mode);
        if (bonus != 0) {
            if (mode == 1 || mode == -1) {
                /* maxMP += bonus */
                out_receipt->max_mp_delta += bonus;
                out_receipt->mp_dirty = 1;
            } else if (mode == 2) {
                /* curMP = clamp(curMP + bonus, 0, 999)
                 * Caller must supply current curMP and apply the clamp. */
                out_receipt->cur_mp_set = 1;
                out_receipt->cur_mp_value = bonus;
                out_receipt->mp_dirty = 1;
            }
            out_receipt->heroflag_or |= 0x800;
        }

        /* --- Ability loop (DBSPEC indices 0x15..0x1B, abilities 0..6) --- */
        for (i = 0; i < DM2_V1_NUM_ABILITIES; i++) {
            bonus = callbacks->retrieve_item_bonus(callbacks->ctx,
                        item_ref, (uint8_t)(0x15 + i), fit_flag, mode);
            if (bonus != 0) {
                if (mode == 1 || mode == -1) {
                    /* eability[i] += (int8_t)bonus */
                    out_receipt->eability_delta[i] += (int8_t)bonus;
                } else {
                    /* hero_2c1d_0300(hero, i, bonus) */
                    out_receipt->ability_adjust[i] += bonus;
                    out_receipt->ability_use_adjust[i] = 1;
                }
                out_receipt->ability_dirty = 1;
                out_receipt->heroflag_or |= 0x3000;
            }
        }
    }

    /* --- Skill bonus loop (DBSPEC indices 0x1E..0x31, skills 0..19) --- */
    for (i = 0; i < DM2_V1_NUM_SKILL_SLOTS; i++) {
        bonus = callbacks->retrieve_item_bonus(callbacks->ctx,
                    item_ref, (uint8_t)(0x1E + i), fit_flag, mode);
        if (bonus != 0) {
            out_receipt->sbonus_delta[i] += (int8_t)bonus;
            out_receipt->skill_dirty = 1;
            out_receipt->heroflag_or |= 0x2000;
            walkspeed_changed = 1;
        }
    }

    /* --- Walkspeed bonus (DBSPEC index 0x33) --- */
    bonus = callbacks->retrieve_item_bonus(callbacks->ctx,
                item_ref, 0x33, fit_flag, mode);
    if (bonus != 0) {
        out_receipt->walkspeed_delta += (int8_t)bonus;
        out_receipt->walkspeed_dirty = 1;
        walkspeed_changed = 1;
    }

    /* --- Light bonus (DBSPEC index 0x32) --- */
    /* Skipped when mode is 2, -2, or 3. */
    if (mode != 2 && mode != -2 && mode != 3) {
        bonus = callbacks->retrieve_item_bonus(callbacks->ctx,
                    item_ref, 0x32, fit_flag, mode);
        if (bonus != 0) {
            out_receipt->light_w00_delta += bonus;
            out_receipt->light_bonus_dirty = 1;
            if (mode != 0) {
                out_receipt->light_bonus_recalc = 1;
                out_receipt->light_recalc_count++;
            }
        }
    }

    /* --- Walkspeed timer (mode == 2 and walkspeed changed) --- */
    if (walkspeed_changed && mode == 2) {
        out_receipt->queue_timer = 1;
        if (callbacks->query_dbspec_word) {
            out_receipt->timer_dbspec_0x13 =
                callbacks->query_dbspec_word(callbacks->ctx, item_ref, 0x13);
        }
        out_receipt->timer_actor = (uint8_t)input->hero_index;
        /* timer_A: source does (hero_index ^ item_ref) & 0x3C then >>10.
         * The XOR is byte-level: low byte of hero_index XOR low byte of
         * item_ref, then high byte AND 0x3C, then shift >>10 as uint16. */
        {
            uint16_t tmp = (uint16_t)input->hero_index;
            tmp = (uint16_t)((tmp & 0xFF00u) |
                  ((uint8_t)tmp ^ (uint8_t)item_ref));
            tmp &= 0x3C00u;
            tmp >>= 10;
            out_receipt->timer_A = tmp;
        }
        if (callbacks->query_cls2_from_record) {
            out_receipt->timer_xB =
                callbacks->query_cls2_from_record(callbacks->ctx, item_ref);
        }
    }

    out_receipt->valid = 1;

weight_check:
    /* Weight recalc when mode != 0. */
    if (mode != 0) {
        out_receipt->weight_recalc = 1;
    }
}

uint16_t dm2_v1_QUERY_PLAYER_SKILL_LV(
    const DM2_V1_PlayerSkillInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt)
{
    uint32_t xp;
    uint16_t level;
    uint16_t maximum;

    dm2_champion_hud_receipt_begin(out_receipt,
                                   "QUERY_PLAYER_SKILL_LV",
                                   "SKWIN/SkWinCore.cpp:8381");
    if (!input) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    maximum = input->maximum_level;
    if (maximum == 0u || maximum > DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL) {
        maximum = DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL;
    }
    level = input->base_level;
    if (level > maximum) {
        level = maximum;
    }
    xp = input->experience;
    while (level < maximum && xp >= ((uint32_t)(level + 1u) * 1024u)) {
        xp -= (uint32_t)(level + 1u) * 1024u;
        ++level;
    }
    if (input->temporary_bonus > 0u) {
        uint32_t boosted = (uint32_t)level + input->temporary_bonus;
        level = boosted > maximum ? maximum : (uint16_t)boosted;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = (int)level;
        out_receipt->dirty = input->temporary_bonus > 0u;
    }
    return level;
}

int dm2_v1_REFRESH_PLAYER_STAT_DISP(
    const DM2_V1_PlayerStatDisplayInput *input,
    DM2_V1_PlayerStatDisplay *out_display,
    DM2_V1_ChampionHudReceipt *out_receipt)
{
    int percent;
    int dirty;

    if (out_display) {
        memset(out_display, 0, sizeof(*out_display));
    }
    dm2_champion_hud_receipt_begin(out_receipt,
                                   "REFRESH_PLAYER_STAT_DISP",
                                   "SKWIN/SkWinCore.cpp:14573");
    if (!input || !out_display || input->maximum_value <= 0 ||
        input->current_value < 0 || input->bar_color < 0) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    out_display->maximum_value = input->maximum_value;
    out_display->current_value =
        dm2_clamp_i16(input->current_value, 0, input->maximum_value);
    out_display->bar_color = input->bar_color;
    percent = ((int)out_display->current_value * 100) /
              (int)out_display->maximum_value;
    out_display->percent = (uint8_t)dm2_clamp_i16(percent, 0, 100);
    out_display->redraw_value =
        out_display->current_value != input->previous_current_value;
    out_display->redraw_maximum =
        out_display->maximum_value != input->previous_maximum_value;
    out_display->redraw_bar =
        out_display->redraw_value || out_display->redraw_maximum;
    dirty = out_display->redraw_value ||
            out_display->redraw_maximum ||
            out_display->redraw_bar;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = out_display->percent;
        out_receipt->dirty = dirty;
    }
    return 1;
}

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (gdat_value >= 0) {
        int16_t color = (int16_t)(256 + gdat_value);
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->color = color;
            out_receipt->gdat_override = 1;
        }
        return color;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->color = default_color;
        out_receipt->gdat_override = 0;
    }
    return default_color;
}

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (gdat_value >= 0) {
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->color = gdat_value;
            out_receipt->gdat_override = 1;
        }
        return gdat_value;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->color = default_color;
        out_receipt->gdat_override = 0;
    }
    return default_color;
}

const char *dm2_v1_champion_hud_helpers_source_evidence(void)
{
    return "skproject SKULLWIN/c_item.cpp PROCESS_ITEM_BONUS:59 "
           "QUERY_PLAYER_SKILL_LV:8381 REFRESH_PLAYER_STAT_DISP:14573 "
           "QUERY_FOOD_WATER_BAR_COLOR:13194 QUERY_3STAT_BAR_COLOR:13203; "
           "bounded champion/HUD receipts only.";
}
