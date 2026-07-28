#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void make_entrance_facts(CSB_V1_StartupHostFacts_PC34 *facts)
{
    csb_v1_startup_host_facts_from_runtime_state_pc34(
        facts,
        0, 0, 0,
        1, 4, 0,
        0, 0,
        0, 0, 0,
        0,
        0,
        0, 0, 0, 0, NULL,
        0, NULL, NULL);
}

int main(void)
{
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_StartupPresentationReceipt_PC34 receipt;
    CSB_V1_StartupTickReceipt_PC34 tick_receipt;

    make_entrance_facts(&facts);
    facts.title_active = 1;
    facts.title_frame = 0;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(0);
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34,
          "source-derived title step reaches the title receipt");

    ++facts.title_source_step;
    check(!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
               &facts, &receipt) &&
              !receipt.valid,
          "stale title source step cannot authorize a title receipt");

    facts.title_frame = csb_v1_startup_title_presents_ticks_pc34();
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              receipt.render_plan.title_dest_w == 48 &&
              receipt.render_plan.title_dest_h == 12,
          "CHAOS begins with ReDMCSB's 48x12 first reverse-zoom bitmap");

    make_entrance_facts(&facts);
    facts.title_active = 1;
    facts.title_frame = csb_v1_startup_title_presents_ticks_pc34() - 1;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_advance_tick_from_host_facts_with_receipt_pc34(
              &facts, &tick_receipt) &&
              tick_receipt.state.title_frame ==
                  csb_v1_startup_title_presents_ticks_pc34() &&
              tick_receipt.state.title_source_step ==
                  (int)csb_v1_startup_title_source_step_for_frame_pc34(
                      tick_receipt.state.title_frame),
          "the first CHAOS tick publishes the current source step, not the stale PRESENTS step");
    facts.title_frame = tick_receipt.state.title_frame;
    facts.title_source_step = tick_receipt.state.title_source_step;
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
          "the post-PRESENTS tick remains receipt-coherent for CHAOS");

    facts.title_frame = csb_v1_startup_title_presents_ticks_pc34() +
        csb_v1_startup_title_chaos_zoom_ticks_pc34() - 1;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) && receipt.render_plan.title_dest_w == 320 &&
              receipt.render_plan.title_dest_h == 80,
          "CHAOS ends on ReDMCSB's full 320x80 bitmap");

    --facts.title_frame;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) && receipt.render_plan.title_dest_w == 320 &&
              receipt.render_plan.title_dest_h == 80,
          "the source Delay(20) CHAOS hold preserves the full bitmap");

    facts.title_frame = csb_v1_startup_title_total_ticks_pc34() - 1;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              receipt.render_plan.title_dest_w == 320 &&
              receipt.render_plan.title_dest_h == 57,
          "STRIKES BACK follows the held full CHAOS bitmap");

    make_entrance_facts(&facts);
    facts.title_active = 1;
    facts.title_frame = csb_v1_startup_title_total_ticks_pc34() - 1;
    facts.title_source_step =
        (int)csb_v1_startup_title_source_step_for_frame_pc34(
            facts.title_frame);
    check(csb_v1_startup_advance_tick_from_host_facts_with_receipt_pc34(
              &facts, &tick_receipt) &&
              tick_receipt.tick_result.title_finished &&
              !tick_receipt.state.title_active &&
              tick_receipt.state.entrance_source_step == 1,
          "title completion enters the first ReDMCSB entrance source step");
    facts.title_active = tick_receipt.state.title_active;
    facts.title_frame = tick_receipt.state.title_frame;
    facts.title_source_step = tick_receipt.state.title_source_step;
    facts.entrance_source_step = tick_receipt.state.entrance_source_step;
    facts.entrance_frame = tick_receipt.entrance_frame;
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34 &&
              receipt.render_plan.render_command_count == 1 &&
              receipt.render_plan.render_commands[0].kind ==
                  CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34,
          "the terminal title tick presents the ReDMCSB entrance black frame before C004/C002/C003");

    make_entrance_facts(&facts);
    facts.credits_active = 1;
    facts.credits_remaining_ticks = 1;
    facts.opening_active = 1;
    check(!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
               &facts, &receipt) &&
              !receipt.valid,
          "credits and door opening cannot share one entrance receipt");

    make_entrance_facts(&facts);
    facts.credits_active = 1;
    check(!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
               &facts, &receipt) &&
              !receipt.valid,
          "expired credits cannot authorize an entrance receipt");

    make_entrance_facts(&facts);
    facts.opening_active = 1;
    facts.opening_delay_ticks = 1;
    facts.opening_step = 0;
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34,
          "pre-open delay may retain the C004/C002/C003 closed-door surface");

    facts.opening_delay_ticks = 0;
    check(!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              !receipt.valid,
          "door opening cannot publish a frame before C002/C003 step 1");

    facts.opening_step = 1;
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34,
          "source-bounded door opening reaches the opening receipt");

    facts.opening_step = facts.door_step_count + 1;
    check(!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              !receipt.valid,
          "door opening cannot publish past ReDMCSB's final C002/C003 step");

    return failures ? 1 : 0;
}
