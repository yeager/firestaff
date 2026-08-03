#ifndef FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H
#define FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL 15u
#define DM2_V1_CHAMPION_HUD_NULL_ITEM 0xffffu
#define DM2_V1_NUM_ABILITIES  7
#define DM2_V1_NUM_SKILL_SLOTS 20

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int result;
    int dirty;
    const char *symbol;
    const char *source_path;
} DM2_V1_ChampionHudReceipt;

/* -------------------------------------------------------------------
 * DM2_PROCESS_ITEM_BONUS (c_item.cpp:59, was SKW_2c1d_03e7)
 *
 * Full item-equip bonus processor.  When a hero equips or unequips an
 * item, this function queries the item's DBSPEC flags and dispatches
 * per-slot RETRIEVE_ITEM_BONUS calls to update MP, abilities, skill
 * bonuses, walkspeed, and ambient light.
 *
 * Parameters from the source (register mapping):
 *   eax = hero_index       (vql_04 -- which hero)
 *   edx = item_ref         (vw_00 -- the item record handle)
 *   ebx = slot             (RG2   -- equip slot index, < 0x1E = equip)
 *   ecx = mode             (RG6   -- operation mode)
 *
 * Mode values:
 *    0  = query only (no MP/ability/skill changes, no timer, no weight)
 *    1  = equip: add to maxMP, eability
 *   -1  = unequip: add to maxMP, eability (negative bonus)
 *    2  = activate: add to curMP (clamped 0..999), hero_2c1d_0300 for
 *         abilities, queue walkspeed timer
 *    3  = skip MP/ability/skill bonus loops entirely
 *   -2  = skip MP/ability/skill bonus loops entirely
 * ------------------------------------------------------------------- */

/* Callbacks for external queries the algorithm needs. */
typedef struct {
    /* DM2_QUERY_GDAT_DBSPEC_WORD_VALUE(item_ref, index). */
    uint16_t (*query_dbspec_word)(void *ctx, uint16_t item_ref,
                                  int16_t index);
    /* DM2_IS_ITEM_FIT_FOR_EQUIP(item_ref, slot, 1). Returns fit flag. */
    int32_t  (*is_item_fit_for_equip)(void *ctx, uint16_t item_ref,
                                       int16_t slot);
    /* DM2_RETRIEVE_ITEM_BONUS(item_ref, dbspec_idx, fit_flag, mode).
     * Returns the signed bonus value (int16_t). */
    int16_t  (*retrieve_item_bonus)(void *ctx, uint16_t item_ref,
                                     uint8_t dbspec_idx,
                                     int32_t fit_flag, int16_t mode);
    /* DM2_QUERY_CLS2_FROM_RECORD(item_ref). */
    uint8_t  (*query_cls2_from_record)(void *ctx, uint16_t item_ref);
    void *ctx;
} DM2_V1_ProcessItemBonusCallbacks;

typedef struct {
    int16_t  hero_index;    /* eax: which hero (< 0 = bail) */
    uint16_t item_ref;      /* edx: item record (0xFFFF = bail) */
    int16_t  slot;          /* ebx: equip slot index */
    int16_t  mode;          /* ecx: operation mode */
} DM2_V1_ProcessItemBonusInput;

/* Receipt: all hero mutations produced by the algorithm.
 * The caller applies these to the actual hero struct. */
typedef struct {
    int valid;              /* 1 if function ran successfully */
    int blocked;            /* 1 if early-exit (bad input) */

    /* Light recalc requests (count: 0, 1, or 2). */
    int light_recalc_count;

    /* MP changes. */
    int mp_dirty;
    int16_t max_mp_delta;   /* add to hero->maxMP */
    int cur_mp_set;         /* 1 if cur_mp_value should replace curMP */
    int16_t cur_mp_value;   /* raw bonus; caller adds to curMP and clamps */

    /* Ability changes. */
    int ability_dirty;
    int8_t eability_delta[DM2_V1_NUM_ABILITIES];   /* add to eability[] */
    int16_t ability_adjust[DM2_V1_NUM_ABILITIES];  /* hero_2c1d_0300 delta */
    int ability_use_adjust[DM2_V1_NUM_ABILITIES];  /* 1 = use adjust path */

    /* Skill bonus changes. */
    int skill_dirty;
    int8_t sbonus_delta[DM2_V1_NUM_SKILL_SLOTS];  /* add to sbonus[i/4][i%4] */

    /* Walkspeed. */
    int walkspeed_dirty;
    int8_t walkspeed_delta; /* add to hero->walkspeed */

    /* Ambient light. */
    int light_bonus_dirty;
    int16_t light_w00_delta; /* add to ddat.savegames1.w_00 */
    int light_bonus_recalc;  /* 1 if RECALC_LIGHT_LEVEL after light bonus */

    /* Heroflag ORs (accumulated). */
    int16_t heroflag_or;

    /* Walkspeed timer (mode == 2 only). */
    int queue_timer;
    uint16_t timer_dbspec_0x13;  /* QUERY_GDAT_DBSPEC_WORD_VALUE(item, 0x13) */
    uint8_t timer_actor;         /* hero_index low byte */
    uint16_t timer_A;            /* computed timer field A */
    uint8_t timer_xB;            /* QUERY_CLS2_FROM_RECORD(item) */

    /* Weight recalc. */
    int weight_recalc;
} DM2_V1_ProcessItemBonusReceipt;

typedef struct {
    uint32_t experience;
    uint16_t base_level;
    uint16_t temporary_bonus;
    uint16_t maximum_level;
} DM2_V1_PlayerSkillInput;

typedef struct {
    int16_t current_value;
    int16_t maximum_value;
    int16_t previous_current_value;
    int16_t previous_maximum_value;
    int16_t bar_color;
} DM2_V1_PlayerStatDisplayInput;

typedef struct {
    int16_t current_value;
    int16_t maximum_value;
    uint8_t percent;
    int16_t bar_color;
    uint8_t redraw_value;
    uint8_t redraw_maximum;
    uint8_t redraw_bar;
} DM2_V1_PlayerStatDisplay;

void dm2_v1_champion_hud_receipt_clear(
    DM2_V1_ChampionHudReceipt *receipt);

/* Full PROCESS_ITEM_BONUS: c_item.cpp:59.
 * Queries item DBSPEC flags and dispatches per-slot bonus calls.
 * All hero mutations are written to out_receipt for the caller to apply. */
void dm2_v1_PROCESS_ITEM_BONUS(
    const DM2_V1_ProcessItemBonusInput *input,
    const DM2_V1_ProcessItemBonusCallbacks *callbacks,
    DM2_V1_ProcessItemBonusReceipt *out_receipt);

uint16_t dm2_v1_QUERY_PLAYER_SKILL_LV(
    const DM2_V1_PlayerSkillInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt);

int dm2_v1_REFRESH_PLAYER_STAT_DISP(
    const DM2_V1_PlayerStatDisplayInput *input,
    DM2_V1_PlayerStatDisplay *out_display,
    DM2_V1_ChampionHudReceipt *out_receipt);

/* Bar color queries.
 * Source: SkWinCore.cpp:13194 QUERY_FOOD_WATER_BAR_COLOR
 *         SkWinCore.cpp:13203 QUERY_3STAT_BAR_COLOR
 *
 * Both accept an optional GDAT override (gdat_value >= 0 means GDAT
 * entry was found; < 0 means not found -> use default). */

#define DM2_V1_CHAMPION_HUD_DEFAULT_FOOD_COLOR   5   /* COLOR_BROWN */
#define DM2_V1_CHAMPION_HUD_DEFAULT_WATER_COLOR  14  /* COLOR_BLUE  */

typedef struct {
    int valid;
    int16_t color;
    int gdat_override;
} DM2_V1_BarColorReceipt;

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt);

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt);

const char *dm2_v1_champion_hud_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H */
