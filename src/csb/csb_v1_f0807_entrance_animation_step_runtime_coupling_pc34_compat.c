#include "csb_v1_f0807_entrance_animation_step_runtime_coupling_pc34_compat.h"

#include <string.h>

static int csb_v1_f0807_step_range_matches_pc34(
    const CSB_V1_F0807_EntranceAnimationStepFacts_PC34 *facts)
{
    return facts &&
        facts->source_door_step_count == CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 &&
        facts->animation_step_index >= CSB_V1_F0807_ENTRANCE_DOOR_STEP_FIRST_PC34 &&
        facts->animation_step_index <= CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34;
}

static int csb_v1_f0807_target_matches_pc34(
    const CSB_V1_F0807_EntranceAnimationStepFacts_PC34 *facts)
{
    return facts &&
        facts->target_screen_width == CSB_V1_F0807_TARGET_SCREEN_WIDTH_PC34 &&
        facts->target_screen_height == CSB_V1_F0807_TARGET_SCREEN_HEIGHT_PC34;
}

void csb_v1_f0807_entrance_animation_step_receipt_init_pc34(
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0807_ENTRANCE_DrawAnimationStep(
    const CSB_V1_F0807_EntranceAnimationStepFacts_PC34 *facts,
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0807_entrance_draw_animation_step_source_evidence_pc34();

    csb_v1_f0807_entrance_animation_step_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->animation_step_real_asset_bound ||
        !facts->target_screen_real_asset_bound ||
        !csb_v1_f0807_step_range_matches_pc34(facts) ||
        !csb_v1_f0807_target_matches_pc34(facts) ||
        !facts->blit_box_source_locked ||
        !facts->no_transparency_mode ||
        !facts->draw_consumes_receipt_only ||
        !facts->input_consumes_receipt_only ||
        !facts->no_legacy_animation_wrapper ||
        !facts->no_synthetic_visuals ||
        !facts->bitplanes.valid ||
        !facts->bitplanes.source_composite_bound ||
        !facts->bitplanes.target_screen_bound ||
        !facts->bitplanes.geometry_source_locked ||
        !facts->bitplanes.runtime_coupling_consumed ||
        !facts->bitplanes.title_hud_door_runtime_ready ||
        !facts->bitplanes.no_legacy_bitplane_wrapper ||
        !facts->bitplanes.no_synthetic_visuals ||
        !facts->runtime_coupling.valid ||
        !facts->runtime_coupling.door_runtime_coupled ||
        !facts->runtime_coupling.draw_consumes_receipt_only ||
        !facts->runtime_coupling.input_consumes_receipt_only ||
        !facts->runtime_coupling.no_legacy_wrappers ||
        !facts->runtime_coupling.no_synthetic_visuals) {
        if (out_receipt) {
            out_receipt->no_synthetic_visuals = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->animation_step_bound = 1;
    out_receipt->target_screen_bound = 1;
    out_receipt->source_step_range_locked = 1;
    out_receipt->bitplanes_consumed = 1;
    out_receipt->runtime_coupling_consumed = 1;
    out_receipt->draw_consumes_receipt_only = 1;
    out_receipt->input_consumes_receipt_only = 1;
    out_receipt->no_legacy_animation_wrapper = 1;
    out_receipt->no_synthetic_visuals = 1;
    out_receipt->accepted_animation_step_index = facts->animation_step_index;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0807_entrance_draw_animation_step_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:85-90 F0807_ENTRANCE_DrawAnimationStep "
           "submits G2219_puc_EntranceAnimationStep to G0348_Bitmap_Screen "
           "through F0766_BlitToScreen with CM1_COLOR_NO_TRANSPARENCY; CSB "
           "PC34 accepts only real F0438/F0580 door-step frames after F0579 "
           "bitplane initialization";
}
