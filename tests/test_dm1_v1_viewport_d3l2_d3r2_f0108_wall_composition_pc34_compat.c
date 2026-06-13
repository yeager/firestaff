#include "firestaff/dm1/v1/viewport/d3l2_d3r2_f0108_wall_composition_pc34_compat.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define EXPECT_TRUE(ID, EXPR, ANCHOR)                                        \
    do {                                                                     \
        ++g_assertions;                                                      \
        if (!(EXPR)) {                                                       \
            printf("FAIL %s anchor=%s\n", (ID), (ANCHOR));                  \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

#define EXPECT_EQ(ID, GOT, WANT, ANCHOR)                                     \
    do {                                                                     \
        int got__ = (int)(GOT);                                              \
        int want__ = (int)(WANT);                                            \
        ++g_assertions;                                                      \
        if (got__ != want__) {                                               \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                    \
                   (ID), got__, want__, (ANCHOR));                           \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

#define EXPECT_U8(ID, GOT, WANT, ANCHOR)                                     \
    do {                                                                     \
        unsigned got__ = (unsigned)(GOT);                                    \
        unsigned want__ = (unsigned)(WANT);                                  \
        ++g_assertions;                                                      \
        if (got__ != want__) {                                               \
            printf("FAIL %s got=0x%02x want=0x%02x anchor=%s\n",            \
                   (ID), got__, want__, (ANCHOR));                           \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

#define EXPECT_U64(ID, GOT, WANT, ANCHOR)                                    \
    do {                                                                     \
        uint64_t got__ = (uint64_t)(GOT);                                    \
        uint64_t want__ = (uint64_t)(WANT);                                  \
        ++g_assertions;                                                      \
        if (got__ != want__) {                                               \
            printf("FAIL %s got=0x%016" PRIx64 " want=0x%016" PRIx64        \
                   " anchor=%s\n", (ID), got__, want__, (ANCHOR));          \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != NULL;
}

static void test_model_header(void)
{
    DM1V1D3L2D3R2F0108WallCompositionModelPc34 a;
    DM1V1D3L2D3R2F0108WallCompositionModelPc34 b;
    const DM1V1D3L2D3R2F0108WallCompositionModelPc34 *model =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_pc34();

    EXPECT_EQ("builder.null",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(NULL),
              0, "builder guard");
    EXPECT_EQ("builder.a",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(&a),
              1, "builder deterministic");
    EXPECT_EQ("builder.b",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_default_model_builder_pc34(&b),
              1, "builder deterministic");
    EXPECT_TRUE("model.present", model != NULL, "model accessor");
    EXPECT_EQ("byte.stable", memcmp(&a, &b, sizeof(a)), 0,
              "contract fields are deterministic");
    EXPECT_U64("hash.null",
               dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_hash_model_pc34(NULL),
               0u, "hash null guard");
    EXPECT_U64("hash.builder", a.deterministic_hash, b.deterministic_hash,
               "builder hashes match");
    EXPECT_U64("hash.model", model ? model->deterministic_hash : 0u,
               a.deterministic_hash, "accessor hash");
    EXPECT_U64("hash.recomputed", model ? model->deterministic_hash : 0u,
               model ? dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_hash_model_pc34(model) : 0u,
               "recomputed hash");
    EXPECT_U64("hash.accessor", model ? model->deterministic_hash : 0u,
               dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_deterministic_hash_pc34(),
               "hash accessor");
    EXPECT_U64("hash.expected", model ? model->deterministic_hash : 0u,
               UINT64_C(0xf278a0245aafca2c),
               "fixed deterministic 64-bit source-lock hash");
    EXPECT_TRUE("hash.nonzero", model && model->deterministic_hash != 0u,
                "64-bit deterministic hash printed by this test");

    EXPECT_EQ("framebuffer.width", model ? model->framebuffer_width : 0, 320,
              "320x200 synthetic frame");
    EXPECT_EQ("framebuffer.height", model ? model->framebuffer_height : 0, 200,
              "320x200 synthetic frame");
    EXPECT_EQ("viewport.width", model ? model->viewport_width : 0, 224,
              "224x136 viewport contract");
    EXPECT_EQ("viewport.height", model ? model->viewport_height : 0, 136,
              "224x136 viewport contract");
    EXPECT_EQ("c10", model ? model->c10_transparent_color : 0, 10,
              "DEFS.H:2088 C10_COLOR_FLESH");
    EXPECT_EQ("row.depth", model ? model->d3_row_depth : 0, 3,
              "DUNVIEW.C F0128:8478-8499 D3 row");
    EXPECT_EQ("row.d3l2", model ? model->d3l2_f0128_order : -1, 0,
              "DUNVIEW.C F0128:8481-8482");
    EXPECT_EQ("row.d3r2", model ? model->d3r2_f0128_order : -1, 1,
              "DUNVIEW.C F0128:8485-8486");
    EXPECT_EQ("row.d3l", model ? model->d3l_f0128_order : -1, 2,
              "DUNVIEW.C F0128:8490-8491");
    EXPECT_EQ("row.d3r", model ? model->d3r_f0128_order : -1, 3,
              "DUNVIEW.C F0128:8494-8495");
    EXPECT_EQ("row.d3c", model ? model->d3c_f0128_order : -1, 4,
              "DUNVIEW.C F0128:8498-8499");
    EXPECT_EQ("row.d2l2", model ? model->d2l2_f0128_order : -1, 5,
              "DUNVIEW.C F0128:8502-8504");
    EXPECT_EQ("row.d2r2", model ? model->d2r2_f0128_order : -1, 6,
              "DUNVIEW.C F0128:8506-8508");
    EXPECT_EQ("pass777.pinned", model ? model->d3c_f0107_pass777_commit_pinned : 0, 1,
              "pass777 commit 83db35b76 is a keepout");
    EXPECT_EQ("cc6b81b59.pinned",
              model ? model->d0l2_d0r2_f0107_cc6b81b59_commit_pinned : 0, 1,
              "D0L2/D0R2 F0107 commit cc6b81b59 is a keepout");
    EXPECT_EQ("door.pass.first", model ? model->door_front_view_drawing_pass_first : 0, 1,
              "DUNVIEW.C F0115:4794-4796 L0175 pass one");
    EXPECT_EQ("door.pass.second", model ? model->door_front_view_drawing_pass_second : 0, 2,
              "DUNVIEW.C F0115:4794-4796 L0175 pass two");
    EXPECT_EQ("contract.only", model ? model->source_locked_contract_only : 0, 1,
              "contract-only gate");
    EXPECT_EQ("contract.no_assets", model ? model->no_real_asset_bitmap_parity : 0, 1,
              "no real-asset bitmap parity");
    EXPECT_EQ("contract.no_data", model ? model->no_game_data_load : 0, 1,
              "no GRAPHICS.DAT load");
}

static void test_source_evidence(void)
{
    const char *source =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_source_evidence_pc34();
    const char *note =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_disjointness_note_pc34();

    EXPECT_TRUE("evidence.f0676", contains(source, "DUNVIEW.C F0676:6226-6291"),
                "mandatory F0676 source anchor");
    EXPECT_TRUE("evidence.f0677", contains(source, "DUNVIEW.C F0677:6293-6358"),
                "mandatory F0677 source anchor");
    EXPECT_TRUE("evidence.f0128.d3l2", contains(source, "F0128:8478-8508"),
                "mandatory F0128 D3L2/D3R2 source anchor");
    EXPECT_TRUE("evidence.f0116", contains(source, "DUNVIEW.C F0116:6361-6480"),
                "mandatory D3L side-wall contrast");
    EXPECT_TRUE("evidence.f0117", contains(source, "DUNVIEW.C F0117:6500-6622"),
                "mandatory D3R side-wall contrast");
    EXPECT_TRUE("evidence.f0104", contains(source, "DUNVIEW.C F0104:3113-3156"),
                "mandatory native bitmap helper anchor");
    EXPECT_TRUE("evidence.f0105", contains(source, "DUNVIEW.C F0105:3185-3247"),
                "mandatory flipped bitmap helper anchor");
    EXPECT_TRUE("evidence.f0107", contains(source, "DUNVIEW.C F0107:3502-3938"),
                "mandatory wall-ornament keepout anchor");
    EXPECT_TRUE("evidence.f0108", contains(source, "DUNVIEW.C F0108:3940-4011"),
                "mandatory F0108 source anchor");
    EXPECT_TRUE("evidence.footprints", contains(source, "MASK0x8000_FOOTPRINTS"),
                "mandatory footprint recursion anchor");
    EXPECT_TRUE("evidence.f0111", contains(source, "DUNVIEW.C F0111:4218-4339"),
                "mandatory F0111 source anchor");
    EXPECT_TRUE("evidence.f0115.pass", contains(source, "DUNVIEW.C F0115:4794-4800"),
                "mandatory L0175 pass source anchor");
    EXPECT_TRUE("evidence.f0115.c10", contains(source, "F0115:5180-5188"),
                "mandatory F0115 C10 source anchor");
    EXPECT_TRUE("evidence.f0163", contains(source, "DUNGEON.C F0163:1769-1838"),
                "mandatory thing-list source anchor");
    EXPECT_TRUE("evidence.f0164", contains(source, "F0164:1840-1905"),
                "mandatory thing-list source anchor");
    EXPECT_TRUE("evidence.f0172", contains(source, "F0172:2466-2523"),
                "mandatory square-aspect source anchor");
    EXPECT_TRUE("evidence.defs.c10", contains(source, "DEFS.H:2088"),
                "mandatory C10 source anchor");
    EXPECT_TRUE("evidence.defs.stairs.up", contains(source, "DEFS.H:2443"),
                "mandatory stair-up ordinal contrast");
    EXPECT_TRUE("evidence.defs.stairs.down", contains(source, "DEFS.H:2450"),
                "mandatory stair-down ordinal contrast");
    EXPECT_TRUE("evidence.defs.d2.view", contains(source, "DEFS.H:2582-2583"),
                "mandatory D2 view-square keepout");
    EXPECT_TRUE("evidence.defs.d2.extended", contains(source, "DEFS.H:2603-2604"),
                "mandatory D2L2/D2R2 keepout");
    EXPECT_TRUE("evidence.defs.d3.extended", contains(source, "DEFS.H:2610-2611"),
                "mandatory D3L2/D3R2 view squares");
    EXPECT_TRUE("evidence.defs.cell.back", contains(source, "DEFS.H:2662"),
                "mandatory back-cell order");
    EXPECT_TRUE("evidence.defs.cell.open", contains(source, "DEFS.H:2676-2677"),
                "mandatory open-row cell order");
    EXPECT_TRUE("evidence.defs.stairs.zones", contains(source, "DEFS.H:4139-4153"),
                "mandatory D3L2/D3R2 stair zones");
    EXPECT_TRUE("evidence.defs.pit.zones", contains(source, "DEFS.H:4197-4198"),
                "mandatory D3L2/D3R2 pit zones");
    EXPECT_TRUE("note.pass777", contains(note, "83db35b76"),
                "D3C F0107 disjointness commit");
    EXPECT_TRUE("note.cc6b81b59", contains(note, "cc6b81b59"),
                "D0L2/D0R2 F0107 disjointness commit");
    EXPECT_TRUE("note.c704", contains(note, "C704"),
                "D3C wall-zone keepout");
    EXPECT_TRUE("note.c716", contains(note, "C716/C717"),
                "D0 side-pair wall-zone keepout");
}

static void test_specs(void)
{
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_spec_at_pc34(0);
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_spec_at_pc34(1);
    const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *specs[2] = { d3l2, d3r2 };
    int i;

    EXPECT_TRUE("spec.oob",
                dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_spec_at_pc34(2) == NULL,
                "spec accessor bounds");
    for (i = 0; i < 2; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionSpecPc34 *s = specs[i];
        const int is_right = i == 1;
        EXPECT_TRUE("spec.present", s != NULL, "DUNVIEW.C F0676/F0677");
        if (!s) continue;
        EXPECT_EQ("spec.side", s->side, i, "side identity");
        EXPECT_TRUE("spec.name", contains(s->side_name, is_right ? "D3R2" : "D3L2"),
                    "side label");
        EXPECT_TRUE("spec.function", contains(s->function_name, is_right ? "F0677" : "F0676"),
                    "function label");
        EXPECT_EQ("spec.depth", s->row_depth, 3, "D3 row depth");
        EXPECT_EQ("spec.lateral", s->relative_lateral, is_right ? 2 : -2,
                  "second side-pair lateral lane");
        EXPECT_EQ("spec.order", s->f0128_row_order, i,
                  "DUNVIEW.C F0128:8478-8486 row order");
        EXPECT_EQ("spec.update_line", s->f0128_update_line, is_right ? 8485 : 8481,
                  "F0150 update before F067x");
        EXPECT_EQ("spec.draw_line", s->f0128_draw_line, is_right ? 8486 : 8482,
                  "F067x draw call");
        EXPECT_EQ("spec.start", s->function_start_line, is_right ? 6293 : 6226,
                  "F067x function start");
        EXPECT_EQ("spec.end", s->function_end_line, is_right ? 6358 : 6291,
                  "F067x function end");
        EXPECT_EQ("spec.view_square", s->view_square, is_right ? 15 : 14,
                  "DEFS.H:2610-2611 C14/C15");
        EXPECT_EQ("spec.view_floor", s->view_floor, is_right ? 1 : 0,
                  "DEFS.H:2750-2751 C00/C01");
        EXPECT_EQ("spec.wall_zone", s->wall_zone, is_right ? 703 : 702,
                  "DEFS.H:4042-4043 C702/C703");
        EXPECT_EQ("spec.wall_view", s->wall_view, is_right ? 1 : 0,
                  "DEFS.H:2696-2697 C00/C01");
        EXPECT_EQ("spec.wall_slot", s->wall_aspect_slot, is_right ? 6 : 4,
                  "DEFS.H:2551/2553 side wall ordinal slots");
        EXPECT_EQ("spec.floor_slot", s->floor_aspect_slot, 5,
                  "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
        EXPECT_EQ("spec.door_state", s->door_state_slot, 3,
                  "DEFS.H:2556 M556_DOOR_STATE");
        EXPECT_EQ("spec.door_thing", s->door_thing_slot, 4,
                  "DEFS.H:2557 M557_DOOR_THING_INDEX");
        EXPECT_EQ("spec.door_zone", s->door_zone, is_right ? 3710 : 3700,
                  "DEFS.H:4249-4251 C3700/C3710");
        EXPECT_EQ("spec.open_order", (int)s->open_cell_order, is_right ? 0x4312 : 0x3421,
                  "DUNVIEW.C F0676:6282/F0677:6349");
        EXPECT_EQ("spec.side_order", (int)s->side_cell_order, is_right ? 0x0412 : 0x0321,
                  "DUNVIEW.C F0676:6267/F0677:6334");
        EXPECT_EQ("spec.pass1", (int)s->door_pass1_cell_order, is_right ? 0x0128 : 0x0218,
                  "DUNVIEW.C F0676:6271/F0677:6338");
        EXPECT_EQ("spec.pass2", (int)s->door_pass2_cell_order, is_right ? 0x0439 : 0x0349,
                  "DUNVIEW.C F0676:6273/F0677:6340");
        EXPECT_EQ("spec.f0108.open", s->f0108_open_line, is_right ? 6351 : 6284,
                  "open row F0108 line");
        EXPECT_EQ("spec.f0108.door", s->f0108_door_front_line, is_right ? 6337 : 6270,
                  "door-front F0108 line");
        EXPECT_EQ("spec.f0115.pass1", s->f0115_pass1_line, is_right ? 6338 : 6271,
                  "door-front pass one line");
        EXPECT_EQ("spec.f0111", s->f0111_line, is_right ? 6339 : 6272,
                  "door-front F0111 line");
        EXPECT_EQ("spec.f0115.pass2", s->f0115_pass2_line, is_right ? 6353 : 6286,
                  "door-front pass two tail line");
        EXPECT_EQ("spec.flip", s->right_side_floor_flips ? 1 : 0, is_right,
                  "DUNVIEW.C F0108:3980 C01_VIEW_FLOOR_D3R2 flip");
        EXPECT_EQ("spec.wall_return", s->wall_case_returns_before_f0108 ? 1 : 0, 1,
                  "wall case returns before F0108");
        EXPECT_EQ("spec.two_pass", s->door_front_uses_two_pass_order ? 1 : 0, 1,
                  "L0175 two-pass order");
        EXPECT_EQ("spec.f0111_contract", s->f0111_transparency_contract_only ? 1 : 0, 1,
                  "F0111 transparency only, no geometry");
        EXPECT_EQ("spec.contract", s->source_locked_contract_only ? 1 : 0, 1,
                  "contract-only source lock");
        EXPECT_EQ("spec.no_assets", s->no_real_asset_bitmap_parity ? 1 : 0, 1,
                  "no real-asset pixel parity");
        EXPECT_EQ("spec.no_data", s->no_game_data_load ? 1 : 0, 1,
                  "no game-data load");
        EXPECT_TRUE("spec.anchor.f067", contains(s->redmcsb_f067x_anchor, is_right ? "F0677" : "F0676"),
                    "F067x anchor");
        EXPECT_TRUE("spec.anchor.f0108", contains(s->redmcsb_f0108_anchor, "F0108"),
                    "F0108 anchor");
        EXPECT_TRUE("spec.anchor.f0111", contains(s->redmcsb_f0111_anchor, "F0111"),
                    "F0111 anchor");
        EXPECT_TRUE("spec.anchor.f0115", contains(s->redmcsb_f0115_anchor, "F0115"),
                    "F0115 anchor");
        EXPECT_TRUE("spec.anchor.f0128", contains(s->redmcsb_f0128_anchor, "F0128"),
                    "F0128 anchor");
        EXPECT_TRUE("spec.anchor.defs", contains(s->redmcsb_defs_anchor, "DEFS.H"),
                    "DEFS anchor");
    }
}

static void test_routes(void)
{
    int i;
    int f0108_count = 0;
    int f0111_count = 0;
    int wall_return_count = 0;
    int d3c_keepout_count = 0;
    int d0_keepout_count = 0;
    int field_tail_count = 0;

    EXPECT_TRUE("route.oob",
                dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_route_at_pc34(
                    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34) == NULL,
                "route accessor bounds");
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ROUTE_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionRoutePc34 *r =
            dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_route_at_pc34((size_t)i);
        EXPECT_TRUE("route.present", r != NULL, "route accessor");
        if (!r) continue;
        EXPECT_EQ("route.index", r->route_index, i, "route order");
        EXPECT_TRUE("route.side", r->side == 0 || r->side == 1, "route side");
        EXPECT_TRUE("route.name", r->element_name != NULL, "route element name");
        EXPECT_EQ("route.supported", r->supported_by_f067x ? 1 : 0, 1,
                  "F0676/F0677 supported route");
        EXPECT_TRUE("route.lines", r->route_start_line > 0 && r->route_end_line >= r->route_start_line,
                    "source line span");
        EXPECT_EQ("route.d3c.keepout", r->d3c_f0107_keepout ? 1 : 0, 1,
                  "pass777 D3C F0107 keepout");
        EXPECT_EQ("route.d0.keepout", r->d0l2_d0r2_f0107_keepout ? 1 : 0, 1,
                  "cc6b81b59 D0L2/D0R2 F0107 keepout");
        EXPECT_TRUE("route.anchor", contains(r->redmcsb_anchor, "DUNVIEW.C"),
                    "route source anchor");
        if (r->calls_f0108) ++f0108_count;
        if (r->calls_f0111) ++f0111_count;
        if (r->returns_after_wall) ++wall_return_count;
        if (r->d3c_f0107_keepout) ++d3c_keepout_count;
        if (r->d0l2_d0r2_f0107_keepout) ++d0_keepout_count;
        if (r->field_tail_after_teleporter) ++field_tail_count;

        if (r->element == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_WALL_PC34) {
            EXPECT_EQ("route.wall.f0107", r->calls_wall_f0107 ? 1 : 0, 1,
                      "wall branch belongs to F0107 keepout only");
            EXPECT_EQ("route.wall.no_f0108", r->calls_f0108 ? 1 : 0, 0,
                      "wall branch returns before F0108");
            EXPECT_EQ("route.wall.returns", r->returns_after_wall ? 1 : 0, 1,
                      "wall branch return");
        } else {
            EXPECT_EQ("route.nonwall.no_f0107", r->calls_wall_f0107 ? 1 : 0, 0,
                      "non-wall composition does not duplicate F0107");
            EXPECT_EQ("route.nonwall.f0108", r->calls_f0108 ? 1 : 0, 1,
                      "non-wall route owns F0108 composition");
        }
        if (r->element == DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_ELEMENT_DOOR_FRONT_PC34) {
            EXPECT_EQ("route.door.pass1", r->calls_f0115_pass1 ? 1 : 0, 1,
                      "door-front pass one");
            EXPECT_EQ("route.door.f0111", r->calls_f0111 ? 1 : 0, 1,
                      "door-front F0111 transparency layer");
            EXPECT_EQ("route.door.pass2", r->calls_f0115_pass2 ? 1 : 0, 1,
                      "door-front pass two");
        }
    }
    EXPECT_EQ("route.f0108_count", f0108_count, 14, "all non-wall D3L2/D3R2 routes");
    EXPECT_EQ("route.f0111_count", f0111_count, 2, "door-front route per side");
    EXPECT_EQ("route.wall_return_count", wall_return_count, 2, "wall route per side");
    EXPECT_EQ("route.d3c_keepout_count", d3c_keepout_count, 16, "all routes reject D3C duplication");
    EXPECT_EQ("route.d0_keepout_count", d0_keepout_count, 16, "all routes reject D0 duplication");
    EXPECT_EQ("route.field_tail_count", field_tail_count, 2, "teleporter field per side");
}

static void test_ordinals_pixels_and_rejections(void)
{
    DM1V1D3L2D3R2F0108WallCompositionOrdinalPc34 ordinal;
    int i;
    int transparent_f0108 = 0;
    int transparent_f0111 = 0;
    int door_cases = 0;
    int rejected_commits = 0;

    EXPECT_EQ("decode.null",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(1, NULL),
              0, "ordinal null guard");
    EXPECT_EQ("decode.zero",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(0, &ordinal),
              1, "DUNVIEW.C F0108:3959 zero skip");
    EXPECT_EQ("decode.zero.has", ordinal.has_input_ordinal ? 1 : 0, 0,
              "zero ordinal no draw");
    EXPECT_EQ("decode.zero.primary", ordinal.primary_draws ? 1 : 0, 0,
              "zero ordinal no primary");
    EXPECT_EQ("decode.zero.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 0,
              "zero ordinal no recursion");
    EXPECT_EQ("decode.primary",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(4, &ordinal),
              1, "DUNVIEW.C F0108:3965 ordinal decrement");
    EXPECT_EQ("decode.primary.has", ordinal.has_input_ordinal ? 1 : 0, 1,
              "primary has ordinal");
    EXPECT_EQ("decode.primary.flag", ordinal.footprint_flag_set ? 1 : 0, 0,
              "ordinary ordinal");
    EXPECT_EQ("decode.primary.draws", ordinal.primary_draws ? 1 : 0, 1,
              "primary draws");
    EXPECT_EQ("decode.primary.ordinal", (int)ordinal.primary_ordinal, 4,
              "primary ordinal");
    EXPECT_EQ("decode.primary.index", ordinal.primary_index, 3,
              "ordinal to index");
    EXPECT_EQ("decode.footprint",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(0x8004u, &ordinal),
              1, "DUNVIEW.C F0108:3960-3962 footprint clear");
    EXPECT_EQ("decode.footprint.flag", ordinal.footprint_flag_set ? 1 : 0, 1,
              "footprint flag");
    EXPECT_EQ("decode.footprint.cleared", (int)ordinal.cleared_ordinal, 4,
              "mask cleared");
    EXPECT_EQ("decode.footprint.primary", ordinal.primary_draws ? 1 : 0, 1,
              "footprint primary draw");
    EXPECT_EQ("decode.footprint.index", ordinal.primary_index, 3,
              "footprint primary index");
    EXPECT_EQ("decode.footprint.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 1,
              "footprint recursion");
    EXPECT_EQ("decode.footprint.recursive_ordinal", (int)ordinal.recursive_footprints_ordinal, 16,
              "C15 footprints ordinal");
    EXPECT_EQ("decode.footprint.recursive_index", ordinal.recursive_footprints_index, 15,
              "C15 footprints index");
    EXPECT_EQ("decode.only_footprint",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_decode_ordinal_pc34(0x8000u, &ordinal),
              1, "footprint-only ordinal");
    EXPECT_EQ("decode.only_footprint.primary", ordinal.primary_draws ? 1 : 0, 0,
              "footprint-only primary skip");
    EXPECT_EQ("decode.only_footprint.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 1,
              "footprint-only recursion");

    EXPECT_U8("blend.transparent",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(0x77u, 10u, 10u),
              0x77u, "C10 preserves destination");
    EXPECT_U8("blend.opaque",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(0x77u, 0x88u, 10u),
              0x88u, "opaque source writes destination");
    EXPECT_EQ("pixel.null",
              dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_case_pc34(NULL),
              0, "pixel null guard");

    EXPECT_TRUE("pixel.oob",
                dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_at_pc34(
                    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34) == NULL,
                "pixel accessor bounds");
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_PIXEL_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionPixelPc34 *pixel =
            dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pixel_at_pc34((size_t)i);
        uint8_t expected;
        EXPECT_TRUE("pixel.present", pixel != NULL, "pixel accessor");
        if (!pixel) continue;
        EXPECT_EQ("pixel.index", pixel->pixel_index, i, "pixel order");
        EXPECT_TRUE("pixel.side", pixel->side == 0 || pixel->side == 1,
                    "pixel side");
        EXPECT_TRUE("pixel.anchor", contains(pixel->redmcsb_anchor, "DUNVIEW.C"),
                    "pixel source anchor");

        expected = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
            pixel->before, pixel->f0108_source, 10u);
        EXPECT_U8("pixel.after_f0108", pixel->after_f0108, expected,
                  "F0108 C10 flow");
        expected = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
            expected, pixel->pass1_source, 10u);
        EXPECT_U8("pixel.after_pass1", pixel->after_pass1, expected,
                  "F0115 pass1 C10 flow");
        if (pixel->door_front_sequence) {
            ++door_cases;
            expected = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
                expected, pixel->f0111_source, 10u);
            EXPECT_U8("pixel.after_f0111", pixel->after_f0111, expected,
                      "F0111 C10 flow");
            expected = dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_blend_pixel_pc34(
                expected, pixel->pass2_source, 10u);
            EXPECT_U8("pixel.after_pass2", pixel->after_pass2, expected,
                      "F0115 pass2 C10 flow");
        } else {
            EXPECT_U8("pixel.after_f0111.noop", pixel->after_f0111, pixel->after_pass1,
                      "no F0111 outside door-front");
            EXPECT_U8("pixel.after_pass2.noop", pixel->after_pass2, pixel->after_pass1,
                      "no pass2 outside door-front");
        }
        if (pixel->f0108_transparent) ++transparent_f0108;
        if (pixel->f0111_transparent) ++transparent_f0111;
    }
    EXPECT_EQ("pixel.f0108_transparent_count", transparent_f0108, 6,
              "C10 flows through both D3L2 and D3R2 F0108 passes");
    EXPECT_EQ("pixel.f0111_transparent_count", transparent_f0111, 2,
              "F0111 C10 transparency flows through both D3L2 and D3R2 door fronts");
    EXPECT_EQ("pixel.door_cases", door_cases, 2,
              "one door-front sequence per side");

    EXPECT_TRUE("rejected.oob",
                dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_rejected_at_pc34(
                    DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34) == NULL,
                "rejected accessor bounds");
    for (i = 0; i < DM1_V1_D3L2_D3R2_F0108_WALL_COMPOSITION_REJECTED_COUNT_PC34; ++i) {
        const DM1V1D3L2D3R2F0108WallCompositionRejectedPc34 *r =
            dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_rejected_at_pc34((size_t)i);
        EXPECT_TRUE("rejected.present", r != NULL, "rejected keepout accessor");
        if (!r) continue;
        EXPECT_TRUE("rejected.name", r->name != NULL, "rejected keepout name");
        EXPECT_EQ("rejected.commit", r->f0107_owner_commit_pinned, 1,
                  "integrated F0107 owner already pinned");
        EXPECT_TRUE("rejected.reason", contains(r->why_disjoint, "commit"),
                    "commit-based disjointness reason");
        if (r->f0107_owner_commit_pinned) ++rejected_commits;
    }
    EXPECT_EQ("rejected.commit_count", rejected_commits, 3,
              "D3C plus D0L2/D0R2 integrated owners");
}

int main(void)
{
    const uint64_t hash =
        dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_deterministic_hash_pc34();

    test_model_header();
    test_source_evidence();
    test_specs();
    test_routes();
    test_ordinals_pixels_and_rejections();

    EXPECT_TRUE("assertion.count", g_assertions >= 200,
                "gate requires 200+ assertions");
    if (g_failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pc34_compat "
               "assertions=%d failures=%d hash=0x%016" PRIx64 "\n",
               g_assertions, g_failures, hash);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pc34_compat "
           "assertions=%d failures=0 hash=0x%016" PRIx64 "\n",
           g_assertions, hash);
    return 0;
}
