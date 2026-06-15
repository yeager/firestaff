#include "dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define CHECK_EQ(ID, GOT, WANT, ANCHOR)                                      \
    do {                                                                     \
        const int got_value__ = (int)(GOT);                                  \
        const int want_value__ = (int)(WANT);                                \
        ++g_assertions;                                                      \
        if (got_value__ != want_value__) {                                   \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                    \
                   (ID), got_value__, want_value__, (ANCHOR));              \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("PASS %s == %d anchor=%s\n",                             \
                   (ID), want_value__, (ANCHOR));                           \
        }                                                                    \
    } while (0)

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void check_spec(
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *s,
    int side,
    int context,
    int order,
    int guard_square,
    int owner_square,
    int floor_view,
    int lateral,
    int flips,
    int ceiling_after,
    int f0111_after,
    int door_zone,
    int frame_zone)
{
    CHECK_EQ("spec.present", s != NULL, 1,
             "DUNVIEW.C F0119/F0120 F0108 call sites");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, "D2L/D2R side identity");
    CHECK_EQ("spec.context", s->context, context,
             "open-row versus door-front composition");
    CHECK_EQ("spec.order", s->draw_order_index, order,
             "DUNVIEW.C F0128 D2L before D2R row order");
    CHECK_EQ("spec.depth", s->relative_depth, 2, "D2 row depth");
    CHECK_EQ("spec.lateral", s->relative_lateral, lateral, "D2 lateral side");
    CHECK_EQ("spec.guard_square", s->requested_guard_view_square, guard_square,
             "DEFS.H:2605-2606 C09/C10 D2L2/D2R2 guard");
    CHECK_EQ("spec.owner_square", s->f0108_owner_view_square, owner_square,
             "DEFS.H:2603-2604 M604/M605 own F0108 route");
    CHECK_EQ("spec.floor_view", s->f0108_floor_view, floor_view,
             "DEFS.H:2742-2757 M591/M593 floor views");
    CHECK_EQ("spec.aspect_slot", s->f0108_square_aspect_slot, 558,
             "DEFS.H:2544/2558 M558_FLOOR_ORNAMENT_ORDINAL");
    CHECK_EQ("spec.zone_base", s->f0108_floor_zone_base, 1500,
             "DUNVIEW.C F0108 PC zone base");
    CHECK_EQ("spec.zone_stride", s->f0108_floor_zone_stride_pc34, 11,
             "DUNVIEW.C:3998/4006 PC34 floor ornament zone stride");
    CHECK_EQ("spec.flips", s->f0108_right_side_flips ? 1 : 0, flips,
             "DUNVIEW.C:3975-3982 F0108 right-side flip gate");
    CHECK_EQ("spec.footprints", s->footprints_recurse_same_view ? 1 : 0, 1,
             "DUNVIEW.C:4008-4010 footprint recursion");
    CHECK_EQ("spec.wall_excludes", s->wall_element_excludes_f0108 ? 1 : 0, 1,
             "DUNVIEW.C:6961-6974/7112-7147 wall cases return");
    CHECK_EQ("spec.guard_excludes", s->d2l2_d2r2_guard_excludes_f0108 ? 1 : 0, 1,
             "DUNVIEW.C F0678/F0679 C09/C10 no F0108 route");
    CHECK_EQ("spec.ceiling_after", s->calls_ceiling_pit_after_f0108 ? 1 : 0,
             ceiling_after, "DUNVIEW.C T0119020/T0120029 ceiling-pit order");
    CHECK_EQ("spec.f0111_after", s->calls_f0111_after_f0108 ? 1 : 0,
             f0111_after, "DUNVIEW.C:6987-7004/7180-7197 door-front order");
    CHECK_EQ("spec.door_zone", s->f0111_door_zone, door_zone,
             "DEFS.H:4239-4257 M627/M629 when applicable");
    CHECK_EQ("spec.frame_zone", s->f0111_door_frame_top_zone, frame_zone,
             "DEFS.H:4087-4089/door frame top zones when applicable");
    CHECK_EQ("spec.open_order", (int)s->f0115_open_cell_order,
             side == 0 ? 0x3421 : 0x4312,
             "DUNVIEW.C:7017/7210 open-row L0207/L0209 orders");
    CHECK_EQ("spec.pass1", (int)s->f0115_door_pass1_cell_order,
             side == 0 ? 0x0218 : 0x0128,
             "DEFS.H:2668-2669 door pass1 order");
    CHECK_EQ("spec.pass2", (int)s->f0115_door_pass2_cell_order,
             side == 0 ? 0x0349 : 0x0439,
             "DEFS.H:2672/2675 door pass2 order");
    CHECK_EQ("spec.c10", s->transparent_color, 10,
             "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("spec.contract", s->source_locked_contract_only ? 1 : 0, 1,
             "contract-only source lock");
    CHECK_EQ("spec.no_assets", s->no_real_asset_bitmap_parity ? 1 : 0, 1,
             "asset-free synthetic pixels");
    CHECK_EQ("spec.no_data", s->no_game_data_load ? 1 : 0, 1,
             "no GRAPHICS.DAT dependency");
    CHECK_EQ("spec.f0108_anchor", strstr(s->redmcsb_f0108_anchor, "F0108") != NULL,
             1, "source anchor carries F0108");
    CHECK_EQ("spec.dispatch_anchor", strstr(s->redmcsb_dispatch_anchor, "DUNVIEW.C") != NULL,
             1, "source anchor carries DUNVIEW.C dispatch");
    CHECK_EQ("spec.defs_anchor", strstr(s->redmcsb_defs_anchor, "DEFS.H") != NULL,
             1, "source anchor carries DEFS.H");
}

static void test_specs(void)
{
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *d2l_open =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34);
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *d2r_open =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34);
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *d2l_door =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34);
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *d2r_door =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34);

    CHECK_EQ("count", dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_count_pc34(),
             4, "D2L/D2R open plus door-front contracts");
    CHECK_EQ("at0", dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_at_pc34(0) == d2l_open,
             1, "stable spec order");
    CHECK_EQ("at3", dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_at_pc34(3) == d2r_door,
             1, "stable spec order");
    CHECK_EQ("at4.null", dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_at_pc34(4) == NULL,
             1, "bounds guard");
    CHECK_EQ("bad.side.null",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
                 (DM1_V1_D2L2D2R2F0108WallSidePc34)9,
                 DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34) == NULL,
             1, "unknown side rejected");

    check_spec(d2l_open, 0, 0, 0, 9, 7, 5, -1, 0, 1, 0, 0, 0);
    check_spec(d2r_open, 1, 0, 1, 10, 8, 7, 1, 1, 1, 0, 0, 0);
    check_spec(d2l_door, 0, 1, 2, 9, 7, 5, -1, 0, 0, 1, 3750, 729);
    check_spec(d2r_door, 1, 1, 3, 10, 8, 7, 1, 1, 0, 1, 3770, 731);
}

static void test_ordinal_decode(void)
{
    DM1_V1_D2L2D2R2F0108WallOrdinalPc34 o;

    CHECK_EQ("decode.null",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(1, NULL),
             0, "null guard");
    CHECK_EQ("decode.zero.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(0, &o),
             1, "DUNVIEW.C:3959 zero ordinal skip");
    CHECK_EQ("decode.zero.has", o.has_input_ordinal ? 1 : 0, 0,
             "zero ordinal draws nothing");
    CHECK_EQ("decode.zero.primary", o.primary_draws ? 1 : 0, 0,
             "zero ordinal no primary draw");
    CHECK_EQ("decode.zero.footprints", o.recursive_footprints_draw ? 1 : 0, 0,
             "zero ordinal no footprint recursion");

    CHECK_EQ("decode.primary.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(4, &o),
             1, "DUNVIEW.C:3965 --ordinal to index");
    CHECK_EQ("decode.primary.has", o.has_input_ordinal ? 1 : 0, 1,
             "nonzero ordinal enters F0108");
    CHECK_EQ("decode.primary.flag", o.footprint_flag_set ? 1 : 0, 0,
             "ordinary floor ornament has no footprint flag");
    CHECK_EQ("decode.primary.draws", o.primary_draws ? 1 : 0, 1,
             "ordinary floor ornament draws primary bitmap");
    CHECK_EQ("decode.primary.index", o.primary_index, 3,
             "M000 ordinal-to-index decrements");
    CHECK_EQ("decode.primary.recursive", o.recursive_footprints_draw ? 1 : 0, 0,
             "ordinary floor ornament does not recurse");

    CHECK_EQ("decode.flag.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(0x8004u, &o),
             1, "DUNVIEW.C:3960-3963 footprint clear");
    CHECK_EQ("decode.flag.set", o.footprint_flag_set ? 1 : 0, 1,
             "MASK0x8000_FOOTPRINTS set");
    CHECK_EQ("decode.flag.cleared", (int)o.cleared_ordinal, 4,
             "M009_CLEAR leaves ordinal 4");
    CHECK_EQ("decode.flag.primary", o.primary_draws ? 1 : 0, 1,
             "nonzero cleared ordinal still draws primary");
    CHECK_EQ("decode.flag.primary_index", o.primary_index, 3,
             "primary index after clear");
    CHECK_EQ("decode.flag.recursive", o.recursive_footprints_draw ? 1 : 0, 1,
             "footprints recurse after primary");
    CHECK_EQ("decode.flag.recursive_ordinal", (int)o.recursive_footprints_ordinal, 16,
             "M000_INDEX_TO_ORDINAL(C15_FLOOR_ORNAMENT_FOOTPRINTS)");
    CHECK_EQ("decode.flag.recursive_index", o.recursive_footprints_index, 15,
             "DEFS.H:2465 C15_FLOOR_ORNAMENT_FOOTPRINTS");

    CHECK_EQ("decode.only_footprints.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(0x8000u, &o),
             1, "DUNVIEW.C:3961 cleared zero jumps to footprint recursion");
    CHECK_EQ("decode.only_footprints.primary", o.primary_draws ? 1 : 0, 0,
             "cleared zero skips primary bitmap");
    CHECK_EQ("decode.only_footprints.index", o.primary_index, -1,
             "no primary index after cleared zero");
    CHECK_EQ("decode.only_footprints.recursive", o.recursive_footprints_draw ? 1 : 0, 1,
             "footprint-only ordinal still recurses");
}

static void test_pixel_composition(void)
{
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *open =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34);
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *door =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34,
            DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34);
    DM1_V1_D2L2D2R2F0108WallPixelPc34 p;

    CHECK_EQ("blend.transparent",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
                 0x44, 10, 10),
             0x44, "DEFS.H:2088 C10 preserves destination");
    CHECK_EQ("blend.opaque",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
                 0x44, 0x55, 10),
             0x55, "opaque F0108 source writes");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 open, 0, 0, 0, 0, 0, 0, NULL),
             0, "null output guard");
    CHECK_EQ("compose.null.spec",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 NULL, 0, 0, 0, 0, 0, 0, &p),
             0, "null spec guard");

    CHECK_EQ("open.compose",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 open, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, &p),
             1, "F0108 -> F0112 -> F0115 open-row order");
    CHECK_EQ("open.sequence", p.door_front_sequence ? 1 : 0, 0,
             "open-row is not door-front");
    CHECK_EQ("open.after_floor", p.after_floor, 0x21,
             "F0108 writes before ceiling pit");
    CHECK_EQ("open.after_ceiling", p.after_ceiling_or_frame, 0x31,
             "F0112 writes after F0108");
    CHECK_EQ("open.after_pass1", p.after_pass1, 0x41,
             "F0115 open-row thing pass writes after ceiling");
    CHECK_EQ("open.after_door", p.after_door, 0x41,
             "no F0111 door layer in open-row route");
    CHECK_EQ("open.after_pass2", p.after_pass2, 0x41,
             "no second thing pass in open-row route");

    CHECK_EQ("open.c10.compose",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 open, 0x66, 10, 10, 10, 0x51, 0x61, &p),
             1, "all open-row layers transparent");
    CHECK_EQ("open.c10.floor", p.floor_transparent ? 1 : 0, 1,
             "F0108 C10 transparent");
    CHECK_EQ("open.c10.ceiling", p.ceiling_or_frame_transparent ? 1 : 0, 1,
             "F0112 synthetic C10 transparent");
    CHECK_EQ("open.c10.pass", p.pass1_transparent ? 1 : 0, 1,
             "F0115 synthetic C10 transparent");
    CHECK_EQ("open.c10.final", p.after_pass2, 0x66,
             "transparent open-row layers preserve destination");

    CHECK_EQ("door.compose",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 door, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, &p),
             1, "F0108 -> F0115 pass1 -> frame -> F0111 -> F0115 pass2");
    CHECK_EQ("door.sequence", p.door_front_sequence ? 1 : 0, 1,
             "door-front route");
    CHECK_EQ("door.after_floor", p.after_floor, 0x22,
             "F0108 writes first on door-front");
    CHECK_EQ("door.after_pass1", p.after_pass1, 0x44,
             "F0115 pass1 writes after F0108");
    CHECK_EQ("door.after_frame", p.after_ceiling_or_frame, 0x33,
             "door-frame top writes before F0111");
    CHECK_EQ("door.after_door", p.after_door, 0x55,
             "F0111 door writes after frame");
    CHECK_EQ("door.after_pass2", p.after_pass2, 0x66,
             "F0115 pass2 writes last");

    CHECK_EQ("door.c10.compose",
             dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
                 door, 0x77, 10, 10, 10, 0x58, 10, &p),
             1, "transparent floor/pass layers around opaque door");
    CHECK_EQ("door.c10.floor", p.floor_transparent ? 1 : 0, 1,
             "F0108 C10 transparent");
    CHECK_EQ("door.c10.pass1", p.pass1_transparent ? 1 : 0, 1,
             "F0115 pass1 C10 transparent");
    CHECK_EQ("door.c10.frame", p.ceiling_or_frame_transparent ? 1 : 0, 1,
             "frame C10 transparent");
    CHECK_EQ("door.c10.pass2", p.pass2_transparent ? 1 : 0, 1,
             "F0115 pass2 C10 transparent");
    CHECK_EQ("door.c10.final", p.after_pass2, 0x58,
             "opaque F0111 door survives transparent pass2");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_source_evidence_pc34();

    check_contains("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                   "mandatory F0108 anchor");
    check_contains("evidence.d2l_door", e, "DUNVIEW.C F0119:6987-7004",
                   "D2L door-front anchor");
    check_contains("evidence.d2r_door", e, "DUNVIEW.C F0120:7180-7197",
                   "D2R door-front anchor");
    check_contains("evidence.open", e, "F0119:7017-7038",
                   "open-row F0108 anchor");
    check_contains("evidence.wall_excludes", e, "wall cases exclude F0108",
                   "wall branch exclusion");
    check_contains("evidence.guard", e, "F0678/F0679:6837-6896",
                   "D2L2/D2R2 guard exclusion");
    check_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                   "C10 transparency anchor");
    check_contains("evidence.squares", e, "DEFS.H:2603-2606",
                   "view-square anchor");
    check_contains("evidence.orders", e, "DEFS.H:2668-2677",
                   "cell-order anchor");
    check_contains("evidence.floor_views", e, "DEFS.H:2742-2757",
                   "floor-view anchor");
    check_contains("evidence.zones", e, "DEFS.H:4239-4257",
                   "door-zone anchor");
}

int main(void)
{
    test_specs();
    test_ordinal_decode();
    test_pixel_composition();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
