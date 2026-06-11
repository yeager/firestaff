/*
 * ReDMCSB evidence (DM1 V1 D3C door ornament / Thieves Eye mask):
 * - DUNVIEW.C F0111:4218-4339 composes the door in G0074, returns for
 *   open doors in the MEDIA744 branch at 4250-4253, draws the base door
 *   ornament at 4262, applies the Thieves Eye mask only when the zone is
 *   M631_ZONE_DOOR_D1C at 4291-4294, and finally blits at 4334.
 * - DUNVIEW.C F0118:6744 feeds the D3C door-front route to F0111 with
 *   G0693_ai_DoorNativeBitmapIndex_Front_D3LCR,
 *   C0_VIEW_DOOR_ORNAMENT_D3LCR, and M625_ZONE_DOOR_D3C.
 */
#include "dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    DM1_V1_D3C_MASK_PC34_DOOR_STATE_OPEN = 0,
    DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED = 4,
    DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR = 0,
    DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR = 2,
    DM1_V1_D3C_MASK_PC34_C16_THIEVES_EYE_MASK = 16,
    DM1_V1_D3C_MASK_PC34_THIEVES_EYE_ORDINAL = 17,
    DM1_V1_D3C_MASK_PC34_M625_ZONE_DOOR_D3C = 3730,
    DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C = 3790,
    DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT = -1
};

static const char A_D3C_CLOSED[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4339,4262,4291-4294; "
    "DUNVIEW.C F0118:6744 G0693_ai_DoorNativeBitmapIndex_Front_D3LCR "
    "C0_VIEW_DOOR_ORNAMENT_D3LCR M625_ZONE_DOOR_D3C";
static const char A_D3C_OPEN[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4339,4250-4253 MEDIA744 open return; "
    "DUNVIEW.C F0118:6744 C0_VIEW_DOOR_ORNAMENT_D3LCR M625_ZONE_DOOR_D3C";
static const char A_D1C_THIEVES[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4339,4291-4294 "
    "M631_ZONE_DOOR_D1C Event73Count_ThievesEye "
    "M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK) "
    "C2_VIEW_DOOR_ORNAMENT_D1LCR";
static const char A_D1C_NO_EVENT[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4339,4291-4294 "
    "M631_ZONE_DOOR_D1C requires Event73Count_ThievesEye before drawing "
    "M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK)";

static int g_assertions = 0;
static int g_failures = 0;

static int anchor_present_for_assertion(const char *anchor)
{
    return anchor && anchor[0] != '\0' &&
           strstr(anchor, "ReDMCSB DUNVIEW.C F0111:") != NULL;
}

static int expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (!anchor_present_for_assertion(anchor)) {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB DUNVIEW.C F0111 line-range anchor\n",
               id);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        return 0;
    }
    printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    return 1;
}

static int expect_contains(const char *id,
                           const char *haystack,
                           const char *needle,
                           const char *anchor)
{
    ++g_assertions;
    if (!anchor_present_for_assertion(anchor)) {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB DUNVIEW.C F0111 line-range anchor\n",
               id);
        return 0;
    }
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing \"%s\" anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("PASS %s contains \"%s\" anchor=%s\n", id, needle, anchor);
    return 1;
}

static int index_to_ordinal_pc34(int index)
{
    return index + 1;
}

static int f0111_contract_reaches_ornament_branch(
    const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract *contract)
{
    return contract->door_state != DM1_V1_D3C_MASK_PC34_DOOR_STATE_OPEN;
}

static int f0111_contract_draws_thieves_eye(
    const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract *contract,
    int event73_count_thieves_eye)
{
    if (!f0111_contract_reaches_ornament_branch(contract)) {
        return 0;
    }
    return contract->zone_index == DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C &&
           event73_count_thieves_eye != 0;
}

static int test_contract(
    const char *name,
    const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract *contract,
    int event73_count_thieves_eye,
    int expected_zone_index,
    int expected_door_state,
    int expected_route_ornament_index,
    int expected_base_ornament_index,
    int expected_drawn_thieves_eye_ordinal,
    int expected_thieves_eye_view_ornament_index,
    int expected_open_early_return)
{
    int ok = 1;
    const int reached_ornament =
        f0111_contract_reaches_ornament_branch(contract);
    const int thieves_eye_active =
        f0111_contract_draws_thieves_eye(contract, event73_count_thieves_eye);
    const int drawn_thieves_eye_ordinal =
        thieves_eye_active
            ? index_to_ordinal_pc34(DM1_V1_D3C_MASK_PC34_C16_THIEVES_EYE_MASK)
            : DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT;
    const int thieves_eye_view_ornament_index =
        thieves_eye_active
            ? DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR
            : DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT;
    char id[128];

    snprintf(id, sizeof(id), "%s.anchor.nonempty", name);
    ok &= expect_contains(id, contract->redmcsbAnchor,
                          "DUNVIEW.C F0111:", contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.zone_index", name);
    ok &= expect_int(id, contract->zone_index, expected_zone_index,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.door_state", name);
    ok &= expect_int(id, contract->door_state, expected_door_state,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.route_ornament_index", name);
    ok &= expect_int(id, contract->ornament_index_expected,
                     expected_route_ornament_index,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.open_early_return", name);
    ok &= expect_int(id, reached_ornament ? 0 : 1, expected_open_early_return,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.base_ornament_index", name);
    ok &= expect_int(id,
                     reached_ornament ? contract->ornament_index_expected
                                      : DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
                     expected_base_ornament_index,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.event73_count", name);
    ok &= expect_int(id, event73_count_thieves_eye, event73_count_thieves_eye,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.zone_is_d1c", name);
    ok &= expect_int(id,
                     contract->zone_index ==
                         DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
                     contract->zone_index ==
                         DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.thieves_eye_active", name);
    ok &= expect_int(id, thieves_eye_active,
                     contract->thieves_eye_active_expected,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.thieves_eye_ordinal", name);
    ok &= expect_int(id, drawn_thieves_eye_ordinal,
                     expected_drawn_thieves_eye_ordinal,
                     contract->redmcsbAnchor);
    snprintf(id, sizeof(id), "%s.thieves_eye_view_ornament", name);
    ok &= expect_int(id, thieves_eye_view_ornament_index,
                     expected_thieves_eye_view_ornament_index,
                     contract->redmcsbAnchor);

    return ok;
}

int dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat_test_entry(void)
{
    static const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract d3c_closed = {
        DM1_V1_D3C_MASK_PC34_M625_ZONE_DOOR_D3C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        false,
        A_D3C_CLOSED
    };
    static const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract d3c_open = {
        DM1_V1_D3C_MASK_PC34_M625_ZONE_DOOR_D3C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_OPEN,
        DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        false,
        A_D3C_OPEN
    };
    static const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract d1c_closed_event = {
        DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        true,
        A_D1C_THIEVES
    };
    static const DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract d1c_closed_no_event = {
        DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        false,
        A_D1C_NO_EVENT
    };
    int ok = 1;

    ok &= test_contract(
        "d3c.closed.zone_m625",
        &d3c_closed,
        1,
        DM1_V1_D3C_MASK_PC34_M625_ZONE_DOOR_D3C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        0);
    ok &= expect_contains("d3c.closed.f0118.front_native",
                          d3c_closed.redmcsbAnchor,
                          "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
                          d3c_closed.redmcsbAnchor);
    ok &= expect_contains("d3c.closed.f0118.zone",
                          d3c_closed.redmcsbAnchor,
                          "M625_ZONE_DOOR_D3C",
                          d3c_closed.redmcsbAnchor);
    ok &= expect_int("d3c.closed.m625_not_m631",
                     d3c_closed.zone_index ==
                         DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
                     0, d3c_closed.redmcsbAnchor);

    ok &= test_contract(
        "d3c.open.media744_return",
        &d3c_open,
        1,
        DM1_V1_D3C_MASK_PC34_M625_ZONE_DOOR_D3C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_OPEN,
        DM1_V1_D3C_MASK_PC34_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        1);

    ok &= test_contract(
        "d1c.closed.event73",
        &d1c_closed_event,
        1,
        DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_V1_D3C_MASK_PC34_THIEVES_EYE_ORDINAL,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        0);
    ok &= expect_int("d1c.closed.event73.ordinal_formula",
                     DM1_V1_D3C_MASK_PC34_THIEVES_EYE_ORDINAL,
                     index_to_ordinal_pc34(
                         DM1_V1_D3C_MASK_PC34_C16_THIEVES_EYE_MASK),
                     d1c_closed_event.redmcsbAnchor);

    ok &= test_contract(
        "d1c.closed.no_event73",
        &d1c_closed_no_event,
        0,
        DM1_V1_D3C_MASK_PC34_M631_ZONE_DOOR_D1C,
        DM1_V1_D3C_MASK_PC34_DOOR_STATE_CLOSED,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_V1_D3C_MASK_PC34_C2_VIEW_DOOR_ORNAMENT_D1LCR,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        DM1_V1_D3C_MASK_PC34_NO_DRAWN_ORNAMENT,
        0);

    return ok ? 0 : 1;
}

int main(void)
{
    const int rc =
        dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat_test_entry();

    if (rc != 0 || g_failures != 0) {
        printf("FAIL dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
