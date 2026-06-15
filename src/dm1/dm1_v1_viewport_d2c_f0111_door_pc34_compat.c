#include "firestaff/dm1/v1/viewport/d2c_f0111_door_pc34_compat.h"

#include <string.h>

enum {
    DM1_D2C_VIEW_SQUARE = 6,
    DM1_DOOR_FRONT_ELEMENT = 17,
    DM1_D2C_DOOR_ZONE = 3760,
    DM1_D2C_DOOR_FRAME_TOP_ZONE = 730,
    DM1_D2C_DOOR_FRAME_LEFT_ZONE = 724,
    DM1_D2C_DOOR_FRAME_RIGHT_ZONE = 725,
    DM1_D2C_DOOR_BITMAP = 694,
    DM1_D2C_DOOR_ORNAMENT_VIEW = 1,
    DM1_D2C_PASS1_ORDER = 0x0218,
    DM1_D2C_PASS2_ORDER = 0x0349,
    DM1_C705_ZONE_WALL_D3L = 705,
    DM1_C706_ZONE_WALL_D3R = 706
};

/*
 * ReDMCSB source lock for the DM1 V1 D2C F0111 door-front contract.
 *
 * Anchors required by the gate:
 * - DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C:7244-7389 is the local D2C
 *   body. Door-front lines 7313-7342 call F0108, F0115 pass 1, F0104/F0105
 *   native C10 door-frame blits, F0111, then F0115 pass 2. The request also
 *   requires the sibling DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C:
 *   7873-7937 anchor; that range is cited in the metadata as a non-D2C
 *   line-map guard because it has the same door-front split for D1C.
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542 dispatches source lines
 *   D3C 8498-8499, D2C 8520-8521, D1C 8532-8533; this model also records the
 *   near-to-far painter-stack contract as D1C -> D2C -> D3C.
 * - DUNVIEW.C F0111_DrawD2C:4311-4334 is the generic F0111 partly-open body
 *   that selects left/right horizontal door halves and keeps C10 transparent.
 * - DUNVIEW.C F0104/F0105:3113-3156/3185-3247 are the native and flipped C10
 *   transparent blits used by D2C frame lines 7328-7330.
 * - DUNVIEW.C F0107:3502-3938 is kept out of the D2C door-front cell; the
 *   D2C wall-only branch calls it at 7308 and returns before the door case.
 * - DUNVIEW.C F0108:3940-4011 handles floor ornaments and the MASK0x8000
 *   footprint recursion before the door-front F0115/F0111 sequence.
 * - DUNVIEW.C F0115:4547-4581, 4923, 5180-5188, 5211-5214, 5668-5671 pins
 *   thing-pass cell ordering and C10 object/projectile transparency.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523 provide
 *   thing-list and square-aspect provenance.
 * - DEFS.H:2088,2596-2611,2662-2677,4045-4046,4139-4153 pins C10, view
 *   squares, cell orders, C705/C706, and the neighboring zone band.
 */
static const char s_source_evidence[] =
    "DM1 V1 D2C F0111 door source-lock; contract_only=1, no_game_data=1. "
    "ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C:7244-7389 is the "
    "actual local D2C body. Its C17_ELEMENT_DOOR_FRONT branch at 7313-7342 "
    "draws F0108 at 7314, F0115 pass 1 with C0x0218 at 7315, native D2C "
    "door-frame blits through F0104/F0105 at 7328-7330, F0111 at "
    "7336-7339 with G0694/C1_VIEW_DOOR_ORNAMENT/M628_ZONE_DOOR_D2C, and "
    "then F0115 pass 2 via C0x0349 at 7341/7368. DUNVIEW.C F0124:"
    "7873-7937 is cited as the required sibling line-map guard; in this "
    "ReDMCSB tree it is D1C, not D2C, but it proves the same door-front "
    "F0115 split pattern is not being borrowed as D2C behavior. DUNVIEW.C "
    "F0128:8318-8542 dispatches D3C at 8498-8499, D2C at 8520-8521, and "
    "D1C at 8532-8533; the painter stack records D2C after nearer D1C and "
    "before farther D3C. DUNVIEW.C F0111_DrawD2C:4311-4334 pins the "
    "partly-open left/right horizontal halves and C10_COLOR_FLESH "
    "transparency. DUNVIEW.C F0104/F0105:3113-3156/3185-3247 pins native "
    "and flipped C10 transparent blits. DUNVIEW.C F0107:3502-3938 is a "
    "D2C wall-only keepout for door-front cells; DUNVIEW.C F0108:3940-4011 "
    "pins MASK0x8000_FOOTPRINTS keepout and C10 floor blits. DUNVIEW.C "
    "F0115:4547-4581/4923/5180-5188/5211-5214/5668-5671 pins cell-order "
    "and thing-pass C10 behavior. DUNGEON.C F0163:1769-1838, F0164:"
    "1840-1905, F0172:2466-2523 anchor thing-list and square-aspect "
    "inputs. DEFS.H:2088,2596-2611,2662-2677,4045-4046,4139-4153 pins "
    "C10, M603 D2C, door-pass cell orders, C705/C706, and the zone band.";

static const DM1_V1_D2CF0111DoorSourceLockPc34 s_lock = {
    1,
    1,
    1,
    DM1_D2C_VIEW_SQUARE,
    2,
    0,
    DM1_DOOR_FRONT_ELEMENT,
    DM1_D2C_DOOR_ZONE,
    DM1_D2C_DOOR_FRAME_TOP_ZONE,
    DM1_D2C_DOOR_FRAME_LEFT_ZONE,
    DM1_D2C_DOOR_FRAME_RIGHT_ZONE,
    DM1_D2C_DOOR_BITMAP,
    DM1_D2C_DOOR_ORNAMENT_VIEW,
    DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH,
    DM1_D2C_PASS1_ORDER,
    DM1_D2C_PASS2_ORDER,
    1,
    1,
    1,
    1,
    DM1_C705_ZONE_WALL_D3L,
    DM1_C706_ZONE_WALL_D3R,
    "ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C:7244-7389; door branch 7313-7342",
    "ReDMCSB DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C:7873-7937 sibling required anchor",
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8542; D2C 8520-8521",
    "ReDMCSB DUNVIEW.C F0111_DrawD2C:4311-4334",
    "ReDMCSB DUNVIEW.C F0104/F0105 native C10 blit:3113-3156/3185-3247",
    "ReDMCSB DUNVIEW.C F0107 wall-ornament keepout:3502-3938",
    "ReDMCSB DUNVIEW.C F0108 floor-ornament MASK0x8000 keepout:3940-4011",
    "ReDMCSB DUNVIEW.C F0115 thing-pass ordering:4547-4581,4923,5180-5188,5211-5214,5668-5671",
    "ReDMCSB DUNGEON.C F0163:1769-1838 + F0164:1840-1905 + F0172:2466-2523",
    "ReDMCSB DEFS.H:2088,2596-2611,2662/2668-2677,4045-4046,4139-4153"
};

static const DM1_V1_D2CF0111DoorOpcodeInfoPc34 s_opcodes[] = {
    { DM1_V1_D2C_F0111_OP_F0128_STACK_D1C_NEARER_PC34,
      "painter stack nearer D1C before D2C", "DUNVIEW.C F0128:8532-8533" },
    { DM1_V1_D2C_F0111_OP_F0128_STACK_D2C_PC34,
      "D2C F0128/F0121 dispatch", "DUNVIEW.C F0128:8520-8521" },
    { DM1_V1_D2C_F0111_OP_F0128_STACK_D3C_FARTHER_PC34,
      "painter stack farther D3C after D2C", "DUNVIEW.C F0128:8498-8499" },
    { DM1_V1_D2C_F0111_OP_F0121_D2C_BODY_PC34,
      "F0121 D2C body", "DUNVIEW.C F0121:7244-7389" },
    { DM1_V1_D2C_F0111_OP_F0108_FLOOR_ORNAMENT_PC34,
      "F0108 D2C floor ornament", "DUNVIEW.C:7314; F0108:3940-4011" },
    { DM1_V1_D2C_F0111_OP_F0108_MASK_0X8000_KEEP_OUT_PC34,
      "F0108 MASK0x8000 footprint keepout", "DUNVIEW.C F0108:3959-3963" },
    { DM1_V1_D2C_F0111_OP_F0115_PASS1_BACK_CELLS_PC34,
      "F0115 door pass 1 back cells", "DUNVIEW.C:7315; DEFS.H:2669" },
    { DM1_V1_D2C_F0111_OP_F0104_LEFT_FRAME_C10_PC34,
      "F0104 native left frame C10 blit", "DUNVIEW.C:7328-7329; F0104:3113-3156" },
    { DM1_V1_D2C_F0111_OP_F0105_RIGHT_FRAME_C10_PC34,
      "F0105 flipped right frame C10 blit", "DUNVIEW.C:7330; F0105:3185-3247" },
    { DM1_V1_D2C_F0111_OP_F0111_DOOR_CLOSED_PC34,
      "F0111 closed door-front", "DUNVIEW.C:7336-7339; F0111:4311-4334" },
    { DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34,
      "F0111 partly-open door-front", "DUNVIEW.C F0111:4311-4334" },
    { DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34,
      "F0111/F0104/F0105 C10 preserves destination", "DEFS.H:2088" },
    { DM1_V1_D2C_F0111_OP_F0115_PASS2_FRONT_CELLS_PC34,
      "F0115 door pass 2 front cells", "DUNVIEW.C:7341/7368; DEFS.H:2672" },
    { DM1_V1_D2C_F0111_OP_F0107_WALL_ORNAMENT_KEEP_OUT_PC34,
      "F0107 wall ornament kept out of door front", "DUNVIEW.C:7308-7313" },
    { DM1_V1_D2C_F0111_OP_ZONE_BAND_C705_C706_PC34,
      "DEFS C705/C706 neighboring zone band", "DEFS.H:4045-4046/4139-4153" },
    { DM1_V1_D2C_F0111_OP_NEGATIVE_NO_DOOR_FRONT_CELL_PC34,
      "no F0111 when D2C cell is not a door front", "DUNVIEW.C F0121:7313 case guard" },
    { DM1_V1_D2C_F0111_OP_NEGATIVE_NO_NATIVE_FRAME_BLIT_PC34,
      "no F0104/F0105 door-frame blit without D2C door-front cell", "DUNVIEW.C:7328-7330 case guard" }
};

static void record_opcode(
    DM1_V1_D2CF0111DoorTracePc34 *trace,
    DM1_V1_D2CF0111DoorOpcodePc34 opcode)
{
    if (trace->opcode_count < DM1_V1_D2C_F0111_DOOR_PC34_MAX_TRACE) {
        trace->opcodes[trace->opcode_count++] = opcode;
    }
    switch (opcode) {
    case DM1_V1_D2C_F0111_OP_F0104_LEFT_FRAME_C10_PC34:
        ++trace->f0104_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0105_RIGHT_FRAME_C10_PC34:
        ++trace->f0105_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0111_DOOR_CLOSED_PC34:
    case DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34:
        ++trace->f0111_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0107_WALL_ORNAMENT_KEEP_OUT_PC34:
        ++trace->f0107_keepout_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0108_MASK_0X8000_KEEP_OUT_PC34:
        ++trace->f0108_mask_keepout_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0115_PASS1_BACK_CELLS_PC34:
    case DM1_V1_D2C_F0111_OP_F0115_PASS2_FRONT_CELLS_PC34:
        ++trace->f0115_pass_count;
        break;
    case DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34:
        ++trace->c10_transparent_count;
        break;
    case DM1_V1_D2C_F0111_OP_NEGATIVE_NO_DOOR_FRONT_CELL_PC34:
    case DM1_V1_D2C_F0111_OP_NEGATIVE_NO_NATIVE_FRAME_BLIT_PC34:
        ++trace->negative_count;
        break;
    default:
        break;
    }
}

uint8_t dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH ?
        destination_pixel : source_pixel;
}

const char *dm1_v1_viewport_d2c_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_D2CF0111DoorSourceLockPc34 *
dm1_v1_viewport_d2c_f0111_door_source_lock_pc34(void)
{
    return &s_lock;
}

size_t dm1_v1_viewport_d2c_f0111_door_opcode_count_pc34(void)
{
    return sizeof(s_opcodes) / sizeof(s_opcodes[0]);
}

const DM1_V1_D2CF0111DoorOpcodeInfoPc34 *
dm1_v1_viewport_d2c_f0111_door_opcode_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2c_f0111_door_opcode_count_pc34()) return 0;
    return &s_opcodes[index];
}

int dm1_v1_viewport_d2c_f0111_door_trace_pc34(
    const DM1_V1_D2CF0111DoorScenarioPc34 *scenario,
    DM1_V1_D2CF0111DoorTracePc34 *out_trace)
{
    if (!scenario || !out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->ok = 1;
    out_trace->after_floor = scenario->destination_pixel;
    out_trace->after_pass1 = scenario->destination_pixel;
    out_trace->after_left_frame = scenario->destination_pixel;
    out_trace->after_right_frame = scenario->destination_pixel;
    out_trace->after_door = scenario->destination_pixel;
    out_trace->after_pass2 = scenario->destination_pixel;

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0128_STACK_D1C_NEARER_PC34);
    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0128_STACK_D2C_PC34);
    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0128_STACK_D3C_FARTHER_PC34);
    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0121_D2C_BODY_PC34);
    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_ZONE_BAND_C705_C706_PC34);

    if (scenario->view_square_index != DM1_D2C_VIEW_SQUARE ||
        scenario->element != DM1_DOOR_FRONT_ELEMENT) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_NEGATIVE_NO_DOOR_FRONT_CELL_PC34);
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_NEGATIVE_NO_NATIVE_FRAME_BLIT_PC34);
        return 1;
    }

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0107_WALL_ORNAMENT_KEEP_OUT_PC34);
    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0108_FLOOR_ORNAMENT_PC34);
    if ((scenario->floor_ornament_ordinal &
         DM1_V1_D2C_F0111_DOOR_PC34_MASK0X8000_FOOTPRINTS) != 0u) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0108_MASK_0X8000_KEEP_OUT_PC34);
    }
    out_trace->after_floor =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            scenario->destination_pixel, scenario->floor_pixel);

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0115_PASS1_BACK_CELLS_PC34);
    out_trace->pass1_cells = 0x21u;
    out_trace->after_pass1 =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            out_trace->after_floor, scenario->pass1_pixel);

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0104_LEFT_FRAME_C10_PC34);
    out_trace->after_left_frame =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            out_trace->after_pass1, scenario->left_frame_pixel);
    if (scenario->left_frame_pixel == DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34);
    }

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0105_RIGHT_FRAME_C10_PC34);
    out_trace->after_right_frame =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            out_trace->after_left_frame, scenario->right_frame_pixel);
    if (scenario->right_frame_pixel == DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34);
    }

    if (scenario->door_state >= DM1_V1_D2C_F0111_DOOR_STATE_ONE_FOURTH_PC34 &&
        scenario->door_state <= DM1_V1_D2C_F0111_DOOR_STATE_THREE_FOURTHS_PC34) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34);
    } else {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_DOOR_CLOSED_PC34);
    }
    out_trace->after_door =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            out_trace->after_right_frame, scenario->door_pixel);
    if (scenario->door_pixel == DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34);
    }

    record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0115_PASS2_FRONT_CELLS_PC34);
    out_trace->pass2_cells = 0x34u;
    out_trace->after_pass2 =
        dm1_v1_viewport_d2c_f0111_door_blend_c10_pc34(
            out_trace->after_door, scenario->pass2_pixel);
    if (scenario->pass2_pixel == DM1_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH) {
        record_opcode(out_trace, DM1_V1_D2C_F0111_OP_F0111_C10_TRANSPARENT_PC34);
    }
    return 1;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

uint32_t dm1_v1_viewport_d2c_f0111_door_hash_trace_pc34(
    const DM1_V1_D2CF0111DoorTracePc34 *trace)
{
    uint32_t h = 2166136261u;
    int i;

    if (!trace) return 0u;
    h = hash_u32(h, (uint32_t)trace->ok);
    h = hash_u32(h, (uint32_t)trace->opcode_count);
    for (i = 0; i < trace->opcode_count; ++i) {
        h = hash_u32(h, (uint32_t)trace->opcodes[i]);
    }
    h = hash_u32(h, (uint32_t)trace->f0111_count);
    h = hash_u32(h, (uint32_t)trace->f0104_count);
    h = hash_u32(h, (uint32_t)trace->f0105_count);
    h = hash_u32(h, (uint32_t)trace->f0107_keepout_count);
    h = hash_u32(h, (uint32_t)trace->f0108_mask_keepout_count);
    h = hash_u32(h, (uint32_t)trace->f0115_pass_count);
    h = hash_u32(h, (uint32_t)trace->c10_transparent_count);
    h = hash_u32(h, (uint32_t)trace->negative_count);
    h = hash_u32(h, (uint32_t)trace->after_pass2);
    h = hash_u32(h, trace->pass1_cells);
    h = hash_u32(h, trace->pass2_cells);
    return h;
}
