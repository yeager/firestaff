#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel DISABLED/UNAVAILABLE ICON STATE gate.
 *
 * Source-locked to ReDMCSB:
 *   CHAMPION.C F0330_CHAMPION_DisableAction:2208-2255
 *     - sets MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION in
 *       M008_SET(Champion->Attributes, ...), then calls
 *       F0292_CHAMPION_DrawState to refresh the action hand redraw.
 *     - schedules a C11_EVENT_ENABLE_CHAMPION_ACTION event with
 *       Map_Time = GameTime + Ticks.  When the existing event index
 *       is already valid, F0330 either delays (>=) or shifts the new
 *       event out by Ticks>>1 (the BUG0_19 ladder).
 *   ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-296
 *     - early-returns when G0509_B_ActionAreaContainsIcons == 0.
 *     - dead-champion early-return at L1183 (CurrentHealth == 0):
 *       fills the C089..C092 zone BLACK and returns.
 *     - blits C201_ICON_ACTION_ICON_EMPTY_HAND when the action hand
 *       slot holds C0xFFFF_THING_NONE.
 *     - otherwise extracts the icon via F0036_OBJECT_ExtractIconFromBitmap,
 *       applies the action-area palette changes, fills C04_COLOR_CYAN,
 *       blits the icon into C093..C096.
 *     - finishes with the SOURCE-LOCKED HATCH gate at line 282:
 *
 *         if (M007_GET(L1183_ps_Champion->Attributes,
 *                      MASK0x0008_DISABLE_ACTION)
 *             || G0299_ui_CandidateChampionOrdinal
 *             || G0300_B_PartyIsResting) {
 *             F0136_VIDEO_HatchScreenBox(...);
 *         }
 *
 *   MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]:27,157
 *     - per-action disabled-tick count for the F0330/F0407 contract:
 *       when G0491[actionIndex] != 0 the action icon is disabled for
 *       that many ticks after the F0330 call.  Pass922 already pins
 *       the table bytes contract-only.
 *
 *   MAGIC + CHAMPION.C runtime shield state (partyShieldDefense,
 *   spellShieldDefense, fireShieldDefense):
 *     - The M11 G0493/G0494/G0495 stack already exposed by
 *       M11_GameView_GetV1StatusShieldBorderGraphicForChampion copies
 *       the ReDMCSB F0292 append order (fire, spell, party — drawn
 *       in reverse).  When ALL three defenses are zero the count is
 *       0, which is the DISABLED side of the shield-border active
 *       state and is the path this gate covers.
 *
 * Companion to:
 *   - dm1_v1_graphic560_action_disabled_ticks_pc34_compat
 *     (G0491 table bytes contract)
 *   - M11_GameView_GetV1StatusShieldBorderGraphicForChampion
 *     (shield-border ENABLED pixel path — separate lane)
 *   - M11_GameView_ShouldHatchV1ActionIconCells
 *     (global hatch gate — separate lane)
 *   - firestaff_dm1_v1_champion_panel_shield_border_pixel_probe
 *     (asset-backed ENABLED shield pixel probe — separate lane)
 *   - firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_probe
 *     (asset-backed icon-direction pixel probe — separate lane)
 *
 * This gate is contract-only.  It does NOT load GRAPHICS.DAT or
 * DUNGEON.DAT, run live rendering, capture pixels, run DOSBox, or
 * claim original-vs-Firestaff parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPDIS_CHAMPION_COUNT_PC34        4
#define DM1_V1_CPDIS_SHADED_TICKS_TABLE_SIZE_PC34 44

/* Source-locked masks from ReDMCSB DEFS.H + CHAMPION.C F0330. */
#define DM1_V1_CPDIS_MASK_DISABLE_ACTION_PC34  0x0008u
#define DM1_V1_CPDIS_MASK_ACTION_HAND_PC34     0x8000u
#define DM1_V1_CPDIS_MASK_NAME_TITLE_PC34      0x0080u
#define DM1_V1_CPDIS_MASK_STATISTICS_PC34      0x0100u
#define DM1_V1_CPDIS_MASK_STATUS_BOX_PC34      0x1000u

/* ReDMCSB C0xFFFF_THING_NONE sentinel for F0386 empty-hand branch. */
#define DM1_V1_CPDIS_THING_NONE_PC34           0xFFFFu

/* The C201_ICON_ACTION_ICON_EMPTY_HAND used by F0386 empty-hand blit. */
#define DM1_V1_CPDIS_ICON_EMPTY_HAND_PC34      201

/* Source-locked action area graphic ids from CHAMDRAW/ACTIDRAW
 * (M11 G0493-style zone ids; harmless if M11 swaps the underlying
 * graphics as long as the predicate stays source-locked). */
#define DM1_V1_CPDIS_GRAPHIC_PARTY_SHIELD_PC34   37
#define DM1_V1_CPDIS_GRAPHIC_FIRE_SHIELD_PC34    38
#define DM1_V1_CPDIS_GRAPHIC_SPELL_SHIELD_PC34   39

/* Per-champion action disabled-ticks record.  Mirrors the
 * Champion.Attributes MASK0x0008_DISABLE_ACTION bit plus the
 * G0491[actionIndex] lookup that F0330 consumes when applying the
 * action.  When the table entry is 0 (N, X, THROW) the icon never
 * gets disabled — that path is also pinned here. */
typedef struct DM1_V1_ChampionPanelDisabledIconChampionPc34Compat {
    int index;
    int current_health;
    bool present;
    bool is_candidate_champion;
    bool party_is_resting;
    bool inventory_open;
    bool action_hand_empty;       /* mirrors Champion.Slots[C01_SLOT_ACTION_HAND] == 0xFFFF */
    int last_action_index;       /* 0..43 or -1 if no action performed */
    int remaining_disabled_ticks;/* non-zero while MASK0x0008 bit is set */
    uint16_t attributes;         /* ReDMCSB Champion.Attributes */
    int party_shield_defense;
    int spell_shield_defense;
    int fire_shield_defense;
} DM1_V1_ChampionPanelDisabledIconChampionPc34Compat;

typedef struct DM1_V1_ChampionPanelDisabledIconStatePc34Compat {
    int party_champion_count;
    int candidate_champion_ordinal;   /* G0299 */
    bool candidate_mirror_panel_active;
    bool party_is_resting;            /* G0300 */
    DM1_V1_ChampionPanelDisabledIconChampionPc34Compat
        champions[DM1_V1_CPDIS_CHAMPION_COUNT_PC34];
} DM1_V1_ChampionPanelDisabledIconStatePc34Compat;

typedef struct DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat {
    bool contract_only;
    const char *f0330_disable_action_anchor;
    const char *f0330_attribute_set_anchor;
    const char *f0330_event11_schedule_anchor;
    const char *f0386_hatch_gate_anchor;
    const char *f0386_dead_champion_anchor;
    const char *f0386_empty_hand_anchor;
    const char *g0491_action_disabled_ticks_anchor;
    const char *shield_border_disabled_anchor;
    const char *global_hatch_gate_anchor;
    const char *defs_action_hand_anchor;
    const char *defs_disable_action_anchor;
    const char *no_real_asset_claim;
    const char *no_dosbox_claim;
    const char *no_pixel_parity_claim;
    const char *non_overlap_with_shield_border;
    const char *non_overlap_with_icon_direction;
    const char *non_overlap_with_g0491_table_only;
} DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat;

/* Predicate outputs.  The gate stores both the per-champion
 * prediction (what F0386 would do for the C089..C092 cell on the
 * next F0292 redraw) and the per-champion disabled reason enum so
 * callers can cross-check without re-reading the source-locked
 * tables. */
typedef enum DM1_V1_ChampionPanelDisabledIconStateIdPc34Compat {
    DM1_V1_CPDIS_STATE_AVAILABLE_PC34 = 0,
    DM1_V1_CPDIS_STATE_DEAD_CHAMPION_PC34 = 1,
    DM1_V1_CPDIS_STATE_PER_CHAMPION_DISABLE_ACTION_PC34 = 2,
    DM1_V1_CPDIS_STATE_CANDIDATE_ACTIVE_PC34 = 3,
    DM1_V1_CPDIS_STATE_PARTY_RESTING_PC34 = 4,
    DM1_V1_CPDIS_STATE_SLOT_EMPTY_PC34 = 5,
    DM1_V1_CPDIS_STATE_PARTY_INCOMPLETE_PC34 = 6
} DM1_V1_ChampionPanelDisabledIconStateIdPc34Compat;

typedef struct DM1_V1_ChampionPanelDisabledIconChampionResultPc34Compat {
    int index;
    DM1_V1_ChampionPanelDisabledIconStateIdPc34Compat state;
    int disabled_ticks_remaining;
    bool should_hatch_cell;
    bool should_hatch_cell_per_champion_only;
    int shield_border_count;
    int shield_border_graphics[3];
    int empty_hand_icon_index;
    bool dead_champion_early_return;
    bool action_hand_bit_set;
    bool disable_action_bit_set;
    const char *anchor;
} DM1_V1_ChampionPanelDisabledIconChampionResultPc34Compat;

typedef struct DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat {
    bool accepted;
    int assertion_count;
    int champions_processed;
    bool any_hatched;
    bool any_unavailable;
    bool any_shield_border_active;
    bool any_dead_champion;
    bool any_per_champion_disable_action;
    bool any_candidate_hatch;
    bool any_resting_hatch;
    bool any_empty_hand;
    int per_state_count[7];
    DM1_V1_ChampionPanelDisabledIconChampionResultPc34Compat
        champion_results[DM1_V1_CPDIS_CHAMPION_COUNT_PC34];
    const DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat *evidence;
    const char *source_evidence_string;
} DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat;

/* Read-only accessor for the G0491 disabled-ticks table bytes
 * (ReDMCSB MENU.C:157).  Returns the size on success or -1 on
 * invalid index.  Pass922 already pins the table bytes; this gate
 * re-uses the values to drive the per-action predicate without
 * pulling in another shared module. */
int DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat(
    int action_index);

const DM1_V1_ChampionPanelDisabledIconEvidencePc34Compat *
DM1_V1_ChampionPanelDisabledIconState_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelDisabledIconState_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelDisabledIconState_InitStatePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int party_champion_count);

int DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int champion_index,
    int action_index);

int DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    int ticks_to_advance);

int DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat(
    const DM1_V1_ChampionPanelDisabledIconStatePc34Compat *state,
    DM1_V1_ChampionPanelDisabledIconResolveResultPc34Compat *out_result);

const char *
DM1_V1_ChampionPanelDisabledIconState_NamePc34Compat(
    DM1_V1_ChampionPanelDisabledIconStateIdPc34Compat state_id);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_PC34_COMPAT_H */
