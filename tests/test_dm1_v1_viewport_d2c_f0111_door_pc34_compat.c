#include "firestaff/dm1/v1/viewport/d2c_f0111_door_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               id, (unsigned)got, (unsigned)want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
    }
}

static uint32_t hash_and_check_trace(
    const char *prefix,
    const DM1_V1_D2CF0111DoorTracePc34 *trace,
    int opcode_count,
    int f0111_count,
    int f0104_count,
    int f0105_count,
    int f0107_keepout_count,
    int f0108_mask_keepout_count,
    int f0115_pass_count,
    int c10_count,
    int negative_count,
    uint8_t final_pixel,
    const char *anchor)
{
    char id[96];
    uint32_t hash = dm1_v1_viewport_d2c_f0111_door_hash_trace_pc34(trace);

    snprintf(id, sizeof(id), "%s.ok", prefix);
    expect_int(id, trace ? trace->ok : 0, 1, anchor);
    snprintf(id, sizeof(id), "%s.opcode_count", prefix);
    expect_int(id, trace ? trace->opcode_count : -1, opcode_count, anchor);
    snprintf(id, sizeof(id), "%s.f0111_count", prefix);
    expect_int(id, trace ? trace->f0111_count : -1, f0111_count, anchor);
    snprintf(id, sizeof(id), "%s.f0104_count", prefix);
    expect_int(id, trace ? trace->f0104_count : -1, f0104_count, anchor);
    snprintf(id, sizeof(id), "%s.f0105_count", prefix);
    expect_int(id, trace ? trace->f0105_count : -1, f0105_count, anchor);
    snprintf(id, sizeof(id), "%s.f0107_keepout", prefix);
    expect_int(id, trace ? trace->f0107_keepout_count : -1,
               f0107_keepout_count, anchor);
    snprintf(id, sizeof(id), "%s.f0108_mask_keepout", prefix);
    expect_int(id, trace ? trace->f0108_mask_keepout_count : -1,
               f0108_mask_keepout_count, anchor);
    snprintf(id, sizeof(id), "%s.f0115_passes", prefix);
    expect_int(id, trace ? trace->f0115_pass_count : -1, f0115_pass_count,
               anchor);
    snprintf(id, sizeof(id), "%s.c10_count", prefix);
    expect_int(id, trace ? trace->c10_transparent_count : -1, c10_count,
               "DEFS.H:2088 C10_COLOR_FLESH");
    snprintf(id, sizeof(id), "%s.negative_count", prefix);
    expect_int(id, trace ? trace->negative_count : -1, negative_count, anchor);
    snprintf(id, sizeof(id), "%s.final_pixel", prefix);
    expect_int(id, trace ? (int)trace->after_pass2 : -1, final_pixel, anchor);
    snprintf(id, sizeof(id), "%s.hash_nonzero", prefix);
    expect_int(id, hash != 0u, 1, "FNV-1a deterministic dispatch sequence");
    return hash;
}

static void test_evidence_and_metadata(void)
{
    const DM1_V1_D2CF0111DoorSourceLockPc34 *m =
        dm1_v1_viewport_d2c_f0111_door_source_lock_pc34();
    const char *e = dm1_v1_viewport_d2c_f0111_door_source_evidence_pc34();

    expect_int("metadata.present", m != NULL, 1, "metadata accessor");
    expect_int("metadata.contract_only", m ? m->contract_only : 0, 1,
               "contract-only=1");
    expect_int("metadata.no_game_data", m ? m->no_game_data : 0, 1,
               "no_game_data=1");
    expect_int("metadata.view_square", m ? m->view_square_d2c : -1, 6,
               "DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("metadata.depth", m ? m->relative_depth : -1, 2,
               "DUNVIEW.C:8520 relative depth");
    expect_int("metadata.lateral", m ? m->relative_lateral : -9, 0,
               "D2C center column");
    expect_int("metadata.door_zone", m ? m->door_zone_d2c : -1, 3760,
               "DUNVIEW.C:7339 M628_ZONE_DOOR_D2C");
    expect_int("metadata.c10", m ? m->transparent_color : -1, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("metadata.pass1", m ? (int)m->f0115_pass1_order : -1, 0x0218,
               "DEFS.H:2669 C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT");
    expect_int("metadata.pass2", m ? (int)m->f0115_pass2_order : -1, 0x0349,
               "DEFS.H:2672 C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT");
    expect_int("metadata.stack_after_d1c", m ? m->painter_stack_after_d1c : 0,
               1, "near-to-far stack D1C -> D2C -> D3C");
    expect_int("metadata.stack_before_d3c", m ? m->painter_stack_before_d3c : 0,
               1, "near-to-far stack D1C -> D2C -> D3C");
    expect_int("metadata.source_after_d3c",
               m ? m->f0128_source_line_order_after_d3c : 0, 1,
               "DUNVIEW.C:8498-8521 source order");
    expect_int("metadata.source_before_d1c",
               m ? m->f0128_source_line_order_before_d1c : 0, 1,
               "DUNVIEW.C:8520-8533 source order");
    expect_int("metadata.c705", m ? m->defs_zone_band_c705 : -1, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("metadata.c706", m ? m->defs_zone_band_c706 : -1, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");

    expect_contains("evidence.f0121", e, "F0121_DUNGEONVIEW_DrawSquareD2C:7244-7389",
                    "DUNVIEW.C D2C body");
    expect_contains("evidence.required_f0124", e, "F0124:7873-7937",
                    "required sibling source anchor");
    expect_contains("evidence.f0111", e, "F0111_DrawD2C:4311-4334",
                    "DUNVIEW.C F0111 generic body");
    expect_contains("evidence.f0104", e, "F0104/F0105:3113-3156/3185-3247",
                    "native C10 frame blits");
    expect_contains("evidence.f0107", e, "F0107:3502-3938",
                    "wall-ornament keepout");
    expect_contains("evidence.f0108", e, "F0108:3940-4011",
                    "floor-ornament MASK0x8000 keepout");
    expect_contains("evidence.f0115", e, "F0115:4547-4581",
                    "thing-pass cell ordering");
    expect_contains("evidence.dungeon", e, "DUNGEON.C F0163:1769-1838",
                    "thing-list source anchor");
    expect_contains("evidence.defs", e, "DEFS.H:2088,2596-2611,2662-2677,4045-4046,4139-4153",
                    "required DEFS.H anchors");
}

static void test_opcode_table(void)
{
    size_t i;
    int f0111_entries = 0;
    int negative_entries = 0;

    expect_int("opcode.count",
               (int)dm1_v1_viewport_d2c_f0111_door_opcode_count_pc34(), 17,
               "kTrace opcode table");
    expect_int("opcode.bounds",
               dm1_v1_viewport_d2c_f0111_door_opcode_at_pc34(17) == NULL, 1,
               "opcode accessor bounds");
    for (i = 0; i < dm1_v1_viewport_d2c_f0111_door_opcode_count_pc34(); ++i) {
        const DM1_V1_D2CF0111DoorOpcodeInfoPc34 *op =
            dm1_v1_viewport_d2c_f0111_door_opcode_at_pc34(i);
        expect_int("opcode.present", op != NULL, 1, "opcode table row");
        if (!op) continue;
        expect_int("opcode.ordinal", (int)op->opcode, (int)i + 1,
                   "opcode enum stays dense for deterministic hash");
        expect_contains("opcode.anchor", op->redmcsb_anchor, ".",
                        "every opcode carries source anchor text");
        if (op->opcode == DM1_V1_D2C_F0111_OP_F0111_DOOR_CLOSED_PC34 ||
            op->opcode == DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34) {
            ++f0111_entries;
        }
        if (op->opcode == DM1_V1_D2C_F0111_OP_NEGATIVE_NO_DOOR_FRONT_CELL_PC34 ||
            op->opcode == DM1_V1_D2C_F0111_OP_NEGATIVE_NO_NATIVE_FRAME_BLIT_PC34) {
            ++negative_entries;
        }
    }
    expect_int("opcode.f0111_entries", f0111_entries, 2,
               "closed and partly-open door-front states");
    expect_int("opcode.negative_entries", negative_entries, 2,
               "negative no-door-cell/no-native-blit guards");
}

static uint32_t test_dispatch_traces(void)
{
    DM1_V1_D2CF0111DoorTracePc34 closed = {0};
    DM1_V1_D2CF0111DoorTracePc34 partly = {0};
    DM1_V1_D2CF0111DoorTracePc34 transparent = {0};
    DM1_V1_D2CF0111DoorTracePc34 negative = {0};
    uint32_t combined = 2166136261u;

    const DM1_V1_D2CF0111DoorScenarioPc34 closed_scenario = {
        6, 17, 4, 1u, 3, 0x20u, 0x31u, 0x41u, 0x42u, 0x51u, 0x61u, 0x71u
    };
    const DM1_V1_D2CF0111DoorScenarioPc34 partly_scenario = {
        6, 17, 2, 0x8000u, 6, 0x44u, 10u, 10u, 0x22u, 10u, 0x33u, 10u
    };
    const DM1_V1_D2CF0111DoorScenarioPc34 transparent_scenario = {
        6, 17, 4, 0x8001u, 1, 0x66u, 10u, 10u, 10u, 10u, 10u, 10u
    };
    const DM1_V1_D2CF0111DoorScenarioPc34 negative_scenario = {
        6, 1, 4, 0u, 0, 0x88u, 10u, 10u, 10u, 10u, 10u, 10u
    };

    expect_int("trace.closed.run",
               dm1_v1_viewport_d2c_f0111_door_trace_pc34(
                   &closed_scenario, &closed),
               1, "DUNVIEW.C:7313-7342 closed D2C door front");
    expect_int("trace.partly.run",
               dm1_v1_viewport_d2c_f0111_door_trace_pc34(
                   &partly_scenario, &partly),
               1, "DUNVIEW.C F0111:4311-4334 partly-open D2C door front");
    expect_int("trace.transparent.run",
               dm1_v1_viewport_d2c_f0111_door_trace_pc34(
                   &transparent_scenario, &transparent),
               1, "DEFS.H:2088 C10 transparent all layers");
    expect_int("trace.negative.run",
               dm1_v1_viewport_d2c_f0111_door_trace_pc34(
                   &negative_scenario, &negative),
               1, "negative: not a D2C door-front cell");

    combined = fnv1a_u32(combined,
        hash_and_check_trace("closed", &closed, 12, 1, 1, 1, 1, 0, 2, 0, 0,
                             0x71u, "DUNVIEW.C:7313-7342"));
    expect_int("closed.first", closed.opcodes[0],
               DM1_V1_D2C_F0111_OP_F0128_STACK_D1C_NEARER_PC34,
               "painter stack after D1C");
    expect_int("closed.d2c", closed.opcodes[1],
               DM1_V1_D2C_F0111_OP_F0128_STACK_D2C_PC34,
               "D2C stack slot");
    expect_int("closed.d3c", closed.opcodes[2],
               DM1_V1_D2C_F0111_OP_F0128_STACK_D3C_FARTHER_PC34,
               "painter stack before D3C");
    expect_int("closed.pass1_cells", (int)closed.pass1_cells, 0x21,
               "DEFS.H:2669 back-left/back-right after marker strip");
    expect_int("closed.pass2_cells", (int)closed.pass2_cells, 0x34,
               "DEFS.H:2672 front-right/front-left after marker strip");

    combined = fnv1a_u32(combined,
        hash_and_check_trace("partly", &partly, 16, 1, 1, 1, 1, 1, 2, 3, 0,
                             0x22u, "DUNVIEW.C F0111:4311-4334"));
    expect_int("partly.has_partly_opcode",
               partly.opcodes[12],
               DM1_V1_D2C_F0111_OP_F0111_DOOR_PARTLY_OPEN_PC34,
               "DUNVIEW.C:4311-4334 partly-open branch");
    expect_int("partly.after_left", partly.after_left_frame, 0x33,
               "F0104 C10 preserves pass1 destination");
    expect_int("partly.after_door", partly.after_door, 0x22,
               "F0111 C10 preserves F0105 destination");

    combined = fnv1a_u32(combined,
        hash_and_check_trace("transparent", &transparent, 17, 1, 1, 1, 1, 1,
                             2, 4, 0, 0x66u,
                             "C10_COLOR_FLESH transparent full stack"));
    expect_int("transparent.after_left", transparent.after_left_frame, 0x66,
               "F0104 native C10 transparent blit");
    expect_int("transparent.after_right", transparent.after_right_frame, 0x66,
               "F0105 flipped native C10 transparent blit");
    expect_int("transparent.after_door", transparent.after_door, 0x66,
               "F0111 C10 transparent door body");

    combined = fnv1a_u32(combined,
        hash_and_check_trace("negative", &negative, 7, 0, 0, 0, 0, 0, 0, 0, 2,
                             0x88u,
                             "negative: door transparency not on non-door cell"));
    expect_int("negative.no_f0104", negative.f0104_count, 0,
               "no F0104 native blit without D2C door-front cell");
    expect_int("negative.no_f0105", negative.f0105_count, 0,
               "no F0105 native blit without D2C door-front cell");
    expect_u32("hash.null", dm1_v1_viewport_d2c_f0111_door_hash_trace_pc34(NULL),
               0u, "hash guard");
    return combined;
}

int main(void)
{
    uint32_t hash;

    printf("probe=firestaff_dm1_v1_viewport_d2c_f0111_door\n");
    test_evidence_and_metadata();
    test_opcode_table();
    hash = test_dispatch_traces();
    printf("deterministic_hash=0x%08x\n", (unsigned)hash);
    printf("summary: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
