#include "dm1/dm1_v1_viewport_d2c_stair_rail_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0121_UP "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7257-7268"
#define A_F0121_DOWN "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7269-7288"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0096 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2517-2518"
#define A_DEFS_SLOTS "DEFS.H:2444/2451"
#define A_DEFS_ZONES "DEFS.H:4145/4158"
#define A_DEFS_D2C "DEFS.H:2602"
#define A_DEFS_C10 "DEFS.H:2088"
#define A_BLIT "BLIT.C:30-31"

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

static void expect_contains(const char *id, const char *anchor,
                            const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s anchor=%s missing=%s\n", id, anchor, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s contains=%s\n", id, anchor, needle);
    }
}

static void expect_bytes(const char *id, const char *anchor,
                         const uint8_t *got, const uint8_t *want, size_t count)
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

static DM1_V1_D2CStairRailBlitInputPc34 blit_input(
    DM1_V1_D2CStairRailRolePc34 role,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D2CStairRailBlitInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.role = role;
    input.source = source;
    input.source_len = source_len;
    input.destination = destination;
    input.destination_len = destination_len;
    input.row_width = row_width;
    input.height = height;
    input.destination_stride = destination_stride;
    input.contract_only = true;
    input.real_asset_claim = false;
    return input;
}

static void test_evidence(void)
{
    size_t count = 0;
    const DM1_V1_D2CStairRailEvidencePc34 *e =
        dm1_v1_viewport_d2c_stair_rail_evidence_pc34(&count);
    const DM1_V1_D2CStairRailEvidencePc34 *up =
        dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
            DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34);
    const DM1_V1_D2CStairRailEvidencePc34 *down =
        dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
            DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34);
    const char *summary = dm1_v1_viewport_d2c_stair_rail_source_evidence_pc34();

    expect_size("evidence.count", A_F0121_UP, count, 2);
    expect_int("evidence.array.nonnull", A_F0121_UP, e != NULL, 1);
    expect_int("evidence.up.exists", A_F0121_UP, up != NULL, 1);
    expect_int("evidence.down.exists", A_F0121_DOWN, down != NULL, 1);
    expect_int("evidence.unknown.missing", A_F0121_UP,
               dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
                   (DM1_V1_D2CStairRailRolePc34)99) == NULL, 1);
    expect_contains("summary.up", A_F0121_UP, summary, A_F0121_UP);
    expect_contains("summary.down", A_F0121_DOWN, summary, A_F0121_DOWN);
    expect_contains("summary.f0104", A_F0104, summary, A_F0104);
    expect_contains("summary.f0096", A_F0096, summary, A_F0096);
    expect_contains("summary.slots", A_DEFS_SLOTS, summary, "DEFS.H:2444/2451");
    expect_contains("summary.zones", A_DEFS_ZONES, summary, "DEFS.H:4145/4158");
    expect_contains("summary.d2c", A_DEFS_D2C, summary, "DEFS.H:2602");
    expect_contains("summary.c10", A_DEFS_C10, summary, "DEFS.H:2088");
    expect_contains("summary.no_real_asset", A_F0104, summary, "no real-asset");

    expect_int("up.contract_only", A_F0121_UP, up ? up->contract_only : 0, 1);
    expect_int("up.no_real_asset", A_F0121_UP, up ? up->real_asset_claim : 1, 0);
    expect_contains("up.dispatch_anchor", A_F0121_UP, up ? up->dispatch_anchor : "", A_F0121_UP);
    expect_contains("up.draw_anchor", A_F0104, up ? up->draw_anchor : "", A_F0104);
    expect_contains("up.defs_anchor.slot", A_DEFS_SLOTS, up ? up->defs_anchor : "",
                    "C03_STAIRS_BITMAP_UP_FRONT_D2C");
    expect_contains("up.defs_anchor.zone", A_DEFS_ZONES, up ? up->defs_anchor : "",
                    "C806_ZONE_STAIRS_UP_FRONT_D2C");

    expect_int("down.contract_only", A_F0121_DOWN, down ? down->contract_only : 0, 1);
    expect_int("down.no_real_asset", A_F0121_DOWN, down ? down->real_asset_claim : 1, 0);
    expect_contains("down.dispatch_anchor", A_F0121_DOWN,
                    down ? down->dispatch_anchor : "", A_F0121_DOWN);
    expect_contains("down.draw_anchor", A_F0104, down ? down->draw_anchor : "", A_F0104);
    expect_contains("down.defs_anchor.slot", A_DEFS_SLOTS, down ? down->defs_anchor : "",
                    "C10_STAIRS_BITMAP_DOWN_FRONT_D2C");
    expect_contains("down.defs_anchor.zone", A_DEFS_ZONES, down ? down->defs_anchor : "",
                    "C819_ZONE_STAIRS_DOWN_FRONT_D2C");
}

static void test_role_dimensions(void)
{
    const DM1_V1_D2CStairRailEvidencePc34 *up =
        dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
            DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34);
    const DM1_V1_D2CStairRailEvidencePc34 *down =
        dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
            DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34);

    expect_int("up.slot", A_DEFS_SLOTS, up ? up->stairs_bitmap_slot : -1, 3);
    expect_int("up.zone", A_DEFS_ZONES, up ? up->zone_index : -1, 806);
    expect_int("up.view_square", A_DEFS_D2C, up ? up->view_square_index : -1, 6);
    expect_int("up.transparent", A_DEFS_C10, up ? up->transparent_color : -1, 10);
    expect_size("up.synthetic_width", A_F0104, up ? up->synthetic_width : 0, 6);
    expect_size("up.synthetic_height", A_F0104, up ? up->synthetic_height : 0, 4);

    expect_int("down.slot", A_DEFS_SLOTS, down ? down->stairs_bitmap_slot : -1, 10);
    expect_int("down.zone", A_DEFS_ZONES, down ? down->zone_index : -1, 819);
    expect_int("down.view_square", A_DEFS_D2C, down ? down->view_square_index : -1, 6);
    expect_int("down.transparent", A_DEFS_C10, down ? down->transparent_color : -1, 10);
    expect_size("down.synthetic_width", A_F0104, down ? down->synthetic_width : 0, 6);
    expect_size("down.synthetic_height", A_F0104, down ? down->synthetic_height : 0, 4);
}

static void test_resolve(void)
{
    DM1_V1_D2CStairRailResolveInputPc34 input;
    DM1_V1_D2CStairRailResolveResultPc34 result;

    memset(&input, 0, sizeof(input));
    input.contract_only = true;
    input.real_asset_claim = false;
    input.wallset_index = 0;
    input.role = DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34;
    expect_int("resolve.up.ok.wallset0", A_F0096,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&input, &result), 1);
    expect_int("resolve.up.first.wallset0", A_F0096, result.first_stairs_graphic_index, 108);
    expect_int("resolve.up.native.wallset0", A_F0121_UP, result.native_bitmap_index, 111);
    expect_int("resolve.up.slot", A_DEFS_SLOTS, result.stairs_bitmap_slot, 3);
    expect_int("resolve.up.zone", A_DEFS_ZONES, result.zone_index, 806);
    expect_int("resolve.up.view_square", A_DEFS_D2C, result.view_square_index, 6);
    expect_int("resolve.up.contract_only", A_F0104, result.contract_only, 1);
    expect_int("resolve.up.no_real_asset", A_F0104, result.real_asset_claim, 0);
    expect_int("resolve.up.evidence", A_F0121_UP, result.evidence != NULL, 1);

    input.wallset_index = 2;
    input.role = DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34;
    expect_int("resolve.down.ok.wallset2", A_F0096,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&input, &result), 1);
    expect_int("resolve.down.first.wallset2", A_F0096, result.first_stairs_graphic_index, 188);
    expect_int("resolve.down.native.wallset2", A_F0121_DOWN, result.native_bitmap_index, 198);
    expect_int("resolve.down.slot", A_DEFS_SLOTS, result.stairs_bitmap_slot, 10);
    expect_int("resolve.down.zone", A_DEFS_ZONES, result.zone_index, 819);
    expect_int("resolve.down.view_square", A_DEFS_D2C, result.view_square_index, 6);
}

static void test_byte_and_row_behavior(void)
{
    const uint8_t source[] = {
        1, 10, 3, 4, 10, 6,
        7, 8, 10, 10, 11, 12
    };
    const uint8_t want[] = {
        1, 0xee, 3, 4, 0xee, 6, 0xee, 0xee,
        7, 8, 0xee, 0xee, 11, 12, 0xee, 0xee
    };
    uint8_t destination[16];
    DM1_V1_D2CStairRailBlitInputPc34 input;
    DM1_V1_D2CStairRailBlitResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = blit_input(DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34,
                       source, sizeof(source), destination, sizeof(destination), 6, 2, 8);
    expect_int("blit.up.ok", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&input, &result), 1);
    expect_bytes("blit.up.bytes", A_BLIT, destination, want, sizeof(want));
    expect_size("blit.up.row_width", A_F0104, result.row_width, 6);
    expect_size("blit.up.height", A_F0104, result.height, 2);
    expect_size("blit.up.byte_count", A_F0104, result.byte_count, 12);
    expect_size("blit.up.destination_stride", A_BLIT, result.destination_stride, 8);
    expect_size("blit.up.writes", A_F0104, result.writes, 8);
    expect_size("blit.up.transparent_skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("blit.up.transparent_seen", A_DEFS_C10, result.transparent_skip_seen, 1);
    expect_int("blit.up.wrote_any", A_F0104, result.wrote_any, 1);
    expect_int("blit.up.first_source", A_BLIT, result.first_source_byte, 1);
    expect_int("blit.up.last_source", A_BLIT, result.last_source_byte, 12);
    expect_int("blit.up.first_destination", A_BLIT, result.first_destination_byte, 1);
    expect_int("blit.up.last_destination", A_BLIT, result.last_destination_byte, 12);
    expect_int("blit.up.row0.padding6", A_BLIT, destination[6], 0xee);
    expect_int("blit.up.row0.padding7", A_BLIT, destination[7], 0xee);
    expect_int("blit.up.row1.first", A_BLIT, destination[8], 7);
    expect_int("blit.up.row1.last", A_BLIT, destination[13], 12);

    {
        const uint8_t down_source[] = { 21, 22, 23, 24, 25, 26 };
        const uint8_t down_want[] = { 21, 22, 23, 24, 25, 26 };

        memset(destination, 0xdd, sizeof(destination));
        input = blit_input(DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34,
                           down_source, sizeof(down_source), destination,
                           sizeof(destination), 6, 1, 6);
        expect_int("blit.down.ok", A_F0104,
                   dm1_v1_viewport_d2c_stair_rail_blit_pc34(&input, &result), 1);
        expect_bytes("blit.down.bytes", A_BLIT, destination, down_want, sizeof(down_want));
        expect_size("blit.down.writes", A_F0104, result.writes, 6);
        expect_size("blit.down.transparent_skips", A_DEFS_C10, result.transparent_skips, 0);
        expect_int("blit.down.evidence", A_F0121_DOWN,
                   result.evidence &&
                   result.evidence->role == DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34, 1);
    }
}

static void test_edges_and_noops(void)
{
    const uint8_t transparent_source[] = { 10, 10, 10, 10 };
    uint8_t destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    const uint8_t original_destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    DM1_V1_D2CStairRailResolveInputPc34 resolve;
    DM1_V1_D2CStairRailResolveResultPc34 resolve_result;
    DM1_V1_D2CStairRailBlitInputPc34 blit;
    DM1_V1_D2CStairRailBlitResultPc34 blit_result;

    memset(&resolve, 0, sizeof(resolve));
    resolve.contract_only = true;
    resolve.real_asset_claim = false;
    resolve.wallset_index = -1;
    resolve.role = DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34;
    expect_int("resolve.reject.negative_wallset", A_F0096,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&resolve, &resolve_result), 0);

    resolve.wallset_index = 0;
    resolve.contract_only = false;
    expect_int("resolve.reject.not_contract", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&resolve, &resolve_result), 0);

    resolve.contract_only = true;
    resolve.real_asset_claim = true;
    expect_int("resolve.reject.real_asset_claim", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&resolve, &resolve_result), 0);

    resolve.real_asset_claim = false;
    resolve.role = (DM1_V1_D2CStairRailRolePc34)42;
    expect_int("resolve.reject.unknown_role", A_F0121_UP,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&resolve, &resolve_result), 0);
    expect_int("resolve.reject.null_input", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(NULL, &resolve_result), 0);
    expect_int("resolve.reject.null_output", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_resolve_pc34(&resolve, NULL), 0);

    blit = blit_input(DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34,
                      transparent_source, sizeof(transparent_source),
                      destination, sizeof(destination), 2, 2, 2);
    expect_int("blit.transparent_only.ok", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 1);
    expect_size("blit.transparent_only.writes", A_DEFS_C10, blit_result.writes, 0);
    expect_size("blit.transparent_only.skips", A_DEFS_C10, blit_result.transparent_skips, 4);
    expect_int("blit.transparent_only.wrote_any", A_DEFS_C10, blit_result.wrote_any, 0);
    expect_bytes("blit.transparent_only.noop", A_DEFS_C10, destination,
                 original_destination, sizeof(destination));

    blit.contract_only = false;
    expect_int("blit.reject.not_contract", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.contract_only = true;
    blit.real_asset_claim = true;
    expect_int("blit.reject.real_asset_claim", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.real_asset_claim = false;
    blit.source = NULL;
    expect_int("blit.reject.null_source", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.source = transparent_source;
    blit.destination = NULL;
    expect_int("blit.reject.null_destination", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.destination = destination;
    blit.row_width = 0;
    expect_int("blit.reject.zero_width", A_BLIT,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.row_width = 2;
    blit.height = 0;
    expect_int("blit.reject.zero_height", A_BLIT,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.height = 2;
    blit.destination_stride = 1;
    expect_int("blit.reject.short_stride", A_BLIT,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.destination_stride = 2;
    blit.source_len = 3;
    expect_int("blit.reject.short_source", A_BLIT,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.source_len = sizeof(transparent_source);
    blit.destination_len = 3;
    expect_int("blit.reject.short_destination", A_BLIT,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    blit.destination_len = sizeof(destination);
    blit.role = (DM1_V1_D2CStairRailRolePc34)77;
    expect_int("blit.reject.unknown_role", A_F0121_DOWN,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, &blit_result), 0);
    expect_int("blit.reject.null_input", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(NULL, &blit_result), 0);
    expect_int("blit.reject.null_output", A_F0104,
               dm1_v1_viewport_d2c_stair_rail_blit_pc34(&blit, NULL), 0);
}

int main(void)
{
    test_evidence();
    test_role_dimensions();
    test_resolve();
    test_byte_and_row_behavior();
    test_edges_and_noops();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
