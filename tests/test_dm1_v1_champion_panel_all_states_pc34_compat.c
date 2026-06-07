#include "dm1_v1_champion_panel_all_states_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%04X (%s)\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_evidence_and_invariants(void)
{
    const dm1_v1_champion_panel_all_states_pc34_compat_evidence_t *evidence =
        dm1_v1_champion_panel_all_states_pc34_compat_evidence();
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t result =
        dm1_v1_champion_panel_all_states_pc34_compat_probe(NULL);
    const char *source =
        dm1_v1_champion_panel_all_states_pc34_compat_source_evidence();

    expect_bool("invariant.contract_only", result.invariant.contract_only, true,
                "CHAMDRAW.C F0293:1117-1143 contract-only dispatcher");
    expect_bool("invariant.no_bitmap_parity", result.invariant.real_asset_bitmap_parity, false,
                "CHAMDRAW.C F0293:1117-1143 no real-asset bitmap parity");
    expect_bool("invariant.no_graphics_dat", result.invariant.loads_graphics_dat, false,
                "CHAMDRAW.C F0293:1117-1143 no GRAPHICS.DAT load");
    expect_bool("invariant.covers_f0293_only", result.invariant.covers_f0293_only, true,
                "CHAMDRAW.C F0293:1117-1143 bounded F0293 slice");
    expect_bool("invariant.no_f0292_internals", result.invariant.covers_f0292_internals, false,
                "CHAMDRAW.C F0293:1138 delegates to F0292 without owning internals");
    expect_bool("invariant.starts_zero", result.invariant.starts_at_champion_zero, true,
                "CHAMDRAW.C F0293:1134 C00_CHAMPION_FIRST loop start");
    expect_bool("invariant.stops_party_count", result.invariant.stops_before_party_count, true,
                "CHAMDRAW.C F0293:1134 championIndex < G0305_ui_PartyChampionCount");
    expect_bool("invariant.ors_before_draw", result.invariant.ors_mask_before_draw_state, true,
                "CHAMDRAW.C F0293:1136-1138 OR mask before F0292");
    expect_bool("invariant.calls_once", result.invariant.calls_f0292_once_per_active_champion, true,
                "CHAMDRAW.C F0293:1138 one F0292 call per loop iteration");
    expect_bool("invariant.preserves_inactive",
                result.invariant.preserves_inactive_champion_attributes, true,
                "CHAMDRAW.C F0293:1134 active-party loop bound");
    expect_bool("invariant.clears_g2149",
                result.invariant.clears_pc34_g2149_after_loop, true,
                "CHAMDRAW.C F0293:1140-1142 G2149_ clear");
    expect_u16("invariant.accepted_dirty_mask",
               result.invariant.accepted_dirty_mask_bits, 0xFF80u,
               "DEFS.H:724-732 dirty flags 0x0080..0x8000");
    expect_int("invariant.max_champions", result.invariant.max_champions, 4,
               "CHAMDRAW.C F0293:1134 G0305 party count over four champion slots");

    expect_str_eq("evidence.function_name", evidence->function_name,
                  "F0293_CHAMPION_DrawAllChampionStates",
                  "CHAMDRAW.C F0293:1117-1143");
    expect_str_eq("evidence.function_anchor", evidence->function_anchor,
                  "CHAMDRAW.C F0293:1117-1143",
                  "CHAMDRAW.C F0293:1117-1143");
    expect_str_eq("evidence.loop_anchor", evidence->loop_anchor,
                  "CHAMDRAW.C F0293:1134-1138",
                  "CHAMDRAW.C F0293:1134-1138");
    expect_str_eq("evidence.mask_anchor", evidence->attribute_mask_anchor,
                  "DEFS.H:724-732",
                  "DEFS.H:724-732 dirty flags");
    expect_str_eq("evidence.prototype_anchor", evidence->prototype_anchor,
                  "DEFS.H:7895-7907",
                  "DEFS.H:7895-7907 F0292/F0293 declarations");
    expect_contains("evidence.scope", evidence->contract_scope,
                    "F0293 dispatch/order/dirty-mask",
                    "CHAMDRAW.C F0293:1117-1143 bounded scope");
    expect_contains("evidence.no_real_asset_claim", evidence->no_real_asset_claim,
                    "no real-asset bitmap parity",
                    "CHAMDRAW.C F0293:1117-1143 contract-only marker");
    expect_contains("source.contract_only", source, "contract_only=1",
                    "CHAMDRAW.C F0293:1117-1143 evidence string");
    expect_contains("source.no_graphics_dat", source, "no GRAPHICS.DAT load",
                    "CHAMDRAW.C F0293:1117-1143 evidence string");
    expect_contains("source.non_overlap", source, "does not cover F0287",
                    "grep-confirmed F0293 non-overlap with existing HUD module");
    expect_bool("null_input.defaults", result.null_input_defaults_used, true,
                "CHAMDRAW.C F0293:1117-1143 synthetic zero-state probe");
}

static void test_three_champion_mask_or_and_draw_order(void)
{
    dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t input = {
        3,
        (uint16_t)(DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ICON_PC34 |
                   DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_STATUS_BOX_PC34),
        { 0x0000u, DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_LOAD_PC34,
          0x0001u, DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ACTION_HAND_PC34 },
        true
    };
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t result =
        dm1_v1_champion_panel_all_states_pc34_compat_probe(&input);

    expect_int("three.party_count", result.party_champion_count_clamped, 3,
               "CHAMDRAW.C F0293:1134 loop upper bound G0305");
    expect_u16("three.mask_applied", result.redraw_mask_applied, 0x1400u,
               "CHAMDRAW.C F0293:1136 OR P2062_ui_ into Attributes");
    expect_int("three.draw_call_count", result.draw_state_call_count, 3,
               "CHAMDRAW.C F0293:1134-1138 active champions call F0292");
    expect_int("three.draw_index0", result.draw_state_indices[0], 0,
               "CHAMDRAW.C F0293:1134 C00_CHAMPION_FIRST first call");
    expect_int("three.draw_index1", result.draw_state_indices[1], 1,
               "CHAMDRAW.C F0293:1134-1138 second active champion");
    expect_int("three.draw_index2", result.draw_state_indices[2], 2,
               "CHAMDRAW.C F0293:1134-1138 third active champion");
    expect_int("three.draw_index3_unused", result.draw_state_indices[3], -1,
               "CHAMDRAW.C F0293:1134 loop stops before inactive champion 3");
    expect_u16("three.attr0", result.final_attributes[0], 0x1400u,
               "CHAMDRAW.C F0293:1136 active champion attribute OR");
    expect_u16("three.attr1", result.final_attributes[1], 0x1600u,
               "CHAMDRAW.C F0293:1136 preserves existing dirty bits");
    expect_u16("three.attr2", result.final_attributes[2], 0x1401u,
               "CHAMDRAW.C F0293:1136 ORs only the PC34 dirty mask bits");
    expect_u16("three.attr3_unchanged", result.final_attributes[3], 0x8000u,
               "CHAMDRAW.C F0293:1134 inactive champion not visited");
    expect_bool("three.g2149_cleared",
                result.pc34_pending_all_state_redraw_after, false,
                "CHAMDRAW.C F0293:1140-1142 G2149_ clear after loop");
    expect_bool("three.no_overlarge_reject", result.rejected_overlarge_party_count, false,
                "CHAMDRAW.C F0293:1134 valid G0305 party count");
}

static void test_zero_and_full_party_bounds(void)
{
    dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t zero = {
        0,
        DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ALL_DIRTY_PC34,
        { 0x0100u, 0x0200u, 0x0400u, 0x0800u },
        true
    };
    dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t full = {
        4,
        DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_WOUNDS_PC34,
        { 0x0000u, 0x0000u, 0x0000u, 0x0000u },
        true
    };
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t zero_result =
        dm1_v1_champion_panel_all_states_pc34_compat_probe(&zero);
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t full_result =
        dm1_v1_champion_panel_all_states_pc34_compat_probe(&full);

    expect_int("zero.draw_call_count", zero_result.draw_state_call_count, 0,
               "CHAMDRAW.C F0293:1134 zero party count has no iterations");
    expect_u16("zero.attr0_preserved", zero_result.final_attributes[0], 0x0100u,
               "CHAMDRAW.C F0293:1134 no inactive attribute writes");
    expect_u16("zero.attr3_preserved", zero_result.final_attributes[3], 0x0800u,
               "CHAMDRAW.C F0293:1134 no inactive attribute writes");
    expect_bool("zero.g2149_cleared", zero_result.pc34_pending_all_state_redraw_after, false,
                "CHAMDRAW.C F0293:1140-1142 G2149_ clear even after empty loop");

    expect_int("full.draw_call_count", full_result.draw_state_call_count, 4,
               "CHAMDRAW.C F0293:1134-1138 four active champions");
    expect_int("full.draw_index3", full_result.draw_state_indices[3], 3,
               "CHAMDRAW.C F0293:1134-1138 champion 3 included when G0305 is 4");
    expect_u16("full.attr0", full_result.final_attributes[0], 0x2000u,
               "CHAMDRAW.C F0293:1136 wounds dirty mask OR");
    expect_u16("full.attr3", full_result.final_attributes[3], 0x2000u,
               "CHAMDRAW.C F0293:1136 wounds dirty mask OR");
}

static void test_mask_filter_and_overlarge_party_count(void)
{
    dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t input = {
        9,
        0xFFFFu,
        { 0x0001u, 0x0002u, 0x0004u, 0x0008u },
        true
    };
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t result =
        dm1_v1_champion_panel_all_states_pc34_compat_probe(&input);

    expect_bool("overlarge.rejected", result.rejected_overlarge_party_count, true,
                "CHAMDRAW.C F0293:1134 synthetic contract clamps invalid G0305");
    expect_int("overlarge.clamped_count", result.party_champion_count_clamped, 4,
               "CHAMDRAW.C F0293:1134 party slots are 0..3 in this contract");
    expect_u16("overlarge.mask_filtered", result.redraw_mask_applied, 0xFF80u,
               "DEFS.H:724-732 accepted dirty flags only");
    expect_int("overlarge.draw_call_count", result.draw_state_call_count, 4,
               "CHAMDRAW.C F0293:1134-1138 clamped active champions");
    expect_u16("overlarge.attr0_filtered_or", result.final_attributes[0], 0xFF81u,
               "CHAMDRAW.C F0293:1136 ORs accepted dirty bits, preserves other bits");
    expect_u16("overlarge.attr3_filtered_or", result.final_attributes[3], 0xFF88u,
               "CHAMDRAW.C F0293:1136 ORs accepted dirty bits, preserves other bits");
    expect_bool("overlarge.g2149_cleared",
                result.pc34_pending_all_state_redraw_after, false,
                "CHAMDRAW.C F0293:1140-1142 G2149_ clear after loop");
}

int main(void)
{
    test_evidence_and_invariants();
    test_three_champion_mask_or_and_draw_order();
    test_zero_and_full_party_bounds();
    test_mask_filter_and_overlarge_party_count();

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_all_states_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_all_states_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
