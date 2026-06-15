#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_ALL_STATES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_ALL_STATES_PC34_COMPAT_H

/*
 * DM1 V1 Champion Panel all-state redraw dispatch contract.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143
 *   iterates C00_CHAMPION_FIRST up to G0305_ui_PartyChampionCount, ORs
 *   the PC34 redraw mask into each active champion Attributes field, calls
 *   F0292_CHAMPION_DrawState for each active champion, then clears G2149_.
 * - ReDMCSB DEFS.H:724-732 defines the champion dirty-flag bits accepted
 *   by the F0293 PC34 mask parameter.
 * - ReDMCSB DEFS.H:7895-7907 declares F0292 and F0293.
 *
 * Contract only: this slice proves the dispatch/order/dirty-mask contract
 * with synthetic state. It does not claim F0292 internals, real-asset bitmap
 * parity, or GRAPHICS.DAT loading.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34 4
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_FIRST_CHAMPION_PC34 0

#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_NAME_TITLE_PC34 0x0080u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_STATISTICS_PC34 0x0100u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_LOAD_PC34 0x0200u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ICON_PC34 0x0400u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_PANEL_PC34 0x0800u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_STATUS_BOX_PC34 0x1000u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_WOUNDS_PC34 0x2000u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_VIEWPORT_PC34 0x4000u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ACTION_HAND_PC34 0x8000u
#define DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ALL_DIRTY_PC34 0xFF80u

typedef struct {
    const char *function_name;
    const char *function_anchor;
    const char *loop_anchor;
    const char *attribute_mask_anchor;
    const char *prototype_anchor;
    const char *non_overlap_anchor;
    const char *contract_scope;
    const char *no_real_asset_claim;
} dm1_v1_champion_panel_all_states_pc34_compat_evidence_t;

typedef struct {
    bool contract_only;
    bool real_asset_bitmap_parity;
    bool loads_graphics_dat;
    bool covers_f0293_only;
    bool covers_f0292_internals;
    bool starts_at_champion_zero;
    bool stops_before_party_count;
    bool ors_mask_before_draw_state;
    bool calls_f0292_once_per_active_champion;
    bool preserves_inactive_champion_attributes;
    bool clears_pc34_g2149_after_loop;
    uint16_t accepted_dirty_mask_bits;
    int max_champions;
} dm1_v1_champion_panel_all_states_pc34_compat_invariant_t;

typedef struct {
    uint16_t party_champion_count;
    uint16_t redraw_mask;
    uint16_t initial_attributes[DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34];
    bool pc34_pending_all_state_redraw;
} dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t;

typedef struct {
    dm1_v1_champion_panel_all_states_pc34_compat_invariant_t invariant;
    dm1_v1_champion_panel_all_states_pc34_compat_evidence_t evidence;
    uint16_t party_champion_count_clamped;
    uint16_t redraw_mask_applied;
    uint16_t final_attributes[DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34];
    int draw_state_call_count;
    int draw_state_indices[DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34];
    bool pc34_pending_all_state_redraw_after;
    bool rejected_overlarge_party_count;
    bool null_input_defaults_used;
} dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t;

dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t
dm1_v1_champion_panel_all_states_pc34_compat_probe(
    const dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t *input);

const dm1_v1_champion_panel_all_states_pc34_compat_evidence_t *
dm1_v1_champion_panel_all_states_pc34_compat_evidence(void);

const char *dm1_v1_champion_panel_all_states_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_ALL_STATES_PC34_COMPAT_H */
