#include "dm1/dm1_v1_viewport_f0099_row_local_flip_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0099 "DUNVIEW.C:F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal:3018-3075"
#define A_F1000_2092 "DUNVIEW.C:F1000_:2092-2092"
#define A_F1000_2101 "DUNVIEW.C:F1000_:2101-2101"
#define A_F0095_2205 "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2205-2205"
#define A_F0095_2206 "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2206-2206"
#define A_F0096_2389 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2389-2389"
#define A_F0096_2397 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2397-2397"
#define A_F0096_2406 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2406-2406"
#define A_F0096_2407 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2407-2407"
#define A_F0096_2408 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2408-2408"
#define A_F0096_2411 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2411-2411"
#define A_F0096_2412 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2412-2412"

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, const char *anchor, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s anchor=%s got=%d want=%d\n", id, anchor, got, want);
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s value=%d\n", id, anchor, want);
    }
}

static void expect_size(const char *id, const char *anchor, size_t got, size_t want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s anchor=%s got=%lu want=%lu\n",
               id, anchor, (unsigned long)got, (unsigned long)want);
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s value=%lu\n", id, anchor, (unsigned long)want);
    }
}

static void expect_contains(const char *id, const char *anchor, const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s anchor=%s missing=%s\n", id, anchor, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s contains=%s\n", id, anchor, needle);
    }
}

static void expect_bytes(
    const char *id,
    const char *anchor,
    const uint8_t *got,
    const uint8_t *want,
    size_t count)
{
    size_t i;
    ++g_assertions;
    for (i = 0; i < count; ++i) {
        if (got[i] != want[i]) {
            printf("FAIL %s anchor=%s index=%lu got=0x%02x want=0x%02x\n",
                   id, anchor, (unsigned long)i, got[i], want[i]);
            ++g_failures;
            return;
        }
    }
    printf("PASS %s anchor=%s bytes=%lu\n", id, anchor, (unsigned long)count);
}

static void run_flip(
    const char *id,
    const char *anchor,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    DM1_V1_F0099RowLocalFlipResultPc34 *result)
{
    DM1_V1_F0099RowLocalFlipStatePc34 state;
    memset(&state, 0, sizeof(state));
    state.source = source;
    state.source_len = source_len;
    state.destination = destination;
    state.destination_len = destination_len;
    state.row_width = row_width;
    state.height = height;
    state.contract_only = true;
    state.real_asset_claim = false;

    expect_int(id, anchor,
               dm1_v1_viewport_f0099_row_local_flip_pc34_compat(&state, result) ? 1 : 0,
               1);
}

static void test_evidence(void)
{
    size_t count = 0;
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *e =
        dm1_v1_viewport_f0099_row_local_flip_evidence_pc34(&count);
    const char *summary = dm1_v1_viewport_f0099_row_local_flip_source_evidence_pc34();
    size_t i;

    expect_size("evidence.count", A_F0099, count, 11);
    expect_contains("evidence.summary.f0099", A_F0099, summary, A_F0099);
    expect_contains("evidence.summary.f1000.2092", A_F1000_2092, summary, A_F1000_2092);
    expect_contains("evidence.summary.f1000.2101", A_F1000_2101, summary, A_F1000_2101);
    expect_contains("evidence.summary.f0095", A_F0095_2205, summary,
                    "DUNVIEW.C:F0095_DUNGEONVIEW_LoadWallSet:2205-2206");
    expect_contains("evidence.summary.f0096", A_F0096_2389, summary,
                    "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2389-2412");

    for (i = 0; i < count; ++i) {
        expect_int("evidence.contract_only", e[i].caller_anchor,
                   e[i].contract_only ? 1 : 0, 1);
        expect_int("evidence.no_real_asset_claim", e[i].caller_anchor,
                   e[i].real_asset_claim ? 1 : 0, 0);
        expect_contains("evidence.copy_anchor", e[i].caller_anchor,
                        e[i].copy_anchor, A_F0099);
    }
}

static void test_role_dimensions(void)
{
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *door =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_DOOR_FRAME_D1C_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d3 =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3L2_TO_D3R2_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d3c =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3C_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d2c =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D2C_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d1 =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D1LCR_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d0l =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0L_TO_D0R_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *d0r =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0R_TO_D0L_WALL_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *floor =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4052_FLOOR_PC34);
    const DM1_V1_F0099RowLocalFlipEvidencePc34 *ceiling =
        dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
            DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4053_CEILING_PC34);

    expect_int("door.exists", A_F0095_2205, door != NULL, 1);
    expect_int("door.width", A_F0095_2205, door ? door->byte_width : -1, 16);
    expect_int("door.height", A_F0095_2205, door ? door->height : -1, 94);
    expect_int("d3l2.d3r2.exists", A_F0095_2206, d3 != NULL, 1);
    expect_int("d3l2.d3r2.width", A_F0095_2206, d3 ? d3->byte_width : -1, 8);
    expect_int("d3l2.d3r2.height", A_F0095_2206, d3 ? d3->height : -1, 49);
    expect_int("d3c.width", A_F0096_2389, d3c ? d3c->byte_width : -1, 64);
    expect_int("d3c.height", A_F0096_2389, d3c ? d3c->height : -1, 51);
    expect_int("d2c.width", A_F0096_2397, d2c ? d2c->byte_width : -1, 72);
    expect_int("d2c.height", A_F0096_2397, d2c ? d2c->height : -1, 71);
    expect_int("d1lcr.width", A_F0096_2406, d1 ? d1->byte_width : -1, 128);
    expect_int("d1lcr.height", A_F0096_2406, d1 ? d1->height : -1, 111);
    expect_int("d0l.width", A_F0096_2407, d0l ? d0l->byte_width : -1, 16);
    expect_int("d0l.height", A_F0096_2407, d0l ? d0l->height : -1, 136);
    expect_int("d0r.width", A_F0096_2408, d0r ? d0r->byte_width : -1, 16);
    expect_int("d0r.height", A_F0096_2408, d0r ? d0r->height : -1, 136);
    expect_int("d0.mutual.width_match", A_F0096_2408,
               d0l && d0r && d0l->byte_width == d0r->byte_width, 1);
    expect_int("d0.mutual.height_match", A_F0096_2408,
               d0l && d0r && d0l->height == d0r->height, 1);
    expect_int("floor.width", A_F0096_2411, floor ? floor->byte_width : -1, 112);
    expect_int("floor.height", A_F0096_2411, floor ? floor->height : -1, 70);
    expect_int("ceiling.width", A_F0096_2412, ceiling ? ceiling->byte_width : -1, 112);
    expect_int("ceiling.height", A_F0096_2412, ceiling ? ceiling->height : -1, 29);
}

static void test_byte_order_and_row_boundaries(void)
{
    const uint8_t four[] = { 0x01, 0x02, 0x04, 0x08 };
    const uint8_t four_want[] = { 0x08, 0x04, 0x02, 0x01 };
    const uint8_t two_rows[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    const uint8_t two_rows_want[] = { 4, 3, 2, 1, 8, 7, 6, 5 };
    uint8_t destination[16];
    DM1_V1_F0099RowLocalFlipResultPc34 result;

    memset(destination, 0xcc, sizeof(destination));
    run_flip("flip.width4.byte_order", A_F0099, four, sizeof(four),
             destination, sizeof(destination), 4, 1, &result);
    expect_bytes("flip.width4.bytes", A_F0099, destination, four_want, sizeof(four_want));
    expect_bytes("flip.width4.source_unchanged", A_F0099, four, (const uint8_t[]){ 1, 2, 4, 8 }, 4);
    expect_int("flip.width4.contract_only", A_F0099, result.contract_only ? 1 : 0, 1);
    expect_int("flip.width4.out_of_place", A_F0099, result.in_place ? 1 : 0, 0);

    memset(destination, 0xcc, sizeof(destination));
    run_flip("flip.width4.two_rows", A_F0099, two_rows, sizeof(two_rows),
             destination, sizeof(destination), 4, 2, &result);
    expect_bytes("flip.width4.two_rows.bytes", A_F0099, destination,
                 two_rows_want, sizeof(two_rows_want));
    expect_int("flip.width4.row0_boundary", A_F0099, destination[3], 1);
    expect_int("flip.width4.row1_boundary", A_F0099, destination[4], 8);
    expect_size("flip.width4.rows_flipped", A_F0099, result.rows_flipped, 2);
}

static void test_width_boundaries_and_edges(void)
{
    const uint8_t eight[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    const uint8_t eight_want[] = { 8, 7, 6, 5, 4, 3, 2, 1 };
    const uint8_t sixteen[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    const uint8_t sixteen_want[] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    };
    const uint8_t one[] = { 9, 8, 7 };
    const uint8_t odd3[] = { 1, 2, 3, 4, 5, 6 };
    const uint8_t odd3_want[] = { 3, 2, 1, 6, 5, 4 };
    const uint8_t odd5[] = { 1, 2, 3, 4, 5 };
    const uint8_t odd5_want[] = { 5, 4, 3, 2, 1 };
    uint8_t destination[32];
    DM1_V1_F0099RowLocalFlipResultPc34 result;

    memset(destination, 0, sizeof(destination));
    run_flip("flip.width8", A_F0095_2206, eight, sizeof(eight),
             destination, sizeof(destination), 8, 1, &result);
    expect_bytes("flip.width8.bytes", A_F0095_2206, destination, eight_want, sizeof(eight_want));
    expect_size("flip.width8.byte_count", A_F0095_2206, result.byte_count, 8);

    memset(destination, 0, sizeof(destination));
    run_flip("flip.width16", A_F0096_2407, sixteen, sizeof(sixteen),
             destination, sizeof(destination), 16, 1, &result);
    expect_bytes("flip.width16.bytes", A_F0096_2407, destination,
                 sixteen_want, sizeof(sixteen_want));
    expect_size("flip.width16.byte_count", A_F0096_2407, result.byte_count, 16);

    memset(destination, 0, sizeof(destination));
    run_flip("flip.width1.edge", A_F0099, one, sizeof(one),
             destination, sizeof(destination), 1, 3, &result);
    expect_bytes("flip.width1.unchanged", A_F0099, destination, one, sizeof(one));
    expect_size("flip.width1.rows", A_F0099, result.rows_flipped, 3);

    memset(destination, 0, sizeof(destination));
    run_flip("flip.odd3.two_rows", A_F0099, odd3, sizeof(odd3),
             destination, sizeof(destination), 3, 2, &result);
    expect_bytes("flip.odd3.bytes", A_F0099, destination, odd3_want, sizeof(odd3_want));
    expect_int("flip.odd3.middle_preserved_row0", A_F0099, destination[1], 2);
    expect_int("flip.odd3.middle_preserved_row1", A_F0099, destination[4], 5);

    memset(destination, 0, sizeof(destination));
    run_flip("flip.odd5", A_F0099, odd5, sizeof(odd5),
             destination, sizeof(destination), 5, 1, &result);
    expect_bytes("flip.odd5.bytes", A_F0099, destination, odd5_want, sizeof(odd5_want));
    expect_int("flip.odd5.middle_preserved", A_F0099, destination[2], 3);
}

static void test_in_place_and_mutual_symmetry(void)
{
    uint8_t in_place[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    const uint8_t in_place_want[] = { 4, 3, 2, 1, 8, 7, 6, 5 };
    uint8_t d0l[] = { 10, 11, 12, 13, 14, 15, 16, 17 };
    uint8_t d0r[8];
    uint8_t d0l_roundtrip[8];
    const uint8_t d0r_want[] = { 17, 16, 15, 14, 13, 12, 11, 10 };
    DM1_V1_F0099RowLocalFlipResultPc34 result;

    run_flip("flip.in_place.first", A_F0099, in_place, sizeof(in_place),
             in_place, sizeof(in_place), 4, 2, &result);
    expect_int("flip.in_place.flag", A_F0099, result.in_place ? 1 : 0, 1);
    expect_bytes("flip.in_place.bytes", A_F0099, in_place,
                 in_place_want, sizeof(in_place_want));
    run_flip("flip.in_place.second", A_F0099, in_place, sizeof(in_place),
             in_place, sizeof(in_place), 4, 2, &result);
    expect_bytes("flip.in_place.roundtrip", A_F0099, in_place,
                 (const uint8_t[]){ 1, 2, 3, 4, 5, 6, 7, 8 }, sizeof(in_place));

    run_flip("flip.d0l.to.d0r", A_F0096_2407, d0l, sizeof(d0l),
             d0r, sizeof(d0r), 8, 1, &result);
    expect_bytes("flip.d0l.to.d0r.bytes", A_F0096_2407, d0r,
                 d0r_want, sizeof(d0r_want));
    run_flip("flip.d0r.back.to.d0l", A_F0096_2408, d0r, sizeof(d0r),
             d0l_roundtrip, sizeof(d0l_roundtrip), 8, 1, &result);
    expect_bytes("flip.d0.mutual.roundtrip", A_F0096_2408, d0l_roundtrip,
                 d0l, sizeof(d0l));
}

static void test_invalid_inputs_remain_contract_only(void)
{
    uint8_t source[] = { 1, 2, 3, 4 };
    uint8_t destination[4];
    DM1_V1_F0099RowLocalFlipStatePc34 state;
    DM1_V1_F0099RowLocalFlipResultPc34 result;

    memset(&state, 0, sizeof(state));
    state.source = source;
    state.source_len = sizeof(source);
    state.destination = destination;
    state.destination_len = sizeof(destination);
    state.row_width = 4;
    state.height = 1;
    state.contract_only = false;
    expect_int("invalid.not_contract_only", A_F0099,
               dm1_v1_viewport_f0099_row_local_flip_pc34_compat(&state, &result) ? 1 : 0,
               0);

    state.contract_only = true;
    state.real_asset_claim = true;
    expect_int("invalid.real_asset_claim", A_F0099,
               dm1_v1_viewport_f0099_row_local_flip_pc34_compat(&state, &result) ? 1 : 0,
               0);

    state.real_asset_claim = false;
    state.row_width = 0;
    expect_int("invalid.zero_width", A_F0099,
               dm1_v1_viewport_f0099_row_local_flip_pc34_compat(&state, &result) ? 1 : 0,
               0);
    state.row_width = 4;
    state.destination_len = 3;
    expect_int("invalid.short_destination", A_F0099,
               dm1_v1_viewport_f0099_row_local_flip_pc34_compat(&state, &result) ? 1 : 0,
               0);
}

int main(void)
{
    test_evidence();
    test_role_dimensions();
    test_byte_order_and_row_boundaries();
    test_width_boundaries_and_edges();
    test_in_place_and_mutual_symmetry();
    test_invalid_inputs_remain_contract_only();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (g_failures) {
        return 1;
    }
    printf("PASS firestaff_dm1_v1_viewport_f0099_row_local_flip_pc34_compat_test assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
