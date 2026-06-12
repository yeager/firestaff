#include "csb_v1_viewport_custom_backgrounds_room_pass_order_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
}

static void check_size(const char *label, size_t got, size_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%zu want=%zu anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("ok %s=%zu anchor=%s\n", label, got, anchor);
}

static void check_u32(const char *label, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               label, got, want, anchor);
        return;
    }
    printf("ok %s=0x%08x anchor=%s\n", label, got, anchor);
}

static void check_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
        return;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, anchor);
}

static void check_before(const char *label, int left, int right, const char *anchor)
{
    ++g_assertions;
    if (left < 0 || right < 0 || left >= right) {
        ++g_failures;
        printf("FAIL %s left=%d right=%d anchor=%s\n", label, left, right, anchor);
        return;
    }
    printf("ok %s left=%d right=%d anchor=%s\n", label, left, right, anchor);
}

static void test_contract_and_evidence(void)
{
    const CSB_V1_CustomBackgroundsRoomPassOrderContract *contract =
        csb_v1_viewport_custom_backgrounds_room_pass_order_contract_pc34();
    const char *evidence =
        csb_v1_viewport_custom_backgrounds_room_pass_order_source_evidence_pc34();

    check_int("contract.present", contract != NULL, 1, "contract");
    if (!contract) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 provides the D3/D2 room
     * order; F0098 lines 2962-3002 provides the floor/ceiling baseline;
     * F0115 line 4547 anchors the thing pass reached from each room draw.
     * CSB-lineage Viewport.cpp lines 6926-7045 insert CustomBackgrounds
     * before those room draws, with ApplyBackground at lines 6451-6505. */
    check_int("contract.contract_only", contract->contract_only, 1,
              contract->source_summary);
    check_int("contract.event_count", contract->event_count, 23,
              contract->redmcsb_f0128_anchor);
    check_int("contract.locks_d3_d2", contract->locks_d3_d2_room_passes, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.f0098_before_room_passes",
              contract->f0098_before_room_passes, 1,
              contract->redmcsb_f0098_anchor);
    check_int("contract.custom_backgrounds_before_room_draw",
              contract->custom_backgrounds_before_room_draw, 1,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.custom_backgrounds_before_f0115",
              contract->custom_backgrounds_before_f0115, 1,
              contract->redmcsb_f0115_anchor);
    check_int("contract.d3l_room", contract->d3l_custom_background_room, 2,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.d3r_room", contract->d3r_custom_background_room, 3,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.d3c_room", contract->d3c_custom_background_room, 4,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.d2l_room", contract->d2l_custom_background_room, 7,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.d2r_room", contract->d2r_custom_background_room, 8,
              contract->csb_lineage_room_dispatch_anchor);
    check_int("contract.d2c_room", contract->d2c_custom_background_room, 9,
              contract->csb_lineage_room_dispatch_anchor);

    check_contains("evidence.f0128", evidence, "F0128:8318-8542",
                   contract->redmcsb_f0128_anchor);
    check_contains("evidence.f0098", evidence, "F0098:2962-3002",
                   contract->redmcsb_f0098_anchor);
    check_contains("evidence.f0115", evidence, "F0115:4547",
                   contract->redmcsb_f0115_anchor);
    check_contains("evidence.applybackground", evidence, "Viewport.cpp:6451-6505",
                   contract->csb_lineage_applybackground_anchor);
    check_contains("evidence.custombackgrounds", evidence, "Viewport.cpp:6574-6622",
                   contract->csb_lineage_custombackgrounds_anchor);
    check_contains("evidence.dispatch", evidence, "Viewport.cpp:6926-7045",
                   contract->csb_lineage_room_dispatch_anchor);
    check_contains("evidence.order", evidence,
                   "CustomBackgrounds precedes room draw and F0115 thing pass",
                   contract->source_summary);
}

static void check_primary_room_order(
    const CSB_V1_CustomBackgroundsRoomPassEvent *events,
    size_t count,
    int room_num,
    const char *label,
    const char *anchor)
{
    char name[96];
    const int custom_idx =
        csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
            events, count, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, room_num);
    const int draw_idx =
        csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
            events, count, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, room_num);
    const int thing_idx =
        csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
            events, count, CSB_V1_ROOM_PASS_EVENT_F0115_THING_PASS, room_num);

    snprintf(name, sizeof(name), "%s.custom_found", label);
    check_int(name, custom_idx >= 0, 1, anchor);
    snprintf(name, sizeof(name), "%s.draw_found", label);
    check_int(name, draw_idx >= 0, 1, anchor);
    snprintf(name, sizeof(name), "%s.f0115_found", label);
    check_int(name, thing_idx >= 0, 1, anchor);
    snprintf(name, sizeof(name), "%s.custom_before_draw", label);
    check_before(name, custom_idx, draw_idx, anchor);
    snprintf(name, sizeof(name), "%s.custom_before_f0115", label);
    check_before(name, custom_idx, thing_idx, anchor);
    snprintf(name, sizeof(name), "%s.draw_enter_before_f0115", label);
    check_before(name, draw_idx, thing_idx, anchor);
}

static void test_trace_order_and_hash(void)
{
    CSB_V1_CustomBackgroundsRoomPassEvent events[32];
    const CSB_V1_CustomBackgroundsRoomPassOrderContract *contract =
        csb_v1_viewport_custom_backgrounds_room_pass_order_contract_pc34();
    const size_t count =
        csb_v1_viewport_custom_backgrounds_room_pass_order_trace_pc34(
            events, sizeof(events) / sizeof(events[0]));
    const uint32_t hash =
        csb_v1_viewport_custom_backgrounds_room_pass_order_hash_pc34(events, count);
    size_t i;

    check_size("trace.count", count, 23u, contract->redmcsb_f0128_anchor);
    check_u32("trace.hash", hash, contract->expected_trace_hash,
              contract->source_summary);

    for (i = 0; i < count; ++i) {
        char label[96];
        snprintf(label, sizeof(label), "trace%02zu.ordinal", i);
        check_int(label, events[i].ordinal, (int)i, events[i].source_lines);
    }

    check_int("trace.first_is_f0098", events[0].kind,
              CSB_V1_ROOM_PASS_EVENT_F0098_BASELINE,
              contract->redmcsb_f0098_anchor);
    check_primary_room_order(events, count, 2, "d3l",
                             contract->csb_lineage_room_dispatch_anchor);
    check_primary_room_order(events, count, 3, "d3r",
                             contract->csb_lineage_room_dispatch_anchor);
    check_primary_room_order(events, count, 4, "d3c",
                             contract->csb_lineage_room_dispatch_anchor);
    check_primary_room_order(events, count, 7, "d2l",
                             contract->csb_lineage_room_dispatch_anchor);
    check_primary_room_order(events, count, 8, "d2r",
                             contract->csb_lineage_room_dispatch_anchor);
    check_primary_room_order(events, count, 9, "d2c",
                             contract->csb_lineage_room_dispatch_anchor);

    check_before("d3l2_before_d3l_draw",
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 0),
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 2),
                 contract->csb_lineage_room_dispatch_anchor);
    check_before("d3r2_before_d3r_draw",
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 1),
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 3),
                 contract->csb_lineage_room_dispatch_anchor);
    check_before("d2l2_before_d2l_draw",
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 5),
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 7),
                 contract->csb_lineage_room_dispatch_anchor);
    check_before("d2r2_before_d2r_draw",
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_CUSTOM_BACKGROUNDS, 6),
                 csb_v1_viewport_custom_backgrounds_room_pass_order_find_pc34(
                     events, count, CSB_V1_ROOM_PASS_EVENT_ROOM_DRAW_ENTER, 8),
                 contract->csb_lineage_room_dispatch_anchor);
}

int main(void)
{
    const CSB_V1_CustomBackgroundsRoomPassOrderContract *contract =
        csb_v1_viewport_custom_backgrounds_room_pass_order_contract_pc34();

    printf("probe=csb_v1_viewport_custom_backgrounds_room_pass_order_pc34_compat\n");
    printf("source=%s\n",
           csb_v1_viewport_custom_backgrounds_room_pass_order_source_evidence_pc34());

    test_contract_and_evidence();
    test_trace_order_and_hash();

    printf("hash=0x%08x\n", contract ? contract->expected_trace_hash : 0u);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
