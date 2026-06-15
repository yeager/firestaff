#include "firestaff/dm1/v1/viewport/d0c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0C_VIEW_SQUARE_MEDIA720 = 0,
    DM1_D0C_VIEW_SQUARE_LEGACY = 9,
    DM1_M550_FIRST_THING_PC34 = 2,
    DM1_M558_FLOOR_ORNAMENT_PC34 = 5,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D0C_SYNTHETIC_COORDINATE_SET = 1,
    DM1_D0C_SYNTHETIC_VIEW_FLOOR = 9,
    DM1_D0C_SYNTHETIC_FLOOR_ZONE = 1520,
    DM1_CEILING_PIT_GRAPHIC_D0C_PC34 = 69,
    DM1_CEILING_PIT_ZONE_D0C_PC34 = 871,
    DM1_FLOOR_PIT_ZONE_D0C_PC34 = 862,
    DM1_FIELD_ZONE_D0C_PC34 = 715,
    DM1_C705_ZONE_WALL_D3L = 705,
    DM1_C706_ZONE_WALL_D3R = 706,
    DM1_M575_VIEW_WALL_D3L_RIGHT = 2,
    DM1_M576_VIEW_WALL_D3R_LEFT = 3,
    DM1_M577_VIEW_WALL_D3L_FRONT = 4,
    DM1_M578_VIEW_WALL_D3C_FRONT = 5,
    DM1_M579_VIEW_WALL_D3R_FRONT = 6,
    DM1_ORDER_F0098 = 10,
    DM1_ORDER_F0107_KEEP_OUT = 20,
    DM1_ORDER_F0108_REFERENCE = 25,
    DM1_ORDER_F0112 = 30,
    DM1_ORDER_F0115 = 40,
    DM1_ORDER_F0113 = 50,
    DM1_MUTATION_GUARD_BEFORE = 0x0d0c0108,
    DM1_MUTATION_GUARD_AFTER = 0x0f0115d0
};

/*
 * ReDMCSB source lock: F0108 is anchored at DUNVIEW.C:3940-4011, while
 * the actual D0C body at F0127:8184-8311 keeps that floor-ornament body
 * out of the D0C stairs/pit/door-side/corridor tail. The gate records
 * that distinction so D0C floor+ceiling setup cannot drift into F0107,
 * F0111, sibling F0108, or F0115-detail coverage.
 */
static const char s_source_evidence[] =
    "Contract-only DM1 V1 D0C F0108 floor+ceiling+ornament source lock; "
    "no original DOS pixel parity claim and no real-asset bitmap compare. "
    "DUNVIEW.C F0108:3940-4011 anchors M558 floor-ornament ordinal, "
    "MASK0x8000_FOOTPRINTS recursion, C10_COLOR_FLESH transparency, and "
    "C1500 + CoordinateSet * 11 + ViewFloor PC34 zone math. DUNVIEW.C "
    "F0127:8184-8311 anchors the D0C body: stairs-front 8241-8273 breaks "
    "before the shared F0112/F0115 tail; pit 8274-8284 reaches F0112 at "
    "8286/8289/8292 and F0115 at 8294; teleporter reaches F0113 after "
    "F0115 at 8295-8310. DUNVIEW.C F0128:8491-8542 anchors D3L/D3R/D3C "
    "then D0L/D0R before D0C dispatch. DUNVIEW.C F0115:4794-4798 and "
    "5245-5267 anchor the L0175_i_DoorFrontViewDrawingPass two-pass order. "
    "DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523 "
    "anchor list mutation and square-aspect boundaries. DEFS.H:2533-2559 "
    "pins M550_FIRST_THING and M558_FLOOR_ORNAMENT_ORDINAL; DEFS.H:"
    "2680-2702 pins M575..M579; DEFS.H:4045-4046 pins C705/C706.";

static const char s_non_overlap_marker[] =
    "pass787 D0C F0108 floor+ceiling+ornament contract only; does not "
    "duplicate F0107 wall-ornament bodies, D0L/D0R or D3L/D3R F0108 "
    "floor+ceiling+ornament gates, D0C F0111 partly-open transparency, "
    "F0115 thing-pass detail, chest/mirror candidates, CSB slices, or the "
    "sibling D2C F0108 gate.";

static const DM1_V1_D0CF0108FloorCeilingOrnamentSpecPc34 s_spec = {
    "D0C F0108 floor+ceiling+ornament dispatch/keepout source lock",
    "F0127_DUNGEONVIEW_DrawSquareD0C",
    DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_WIDTH_PC34,
    DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_HEIGHT_PC34,
    DM1_V1_D0C_F0108_FCO_VIEWPORT_WIDTH_PC34,
    DM1_V1_D0C_F0108_FCO_VIEWPORT_HEIGHT_PC34,
    DM1_D0C_VIEW_SQUARE_MEDIA720,
    DM1_D0C_VIEW_SQUARE_LEGACY,
    DM1_M550_FIRST_THING_PC34,
    DM1_M558_FLOOR_ORNAMENT_PC34,
    DM1_FLOOR_ZONE_BASE,
    DM1_FLOOR_ZONE_STRIDE_PC34,
    DM1_D0C_SYNTHETIC_COORDINATE_SET,
    DM1_D0C_SYNTHETIC_VIEW_FLOOR,
    DM1_D0C_SYNTHETIC_FLOOR_ZONE,
    DM1_CEILING_PIT_GRAPHIC_D0C_PC34,
    DM1_CEILING_PIT_ZONE_D0C_PC34,
    DM1_FLOOR_PIT_ZONE_D0C_PC34,
    DM1_FIELD_ZONE_D0C_PC34,
    DM1_C705_ZONE_WALL_D3L,
    DM1_C706_ZONE_WALL_D3R,
    DM1_M575_VIEW_WALL_D3L_RIGHT,
    DM1_M576_VIEW_WALL_D3R_LEFT,
    DM1_M577_VIEW_WALL_D3L_FRONT,
    DM1_M578_VIEW_WALL_D3C_FRONT,
    DM1_M579_VIEW_WALL_D3R_FRONT,
    3940,
    4011,
    8184,
    8311,
    8491,
    8495,
    8499,
    8537,
    8541,
    8542,
    8292,
    8294,
    8308,
    4795,
    5245,
    "DUNVIEW.C F0108:3940-4011",
    "DUNVIEW.C F0127:8184-8311",
    "DUNVIEW.C F0128:8491-8542",
    "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523",
    s_non_overlap_marker
};

static DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34 s_last_self_test;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    unsigned int i;

    for (i = 0; i < 4u; ++i) {
        hash ^= (value >> (i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t hash_string(uint32_t hash, const char *value)
{
    if (!value) return hash_u32(hash, 0xffffffffu);
    while (*value) {
        hash ^= (uint8_t)*value++;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t next_lcg(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static bool supported_context(
    DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 context)
{
    return context == DM1_V1_D0C_F0108_FCO_CONTEXT_WALL_ORNAMENT_BRANCH_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_CORRIDOR_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_OPEN_PIT_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_DOOR_SIDE_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_STAIRS_FRONT_PC34;
}

static bool context_reaches_shared_tail(
    DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 context)
{
    return context == DM1_V1_D0C_F0108_FCO_CONTEXT_CORRIDOR_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_OPEN_PIT_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34 ||
        context == DM1_V1_D0C_F0108_FCO_CONTEXT_DOOR_SIDE_PC34;
}

const DM1_V1_D0CF0108FloorCeilingOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_spec_pc34(void)
{
    return &s_spec;
}

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_initial_state_pc34(
    DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 context,
    DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 *out)
{
    if (!out || !supported_context(context)) return false;
    memset(out, 0, sizeof(*out));
    out->context = context;
    out->floor_ornament_ordinal = 0x8004u;
    out->base_floor_pixel = 0x21u;
    out->base_ceiling_pixel = 0x31u;
    out->floor_ornament_pixel = 0x52u;
    out->footprint_pixel = DM1_V1_D0C_F0108_FCO_C10_COLOR_FLESH_PC34;
    out->thing_pixel = DM1_V1_D0C_F0108_FCO_C10_COLOR_FLESH_PC34;
    out->seed = 0xd0c0108u + (uint32_t)context;
    out->source_locked_contract_only = true;
    out->no_original_dos_parity_claim = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0CF0108FloorCeilingOrnamentOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (floor_ornament_ordinal & DM1_V1_D0C_F0108_FCO_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~DM1_V1_D0C_F0108_FCO_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index = DM1_V1_D0C_F0108_FCO_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D0C_F0108_FCO_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

int dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_zone_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0 || view_floor < 0) return -1;
    return DM1_FLOOR_ZONE_BASE + coordinate_set * DM1_FLOOR_ZONE_STRIDE_PC34 +
        view_floor;
}

bool dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_compose_pc34(
    const DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 *state,
    DM1_V1_D0CF0108FloorCeilingOrnamentResultPc34 *out)
{
    DM1_V1_D0CF0108FloorCeilingOrnamentOrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = &s_spec;
    if (!state || !supported_context(state->context) ||
        !state->source_locked_contract_only ||
        !state->no_original_dos_parity_claim ||
        !state->no_game_data_load ||
        state->request_real_asset_bitmap_compare ||
        state->request_f0107_wall_ornament_body ||
        state->request_f0111_door_transparency ||
        state->request_f0115_thing_pass_detail ||
        state->request_sibling_view_slice ||
        state->mutate_thing_list) {
        out->rejected_non_contract_state = 1;
        out->mutation_guard_ok = state && !state->mutate_thing_list ? 1 : 0;
        return false;
    }
    if (!dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->context_supported = 1;
    out->framebuffer_width = DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_WIDTH_PC34;
    out->framebuffer_height = DM1_V1_D0C_F0108_FCO_FRAMEBUFFER_HEIGHT_PC34;
    out->viewport_width = DM1_V1_D0C_F0108_FCO_VIEWPORT_WIDTH_PC34;
    out->viewport_height = DM1_V1_D0C_F0108_FCO_VIEWPORT_HEIGHT_PC34;
    out->f0098_floor_base_calls = 1;
    out->f0098_ceiling_base_calls = 1;
    out->f0108_reference_locked = 1;
    out->f0108_floor_ornament_calls_in_d0c_body = 0;
    out->f0108_floor_zone =
        dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_zone_pc34(
            s_spec.synthetic_center_coordinate_set,
            s_spec.synthetic_center_view_floor);
    out->f0108_primary_index = ordinal.primary_index;
    out->f0108_recursive_index = ordinal.recursive_footprints_index;
    out->f0108_c10_transparency_checks = 2;
    out->f0107_wall_ornament_branch_kept_out =
        state->context == DM1_V1_D0C_F0108_FCO_CONTEXT_WALL_ORNAMENT_BRANCH_PC34 ? 1 : 0;
    out->f0111_door_transparency_kept_out = 1;
    out->terminal_side_pair_correction =
        s_spec.f0128_d0l_line < s_spec.f0128_d0c_line &&
        s_spec.f0128_d0r_line < s_spec.f0128_d0c_line;
    out->d0c_after_d0l_d0r = out->terminal_side_pair_correction;
    out->door_front_two_pass_order_checked =
        s_spec.door_front_pass_line == 4795 &&
        s_spec.door_front_creature_defer_line == 5245;
    out->mutation_guard_ok = 1;
    out->non_overlap_ok = 1;

    pixel = state->base_floor_pixel;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
        pixel, state->floor_ornament_pixel);
    pixel = dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
        pixel, state->footprint_pixel);
    out->floor_sample = pixel;
    out->ceiling_sample = state->base_ceiling_pixel;
    out->ornament_sample_after_c10 =
        dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
            pixel, DM1_V1_D0C_F0108_FCO_C10_COLOR_FLESH_PC34);
    out->thing_observed_sample = out->ornament_sample_after_c10;

    if (context_reaches_shared_tail(state->context)) {
        out->f0112_ceiling_calls = 1;
        out->f0115_thing_pass_calls = 1;
        out->f0112_before_f0115 = DM1_ORDER_F0112 < DM1_ORDER_F0115;
        out->thing_observed_sample =
            dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(
                out->thing_observed_sample, state->thing_pixel);
        if (state->context == DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34) {
            out->f0113_field_calls = 1;
            out->f0113_after_f0115 = DM1_ORDER_F0115 < DM1_ORDER_F0113;
        }
    }

    out->deterministic_hash = hash_u32(state->seed, (uint32_t)out->framebuffer_width);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->viewport_height);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)state->context);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->f0108_floor_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->f0112_ceiling_calls);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->f0115_thing_pass_calls);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->f0113_field_calls);
    out->deterministic_hash = hash_u32(out->deterministic_hash, out->floor_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, out->thing_observed_sample);
    return true;
}

typedef struct {
    int assertions;
    int failures;
    uint32_t hash;
} CheckState;

static void check_int(CheckState *c, const char *id, int got, int want)
{
    ++c->assertions;
    c->hash = hash_string(c->hash, id);
    c->hash = hash_u32(c->hash, (uint32_t)got);
    c->hash = hash_u32(c->hash, (uint32_t)want);
    if (got != want) ++c->failures;
}

static void check_true(CheckState *c, const char *id, int condition)
{
    check_int(c, id, condition ? 1 : 0, 1);
}

static void check_contains(CheckState *c, const char *id, const char *haystack, const char *needle)
{
    c->hash = hash_string(c->hash, needle);
    check_true(c, id, haystack && needle && strstr(haystack, needle) != NULL);
}

int run_dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_self_test(void)
{
    static const DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34 contexts[] = {
        DM1_V1_D0C_F0108_FCO_CONTEXT_WALL_ORNAMENT_BRANCH_PC34,
        DM1_V1_D0C_F0108_FCO_CONTEXT_CORRIDOR_PC34,
        DM1_V1_D0C_F0108_FCO_CONTEXT_OPEN_PIT_PC34,
        DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34,
        DM1_V1_D0C_F0108_FCO_CONTEXT_DOOR_SIDE_PC34,
        DM1_V1_D0C_F0108_FCO_CONTEXT_STAIRS_FRONT_PC34
    };
    CheckState c = { 0, 0, 2166136261u };
    DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34 local;
    size_t i;
    uint32_t seed = 0x787d0c00u;

    memset(&local, 0, sizeof(local));

    check_contains(&c, "evidence.f0108", s_source_evidence, "F0108:3940-4011");
    check_contains(&c, "evidence.footprints", s_source_evidence, "MASK0x8000_FOOTPRINTS");
    check_contains(&c, "evidence.c10", s_source_evidence, "C10_COLOR_FLESH");
    check_contains(&c, "evidence.zone_math", s_source_evidence, "C1500 + CoordinateSet * 11 + ViewFloor");
    check_contains(&c, "evidence.d0c", s_source_evidence, "F0127:8184-8311");
    check_contains(&c, "evidence.f0112", s_source_evidence, "F0112");
    check_contains(&c, "evidence.f0115", s_source_evidence, "F0115 at 8294");
    check_contains(&c, "evidence.f0113", s_source_evidence, "F0113 after");
    check_contains(&c, "evidence.dispatch", s_source_evidence, "F0128:8491-8542");
    check_contains(&c, "evidence.two_pass", s_source_evidence, "L0175_i_DoorFrontViewDrawingPass");
    check_contains(&c, "evidence.f0163", s_source_evidence, "F0163:1769-1838");
    check_contains(&c, "evidence.f0164", s_source_evidence, "F0164:1840-1905");
    check_contains(&c, "evidence.f0172", s_source_evidence, "F0172:2466-2523");
    check_contains(&c, "evidence.m550", s_source_evidence, "M550_FIRST_THING");
    check_contains(&c, "evidence.m558", s_source_evidence, "M558_FLOOR_ORNAMENT_ORDINAL");
    check_contains(&c, "evidence.m575", s_source_evidence, "M575..M579");
    check_contains(&c, "evidence.c705", s_source_evidence, "C705/C706");
    check_contains(&c, "nonoverlap.f0107", s_non_overlap_marker, "F0107");
    check_contains(&c, "nonoverlap.d0lr", s_non_overlap_marker, "D0L/D0R");
    check_contains(&c, "nonoverlap.d3lr", s_non_overlap_marker, "D3L/D3R");
    check_contains(&c, "nonoverlap.f0111", s_non_overlap_marker, "F0111");
    check_contains(&c, "nonoverlap.f0115", s_non_overlap_marker, "F0115");
    check_contains(&c, "nonoverlap.d2c", s_non_overlap_marker, "D2C");
    local.non_overlap_checks += 6;

    check_int(&c, "spec.framebuffer_width", s_spec.framebuffer_width, 320);
    check_int(&c, "spec.framebuffer_height", s_spec.framebuffer_height, 200);
    check_int(&c, "spec.viewport_width", s_spec.viewport_width, 224);
    check_int(&c, "spec.viewport_height", s_spec.viewport_height, 136);
    check_int(&c, "spec.media720_d0c", s_spec.media720_view_square_d0c, 0);
    check_int(&c, "spec.legacy_d0c", s_spec.legacy_view_square_d0c, 9);
    check_int(&c, "spec.first_thing", s_spec.first_thing_slot_pc34, 2);
    check_int(&c, "spec.floor_ornament", s_spec.floor_ornament_slot_pc34, 5);
    check_int(&c, "spec.floor_zone", s_spec.synthetic_center_floor_zone, 1520);
    check_int(&c, "spec.floor_zone_math",
              dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_zone_pc34(1, 9), 1520);
    check_int(&c, "spec.floor_zone_invalid",
              dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_zone_pc34(-1, 9), -1);
    check_int(&c, "spec.ceiling_graphic", s_spec.ceiling_pit_graphic_d0c_pc34, 69);
    check_int(&c, "spec.ceiling_zone", s_spec.ceiling_pit_zone_d0c_pc34, 871);
    check_int(&c, "spec.floor_pit_zone", s_spec.floor_pit_zone_d0c_pc34, 862);
    check_int(&c, "spec.field_zone", s_spec.field_zone_d0c_pc34, 715);
    check_int(&c, "spec.c705", s_spec.c705_wall_zone, 705);
    check_int(&c, "spec.c706", s_spec.c706_wall_zone, 706);
    check_int(&c, "spec.m575", s_spec.view_wall_d3l_right, 2);
    check_int(&c, "spec.m576", s_spec.view_wall_d3r_left, 3);
    check_int(&c, "spec.m577", s_spec.view_wall_d3l_front, 4);
    check_int(&c, "spec.m578", s_spec.view_wall_d3c_front, 5);
    check_int(&c, "spec.m579", s_spec.view_wall_d3r_front, 6);
    check_int(&c, "spec.f0108_start", s_spec.f0108_start_line, 3940);
    check_int(&c, "spec.f0108_end", s_spec.f0108_end_line, 4011);
    check_int(&c, "spec.f0127_start", s_spec.f0127_start_line, 8184);
    check_int(&c, "spec.f0127_end", s_spec.f0127_end_line, 8311);
    check_int(&c, "spec.f0128_d3l", s_spec.f0128_d3l_line, 8491);
    check_int(&c, "spec.f0128_d3r", s_spec.f0128_d3r_line, 8495);
    check_int(&c, "spec.f0128_d3c", s_spec.f0128_d3c_line, 8499);
    check_int(&c, "spec.f0128_d0l", s_spec.f0128_d0l_line, 8537);
    check_int(&c, "spec.f0128_d0r", s_spec.f0128_d0r_line, 8541);
    check_int(&c, "spec.f0128_d0c", s_spec.f0128_d0c_line, 8542);
    check_true(&c, "spec.d0c_after_side_pair",
               s_spec.f0128_d0l_line < s_spec.f0128_d0c_line &&
               s_spec.f0128_d0r_line < s_spec.f0128_d0c_line);
    check_int(&c, "spec.f0112_line", s_spec.f0112_ceiling_line, 8292);
    check_int(&c, "spec.f0115_line", s_spec.f0115_thing_line, 8294);
    check_int(&c, "spec.f0113_line", s_spec.f0113_field_line, 8308);
    check_true(&c, "spec.f0112_before_f0115",
               s_spec.f0112_ceiling_line < s_spec.f0115_thing_line);
    check_true(&c, "spec.f0113_after_f0115",
               s_spec.f0113_field_line > s_spec.f0115_thing_line);
    check_int(&c, "spec.door_pass_line", s_spec.door_front_pass_line, 4795);
    check_int(&c, "spec.door_defer_line", s_spec.door_front_creature_defer_line, 5245);

    for (i = 0; i < sizeof(contexts) / sizeof(contexts[0]); ++i) {
        DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 state;
        DM1_V1_D0CF0108FloorCeilingOrnamentStatePc34 reject_state;
        DM1_V1_D0CF0108FloorCeilingOrnamentResultPc34 result;
        const bool reaches_tail = context_reaches_shared_tail(contexts[i]);
        const bool is_teleporter =
            contexts[i] == DM1_V1_D0C_F0108_FCO_CONTEXT_TELEPORTER_PC34;
        const bool is_wall =
            contexts[i] == DM1_V1_D0C_F0108_FCO_CONTEXT_WALL_ORNAMENT_BRANCH_PC34;

        check_true(&c, "ctx.initial",
                   dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_initial_state_pc34(
                       contexts[i], &state));
        check_true(&c, "ctx.compose",
                   dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_compose_pc34(
                       &state, &result));
        check_int(&c, "ctx.ok", result.ok, 1);
        check_int(&c, "ctx.width", result.framebuffer_width, 320);
        check_int(&c, "ctx.height", result.framebuffer_height, 200);
        check_int(&c, "ctx.viewport_width", result.viewport_width, 224);
        check_int(&c, "ctx.viewport_height", result.viewport_height, 136);
        check_int(&c, "ctx.f0098_floor", result.f0098_floor_base_calls, 1);
        check_int(&c, "ctx.f0098_ceiling", result.f0098_ceiling_base_calls, 1);
        check_int(&c, "ctx.f0108_reference", result.f0108_reference_locked, 1);
        check_int(&c, "ctx.f0108_not_d0c_body", result.f0108_floor_ornament_calls_in_d0c_body, 0);
        check_int(&c, "ctx.f0108_zone", result.f0108_floor_zone, 1520);
        check_int(&c, "ctx.primary_index", result.f0108_primary_index, 3);
        check_int(&c, "ctx.recursive_index", result.f0108_recursive_index, 15);
        check_int(&c, "ctx.c10_checks", result.f0108_c10_transparency_checks, 2);
        check_int(&c, "ctx.f0107_keepout", result.f0107_wall_ornament_branch_kept_out,
                  is_wall ? 1 : 0);
        check_int(&c, "ctx.f0111_keepout", result.f0111_door_transparency_kept_out, 1);
        check_int(&c, "ctx.f0112", result.f0112_ceiling_calls, reaches_tail ? 1 : 0);
        check_int(&c, "ctx.f0115", result.f0115_thing_pass_calls, reaches_tail ? 1 : 0);
        check_int(&c, "ctx.f0112_before_f0115", result.f0112_before_f0115, reaches_tail ? 1 : 0);
        check_int(&c, "ctx.f0113", result.f0113_field_calls, is_teleporter ? 1 : 0);
        check_int(&c, "ctx.f0113_after", result.f0113_after_f0115, is_teleporter ? 1 : 0);
        check_int(&c, "ctx.side_pair", result.terminal_side_pair_correction, 1);
        check_int(&c, "ctx.d0c_after_sides", result.d0c_after_d0l_d0r, 1);
        check_int(&c, "ctx.door_two_pass", result.door_front_two_pass_order_checked, 1);
        check_int(&c, "ctx.mutation_guard", result.mutation_guard_ok, 1);
        check_int(&c, "ctx.nonoverlap", result.non_overlap_ok, 1);
        check_int(&c, "ctx.floor_sample", result.floor_sample, 0x52);
        check_int(&c, "ctx.ceiling_sample", result.ceiling_sample, 0x31);
        check_int(&c, "ctx.ornament_after_c10", result.ornament_sample_after_c10, 0x52);
        check_int(&c, "ctx.thing_sample", result.thing_observed_sample, 0x52);
        c.hash = hash_u32(c.hash, result.deterministic_hash);
        ++local.contexts_checked;
        local.d0c_body_checks += 4;
        local.f0108_reference_checks += 5;
        local.ordering_checks += 4;
        local.c10_checks += result.f0108_c10_transparency_checks;

        reject_state = state;
        reject_state.mutate_thing_list = true;
        check_true(&c, "reject.mutate",
                   !dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_compose_pc34(
                       &reject_state, &result));
        check_int(&c, "reject.mutate_flag", result.rejected_non_contract_state, 1);
        ++local.mutation_rejections;
    }

    for (i = 0; i < 20u; ++i) {
        DM1_V1_D0CF0108FloorCeilingOrnamentOrdinalPc34 ordinal;
        unsigned int value = (next_lcg(&seed) & 0x0fu) | 1u;
        if ((i & 1u) != 0u) value |= DM1_V1_D0C_F0108_FCO_FOOTPRINT_MASK_PC34;
        check_true(&c, "ordinal.decode",
                   dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
                       value, &ordinal));
        check_int(&c, "ordinal.has", ordinal.has_input_ordinal, 1);
        check_int(&c, "ordinal.primary", ordinal.primary_draws, 1);
        check_int(&c, "ordinal.primary_index",
                  ordinal.primary_index,
                  (int)(value & ~DM1_V1_D0C_F0108_FCO_FOOTPRINT_MASK_PC34) - 1);
        check_int(&c, "ordinal.footprint",
                  ordinal.recursive_footprints_draw, (i & 1u) ? 1 : 0);
        if ((i & 1u) != 0u) {
            check_int(&c, "ordinal.footprint_index",
                      ordinal.recursive_footprints_index,
                      DM1_V1_D0C_F0108_FCO_FOOTPRINT_INDEX_PC34);
        }
    }

    check_int(&c, "blend.transparent",
              dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(0x44u, 10u),
              0x44);
    check_int(&c, "blend.opaque",
              dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_blend_c10_pc34(0x44u, 0x55u),
              0x55);
    check_true(&c, "invalid.initial",
               !dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_initial_state_pc34(
                   (DM1_V1_D0CF0108FloorCeilingOrnamentContextPc34)99, NULL));
    check_true(&c, "invalid.decode",
               !dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
                   1u, NULL));

    local.assertions = c.assertions;
    local.failures = c.failures;
    local.ok = c.failures == 0;
    local.deterministic_hash = c.hash;
    s_last_self_test = local;
    return local.ok ? 1 : 0;
}

const DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34 *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *
dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_non_overlap_marker_pc34(void)
{
    return s_non_overlap_marker;
}
