/**
 * DM1 V1 Light & Torch System — source-locked to ReDMCSB
 *
 * Every function cites its ReDMCSB source. See header for full reference list.
 */

#include "dm1_v1_light_pc34_compat.h"
#include <string.h>

/* ── Lookup tables from ReDMCSB DATA.C ──────────────────────────────── */

/* ReDMCSB DATA.C line 359/1088 */
const int16_t dm1_light_power_to_amount[16] = {
    0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100
};

/* ReDMCSB DATA.C line 360/1089 */
const int16_t dm1_palette_index_to_light_amount[6] = {
    99, 75, 50, 25, 1, 0
};

/* ReDMCSB DATA.C line 263/926 */
const uint8_t dm1_charge_count_to_torch_type[16] = {
    0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
};

/* ── Internal helpers ───────────────────────────────────────────────── */

/** ReDMCSB DEFS.H: F0025_MAIN_GetMaximumValue */
static inline int dm1_max(int a, int b) { return a > b ? a : b; }

/** Clamp charge to valid 4-bit range */
static inline int16_t dm1_clamp_charge(int16_t c) {
    if (c < 0) return 0;
    if (c > DM1_TORCH_MAX_CHARGES) return DM1_TORCH_MAX_CHARGES;
    return c;
}

/** Get torch slot index from champion/hand */
static inline int dm1_slot_index(int champion_idx, int hand) {
    return champion_idx * DM1_CHAMPION_HAND_SLOTS + hand;
}

/* ── Initialization ─────────────────────────────────────────────────── */

void dm1_light_init(DM1_LightState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    /* Start in full darkness: palette index 5 (darkest)
     * ReDMCSB: initial G0304_i_DungeonViewPaletteIndex = 5 with no light */
    s->dungeon_view_palette_idx = DM1_LIGHT_PALETTE_DARKEST;
    s->refresh_palette_requested = true;
}

void dm1_light_set_champion_count(DM1_LightState *s, int count) {
    if (!s) return;
    if (count < 0) count = 0;
    if (count > DM1_PARTY_MAX_CHAMPIONS) count = DM1_PARTY_MAX_CHAMPIONS;
    s->champion_count = count;
}

/* ── Torch management ───────────────────────────────────────────────── */

/**
 * ReDMCSB CHAMPION.C F0299/~line 631:
 *   When placing torch in hand slot:
 *     L0901_ps_Weapon->Lit = C1_TRUE;
 *     F0337_INVENTORY_SetDungeonViewPalette();
 */
void dm1_light_set_torch(DM1_LightState *s, int champion_idx, int hand,
                         int16_t charge_count, bool lit) {
    if (!s) return;
    if (champion_idx < 0 || champion_idx >= DM1_PARTY_MAX_CHAMPIONS) return;
    if (hand < 0 || hand >= DM1_CHAMPION_HAND_SLOTS) return;
    int idx = dm1_slot_index(champion_idx, hand);
    s->torch_slots[idx].charge_count = dm1_clamp_charge(charge_count);
    s->torch_slots[idx].lit = lit;
    s->torch_slots[idx].do_not_discard = (charge_count > 0);
    dm1_light_recalculate_palette(s);
}

/**
 * ReDMCSB CHAMPION.C ~line 552:
 *   When removing torch from hand:
 *     L0897_ps_Weapon->Lit = C0_FALSE;
 *     F0337_INVENTORY_SetDungeonViewPalette();
 */
void dm1_light_clear_torch(DM1_LightState *s, int champion_idx, int hand) {
    if (!s) return;
    if (champion_idx < 0 || champion_idx >= DM1_PARTY_MAX_CHAMPIONS) return;
    if (hand < 0 || hand >= DM1_CHAMPION_HAND_SLOTS) return;
    int idx = dm1_slot_index(champion_idx, hand);
    s->torch_slots[idx].charge_count = 0;
    s->torch_slots[idx].lit = false;
    s->torch_slots[idx].do_not_discard = false;
    dm1_light_recalculate_palette(s);
}

/* ── Magical light ──────────────────────────────────────────────────── */

/**
 * ReDMCSB MENU.C ~line 1936 (FUL/OH IR RA spells):
 *   G0407_s_Party.MagicalLightAmount += G0039_ai_Graphic562_LightPowerToLightAmount[power];
 *   F0404_MENUS_CreateEvent70_Light(-power, ticks);
 *
 * ReDMCSB MENU.C ~line 1608 (ACTION_LIGHT / Yew Staff):
 *   MagicalLightAmount += LightPowerToLightAmount[2];
 *   F0404_MENUS_CreateEvent70_Light(-2, 2500);
 */
void dm1_light_add_magical(DM1_LightState *s, int16_t light_power,
                           int32_t duration_ticks) {
    if (!s) return;
    if (light_power < 1 || light_power > 15) return;

    /* Immediately add light */
    s->magical_light_amount += dm1_light_power_to_amount[light_power];

    /* Schedule fade event (negative power = light fading away) */
    if (s->light_event_count < DM1_MAX_LIGHT_EVENTS) {
        DM1_LightEvent *ev = &s->light_events[s->light_event_count++];
        ev->light_power = -light_power;
        ev->expire_tick = (int32_t)s->game_time + duration_ticks;
    }

    dm1_light_recalculate_palette(s);
}

/**
 * ReDMCSB MENU.C ~line 1941 (DES IR SAR darkness spell):
 *   G0407_s_Party.MagicalLightAmount -= LightPowerToLightAmount[power];
 *   F0404_MENUS_CreateEvent70_Light(+power, 98);
 */
void dm1_light_add_darkness(DM1_LightState *s, int16_t light_power) {
    if (!s) return;
    if (light_power < 1 || light_power > 15) return;

    /* Immediately remove light */
    s->magical_light_amount -= dm1_light_power_to_amount[light_power];

    /* Schedule recovery event (positive power = darkness fading away) */
    if (s->light_event_count < DM1_MAX_LIGHT_EVENTS) {
        DM1_LightEvent *ev = &s->light_events[s->light_event_count++];
        ev->light_power = light_power;
        ev->expire_tick = (int32_t)s->game_time + 98;
    }

    dm1_light_recalculate_palette(s);
}

/* ── MENU.C F0412 light/darkness spell effects ─────────────────────── */

/**
 * ReDMCSB MENU.C F0412 lines 1922-1942:
 *   SpellPower = (PowerSymbolOrdinal + 1) << 2
 *   Light:    ticks = 10000 + ((SpellPower - 8) << 9)
 *             lightPower = (SpellPower >> 1) - 1
 *             MagicalLightAmount += LightPowerToLightAmount[lightPower]
 *             CreateEvent70_Light(-lightPower, ticks)
 *   Darkness: lightPower = SpellPower >> 2
 *             MagicalLightAmount -= LightPowerToLightAmount[lightPower]
 *             CreateEvent70_Light(+lightPower, 98)
 */
int dm1_light_apply_other_spell_effect(DM1_LightState *s, int other_spell_type,
                                       int power_symbol_ordinal) {
    if (!s) return 0;
    if (power_symbol_ordinal < DM1_LIGHT_POWER_SYMBOL_ORDINAL_MIN ||
        power_symbol_ordinal > DM1_LIGHT_POWER_SYMBOL_ORDINAL_MAX) {
        return 0;
    }

    int spell_power = (power_symbol_ordinal + 1) << 2;

    switch (other_spell_type) {
    case DM1_LIGHT_SPELL_TYPE_OTHER_LIGHT: {
        int16_t light_power = (int16_t)((spell_power >> 1) - 1);
        int32_t ticks = 10000 + ((spell_power - 8) << 9);
        dm1_light_add_magical(s, light_power, ticks);
        return 1;
    }
    case DM1_LIGHT_SPELL_TYPE_OTHER_DARKNESS:
        dm1_light_add_darkness(s, (int16_t)(spell_power >> 2));
        return 1;
    default:
        return 0;
    }
}

/* ── F0337_INVENTORY_SetDungeonViewPalette ──────────────────────────── */

/**
 * ReDMCSB PANEL.C F0337 (lines 329-432):
 *
 * Algorithm:
 *   1. Collect torch light powers from all 8 slots (4 champions × 2 hands)
 *   2. Sort: find top 4 values via selection sort
 *   3. Sum top 5 torches with decreasing weight:
 *      multiplier starts at 6, decreases by 1 (min 0) for each torch
 *      contribution = (LightPowerToLightAmount[chargeCount] << multiplier) >> 6
 *   4. Add MagicalLightAmount
 *   5. Select palette: walk PaletteIndexToLightAmount[] to find threshold
 *
 * BUG0_01: ReDMCSB always inspects 4 champions even if fewer exist.
 * We replicate this: unused slots have charge_count=0, which contributes 0.
 */
void dm1_light_recalculate_palette(DM1_LightState *s) {
    if (!s) return;

    int16_t torch_powers[DM1_MAX_TORCH_SLOTS];

    /* Step 1: Collect torch light powers from all 8 slots
     * ReDMCSB: L1038_i_Counter = 4; BUG0_01 */
    for (int i = 0; i < DM1_MAX_TORCH_SLOTS; i++) {
        /* Only lit torches with charge contribute light power.
         * ReDMCSB checks icon index range (TORCH_UNLIT..TORCH_LIT);
         * we use the lit flag + charge as equivalent. */
        if (s->torch_slots[i].lit && s->torch_slots[i].charge_count > 0) {
            torch_powers[i] = s->torch_slots[i].charge_count;
        } else {
            torch_powers[i] = 0;
        }
    }

    /* Step 2: Selection sort - put top 4 in first 4 positions
     * ReDMCSB PANEL.C ~line 395-408:
     *   for D4W=0; D4W!=4; D4W++
     *     for each remaining, swap if larger */
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < DM1_MAX_TORCH_SLOTS; j++) {
            if (torch_powers[j] > torch_powers[i]) {
                int16_t tmp = torch_powers[j];
                torch_powers[j] = torch_powers[i];
                torch_powers[i] = tmp;
            }
        }
    }

    /* Step 3: Sum top 5 torches with weighted formula
     * ReDMCSB PANEL.C ~line 409-416:
     *   L1037_ui_TorchLightAmountMultiplier = 6;
     *   AL1039_ui_Counter = 5;
     *   for each: amount += (LightPowerToLightAmount[power] << mult) >> 6;
     *             mult = max(0, mult - 1); */
    int16_t total = 0;
    int multiplier = 6;
    for (int i = 0; i < 5; i++) {
        if (torch_powers[i] > 0) {
            int16_t power = torch_powers[i];
            if (power > 15) power = 15;
            total += (int16_t)((dm1_light_power_to_amount[power] << multiplier) >> 6);
            multiplier = dm1_max(0, multiplier - 1);
        }
    }

    /* Step 4: Add magical light
     * ReDMCSB: L1036_i_TotalLightAmount += G0407_s_Party.MagicalLightAmount; */
    total += s->magical_light_amount;
    s->total_light_amount = total;

    /* Step 5: Select palette index from thresholds
     * ReDMCSB PANEL.C ~line 418-427:
     *   if (total > 0) { idx=0; while (threshold[idx] > total) idx++; }
     *   else { idx = 5; } */
    int palette_idx;
    if (total > 0) {
        palette_idx = 0;
        const int16_t *threshold = dm1_palette_index_to_light_amount;
        while (palette_idx < DM1_LIGHT_PALETTE_DARKEST && *threshold > total) {
            threshold++;
            palette_idx++;
        }
    } else {
        palette_idx = DM1_LIGHT_PALETTE_DARKEST;
    }

    s->dungeon_view_palette_idx = palette_idx;
    s->refresh_palette_requested = true;
}

/* ── F0338_INVENTORY_DecreaseTorchesLightPower_CPSE ─────────────────── */

/**
 * ReDMCSB PANEL.C F0338 (lines 434-499):
 *
 * For each champion's action hand (slot 1) and ready hand (slot 0):
 *   If icon is TORCH_UNLIT..TORCH_LIT and ChargeCount > 0:
 *     --ChargeCount
 *     If ChargeCount == 0: DoNotDiscard = false
 *     Set TorchChargeCountChanged = true
 *
 * If any charge changed: recalculate palette and redraw icons.
 *
 * Note: ReDMCSB iterates slots 0,1 (ready hand first, then action hand).
 *       We iterate all champion hand slots in the same order.
 */
void dm1_light_decrease_torches(DM1_LightState *s) {
    if (!s) return;

    bool changed = false;
    /* ReDMCSB: L1046_i_ChampionCount = G0305_ui_PartyChampionCount
     * BUG0_01 applies here too but F0338 uses actual champion count */
    int count = s->champion_count;
    for (int c = 0; c < count; c++) {
        /* ReDMCSB: L1047_i_SlotIndex = C01_SLOT_ACTION_HAND + 1;
         *          while (L1047_i_SlotIndex--) { ... }
         * This iterates slot 1 (action) then slot 0 (ready) */
        for (int h = DM1_CHAMPION_HAND_SLOTS - 1; h >= 0; h--) {
            int idx = dm1_slot_index(c, h);
            DM1_TorchSlot *t = &s->torch_slots[idx];
            /* Only deplete lit torches with remaining charge */
            if (t->lit && t->charge_count > 0) {
                t->charge_count--;
                if (t->charge_count == 0) {
                    t->do_not_discard = false;
                }
                changed = true;
            }
        }
    }

    if (changed) {
        dm1_light_recalculate_palette(s);
    }
}

/* ── F0257_TIMELINE_ProcessEvent70_Light ────────────────────────────── */

/**
 * ReDMCSB TIMELINE.C F0257 (lines 1720-1772):
 *
 * if (lightPower == 0) return;
 * negative = (lightPower < 0);
 * if (negative) lightPower = -lightPower;
 * weakerPower = lightPower - 1;
 * lightAmount = LightPowerToLightAmount[lightPower] - LightPowerToLightAmount[weakerPower];
 * if (negative) { lightAmount = -lightAmount; weakerPower = -weakerPower; }
 * MagicalLightAmount += lightAmount;
 * if (weakerPower != 0) schedule new Event70 with weakerPower at GameTime+4;
 */
void dm1_light_process_events(DM1_LightState *s) {
    if (!s) return;

    bool any_processed = false;

    for (int i = 0; i < s->light_event_count; ) {
        DM1_LightEvent *ev = &s->light_events[i];

        if ((int32_t)s->game_time < ev->expire_tick) {
            i++;
            continue;
        }

        /* Process this event */
        int16_t lp = ev->light_power;
        if (lp == 0) {
            /* Remove expired zero-power event */
            goto remove_event;
        }

        bool negative = (lp < 0);
        int16_t abs_power = negative ? -lp : lp;
        int16_t weaker = abs_power - 1;

        /* Calculate light delta for this step */
        int16_t delta = dm1_light_power_to_amount[abs_power]
                      - dm1_light_power_to_amount[weaker];

        if (negative) {
            s->magical_light_amount -= delta;
        } else {
            s->magical_light_amount += delta;
        }

        any_processed = true;

        if (weaker > 0) {
            /* Schedule next step: same sign, reduced power, +4 ticks */
            ev->light_power = negative ? -weaker : weaker;
            ev->expire_tick = (int32_t)s->game_time + DM1_LIGHT_EVENT_FADE_TICKS;
            i++;
            continue;
        }

    remove_event:
        /* Remove this event by shifting remaining */
        for (int j = i; j < s->light_event_count - 1; j++) {
            s->light_events[j] = s->light_events[j + 1];
        }
        s->light_event_count--;
        /* Don't increment i — next event is now at this index */
    }

    if (any_processed) {
        dm1_light_recalculate_palette(s);
    }
}

/* ── Main tick ──────────────────────────────────────────────────────── */

/**
 * ReDMCSB GAMELOOP.C ~line 124-126:
 *   G0313_ul_GameTime++;
 *   if (!((int16_t)G0313_ul_GameTime & 511)) {
 *       F0338_INVENTORY_DecreaseTorchesLightPower_CPSE();
 *   }
 */
void dm1_light_tick(DM1_LightState *s) {
    if (!s) return;

    s->game_time++;

    /* Torch fuel depletion every 512 ticks
     * ReDMCSB uses (int16_t)GameTime & 511, casting to signed 16-bit.
     * We replicate this cast for exact parity. */
    if (!((int16_t)s->game_time & DM1_TORCH_DEPLETION_MASK)) {
        dm1_light_decrease_torches(s);
    }

    /* Process any expired light events */
    dm1_light_process_events(s);
}

/* ── Query functions ────────────────────────────────────────────────── */

int dm1_light_get_palette_index(const DM1_LightState *s) {
    if (!s) return DM1_LIGHT_PALETTE_DARKEST;
    return s->dungeon_view_palette_idx;
}

int16_t dm1_light_get_total_amount(const DM1_LightState *s) {
    if (!s) return 0;
    return s->total_light_amount;
}

/**
 * ReDMCSB DATA.C: G0029_auc_Graphic562_ChargeCountToTorchType
 */
int dm1_light_get_torch_type(const DM1_LightState *s, int champion_idx, int hand) {
    if (!s) return 0;
    if (champion_idx < 0 || champion_idx >= DM1_PARTY_MAX_CHAMPIONS) return 0;
    if (hand < 0 || hand >= DM1_CHAMPION_HAND_SLOTS) return 0;
    int idx = dm1_slot_index(champion_idx, hand);
    int16_t cc = s->torch_slots[idx].charge_count;
    if (cc < 0) cc = 0;
    if (cc > 15) cc = 15;
    return dm1_charge_count_to_torch_type[cc];
}

/* ── F0431_STARTEND_GetDarkenedColor ────────────────────────────────── */

/**
 * ReDMCSB DARKCOLR.C F0431:
 *   For each RGB component (4 bits each in 0x0RGB format):
 *     if (component > 0) component--;
 *
 * Masks from ReDMCSB:
 *   M512_MASK_BLUE_COMPONENT  = 0x000F
 *   M511_MASK_GREEN_COMPONENT = 0x00F0
 *   M510_MASK_RED_COMPONENT   = 0x0F00
 */
uint16_t dm1_light_darken_rgb444(uint16_t rgb444) {
    /* Blue component (bits 0-3) */
    if (rgb444 & 0x000F) {
        rgb444--;
    }
    /* Green component (bits 4-7) */
    if (rgb444 & 0x00F0) {
        rgb444 -= 0x0010;
    }
    /* Red component (bits 8-11) */
    if (rgb444 & 0x0F00) {
        rgb444 -= 0x0100;
    }
    return rgb444;
}

/* ── Legacy API wrappers ────────────────────────────────────────────── */

void m11_light_init(M11_LightState *s) {
    dm1_light_init(s);
}

void m11_light_tick(M11_LightState *s, int tick_ms) {
    if (!s) return;
    /* Legacy API used milliseconds; convert roughly.
     * One DM1 game tick ≈ 1/6 second. */
    (void)tick_ms;
    dm1_light_tick(s);
}

int m11_light_get_level_at_depth(const M11_LightState *s, int view_depth) {
    if (!s) return 0;
    /* Approximate: total light reduced by depth.
     * ReDMCSB doesn't have a per-depth light level directly —
     * it uses palette index for the entire viewport.
     * This provides a useful approximation for callers that need it. */
    int level = s->total_light_amount - view_depth * 20;
    return level > 0 ? level : 0;
}

int m11_light_get_darkness_mask(const M11_LightState *s, int view_depth) {
    return 100 - m11_light_get_level_at_depth(s, view_depth);
}

void m11_light_apply_torch(M11_LightState *s, int torch_power) {
    if (!s) return;
    /* Legacy: add torch to first available slot of champion 0 */
    dm1_light_set_champion_count(s, 1);
    dm1_light_set_torch(s, 0, 1, (int16_t)torch_power, true);
}
