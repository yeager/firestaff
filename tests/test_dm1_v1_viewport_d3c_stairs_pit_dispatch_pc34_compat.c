#include "dm1_v1_viewport_d3c_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0118_UP "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6666-6676"
#define A_F0118_DOWN "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6677-6696"
#define A_F0118_PIT "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6748-6763"
#define A_F0118_OPEN "DUNVIEW.C:F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6811-6816"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0096 "DUNVIEW.C:F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF:2517-2518"
#define A_DEFS_D3C_SLOTS "DEFS.H:2442/2449"
#define A_DEFS_D3C_ZONES "DEFS.H:4142/4155"
#define A_DEFS_PIT "DEFS.H:2334/4200"
#define A_DEFS_D3C "DEFS.H:2607"
#define A_DEFS_C10 "DEFS.H:2088"

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

static DM1_V1_D3CStairsPitDispatchResultPc34 probe(
    DM1_V1_D3CStairsPitDispatchInputPc34 input,
    const char *id,
    const char *anchor)
{
    DM1_V1_D3CStairsPitDispatchResultPc34 result;
    expect_int(id, anchor,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(&input, &result),
               1);
    return result;
}

static DM1_V1_D3CStairsPitBlitInputPc34 blit_input(
    DM1_V1_D3CStairsPitRolePc34 role,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D3CStairsPitBlitInputPc34 input;
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

static void test_evidence_table(void)
{
    size_t count = 0;
    const DM1_V1_D3CStairsPitEvidencePc34 *all =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_pc34(&count);
    const DM1_V1_D3CStairsPitEvidencePc34 *up =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34);
    const DM1_V1_D3CStairsPitEvidencePc34 *down =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34);
    const DM1_V1_D3CStairsPitEvidencePc34 *pit =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34);

    expect_size("evidence.count", A_F0118_UP, count, 3);
    expect_int("evidence.array.nonnull", A_F0118_UP, all != NULL, 1);
    expect_int("evidence.up.exists", A_F0118_UP, up != NULL, 1);
    expect_int("evidence.down.exists", A_F0118_DOWN, down != NULL, 1);
    expect_int("evidence.pit.exists", A_F0118_PIT, pit != NULL, 1);
    expect_int("evidence.unknown.missing", A_F0118_UP,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
                   (DM1_V1_D3CStairsPitRolePc34)99) == NULL, 1);

    expect_contains("up.dispatch_anchor", A_F0118_UP,
                    up ? up->dispatch_anchor : "", "6666-6676");
    expect_contains("down.dispatch_anchor", A_F0118_DOWN,
                    down ? down->dispatch_anchor : "", "6677-6696");
    expect_contains("pit.dispatch_anchor", A_F0118_PIT,
                    pit ? pit->dispatch_anchor : "", "6748-6763");
    expect_contains("up.draw_anchor", A_F0104, up ? up->draw_anchor : "", "F0104");
    expect_contains("down.draw_anchor", A_F0104, down ? down->draw_anchor : "", "F0104");
    expect_contains("pit.draw_anchor", A_F0104, pit ? pit->draw_anchor : "", "F0104");
}

static void test_route_constants(void)
{
    const DM1_V1_D3CStairsPitEvidencePc34 *up =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34);
    const DM1_V1_D3CStairsPitEvidencePc34 *down =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34);
    const DM1_V1_D3CStairsPitEvidencePc34 *pit =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_for_role_pc34(
            DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34);

    expect_int("up.slot", A_DEFS_D3C_SLOTS, up ? up->native_bitmap_slot_or_graphic : -1, 1);
    expect_int("up.zone", A_DEFS_D3C_ZONES, up ? up->zone_index : -1, 803);
    expect_int("down.slot", A_DEFS_D3C_SLOTS,
               down ? down->native_bitmap_slot_or_graphic : -1, 8);
    expect_int("down.zone", A_DEFS_D3C_ZONES, down ? down->zone_index : -1, 816);
    expect_int("pit.graphic", A_DEFS_PIT, pit ? pit->native_bitmap_slot_or_graphic : -1, 51);
    expect_int("pit.zone", A_DEFS_PIT, pit ? pit->zone_index : -1, 853);
    expect_int("up.view_square", A_DEFS_D3C, up ? up->view_square_index : -1, 11);
    expect_int("down.view_square", A_DEFS_D3C, down ? down->view_square_index : -1, 11);
    expect_int("pit.view_square", A_DEFS_D3C, pit ? pit->view_square_index : -1, 11);
    expect_int("up.transparent", A_DEFS_C10, up ? up->transparent_color : -1, 10);
    expect_int("down.transparent", A_DEFS_C10, down ? down->transparent_color : -1, 10);
    expect_int("pit.transparent", A_DEFS_C10, pit ? pit->transparent_color : -1, 10);
}

static void test_exclusion_flags(void)
{
    size_t i;
    size_t count = 0;
    const DM1_V1_D3CStairsPitEvidencePc34 *all =
        dm1_v1_viewport_d3c_stairs_pit_dispatch_evidence_pc34(&count);

    for (i = 0; i < count; ++i) {
        expect_int("role.contract_only", all[i].dispatch_anchor, all[i].contract_only, 1);
        expect_int("role.no_real_asset", all[i].dispatch_anchor, all[i].real_asset_claim, 0);
        expect_int("role.uses_f0104", all[i].dispatch_anchor, all[i].uses_f0104, 1);
        expect_int("role.no_f0107", all[i].dispatch_anchor, all[i].uses_f0107, 0);
        expect_int("role.no_f0108_metadata", all[i].dispatch_anchor,
                   all[i].uses_f0108_metadata, 0);
        expect_int("role.no_f0111", all[i].dispatch_anchor, all[i].uses_f0111, 0);
        expect_int("role.no_f0115", all[i].dispatch_anchor, all[i].uses_f0115_thing_pass, 0);
        expect_int("role.no_f0128_followup_writes", all[i].dispatch_anchor,
                   all[i].uses_f0128_wall_followup_writes, 0);
        expect_int("role.open_order", A_F0118_OPEN, all[i].cell_order, 0x3421);
    }
}

static void test_probe_routes(void)
{
    const DM1_V1_D3CStairsPitDispatchInputPc34 up = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, true, false, true, false
    };
    const DM1_V1_D3CStairsPitDispatchInputPc34 down = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, false, false, true, false
    };
    const DM1_V1_D3CStairsPitDispatchInputPc34 pit = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_PIT, false, false, true, false
    };
    const DM1_V1_D3CStairsPitDispatchResultPc34 up_out =
        probe(up, "probe.up.ok", A_F0118_UP);
    const DM1_V1_D3CStairsPitDispatchResultPc34 down_out =
        probe(down, "probe.down.ok", A_F0118_DOWN);
    const DM1_V1_D3CStairsPitDispatchResultPc34 pit_out =
        probe(pit, "probe.pit.ok", A_F0118_PIT);

    expect_int("probe.up.role", A_F0118_UP, up_out.role,
               DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34);
    expect_int("probe.up.slot", A_DEFS_D3C_SLOTS, up_out.native_bitmap_slot_or_graphic, 1);
    expect_int("probe.up.native", A_F0096, up_out.native_bitmap_index, 109);
    expect_int("probe.up.zone", A_DEFS_D3C_ZONES, up_out.zone_index, 803);
    expect_int("probe.up.f0104", A_F0118_UP, up_out.used_f0104, 1);
    expect_int("probe.up.no_f0115", A_F0118_UP, up_out.used_f0115_thing_pass, 0);

    expect_int("probe.down.role", A_F0118_DOWN, down_out.role,
               DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34);
    expect_int("probe.down.slot", A_DEFS_D3C_SLOTS, down_out.native_bitmap_slot_or_graphic, 8);
    expect_int("probe.down.native", A_F0096, down_out.native_bitmap_index, 116);
    expect_int("probe.down.zone", A_DEFS_D3C_ZONES, down_out.zone_index, 816);
    expect_int("probe.down.f0104", A_F0118_DOWN, down_out.used_f0104, 1);
    expect_int("probe.down.no_f0115", A_F0118_DOWN, down_out.used_f0115_thing_pass, 0);

    expect_int("probe.pit.role", A_F0118_PIT, pit_out.role,
               DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34);
    expect_int("probe.pit.graphic", A_DEFS_PIT, pit_out.native_bitmap_slot_or_graphic, 51);
    expect_int("probe.pit.native", A_DEFS_PIT, pit_out.native_bitmap_index, 51);
    expect_int("probe.pit.zone", A_DEFS_PIT, pit_out.zone_index, 853);
    expect_int("probe.pit.cell_order", A_F0118_OPEN, pit_out.cell_order, 0x3421);
    expect_int("probe.pit.f0104", A_F0118_PIT, pit_out.used_f0104, 1);
    expect_int("probe.pit.no_f0108_metadata", A_F0118_PIT,
               pit_out.used_f0108_metadata, 0);
    expect_int("probe.pit.no_f0115", A_F0118_PIT, pit_out.used_f0115_thing_pass, 0);
}

static void test_probe_rejections_and_visible_pit(void)
{
    DM1_V1_D3CStairsPitDispatchResultPc34 result;
    const DM1_V1_D3CStairsPitDispatchInputPc34 visible_pit = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_PIT, false, true, true, false
    };
    const DM1_V1_D3CStairsPitDispatchInputPc34 teleporter = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_TELEPORTER, false, true, true, false
    };
    const DM1_V1_D3CStairsPitDispatchInputPc34 non_contract = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, true, false, false, false
    };
    const DM1_V1_D3CStairsPitDispatchInputPc34 real_asset = {
        DM1_V1_D3C_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, true, false, true, true
    };

    expect_int("probe.reject.null_input", A_F0118_UP,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(NULL, &result), 0);
    expect_int("probe.reject.null_output", A_F0118_UP,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(&visible_pit, NULL), 0);
    expect_int("probe.reject.non_contract", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(&non_contract, &result), 0);
    expect_int("probe.reject.real_asset", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_probe_pc34(&real_asset, &result), 0);

    result = probe(visible_pit, "probe.visible_pit.ok", A_F0118_PIT);
    expect_int("probe.visible_pit.unsupported", A_F0118_PIT, result.unsupported_element, 1);
    expect_int("probe.visible_pit.no_f0104", A_F0118_PIT, result.used_f0104, 0);
    expect_int("probe.visible_pit.no_f0115", A_F0118_PIT, result.used_f0115_thing_pass, 0);

    result = probe(teleporter, "probe.teleporter.ok", A_F0118_PIT);
    expect_int("probe.teleporter.unsupported", A_F0118_PIT, result.unsupported_element, 1);
    expect_int("probe.teleporter.no_f0104", A_F0118_PIT, result.used_f0104, 0);
    expect_int("probe.teleporter.no_f0115", A_F0118_PIT, result.used_f0115_thing_pass, 0);
}

static void test_blit_preserves_c10(void)
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
    DM1_V1_D3CStairsPitBlitInputPc34 input;
    DM1_V1_D3CStairsPitBlitResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = blit_input(DM1_V1_D3C_STAIRS_PIT_ROLE_UP_FRONT_PC34,
                       source, sizeof(source), destination, sizeof(destination), 6, 2, 8);
    expect_int("blit.up.ok", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 1);
    expect_bytes("blit.up.bytes", A_F0104, destination, want, sizeof(want));
    expect_size("blit.up.writes", A_F0104, result.writes, 8);
    expect_size("blit.up.transparent_skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("blit.up.transparent_seen", A_DEFS_C10, result.transparent_skip_seen, 1);
    expect_int("blit.up.first_destination", A_F0104, result.first_destination_byte, 1);
    expect_int("blit.up.last_destination", A_F0104, result.last_destination_byte, 12);
    expect_int("blit.up.padding6", A_F0104, destination[6], 0xee);
    expect_int("blit.up.padding15", A_F0104, destination[15], 0xee);

    {
        const uint8_t pit_source[] = { 10, 21, 22, 10, 23, 24 };
        const uint8_t pit_want[] = { 0xdd, 21, 22, 0xdd, 23, 24 };

        memset(destination, 0xdd, sizeof(destination));
        input = blit_input(DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34,
                           pit_source, sizeof(pit_source), destination,
                           sizeof(destination), 6, 1, 6);
        expect_int("blit.pit.ok", A_F0104,
                   dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 1);
        expect_bytes("blit.pit.bytes", A_F0104, destination, pit_want, sizeof(pit_want));
        expect_size("blit.pit.writes", A_F0104, result.writes, 4);
        expect_size("blit.pit.transparent_skips", A_DEFS_C10, result.transparent_skips, 2);
        expect_int("blit.pit.evidence", A_F0118_PIT,
                   result.evidence &&
                   result.evidence->role == DM1_V1_D3C_STAIRS_PIT_ROLE_OPEN_PIT_PC34, 1);
    }
}

static void test_blit_edges(void)
{
    const uint8_t transparent_source[] = { 10, 10, 10, 10 };
    uint8_t destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    const uint8_t original_destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    DM1_V1_D3CStairsPitBlitInputPc34 input;
    DM1_V1_D3CStairsPitBlitResultPc34 result;

    input = blit_input(DM1_V1_D3C_STAIRS_PIT_ROLE_DOWN_FRONT_PC34,
                       transparent_source, sizeof(transparent_source),
                       destination, sizeof(destination), 2, 2, 2);
    expect_int("blit.transparent.ok", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 1);
    expect_bytes("blit.transparent.unchanged", A_DEFS_C10,
                 destination, original_destination, sizeof(destination));
    expect_size("blit.transparent.writes", A_DEFS_C10, result.writes, 0);
    expect_size("blit.transparent.skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("blit.transparent.wrote_any", A_DEFS_C10, result.wrote_any, 0);

    expect_int("blit.reject.null_input", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(NULL, &result), 0);
    expect_int("blit.reject.null_output", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, NULL), 0);
    input.contract_only = false;
    expect_int("blit.reject.non_contract", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 0);
    input.contract_only = true;
    input.real_asset_claim = true;
    expect_int("blit.reject.real_asset", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 0);
    input.real_asset_claim = false;
    input.destination_stride = 1;
    expect_int("blit.reject.short_stride", A_F0104,
               dm1_v1_viewport_d3c_stairs_pit_dispatch_blit_pc34(&input, &result), 0);
}

static void test_source_evidence(void)
{
    const char *summary = dm1_v1_viewport_d3c_stairs_pit_dispatch_source_evidence_pc34();

    expect_contains("summary.contract", A_F0118_UP, summary, "contract_only=1");
    expect_contains("summary.no_real_asset", A_F0104, summary, "no real-asset");
    expect_contains("summary.up", A_F0118_UP, summary, "6666-6676");
    expect_contains("summary.down", A_F0118_DOWN, summary, "6677-6696");
    expect_contains("summary.pit", A_F0118_PIT, summary, "6748-6763");
    expect_contains("summary.open_order", A_F0118_OPEN, summary, "C0x3421");
    expect_contains("summary.f0104", A_F0104, summary, "F0104");
    expect_contains("summary.f0096", A_F0096, summary, "2517-2518");
    expect_contains("summary.c10", A_DEFS_C10, summary, "C10");
    expect_contains("summary.no_f0107", A_F0118_UP, summary, "F0107");
    expect_contains("summary.no_f0108_metadata", A_F0118_PIT, summary, "F0108 metadata");
    expect_contains("summary.no_f0111", A_F0118_DOWN, summary, "F0111");
    expect_contains("summary.no_f0115", A_F0118_PIT, summary, "F0115 thing pass");
    expect_contains("summary.no_f0128_followup", A_F0118_OPEN, summary,
                    "F0128 post-D3C wall-followup writes");
}

int main(void)
{
    test_evidence_table();
    test_route_constants();
    test_exclusion_flags();
    test_probe_routes();
    test_probe_rejections_and_visible_pit();
    test_blit_preserves_c10();
    test_blit_edges();
    test_source_evidence();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
