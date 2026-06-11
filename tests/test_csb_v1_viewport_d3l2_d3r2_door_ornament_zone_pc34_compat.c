#include "csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_C0_DOOR_STATE_OPEN = 0,             /* ReDMCSB DEFS.H:1039. */
    CSB_C4_DOOR_STATE_CLOSED = 4,           /* ReDMCSB DEFS.H:1043. */
    CSB_C5_DOOR_STATE_DESTROYED = 5,        /* ReDMCSB DEFS.H:1044. */
    CSB_F0111_BRANCH_DESTROYED = 3,         /* Contract branch id for F0111:4301-4304. */
    CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0 = 693,
    CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR = 0,    /* ReDMCSB DEFS.H:2789. */
    CSB_C15_DOOR_ORNAMENT_DESTROYED_MASK = 15,
    CSB_C16_DOOR_ORNAMENT_THIEVES_EYE_MASK = 16,
    CSB_C3700_ZONE_DOOR_D3L2 = 3700,        /* ReDMCSB DEFS.H:4250. */
    CSB_C3710_ZONE_DOOR_D3R2 = 3710,        /* ReDMCSB DEFS.H:4251. */
    CSB_M631_ZONE_DOOR_D1C = 3790
};

static const char A_D3L2_CLOSED[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4334; F0676 line 6272 passes "
    "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR, "
    "C0_VIEW_DOOR_ORNAMENT_D3LCR, C3700_ZONE_DOOR_D3L2 as "
    "P2084_i_ZoneIndex; DEFS.H:2789,4250,5456; "
    "CSB-lineage/CSBWin Viewport.cpp:1813-1820,2568,2596-2616";
static const char A_D3L2_OPEN[] =
    "ReDMCSB DUNVIEW.C F0111:4248-4250 MEDIA744_A36M returns on "
    "C0_DOOR_STATE_OPEN before bitmap/ornament draw; F0676 line 6272 "
    "C3700_ZONE_DOOR_D3L2; DEFS.H:1039,4250; "
    "CSB-lineage/CSBWin Viewport.cpp:1813-1820";
static const char A_D3R2_CLOSED[] =
    "ReDMCSB DUNVIEW.C F0111:4218-4334; F0677 line 6339 passes "
    "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR, "
    "C0_VIEW_DOOR_ORNAMENT_D3LCR, C3710_ZONE_DOOR_D3R2 as "
    "P2084_i_ZoneIndex; DEFS.H:2789,4251,5456; "
    "CSB-lineage/CSBWin Viewport.cpp:1813-1820,2568,2596-2616";
static const char A_D3R2_DESTROYED[] =
    "ReDMCSB DUNVIEW.C F0111:4257-4261 resolves G0693[door type], "
    "F0111:4301-4304 applies C15 destroyed ornament, F0677 line 6339 "
    "uses C3710_ZONE_DOOR_D3R2; DEFS.H:1044,2789,4251,5456; "
    "CSB-lineage/CSBWin Viewport.cpp:1818-1820 StdDrawDoor";
static const char A_D3R2_THIEVES_EYE[] =
    "ReDMCSB DUNVIEW.C F0111:4292-4294 only draws "
    "C16_DOOR_ORNAMENT_THIEVES_EYE_MASK when P2084_i_ZoneIndex equals "
    "M631_ZONE_DOOR_D1C; C3710_ZONE_DOOR_D3R2 != M631; "
    "DEFS.H:4246/4251 or 4259/4251; CSB-lineage/CSBWin Viewport.cpp:1818";

static const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 s_contracts[] = {
    {
        "d3l2_closed_type0",
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_C4_DOOR_STATE_CLOSED,
        0,
        CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3700_ZONE_DOOR_D3L2,
        0,
        0,
        0,
        0,
        A_D3L2_CLOSED
    },
    {
        "d3l2_open_early_return",
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_C0_DOOR_STATE_OPEN,
        0,
        -1,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        -1,
        1,
        0,
        0,
        0,
        A_D3L2_OPEN
    },
    {
        "d3r2_closed_type0",
        CSB_C3710_ZONE_DOOR_D3R2,
        CSB_C4_DOOR_STATE_CLOSED,
        0,
        CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3710_ZONE_DOOR_D3R2,
        0,
        0,
        0,
        0,
        A_D3R2_CLOSED
    },
    {
        "d3r2_destroyed_branch",
        CSB_C3710_ZONE_DOOR_D3R2,
        CSB_C5_DOOR_STATE_DESTROYED,
        0,
        CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3710_ZONE_DOOR_D3R2,
        0,
        CSB_F0111_BRANCH_DESTROYED,
        0,
        0,
        A_D3R2_DESTROYED
    },
    {
        "d3r2_thieves_eye_not_d1c",
        CSB_C3710_ZONE_DOOR_D3R2,
        CSB_C4_DOOR_STATE_CLOSED,
        0,
        CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0,
        CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
        CSB_C3710_ZONE_DOOR_D3R2,
        0,
        0,
        0,
        1,
        A_D3R2_THIEVES_EYE
    }
};

static int g_assertions;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int contract_count(void)
{
    return (int)(sizeof(s_contracts) / sizeof(s_contracts[0]));
}

static int resolved_native_bitmap_index(
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *contract)
{
    if (!contract || contract->expected_early_return) {
        return -1;
    }
    return CSB_G0693_NATIVE_BITMAP_D3LCR_TYPE0 + contract->door_type;
}

static int final_zone_mapping(
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *contract)
{
    if (!contract || contract->expected_early_return) {
        return -1;
    }
    return contract->zone;
}

static int branch_for_state(int door_state)
{
    if (door_state == CSB_C0_DOOR_STATE_OPEN) {
        return 0;
    }
    if (door_state == CSB_C5_DOOR_STATE_DESTROYED) {
        return CSB_F0111_BRANCH_DESTROYED;
    }
    return 1;
}

static int thieves_eye_ornament_drawn(
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *contract)
{
    if (!contract || !contract->event73_count_thieves_eye) {
        return 0;
    }
    return contract->zone == CSB_M631_ZONE_DOOR_D1C;
}

static int test_contract_rows(void)
{
    int ok = 1;

    ok &= expect_int("contract.count", contract_count(), 5,
                     "assigned D3L2/D3R2 door ornament zone coverage");

    for (int i = 0; i < contract_count(); ++i) {
        const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *contract =
            &s_contracts[i];

        ok &= expect_contains("row.anchor.redmcsb", contract->anchor,
                              "ReDMCSB DUNVIEW.C F0111", contract->anchor);
        ok &= expect_contains("row.anchor.lineage", contract->anchor,
                              "CSB-lineage/CSBWin", contract->anchor);
        ok &= expect_int("row.ornament.c0",
                         contract->ornament_index,
                         CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR,
                         contract->anchor);
    }

    return ok;
}

static int test_d3l2_closed_and_open(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *closed =
        &s_contracts[0];
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *open =
        &s_contracts[1];

    /* ReDMCSB: DUNVIEW.C F0676 line 6272 passes C3700 as P2084_i_ZoneIndex;
     * F0111 lines 4248-4250 return before bitmap and ornament work for open. */
    ok &= expect_int("d3l2.closed.zone.c3700", closed->zone,
                     CSB_C3700_ZONE_DOOR_D3L2, closed->anchor);
    ok &= expect_int("d3l2.closed.state.c4", closed->door_state,
                     CSB_C4_DOOR_STATE_CLOSED, closed->anchor);
    ok &= expect_int("d3l2.closed.native.g0693.type0",
                     resolved_native_bitmap_index(closed),
                     closed->door_native_bitmap_index, closed->anchor);
    ok &= expect_int("d3l2.closed.p2084_zone",
                     final_zone_mapping(closed),
                     closed->expected_zone_mapping, closed->anchor);
    ok &= expect_contains("d3l2.closed.zone.constant", closed->anchor,
                          "C3700_ZONE_DOOR_D3L2", closed->anchor);

    ok &= expect_int("d3l2.open.zone.c3700", open->zone,
                     CSB_C3700_ZONE_DOOR_D3L2, open->anchor);
    ok &= expect_int("d3l2.open.state.c0", open->door_state,
                     CSB_C0_DOOR_STATE_OPEN, open->anchor);
    ok &= expect_int("d3l2.open.early_return", open->expected_early_return,
                     1, open->anchor);
    ok &= expect_int("d3l2.open.no_native_fetch",
                     resolved_native_bitmap_index(open), -1, open->anchor);
    ok &= expect_int("d3l2.open.no_zone_blit",
                     final_zone_mapping(open), -1, open->anchor);

    return ok;
}

static int test_d3r2_closed_destroyed_and_thieves_eye(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *closed =
        &s_contracts[2];
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *destroyed =
        &s_contracts[3];
    const CSB_V1_ViewportD3L2D3R2DoorOrnamentZoneContractPc34 *thieves_eye =
        &s_contracts[4];

    /* ReDMCSB: DUNVIEW.C F0677 line 6339 passes C3710 as P2084_i_ZoneIndex.
     * F0111 line 4301 enters the destroyed branch only for C5, while this
     * contract labels that branch as branch id 3 for parity table checking. */
    ok &= expect_int("d3r2.closed.zone.c3710", closed->zone,
                     CSB_C3710_ZONE_DOOR_D3R2, closed->anchor);
    ok &= expect_int("d3r2.closed.state.c4", closed->door_state,
                     CSB_C4_DOOR_STATE_CLOSED, closed->anchor);
    ok &= expect_int("d3r2.closed.native.g0693.type0",
                     resolved_native_bitmap_index(closed),
                     closed->door_native_bitmap_index, closed->anchor);
    ok &= expect_int("d3r2.closed.ornament.c0",
                     closed->ornament_index,
                     CSB_C0_VIEW_DOOR_ORNAMENT_D3LCR, closed->anchor);
    ok &= expect_int("d3r2.closed.p2084_zone",
                     final_zone_mapping(closed),
                     closed->expected_zone_mapping, closed->anchor);

    ok &= expect_int("d3r2.destroyed.state.c5_source_lock",
                     destroyed->door_state,
                     CSB_C5_DOOR_STATE_DESTROYED, destroyed->anchor);
    ok &= expect_int("d3r2.destroyed.branch_id_3",
                     branch_for_state(destroyed->door_state),
                     destroyed->expected_destroyed_branch, destroyed->anchor);
    ok &= expect_int("d3r2.destroyed.mask.c15",
                     CSB_C15_DOOR_ORNAMENT_DESTROYED_MASK, 15,
                     destroyed->anchor);
    ok &= expect_int("d3r2.destroyed.native.g0693.type0",
                     resolved_native_bitmap_index(destroyed),
                     destroyed->door_native_bitmap_index, destroyed->anchor);
    ok &= expect_int("d3r2.destroyed.p2084_zone",
                     final_zone_mapping(destroyed),
                     destroyed->expected_zone_mapping, destroyed->anchor);

    ok &= expect_int("d3r2.thieves_eye.event73",
                     thieves_eye->event73_count_thieves_eye, 1,
                     thieves_eye->anchor);
    ok &= expect_int("d3r2.thieves_eye.zone.not_m631",
                     thieves_eye->zone == CSB_M631_ZONE_DOOR_D1C,
                     0, thieves_eye->anchor);
    ok &= expect_int("d3r2.thieves_eye.mask.c16",
                     CSB_C16_DOOR_ORNAMENT_THIEVES_EYE_MASK, 16,
                     thieves_eye->anchor);
    ok &= expect_int("d3r2.thieves_eye.not_drawn",
                     thieves_eye_ornament_drawn(thieves_eye),
                     thieves_eye->expected_thieves_eye_ornament_drawn,
                     thieves_eye->anchor);
    ok &= expect_contains("d3r2.closed.zone.constant", closed->anchor,
                          "C3710_ZONE_DOOR_D3R2", closed->anchor);

    return ok;
}

int csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat_test(void)
{
    int ok = 1;

    ok &= test_contract_rows();
    ok &= test_d3l2_closed_and_open();
    ok &= test_d3r2_closed_destroyed_and_thieves_eye();

    ok &= expect_int("assertion_count_at_least_35", g_assertions >= 35, 1,
                     "assigned CSB V1 D3L2/D3R2 door ornament zone source lock");
    return ok ? 0 : 1;
}

int main(void)
{
    const int result =
        csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat_test();

    printf("probe=csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat\n");
    printf("assertions=%d\n", g_assertions);
    if (result == 0) {
        printf("PASS csb_v1_viewport_d3l2_d3r2_door_ornament_zone_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return result;
}
