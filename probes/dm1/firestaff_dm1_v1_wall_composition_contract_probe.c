#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "dm1_v1_viewport_3d_pc34_compat.h"

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_bool(const char *label, bool got, bool want)
{
    return expect_int(label, got ? 1 : 0, want ? 1 : 0);
}

static int expect_contains(const char *label, const char *text, const char *needle)
{
    if (!text || !strstr(text, needle)) {
        fprintf(stderr, "FAIL %s missing needle=%s text=%s\n", label, needle, text ? text : "(null)");
        return 0;
    }
    return 1;
}

struct ExpectedWallContract {
    DM1_ViewSquareIndex square;
    DM1_WallSetIndex native_wall;
    DM1_WallSetIndex parity_wall;
    uint16_t zone;
    bool flips;
    bool center_wall;
    bool returns;
    bool front_alcove_reveals;
    const char *source_anchor;
    const char *occlusion_anchor;
};

static int verify_wall_specs(void)
{
    static const struct ExpectedWallContract expected[] = {
        { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, DM1_PC34_ZONE_WALL_D3L2, true,  false, true,  false, "DUNVIEW.C:6254-6260", "DUNVIEW.C:6263-6264" },
        { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, DM1_PC34_ZONE_WALL_D3R2, true,  false, true,  false, "DUNVIEW.C:6321-6327", "DUNVIEW.C:6330-6331" },
        { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  DM1_PC34_ZONE_WALL_D3L,  true,  false, true,  true,  "DUNVIEW.C:6421-6427", "DUNVIEW.C:6432-6437" },
        { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  DM1_PC34_ZONE_WALL_D3R,  true,  false, true,  true,  "DUNVIEW.C:6554-6564", "DUNVIEW.C:6568-6573" },
        { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  DM1_PC34_ZONE_WALL_D3C,  true,  true,  true,  true,  "DUNVIEW.C:6707-6714", "DUNVIEW.C:6716-6720" },
        { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, DM1_PC34_ZONE_WALL_D2L2, true,  false, true,  false, "DUNVIEW.C:6849-6858", "DUNVIEW.C:6848-6862" },
        { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, DM1_PC34_ZONE_WALL_D2R2, true,  false, true,  false, "DUNVIEW.C:6880-6889", "DUNVIEW.C:6882-6893" },
        { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  DM1_PC34_ZONE_WALL_D2L,  true,  false, true,  true,  "DUNVIEW.C:6954-6964", "DUNVIEW.C:6968-6973" },
        { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  DM1_PC34_ZONE_WALL_D2R,  true,  false, true,  true,  "DUNVIEW.C:7105-7115", "DUNVIEW.C:7119-7123" },
        { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  DM1_PC34_ZONE_WALL_D2C,  true,  true,  true,  true,  "DUNVIEW.C:7299-7306", "DUNVIEW.C:7308-7312" },
        { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  DM1_PC34_ZONE_WALL_D1L,  true,  false, true,  false, "DUNVIEW.C:7445-7455", "DUNVIEW.C:7459-7460" },
        { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  DM1_PC34_ZONE_WALL_D1R,  true,  false, true,  false, "DUNVIEW.C:7613-7623", "DUNVIEW.C:7627-7628" },
        { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  DM1_PC34_ZONE_WALL_D1C,  true,  true,  false, true,  "DUNVIEW.C:7833-7840", "DUNVIEW.C:7842-7843" },
        { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  DM1_PC34_ZONE_WALL_D0L,  true,  false, true,  false, "DUNVIEW.C:8016-8033", "DUNVIEW.C:8036-8038" },
        { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  DM1_PC34_ZONE_WALL_D0R,  true,  false, true,  false, "DUNVIEW.C:8126-8139", "DUNVIEW.C:8142-8144" },
    };
    int ok = 1;

    ok &= expect_int("wall spec count", (int)dm1_viewport_3d_wall_draw_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));

    printf("wallCompositionMatrix source=DUNVIEW.C:6226-6331,6837-6893,6406-6437,6545-6573,7244-7312,7727-7843,7960-8162\n");
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const struct ExpectedWallContract *want = &expected[i];
        const DM1_ViewportWallDrawSpec *spec = dm1_viewport_3d_get_wall_draw_spec_for_square(want->square);
        bool flip = false;
        DM1_WallSetIndex native_sel;
        DM1_WallSetIndex parity_sel;

        ok &= expect_bool("spec exists", spec != NULL, true);
        if (!spec) continue;

        native_sel = dm1_viewport_3d_select_wall_bitmap(spec, false, &flip);
        ok &= expect_int("native wall select", native_sel, want->native_wall);
        ok &= expect_bool("native select no flip", flip, false);
        parity_sel = dm1_viewport_3d_select_wall_bitmap(spec, true, &flip);
        ok &= expect_int("parity wall select", parity_sel, want->parity_wall);
        ok &= expect_bool("parity flip flag", flip, want->flips);

        ok &= expect_int("pc34 zone", spec->pc34_zone, want->zone);
        ok &= expect_bool("center wall flag", spec->center_wall, want->center_wall);
        ok &= expect_bool("wall return/stop flag", spec->wall_case_returns, want->returns);
        ok &= expect_bool("front alcove reveal flag", spec->front_alcove_reveals_contents, want->front_alcove_reveals);
        ok &= expect_contains("source anchor", spec->source_lines, want->source_anchor);
        ok &= expect_contains("occlusion anchor", spec->occlusion_source_lines, want->occlusion_anchor);

        printf("wallSpec square=%d native=%d parity=%d flip=%d zone=%u center=%d returns=%d alcoveReveal=%d source=%s occlusion=%s\n",
            (int)want->square, (int)native_sel, (int)parity_sel, flip ? 1 : 0,
            spec->pc34_zone, spec->center_wall ? 1 : 0, spec->wall_case_returns ? 1 : 0,
            spec->front_alcove_reveals_contents ? 1 : 0, spec->source_lines, spec->occlusion_source_lines);
    }

    return ok;
}

struct ExpectedDoorFrontContract {
    DM1_ViewSquareIndex square;
    uint16_t rear_order;
    uint16_t front_order;
    unsigned int rear_pass;
    unsigned int front_pass;
    unsigned int rear_count;
    unsigned int front_count;
};

static int verify_door_front_orders(void)
{
    static const struct ExpectedDoorFrontContract expected[] = {
        { DM1_VIEW_SQUARE_D3L2, 0x0218, 0x0349, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D3R2, 0x0128, 0x0439, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D3L,  0x0218, 0x0349, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D3R,  0x0128, 0x0439, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D3C,  0x0218, 0x0349, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D2L,  0x0218, 0x0349, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D2R,  0x0128, 0x0439, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D2C,  0x0218, 0x0349, 1, 2, 2, 2 },
        { DM1_VIEW_SQUARE_D1L,  0x0028, 0x0039, 1, 2, 1, 1 },
        { DM1_VIEW_SQUARE_D1R,  0x0018, 0x0049, 1, 2, 1, 1 },
        { DM1_VIEW_SQUARE_D1C,  0x0218, 0x0349, 1, 2, 2, 2 },
    };
    int ok = 1;

    ok &= expect_int("door front occlusion spec count", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), (int)(sizeof(expected) / sizeof(expected[0])));
    printf("doorFrontComposition source=DUNVIEW.C:6270-6286,6337-6353,6443-6459,6579-6601,6722-6746,6988-7003,7181-7196,7314-7341,7493-7536,7661-7704,7874-7937\n");
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const struct ExpectedDoorFrontContract *want = &expected[i];
        const DM1_ViewportDoorFrontOcclusionSpec *spec = dm1_viewport_3d_get_door_front_occlusion_spec_for_square(want->square);
        DM1_ViewportCellOrder rear;
        DM1_ViewportCellOrder front;

        ok &= expect_bool("door spec exists", spec != NULL, true);
        if (!spec) continue;

        rear = dm1_viewport_3d_decode_cell_order(spec->rear_cell_order);
        front = dm1_viewport_3d_decode_cell_order(spec->front_cell_order);
        ok &= expect_int("door rear order", spec->rear_cell_order, want->rear_order);
        ok &= expect_int("door front order", spec->front_cell_order, want->front_order);
        ok &= expect_int("door rear pass marker", rear.door_pass, want->rear_pass);
        ok &= expect_int("door front pass marker", front.door_pass, want->front_pass);
        ok &= expect_int("door rear cell count", rear.cell_count, want->rear_count);
        ok &= expect_int("door front cell count", front.cell_count, want->front_count);

        printf("doorSpec square=%d rearOrder=0x%04x frontOrder=0x%04x rearPass=%u frontPass=%u source=%s;%s;%s;%s\n",
            (int)want->square, spec->rear_cell_order, spec->front_cell_order,
            rear.door_pass, front.door_pass, spec->rear_pass_source_lines,
            spec->frame_source_lines, spec->door_source_lines, spec->front_pass_source_lines);
    }

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=firestaff_dm1_v1_wall_composition_contract_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:2962-3003,3048-3078,6226-6331,6837-6893,6406-6437,6545-6573,7244-7312,7727-7843,7960-8162,8318-8542\n");

    ok &= verify_wall_specs();
    ok &= verify_door_front_orders();
    ok &= expect_int("null wall select sentinel",
        dm1_viewport_3d_select_wall_bitmap(NULL, true, NULL), DM1_WALL_SET_COUNT);

    if (!ok) {
        fprintf(stderr, "probe failed\n");
        return 1;
    }
    printf("result=pass\n");
    return 0;
}
