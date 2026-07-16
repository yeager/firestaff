#include "csb_v1_f0436_startend_fade_palette_runtime_coupling_pc34_compat.h"

#include <string.h>

static int csb_v1_f0436_step_count_matches_pc34(int step_count)
{
    return step_count == CSB_V1_F0436_AMIGA_FADE_STEP_COUNT_PC34 ||
        step_count == CSB_V1_F0436_GENERIC_FADE_STEP_COUNT_PC34;
}

static int csb_v1_f0436_title_route_ready_pc34(
    const CSB_V1_F0436_FadePaletteFacts_PC34 *facts)
{
    return (facts->route_mask & CSB_V1_F0436_ROUTE_TITLE_ANY_PC34) &&
        facts->title_palette_route &&
        facts->runtime_coupling.valid &&
        facts->runtime_coupling.real_startup_assets_bound &&
        facts->runtime_coupling.title_phase_route_complete &&
        facts->runtime_coupling.draw_consumes_receipt_only &&
        facts->runtime_coupling.no_legacy_wrappers &&
        facts->runtime_coupling.no_synthetic_visuals;
}

static int csb_v1_f0436_entrance_route_ready_pc34(
    const CSB_V1_F0436_FadePaletteFacts_PC34 *facts)
{
    const uint32_t entrance_mask =
        facts->route_mask & CSB_V1_F0436_ROUTE_ENTRANCE_ANY_PC34;

    return entrance_mask &&
        facts->entrance_palette_route &&
        facts->entrance_boundary.valid &&
        facts->entrance_boundary.real_asset_matched &&
        facts->entrance_boundary.host_view_consumed &&
        facts->entrance_boundary.host_draw_consumed &&
        facts->entrance_boundary.draw_consumes_receipt_only &&
        facts->entrance_boundary.no_synthetic_payloads &&
        facts->entrance_boundary.no_fallback_graphics &&
        facts->entrance_boundary.route_wrappers_retired;
}

void csb_v1_f0436_fade_palette_receipt_init_pc34(
    CSB_V1_F0436_FadePaletteReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0436_STARTEND_FadeToPalette(
    const CSB_V1_F0436_FadePaletteFacts_PC34 *facts,
    CSB_V1_F0436_FadePaletteReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0436_startend_fade_to_palette_source_evidence_pc34();
    const int title_ready = facts ? csb_v1_f0436_title_route_ready_pc34(facts) : 0;
    const int entrance_ready =
        facts ? csb_v1_f0436_entrance_route_ready_pc34(facts) : 0;

    csb_v1_f0436_fade_palette_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        facts->route_mask == 0u ||
        !facts->target_palette_real_asset_bound ||
        facts->palette_entry_count != CSB_V1_F0436_PALETTE_ENTRY_COUNT_PC34 ||
        !csb_v1_f0436_step_count_matches_pc34(facts->fade_step_count) ||
        !facts->component_masks_source_locked ||
        !facts->vertical_blank_synchronized ||
        !facts->no_renderer_palette_substitute ||
        !facts->no_legacy_palette_wrapper ||
        !facts->no_synthetic_palette ||
        (!title_ready && !entrance_ready) ||
        (facts->credits_palette_route &&
         !(facts->route_mask & CSB_V1_F0436_ROUTE_ENTRANCE_CREDITS_PC34))) {
        if (out_receipt) {
            out_receipt->no_synthetic_palette = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->accepted_route_mask = facts->route_mask;
    out_receipt->target_palette_bound = 1;
    out_receipt->palette_entry_count_source_locked = 1;
    out_receipt->fade_step_count_source_locked = 1;
    out_receipt->title_runtime_consumed = title_ready ? 1 : 0;
    out_receipt->entrance_boundary_consumed = entrance_ready ? 1 : 0;
    out_receipt->component_masks_source_locked = 1;
    out_receipt->vertical_blank_synchronized = 1;
    out_receipt->no_renderer_palette_substitute = 1;
    out_receipt->no_legacy_palette_wrapper = 1;
    out_receipt->no_synthetic_palette = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0436_startend_fade_to_palette_source_evidence_pc34(void)
{
    return "ReDMCSB PALETTE.C:209-395 F0436_STARTEND_FadeToPalette fades "
           "16 source palette entries with component masks and VBlank timing; "
           "CSB TITLE.C:428-462 uses it for PRESENTS/CHAOS/STRIKES palettes "
           "and ENTRANCE.C:427-595/1061 uses it for entrance and credits";
}
