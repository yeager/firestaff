#include "dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_pc34_compat.h"

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
    const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *s,
    int side,
    int order,
    int lateral2_square,
    int owner_square,
    int view_floor,
    int flips,
    unsigned int corridor_order,
    unsigned int door1,
    unsigned int door2)
{
    CHECK_EQ("spec.present", s != NULL, 1, "DUNVIEW.C F0128:8503-8517");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, "D2L/D2R side identity");
    CHECK_EQ("spec.order", s->f0128_dispatch_order, order,
             "DUNVIEW.C F0128:8503-8517 post-D2L2/D2R2 order");
    CHECK_EQ("spec.lateral2", s->f067x_lateral2_view_square, lateral2_square,
             "DEFS.H:2596-2604 C09/C10 lateral-2 squares");
    CHECK_EQ("spec.owner", s->f0108_owner_view_square, owner_square,
             "DEFS.H:2596-2604 M604/M605 F0108 owners");
    CHECK_EQ("spec.floor", s->f0108_view_floor, view_floor,
             "DEFS.H:2742-2757 M591/M593");
    CHECK_EQ("spec.aspect_slot", s->floor_ornament_aspect_slot, 5,
             "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
    CHECK_EQ("spec.zone_base", s->floor_ornament_zone_base, 1500,
             "DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT");
    CHECK_EQ("spec.zone_stride", s->floor_ornament_zone_stride_pc34, 11,
             "DUNVIEW.C F0108:3998 PC34 zone stride");
    CHECK_EQ("spec.flip", s->right_side_flip, flips,
             "DUNVIEW.C F0108:3980-3985 right-floor flip");
    CHECK_EQ("spec.m575", s->m575_view_wall_d3l_right, 2,
             "DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT");
    CHECK_EQ("spec.m576", s->m576_view_wall_d3r_left, 3,
             "DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT");
    CHECK_EQ("spec.m577", s->m577_view_wall_d3l_front, 4,
             "DEFS.H:2700 M577_VIEW_WALL_D3L_FRONT");
    CHECK_EQ("spec.m578", s->m578_view_wall_d3c_front, 5,
             "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT");
    CHECK_EQ("spec.m579", s->m579_view_wall_d3r_front, 6,
             "DEFS.H:2702 M579_VIEW_WALL_D3R_FRONT");
    CHECK_EQ("spec.corridor_order", (int)s->corridor_cell_order, (int)corridor_order,
             "DEFS.H:2676-2677 D2L/D2R corridor orders");
    CHECK_EQ("spec.door1", (int)s->door_pass1_cell_order, (int)door1,
             "DEFS.H:2668-2669 door pass1 orders");
    CHECK_EQ("spec.door2", (int)s->door_pass2_cell_order, (int)door2,
             "DEFS.H:2672/2675 door pass2 orders");
    CHECK_EQ("spec.c10", s->transparent_color, 10,
             "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("spec.f0108_anchor", strstr(s->redmcsb_f0108_anchor, "F0108") != NULL,
             1, "source anchor carries F0108");
    CHECK_EQ("spec.dispatch_anchor", strstr(s->redmcsb_dispatch_anchor, "F0128") != NULL,
             1, "source anchor carries F0128");
    CHECK_EQ("spec.f0115_anchor", strstr(s->redmcsb_f0115_anchor, "F0115") != NULL,
             1, "source anchor carries F0115");
    CHECK_EQ("spec.defs_anchor", strstr(s->redmcsb_defs_anchor, "DEFS.H") != NULL,
             1, "source anchor carries DEFS.H");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_source_evidence_pc34();

    check_contains("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                   "mandatory F0108 anchor");
    check_contains("evidence.f0118", e, "DUNVIEW.C F0118:6642-6763",
                   "D3C dispatch after F0108 anchor");
    check_contains("evidence.f0115.loop", e, "DUNVIEW.C F0115:4547-4581",
                   "thing-pass loop anchor");
    check_contains("evidence.f0115.row", e, "DUNVIEW.C F0115:5668-5671",
                   "thing-pass row guard anchor");
    check_contains("evidence.f0678", e, "DUNVIEW.C F0678/F0679:6837-6896",
                   "lateral-2 frame anchor");
    check_contains("evidence.f0128", e, "DUNVIEW.C F0128:8503-8517",
                   "D2L2/D2R2 to D2L/D2R dispatch anchor");
    check_contains("evidence.f0119", e, "DUNVIEW.C F0119:6987-7031",
                   "D2L floor-ornament route");
    check_contains("evidence.f0120", e, "F0120:7180-7224",
                   "D2R floor-ornament route");
    check_contains("evidence.f0163", e, "DUNGEON.C F0163:1769-1838",
                   "thing link mutation anchor");
    check_contains("evidence.f0164", e, "F0164:1840-1905",
                   "thing unlink mutation anchor");
    check_contains("evidence.f0172", e, "F0172:2466-2523",
                   "square aspect anchor");
    check_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                   "C10 anchor");
    check_contains("evidence.c12c13", e, "DEFS.H:2443-2452",
                   "requested C12/C13-adjacent stair bitmap range");
    check_contains("evidence.squares.old", e, "DEFS.H:2582-2583",
                   "M604/M605 old media anchor");
    check_contains("evidence.squares.pc34", e, "2596-2604",
                   "M604/M605 PC34 anchor");
    check_contains("evidence.order.21", e, "DEFS.H:2662",
                   "C0x0021 order anchor");
    check_contains("evidence.orders", e, "2668-2677",
                   "door/open cell order anchors");
    check_contains("evidence.m575", e, "M575..M579",
                   "wall-view ordinal metadata anchor");
    check_contains("evidence.zones1", e, "DEFS.H:4144-4162",
                   "D2 stair zone range");
    check_contains("evidence.zones2", e, "4202-4207",
                   "D2 pit zone range");
    check_contains("evidence.floorzone", e, "DEFS.H:4223",
                   "floor ornament metadata zone");
}

static void test_specs(void)
{
    const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *d2l =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34);
    const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *d2r =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34);

    CHECK_EQ("count", dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_count_pc34(),
             2, "D2L and D2R floor-ornament specs");
    CHECK_EQ("at0", dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_at_pc34(0) == d2l,
             1, "stable D2L spec order");
    CHECK_EQ("at1", dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_at_pc34(1) == d2r,
             1, "stable D2R spec order");
    CHECK_EQ("at2.null", dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_at_pc34(2) == NULL,
             1, "bounds guard");
    CHECK_EQ("bad.side.null",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(
                 (DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34)7) == NULL,
             1, "unknown side rejected");

    check_spec(d2l, 0, 0, 9, 7, 5, 0, 0x3421u, 0x0218u, 0x0349u);
    check_spec(d2r, 1, 1, 10, 8, 7, 1, 0x4312u, 0x0128u, 0x0439u);
}

static void test_initial_state(void)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 left =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 right =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_FRONT_PC34);

    CHECK_EQ("initial.left.side", left.side, 0, "D2L initial state");
    CHECK_EQ("initial.left.context", left.context, 0, "corridor context");
    CHECK_EQ("initial.left.ordinal", (int)left.floor_ornament_ordinal, 0x8004,
             "F0108 footprint-marked floor ornament");
    CHECK_EQ("initial.left.thing", left.first_thing_before, 0x1201,
             "DUNGEON.C F0163/F0164 thing-list sentinel");
    CHECK_EQ("initial.left.dst", left.destination_pixel_before, 0x31,
             "synthetic destination pixel");
    CHECK_EQ("initial.left.src", left.floor_ornament_pixel, 0x41,
             "synthetic floor-ornament pixel");
    CHECK_EQ("initial.left.contract", left.contract_enabled ? 1 : 0, 1,
             "contract state starts enabled");
    CHECK_EQ("initial.left.f0107", left.allow_f0107_wall_overlap ? 1 : 0, 0,
             "F0107 non-overlap default");
    CHECK_EQ("initial.left.f0111", left.allow_f0111_door_overlap ? 1 : 0, 0,
             "F0111 non-overlap default");
    CHECK_EQ("initial.left.mutate", left.mutate_thing_list ? 1 : 0, 0,
             "thing mutation disabled");

    CHECK_EQ("initial.right.side", right.side, 1, "D2R initial state");
    CHECK_EQ("initial.right.context", right.context, 2, "door-front context");
    CHECK_EQ("initial.right.ordinal", (int)right.floor_ornament_ordinal, 0x8004,
             "same F0108 ordinal contract");
    CHECK_EQ("initial.right.thing", right.first_thing_before, 0x1202,
             "right thing sentinel");
    CHECK_EQ("initial.right.dst", right.destination_pixel_before, 0x32,
             "right destination pixel");
    CHECK_EQ("initial.right.src", right.floor_ornament_pixel, 0x42,
             "right floor-ornament pixel");
    CHECK_EQ("initial.right.contract", right.contract_enabled ? 1 : 0, 1,
             "right contract enabled");
}

static void test_ordinal_decode(void)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentOrdinalPc34 o;

    CHECK_EQ("decode.null",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(1, NULL),
             0, "null guard");
    CHECK_EQ("decode.zero.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(0, &o),
             1, "DUNVIEW.C F0108:3959 zero skip");
    CHECK_EQ("decode.zero.has", o.has_input_ordinal ? 1 : 0, 0,
             "zero ordinal has no draw");
    CHECK_EQ("decode.zero.primary", o.primary_draws ? 1 : 0, 0,
             "zero ordinal no primary");
    CHECK_EQ("decode.zero.recursive", o.recursive_footprints_draw ? 1 : 0, 0,
             "zero ordinal no recursion");
    CHECK_EQ("decode.zero.count", o.metadata_blit_count, 0,
             "zero ordinal no metadata blit");

    CHECK_EQ("decode.primary.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(4, &o),
             1, "DUNVIEW.C F0108:3965 ordinal to index");
    CHECK_EQ("decode.primary.has", o.has_input_ordinal ? 1 : 0, 1,
             "nonzero ordinal enters F0108");
    CHECK_EQ("decode.primary.flag", o.footprint_flag_set ? 1 : 0, 0,
             "ordinary floor ornament has no footprint flag");
    CHECK_EQ("decode.primary.draws", o.primary_draws ? 1 : 0, 1,
             "ordinary ordinal draws");
    CHECK_EQ("decode.primary.ordinal", (int)o.primary_ordinal, 4,
             "primary ordinal retained");
    CHECK_EQ("decode.primary.index", o.primary_index, 3,
             "ordinal decremented to index");
    CHECK_EQ("decode.primary.recursive", o.recursive_footprints_draw ? 1 : 0, 0,
             "ordinary ordinal no footprint recursion");
    CHECK_EQ("decode.primary.count", o.metadata_blit_count, 1,
             "primary blit count");

    CHECK_EQ("decode.flag.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(0x8004u, &o),
             1, "DUNVIEW.C F0108:3960-3963 footprint clear");
    CHECK_EQ("decode.flag.set", o.footprint_flag_set ? 1 : 0, 1,
             "MASK0x8000_FOOTPRINTS set");
    CHECK_EQ("decode.flag.cleared", (int)o.cleared_ordinal, 4,
             "M009_CLEAR leaves ordinal 4");
    CHECK_EQ("decode.flag.primary", o.primary_draws ? 1 : 0, 1,
             "cleared nonzero draws primary");
    CHECK_EQ("decode.flag.primary_index", o.primary_index, 3,
             "primary index after clear");
    CHECK_EQ("decode.flag.recursive", o.recursive_footprints_draw ? 1 : 0, 1,
             "footprints recurse");
    CHECK_EQ("decode.flag.recursive_ordinal", (int)o.recursive_footprints_ordinal, 16,
             "M000_INDEX_TO_ORDINAL(C15_FLOOR_ORNAMENT_FOOTPRINTS)");
    CHECK_EQ("decode.flag.recursive_index", o.recursive_footprints_index, 15,
             "DEFS.H:2465 C15_FLOOR_ORNAMENT_FOOTPRINTS");
    CHECK_EQ("decode.flag.count", o.metadata_blit_count, 2,
             "primary plus footprint metadata blits");

    CHECK_EQ("decode.only.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(0x8000u, &o),
             1, "DUNVIEW.C F0108:3961 cleared zero goto");
    CHECK_EQ("decode.only.primary", o.primary_draws ? 1 : 0, 0,
             "cleared zero skips primary");
    CHECK_EQ("decode.only.index", o.primary_index, -1,
             "no primary index");
    CHECK_EQ("decode.only.recursive", o.recursive_footprints_draw ? 1 : 0, 1,
             "footprint-only still recurses");
    CHECK_EQ("decode.only.count", o.metadata_blit_count, 1,
             "only footprint metadata blit");
}

static void check_result_common(
    const char *prefix,
    const DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34 *r,
    int side)
{
    char id[96];

    snprintf(id, sizeof(id), "%s.spec", prefix);
    CHECK_EQ(id, r->spec != NULL, 1, "valid D2 floor-ornament spec");
    snprintf(id, sizeof(id), "%s.f0678", prefix);
    CHECK_EQ(id, r->f0678FrameCount, side == 0 ? 1 : 0,
             "DUNVIEW.C F0678/F0679:6837-6896");
    snprintf(id, sizeof(id), "%s.f0679", prefix);
    CHECK_EQ(id, r->f0679FrameCount, side == 1 ? 1 : 0,
             "DUNVIEW.C F0678/F0679:6837-6896");
    snprintf(id, sizeof(id), "%s.f0128", prefix);
    CHECK_EQ(id, r->f0128PostCount, 1, "DUNVIEW.C F0128:8503-8517");
    snprintf(id, sizeof(id), "%s.thing_noop", prefix);
    CHECK_EQ(id, r->f0115ThingPassNoOpCount, 1,
             "DUNVIEW.C F0115:4547-4581/5668-5671");
    snprintf(id, sizeof(id), "%s.d2l2_thing", prefix);
    CHECK_EQ(id, r->d2l2CellThingUnchanged, side == 0 ? 1 : 0,
             "DUNGEON.C F0163/F0164 mutation guard");
    snprintf(id, sizeof(id), "%s.d2r2_thing", prefix);
    CHECK_EQ(id, r->d2r2CellThingUnchanged, side == 1 ? 1 : 0,
             "DUNGEON.C F0163/F0164 mutation guard");
    snprintf(id, sizeof(id), "%s.mutation", prefix);
    CHECK_EQ(id, r->mutationGuardsOk, 1, "thing-list mutation guards");
    snprintf(id, sizeof(id), "%s.nonoverlap", prefix);
    CHECK_EQ(id, r->nonOverlapWithF0107F0111, 1,
             "F0108-to-F0115 excludes F0107/F0111 overlap");
    snprintf(id, sizeof(id), "%s.rejected", prefix);
    CHECK_EQ(id, r->rejected_non_contract_state ? 1 : 0, 0,
             "accepted contract state");
}

static void test_composition(void)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 state;
    DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34 r;

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    CHECK_EQ("compose.left.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             1, "D2L corridor F0108 composition");
    check_result_common("compose.left", &r, 0);
    CHECK_EQ("compose.left.recursion", r.f0108FootprintRecursionCount, 1,
             "DUNVIEW.C F0108:4006-4008 footprint recursion");
    CHECK_EQ("compose.left.metadata", r.f0108OrnamentMetadataCount, 2,
             "DUNVIEW.C F0108:3965/3998 metadata blits");
    CHECK_EQ("compose.left.open_skip", r.f0108OpenPitSkipCount, 0,
             "corridor does not trigger open-pit skip");
    CHECK_EQ("compose.left.transparent", r.c10TransparentBlitCount, 0,
             "opaque floor pixel");
    CHECK_EQ("compose.left.drawn", r.floor_ornament_drawn ? 1 : 0, 1,
             "floor ornament was drawn");
    CHECK_EQ("compose.left.after", r.destination_pixel_after, 0x41,
             "opaque F0108 source replaces destination");

    state.floor_ornament_pixel =
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    CHECK_EQ("compose.left.c10.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             1, "C10 transparent F0108 composition");
    check_result_common("compose.left.c10", &r, 0);
    CHECK_EQ("compose.left.c10.count", r.c10TransparentBlitCount, 1,
             "DEFS.H:2088 C10 transparent blit count");
    CHECK_EQ("compose.left.c10.after", r.destination_pixel_after, 0x31,
             "C10 preserves destination");
    CHECK_EQ("compose.left.c10.drawn", r.floor_ornament_drawn ? 1 : 0, 0,
             "transparent pixel not counted as visible draw");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_FRONT_PC34);
    CHECK_EQ("compose.right.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             1, "D2R door-front F0108 before F0115");
    check_result_common("compose.right", &r, 1);
    CHECK_EQ("compose.right.recursion", r.f0108FootprintRecursionCount, 1,
             "right footprint recursion");
    CHECK_EQ("compose.right.metadata", r.f0108OrnamentMetadataCount, 2,
             "right metadata blits");
    CHECK_EQ("compose.right.after", r.destination_pixel_after, 0x42,
             "right opaque floor ornament");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34);
    CHECK_EQ("compose.pit.left.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             1, "open-pit preservation");
    check_result_common("compose.pit.left", &r, 0);
    CHECK_EQ("compose.pit.left.skip", r.f0108OpenPitSkipCount, 1,
             "open pit has no floor-ornament blit on that cell");
    CHECK_EQ("compose.pit.left.metadata", r.f0108OrnamentMetadataCount, 0,
             "open pit skips metadata blit");
    CHECK_EQ("compose.pit.left.recursion", r.f0108FootprintRecursionCount, 0,
             "open pit skips footprint recursion");
    CHECK_EQ("compose.pit.left.after", r.destination_pixel_after, 0x31,
             "open pit preserves destination");
    CHECK_EQ("compose.pit.left.drawn", r.floor_ornament_drawn ? 1 : 0, 0,
             "open pit has no visible floor ornament");
    CHECK_EQ("compose.pit.left.preserved", r.open_pit_preserved ? 1 : 0, 1,
             "open-pit ordering invariant");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34);
    CHECK_EQ("compose.pit.right.ok",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             1, "right open-pit preservation");
    check_result_common("compose.pit.right", &r, 1);
    CHECK_EQ("compose.pit.right.skip", r.f0108OpenPitSkipCount, 1,
             "right open pit has no floor-ornament blit");
    CHECK_EQ("compose.pit.right.metadata", r.f0108OrnamentMetadataCount, 0,
             "right open pit skips metadata blit");
    CHECK_EQ("compose.pit.right.after", r.destination_pixel_after, 0x32,
             "right open pit preserves destination");
}

static void test_non_contract_rejection(void)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 state =
        dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34 r;

    CHECK_EQ("compose.null.state",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(NULL, &r),
             0, "null state guard");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, NULL),
             0, "null output guard");

    state.side = (DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34)9;
    CHECK_EQ("reject.bad.side",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             0, "bad side rejected");
    CHECK_EQ("reject.bad.side.flag", r.rejected_non_contract_state ? 1 : 0, 1,
             "bad side rejection flag");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    state.contract_enabled = false;
    CHECK_EQ("reject.disabled",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             0, "disabled contract rejected");
    CHECK_EQ("reject.disabled.flag", r.rejected_non_contract_state ? 1 : 0, 1,
             "disabled rejection flag");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    state.allow_f0107_wall_overlap = true;
    CHECK_EQ("reject.f0107",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             0, "F0107 overlap rejected");
    CHECK_EQ("reject.f0107.flag", r.rejected_non_contract_state ? 1 : 0, 1,
             "F0107 rejection flag");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_FRONT_PC34);
    state.allow_f0111_door_overlap = true;
    CHECK_EQ("reject.f0111",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             0, "F0111 overlap rejected");
    CHECK_EQ("reject.f0111.flag", r.rejected_non_contract_state ? 1 : 0, 1,
             "F0111 rejection flag");

    state = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34);
    state.mutate_thing_list = true;
    CHECK_EQ("reject.mutate",
             dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(&state, &r),
             0, "thing-list mutation rejected");
    CHECK_EQ("reject.mutate.flag", r.rejected_non_contract_state ? 1 : 0, 1,
             "mutation rejection flag");
}

int main(void)
{
    test_source_evidence();
    test_specs();
    test_initial_state();
    test_ordinal_decode();
    test_composition();
    test_non_contract_rejection();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
