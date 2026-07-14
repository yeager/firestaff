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
    facts.opening_step = 1;
    check(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &receipt) &&
              receipt.valid &&
              receipt.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34,
          "source-bounded door opening reaches the opening receipt");

    return failures ? 1 : 0;
}
