/**
 * DM1 V1 Light & Torch System — source-locked to ReDMCSB
 *
 * ReDMCSB source references:
 *   DEFS.H        — Light constants (C00_LIGHT0..C05_LIGHT5), WEAPON struct (Lit, ChargeCount),
 *                   C70_EVENT_LIGHT, G0039/G0040 lookup tables, C02_WEAPON_TORCH
 *   PANEL.C       — F0337_INVENTORY_SetDungeonViewPalette: calculates total light from
 *                   torches + MagicalLightAmount, selects palette index 0-5
 *                 — F0338_INVENTORY_DecreaseTorchesLightPower_CPSE: per-tick torch fuel depletion
 *   GAMELOOP.C    — Line 126: calls F0338 every 512 game ticks (GameTime & 511 == 0)
 *   TIMELINE.C    — F0257_TIMELINE_ProcessEvent70_Light: gradual fade of magical light
 *   MENU.C        — F0404_MENUS_CreateEvent70_Light: schedules light fade events
 *                 — Spell effects: FUL (Light), OH IR RA (Magic Torch), DES IR SAR (Darkness)
 *                 — C038_ACTION_LIGHT: weapon light action (Yew Staff)
 *   CHAMPION.C    — Equip/unequip: torch Lit flag, Illumulet MagicalLightAmount adjustment
 *   DATA.C        — LightPowerToLightAmount[16], PaletteIndexToLightAmount[6],
 *                   ChargeCountToTorchType[16]
 *   DARKCOLR.C    — F0431_STARTEND_GetDarkenedColor: per-component RGB darkening
 *   DRAWVIEW.C    — Palette switch: G0304_i_DungeonViewPaletteIndex selects from
 *                   G0021_aaui_Graphic562_Palette_DungeonView[6]
 *
 * CSBWin cross-reference (CSBCode.cpp ~7274): identical algorithm confirmed
 */

#ifndef FIRESTAFF_DM1_V1_LIGHT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LIGHT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants from ReDMCSB DEFS.H ──────────────────────────────────── */

/** Palette brightness levels (6 palettes: 0=brightest, 5=darkest)
 *  ReDMCSB: C00_LIGHT0..C05_LIGHT5 */
#define DM1_LIGHT_PALETTE_BRIGHTEST  0
#define DM1_LIGHT_PALETTE_DARKEST    5
#define DM1_LIGHT_PALETTE_COUNT      6

/** Maximum torch charges (4-bit field in WEAPON struct) */
#define DM1_TORCH_MAX_CHARGES        15

/** Max champions in party */
#define DM1_PARTY_MAX_CHAMPIONS      4

/** Number of hand slots per champion (action hand + ready hand) */
#define DM1_CHAMPION_HAND_SLOTS      2

/** Total possible torch slots: 4 champions × 2 hands */
#define DM1_MAX_TORCH_SLOTS          (DM1_PARTY_MAX_CHAMPIONS * DM1_CHAMPION_HAND_SLOTS)

/** Torch fuel depletion interval: every 512 game ticks
 *  ReDMCSB GAMELOOP.C: if (!((int16_t)G0313_ul_GameTime & 511)) */
#define DM1_TORCH_DEPLETION_MASK     511

/** Light event gradual fade interval: 4 game ticks
 *  ReDMCSB TIMELINE.C F0257: GameTime + 4 */
#define DM1_LIGHT_EVENT_FADE_TICKS   4

/* ── Lookup tables from ReDMCSB DATA.C ──────────────────────────────── */

/** LightPowerToLightAmount[16]: maps torch ChargeCount (0-15) to light amount
 *  ReDMCSB DATA.C line 359/1088:
 *    { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 } */
extern const int16_t dm1_light_power_to_amount[16];

/** PaletteIndexToLightAmount[6]: thresholds for palette selection
 *  ReDMCSB DATA.C line 360/1089:
 *    { 99, 75, 50, 25, 1, 0 } */
extern const int16_t dm1_palette_index_to_light_amount[6];

/** ChargeCountToTorchType[16]: maps charge count to visual torch type (0-3)
 *  ReDMCSB DATA.C line 263/926:
 *    { 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 } */
extern const uint8_t dm1_charge_count_to_torch_type[16];

/* ── Data structures ────────────────────────────────────────────────── */

/** Per-hand torch state.
 *  ReDMCSB DEFS.H WEAPON struct: ChargeCount (4-bit), Lit (1-bit) */
typedef struct {
    int16_t charge_count;   /**< 0-15: remaining fuel (ReDMCSB: ChargeCount) */
    bool    lit;            /**< true if torch is actively lit */
    bool    do_not_discard; /**< ReDMCSB: DoNotDiscard flag, cleared when charge=0 */
} DM1_TorchSlot;

/** Pending magical light fade event.
 *  ReDMCSB TIMELINE.C F0257: Event70_Light uses LightPower stepping
 *  down one level per event, 4 ticks apart. */
typedef struct {
    int16_t light_power;    /**< Current light power level (negative=fading light, positive=fading darkness) */
    int32_t expire_tick;    /**< Game tick when this fade step fires */
} DM1_LightEvent;

#define DM1_MAX_LIGHT_EVENTS 16

/** Complete light system state.
 *  Models the full ReDMCSB light pipeline. */
typedef struct {
    /* Torch state: 4 champions x 2 hands = 8 slots */
    DM1_TorchSlot torch_slots[DM1_MAX_TORCH_SLOTS];
    int           champion_count;       /**< Active champions (0-4) */

    /* Magical light (spells, items like Illumulet)
     * ReDMCSB: G0407_s_Party.MagicalLightAmount */
    int16_t       magical_light_amount;

    /* Pending light fade events
     * ReDMCSB TIMELINE.C F0257: Event70_Light chain */
    DM1_LightEvent light_events[DM1_MAX_LIGHT_EVENTS];
    int            light_event_count;

    /* Computed outputs */
    int16_t       total_light_amount;       /**< Combined torch + magic light */
    int           dungeon_view_palette_idx; /**< 0-5 (ReDMCSB: G0304_i_DungeonViewPaletteIndex) */
    bool          refresh_palette_requested;/**< ReDMCSB: G0342_B_RefreshDungeonViewPaletteRequested */

    /* Game time tracking */
    uint32_t      game_time;                /**< Current game tick */
} DM1_LightState;

/* ── API ────────────────────────────────────────────────────────────── */

/** Initialize light state to full darkness (no light sources). */
void dm1_light_init(DM1_LightState *s);

/** Set the number of active champions. */
void dm1_light_set_champion_count(DM1_LightState *s, int count);

/** Set a torch in a specific slot (champion_idx 0-3, hand 0=ready, 1=action).
 *  ReDMCSB CHAMPION.C: equip sets Lit=true, unequip sets Lit=false */
void dm1_light_set_torch(DM1_LightState *s, int champion_idx, int hand,
                         int16_t charge_count, bool lit);

/** Clear a torch slot. */
void dm1_light_clear_torch(DM1_LightState *s, int champion_idx, int hand);

/** Add magical light (FUL spell, Illumulet, OH IR RA, weapon Light action).
 *  ReDMCSB MENU.C: MagicalLightAmount += LightPowerToLightAmount[power];
 *                  CreateEvent70_Light(-power, duration); */
void dm1_light_add_magical(DM1_LightState *s, int16_t light_power,
                           int32_t duration_ticks);

/** Apply darkness spell effect.
 *  ReDMCSB MENU.C C1_SPELL_TYPE_OTHER_DARKNESS */
void dm1_light_add_darkness(DM1_LightState *s, int16_t light_power);

/** Advance game time by one tick. Handles torch depletion and light events. */
void dm1_light_tick(DM1_LightState *s);

/** Recalculate total light and palette index.
 *  Implements F0337_INVENTORY_SetDungeonViewPalette. */
void dm1_light_recalculate_palette(DM1_LightState *s);

/** Decrease torch fuel for all lit torches.
 *  Implements F0338_INVENTORY_DecreaseTorchesLightPower_CPSE. */
void dm1_light_decrease_torches(DM1_LightState *s);

/** Process pending light fade events.
 *  Implements F0257_TIMELINE_ProcessEvent70_Light. */
void dm1_light_process_events(DM1_LightState *s);

/** Get the current dungeon view palette index (0=brightest, 5=darkest). */
int dm1_light_get_palette_index(const DM1_LightState *s);

/** Get total computed light amount. */
int16_t dm1_light_get_total_amount(const DM1_LightState *s);

/** Get torch visual type (0-3) for a slot. */
int dm1_light_get_torch_type(const DM1_LightState *s, int champion_idx, int hand);

/** Darken an RGB444 color by one step per component.
 *  Implements F0431_STARTEND_GetDarkenedColor from DARKCOLR.C. */
uint16_t dm1_light_darken_rgb444(uint16_t rgb444);

/* ── Legacy API (thin wrappers for backwards compat) ────────────────── */

typedef DM1_LightState M11_LightState;
typedef DM1_TorchSlot  M11_LightSource;

#define M11_LIGHT_MAX_SOURCES DM1_MAX_TORCH_SLOTS
enum { DM1_LIGHT_SOURCE_TORCH = 0, DM1_LIGHT_SOURCE_SPELL, DM1_LIGHT_SOURCE_ITEM, DM1_LIGHT_SOURCE_COUNT };

void m11_light_init(M11_LightState *s);
void m11_light_tick(M11_LightState *s, int tick_ms);
int  m11_light_get_level_at_depth(const M11_LightState *s, int view_depth);
int  m11_light_get_darkness_mask(const M11_LightState *s, int view_depth);
void m11_light_apply_torch(M11_LightState *s, int torch_power);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_LIGHT_PC34_COMPAT_H */
