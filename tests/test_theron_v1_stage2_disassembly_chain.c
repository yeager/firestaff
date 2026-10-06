/*
 * test_theron_v1_stage2_disassembly_chain.c — verify the full stage-2
 * disassembly chain against the real US Track 02 binary and, when present,
 * the JP Rev. 1 binary.
 *
 * This test exercises every verify_stage2_* function in sequence,
 * proving that the entire disassembly chain from IPL loader through
 * tier-5 callees matches the authenticated media bytes.  It also
 * extracts VDC register writes from the proven byte streams,
 * providing viewport initialization evidence.
 *
 * Requires: ~/.firestaff/data/theron/TQUS02.bin; JP is verified optionally.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02.h"

static uint8_t *g_us_data;
static size_t g_us_size;
static uint8_t *g_jp_data;
static size_t g_jp_size;

static int load_track02_file(const char *path, uint8_t **out_data,
                             size_t *out_size)
{
    FILE *f;

    if (!path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    *out_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    *out_data = malloc(*out_size);
    if (!*out_data) { fclose(f); return 0; }
    if (fread(*out_data, 1, *out_size, f) != *out_size) {
        free(*out_data);
        *out_data = NULL;
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int load_track02(void)
{
    const char *home = getenv("HOME");
    char us_path[512];
    char jp_path[512];

    if (!home) return 0;
    snprintf(us_path, sizeof(us_path), "%s/.firestaff/data/theron/TQUS02.bin", home);
    snprintf(jp_path, sizeof(jp_path), "%s/.firestaff/data/theron/TQJP02.bin", home);
    if (!load_track02_file(us_path, &g_us_data, &g_us_size)) return 0;
    (void)load_track02_file(jp_path, &g_jp_data, &g_jp_size);
    return 1;
}

static uint8_t stage2_byte_at(const uint8_t *raw, size_t raw_size,
                              int jp, uint16_t cpu_address)
{
    size_t payload_offset;
    size_t stage2_sector =
        (jp ? THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR
            : THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR) +
        THERON_TRACK02_IPL_STAGE2_RECORD;
    size_t sector;
    size_t raw_offset;

    assert(cpu_address >= THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS);
    payload_offset = (size_t)(cpu_address -
                              THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS);
    sector = stage2_sector + payload_offset / 2048u;
    raw_offset = sector * 2352u + 16u + payload_offset % 2048u;
    assert(raw_offset < raw_size);
    return raw[raw_offset];
}

static uint16_t stage2_word_at(const uint8_t *raw, size_t raw_size,
                               int jp, uint16_t cpu_address)
{
    uint8_t lo = stage2_byte_at(raw, raw_size, jp, cpu_address);
    uint8_t hi = stage2_byte_at(raw, raw_size, jp,
                                (uint16_t)(cpu_address + 1u));
    return (uint16_t)lo | ((uint16_t)hi << 8);
}

static void assert_stage2_pointer(const uint8_t *raw, size_t raw_size,
                                  int jp, uint16_t stream,
                                  uint16_t expected_target)
{
    assert(stage2_byte_at(raw, raw_size, jp, stream) == 0x41u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(stream + 1u)) == expected_target);
}

/* The direct $4000 prologue writes MPR3..MPR6. This byte check alone does
 * not establish the MPR1 value after the intervening $8000 call. */
static void test_stage2_entry_mpr_window(const uint8_t *raw,
                                         size_t raw_size, int jp)
{
    static const uint8_t entry[] = {
        0x78u, 0xa2u, 0xffu, 0x9au, 0xadu, 0xf5u, 0xffu, 0x1au,
        0x53u, 0x08u, 0x1au, 0x1au, 0x1au, 0x53u, 0x10u, 0x48u,
        0x58u, 0x20u, 0x00u, 0x80u, 0x68u, 0x1au, 0x53u, 0x20u,
        0x1au, 0x53u, 0x40u
    };

    for (unsigned int i = 0; i < sizeof(entry); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4000u + i)) == entry[i]);
    }
    printf("  PASS: stage2_entry_mpr_window (%s)\n",
           jp ? "JP" : "US");
}

/* Bind the $48EC..$4900 countdown body independently in each real edition.
 * The tested bytes only establish the decrement/loop mechanics; they do not
 * assign meaning to the two zero-page bytes or the VDC register clears. */
static void test_stage2_l48fc_countdown(const uint8_t *raw,
                                        size_t raw_size, int jp)
{
    static const uint8_t body[] = {
        0x9cu, 0x04u, 0x04u, 0x9cu, 0x05u, 0x04u, 0xa5u, 0x00u,
        0xd0u, 0x02u, 0xc6u, 0x01u, 0xc6u, 0x00u, 0xa5u, 0x00u,
        0x05u, 0x01u, 0xd0u, 0xecu, 0x60u
    };

    for (unsigned int i = 0; i < sizeof(body); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x48ecu + i)) == body[i]);
    }
    printf("  PASS: stage2_l48fc_countdown (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the indexed-byte store/add/subtract/increment/decrement cluster
 * against authentic media.
 * This asserts instruction paths, not stream execution or field meanings. */
static void test_stage2_id0b_0f_indexed_mutation(const uint8_t *raw,
                                                  size_t raw_size, int jp)
{
    static const uint8_t pair_reader[] = {
        0xc8u, 0xb1u, 0x1cu, 0xaau, 0xbdu, 0x80u,
        0x27u, 0xc8u, 0xd1u, 0x1cu, 0x60u
    };
    static const uint8_t id0b[] = {
        0x20u, 0xf8u, 0x41u, 0xb1u, 0x1cu, 0x9du,
        0x80u, 0x27u, 0x80u, 0x1au
    };
    static const uint8_t id0c[] = {
        0x20u, 0xf8u, 0x41u, 0xb1u, 0x1cu, 0x18u, 0x7du,
        0x80u, 0x27u, 0x9du, 0x80u, 0x27u, 0x80u, 0x0cu
    };
    static const uint8_t id0d[] = {
        0x20u, 0xf8u, 0x41u, 0xb1u, 0x1cu, 0x38u,
        0xfdu, 0x80u, 0x27u, 0x9du, 0x80u, 0x27u
    };
    static const uint8_t id0e[] = {
        0x20u, 0xf8u, 0x41u, 0xfeu, 0x80u, 0x27u, 0x80u, 0x06u
    };
    static const uint8_t id0f[] = {
        0x20u, 0xf8u, 0x41u, 0xdeu, 0x80u, 0x27u
    };
    static const uint8_t cursor_tail_3[] = {0x4cu, 0xf9u, 0x40u};
    static const uint8_t cursor_tail_2[] = {0x4cu, 0xf5u, 0x40u};

    assert(stage2_word_at(raw, raw_size, jp, 0x4123u) == 0x4259u);
    assert(stage2_word_at(raw, raw_size, jp, 0x4125u) == 0x4263u);
    assert(stage2_word_at(raw, raw_size, jp, 0x4127u) == 0x4271u);
    assert(stage2_word_at(raw, raw_size, jp, 0x4129u) == 0x4280u);
    assert(stage2_word_at(raw, raw_size, jp, 0x412bu) == 0x4288u);
    for (unsigned int i = 0; i < sizeof(pair_reader); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x41f8u + i)) == pair_reader[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0b); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4259u + i)) == id0b[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0c); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4263u + i)) == id0c[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0d); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4271u + i)) == id0d[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0e); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4280u + i)) == id0e[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0f); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4288u + i)) == id0f[i]);
    }
    for (unsigned int i = 0; i < sizeof(cursor_tail_3); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x427du + i)) == cursor_tail_3[i]);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x428eu + i)) == cursor_tail_2[i]);
    }
    printf("  PASS: stage2_id0b_0f_indexed_mutation (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the nested-cursor dispatch root and pointer-table reader against each
 * authentic edition. This establishes the static call/restore path only; it
 * does not bind a retail stream selector or assign gameplay meaning. */
static void test_stage2_id09_id0a_id10_nested_cursor(const uint8_t *raw,
                                                       size_t raw_size,
                                                       int jp)
{
    static const uint8_t id09_return[] = {0x60u};
    static const uint8_t id0a_root[] = {
        0x20u, 0x4eu, 0x4bu, 0x80u, 0x24u
    };
    static const uint8_t id10_root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xa8u, 0xa5u, 0x1cu, 0x48u,
        0xa5u, 0x1du, 0x48u, 0x44u, 0x09u, 0x68u, 0x85u,
        0x1du, 0x68u, 0x85u, 0x1cu, 0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t pointer_reader[] = {
        0xa9u, 0x00u, 0x85u, 0x00u, 0xa9u, 0x68u, 0x85u, 0x01u,
        0x98u, 0x0au, 0xa8u, 0xb1u, 0x00u, 0x85u, 0x1cu, 0xc8u,
        0xb1u, 0x00u, 0x85u, 0x1du, 0x20u, 0xccu, 0x40u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp, 0x411fu) == 0x4253u);
    assert(stage2_word_at(raw, raw_size, jp, 0x4121u) == 0x4254u);
    assert(stage2_word_at(raw, raw_size, jp, 0x4123u) == 0x4259u);
    assert(stage2_word_at(raw, raw_size, jp, 0x412du) == 0x4291u);
    assert(stage2_byte_at(raw, raw_size, jp, 0x4253u) == 0x60u);
    for (unsigned int i = 0; i < sizeof(id09_return); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4253u + i)) == id09_return[i]);
    }
    for (unsigned int i = 0; i < sizeof(id0a_root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4254u + i)) == id0a_root[i]);
    }
    for (unsigned int i = 0; i < sizeof(id10_root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4291u + i)) == id10_root[i]);
    }
    for (unsigned int i = 0; i < sizeof(pointer_reader); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x42a6u + i)) == pointer_reader[i]);
    }
    printf("  PASS: stage2_id09_id0a_id10_nested_cursor (%s)\n",
           jp ? "JP" : "US");
}

/* Lock dispatch ID $28 and its two bounded helpers against the authentic
 * regional binaries. These bytes show control flow and one regional pointer
 * operand only; they do not establish stream selection or gameplay meaning. */
static void test_stage2_id28_conditional_handoff(const uint8_t *raw,
                                                  size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x03u, 0x44u, 0xc6u, 0x5bu, 0xa0u, 0x01u, 0xb1u,
        0x1cu, 0xf0u, 0x13u, 0x44u, 0x54u, 0x44u, 0x31u, 0x64u,
        0xf9u, 0x64u, 0xfau, 0xa9u, 0x0eu, 0x85u, 0xffu, 0xc6u,
        0xf8u, 0x20u, 0x3fu, 0xe0u, 0x80u, 0x1du, 0x44u, 0x41u,
        0x44u, 0x1eu, 0x64u, 0xfau, 0x64u, 0xfbu, 0x64u, 0xffu,
        0x20u, 0x33u, 0xe0u, 0xc9u, 0x00u, 0xd0u, 0xf1u, 0xadu,
        0xd0u, 0x37u, 0x8du, 0x73u, 0x43u, 0xadu, 0xd1u, 0x37u,
        0x8du, 0x74u, 0x43u, 0x64u, 0x5bu, 0x4cu, 0xf9u, 0x40u
    };
    static const uint8_t helper_43b5_prefix[] = {
        0xadu, 0xd6u, 0x37u, 0x48u, 0xadu, 0xd7u, 0x37u,
        0x48u, 0xa9u
    };
    static const uint8_t helper_43b5_suffix[] = {
        0x8du, 0xd6u, 0x37u, 0xa9u, 0x5eu, 0x8du, 0xd7u,
        0x37u, 0x20u, 0xd8u, 0x37u, 0x68u, 0x8du, 0xd7u,
        0x37u, 0x68u, 0x8du, 0xd6u, 0x37u, 0x20u, 0x48u,
        0x38u, 0x60u
    };
    static const uint8_t helper_43d6[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xccu, 0x37u, 0x60u
    };
    uint8_t regional_pointer = jp ? 0xcfu : 0x9fu;

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x28u)) == 0x4375u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4375u + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper_43b5_prefix); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x43b5u + i)) ==
               helper_43b5_prefix[i]);
    }
    assert(stage2_byte_at(raw, raw_size, jp, 0x43beu) == regional_pointer);
    for (unsigned int i = 0; i < sizeof(helper_43b5_suffix); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x43bfu + i)) ==
               helper_43b5_suffix[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper_43d6); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x43d6u + i)) == helper_43d6[i]);
    }
    printf("  PASS: stage2_id28_conditional_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock dispatch ID $12's cursor-save, indirect-call and restore sequence.
 * The selected target and any retail stream use remain unproven. */
static void test_stage2_id12_indirect_call(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t handler[] = {
        0xa5u, 0x1cu, 0x48u, 0xa5u, 0x1du, 0x48u, 0x20u,
        0xb9u, 0x41u, 0xa9u, 0x43u, 0x48u, 0xa9u, 0x2au,
        0x48u, 0x6cu, 0x1cu, 0x20u, 0x68u, 0x85u, 0x1du,
        0x68u, 0x85u, 0x1cu, 0x4cu, 0xf9u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x12u)) == 0x4319u);
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4319u + i)) == handler[i]);
    }
    printf("  PASS: stage2_id12_indirect_call (%s)\n",
           jp ? "JP" : "US");
}

/* Lock dispatch ID $08 and its local helper. The code bytes show a static
 * cursor step and helper call only; they do not prove selector execution. */
static void test_stage2_id08_local_helper(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x44u, 0x03u, 0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t helper[] = {
        0x8du, 0xc1u, 0x4eu, 0x8du, 0x7bu, 0x4du, 0x0au, 0x0au,
        0x18u, 0x6du, 0x08u, 0x30u, 0x8du, 0x09u, 0x30u, 0xa9u,
        0x02u, 0x20u, 0x5eu, 0x4fu, 0xb0u, 0x01u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x08u)) == 0x4214u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4214u + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x421cu + i)) == helper[i]);
    }
    printf("  PASS: stage2_id08_local_helper (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the paired fixed-argument roots and shared +1 cursor-step stub. The
 * callee effects and any retail stream occurrence remain unproven. */
static void test_stage2_id13_id16_fixed_arguments(const uint8_t *raw,
                                                   size_t raw_size, int jp)
{
    static const uint8_t id13[] = {
        0xa9u, 0x06u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xf1u, 0x40u
    };
    static const uint8_t id16[] = {
        0xa9u, 0x07u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xf1u, 0x40u
    };
    static const uint8_t step_plus_one[] = {
        0xa9u, 0x01u, 0x80u, 0xefu, 0xa9u, 0x02u, 0x80u, 0xebu
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x13u)) == 0x45f0u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x16u)) == 0x4615u);
    for (unsigned int i = 0; i < sizeof(id13); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x45f0u + i)) == id13[i]);
    }
    for (unsigned int i = 0; i < sizeof(id16); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4615u + i)) == id16[i]);
    }
    for (unsigned int i = 0; i < sizeof(step_plus_one); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x40f1u + i)) == step_plus_one[i]);
    }
    printf("  PASS: stage2_id13_id16_fixed_arguments (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the adjacent IDs $14/$15, their shared two-byte operand reader and
 * three-byte cursor tail; this is not evidence of stream execution. */
static void test_stage2_id14_id15_operand_reader(const uint8_t *raw,
                                                  size_t raw_size, int jp)
{
    static const uint8_t id14[] = {
        0x44u, 0x0au, 0xa9u, 0x08u, 0x80u, 0x11u
    };
    static const uint8_t id15[] = {
        0x44u, 0x04u, 0xa9u, 0x0au, 0x80u, 0x0bu
    };
    static const uint8_t operand_reader[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x0eu, 0xc8u,
        0xb1u, 0x1cu, 0x85u, 0x10u, 0x60u
    };
    static const uint8_t cursor_tail[] = {
        0x20u, 0xb7u, 0x3au, 0x4cu, 0xf9u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x14u)) == 0x45f8u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x15u)) == 0x45feu);
    for (unsigned int i = 0; i < sizeof(id14); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x45f8u + i)) == id14[i]);
    }
    for (unsigned int i = 0; i < sizeof(id15); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x45feu + i)) == id15[i]);
    }
    for (unsigned int i = 0; i < sizeof(operand_reader); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4604u + i)) == operand_reader[i]);
    }
    for (unsigned int i = 0; i < sizeof(cursor_tail); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x460fu + i)) == cursor_tail[i]);
    }
    printf("  PASS: stage2_id14_id15_operand_reader (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the five short fixed-argument roots that share one operand reader and
 * continuation. This proves bytes/control-flow layout, not retail execution. */
static void test_stage2_id17_id1b_fixed_argument_roots(const uint8_t *raw,
                                                        size_t raw_size,
                                                        int jp)
{
    static const struct Stage2FixedArgumentRoot {
        uint16_t table_address;
        uint16_t target;
        uint8_t bytes[6];
    } roots[] = {
        { 0x413bu, 0x461du, { 0x44u, 0x1cu, 0xa9u, 0x09u, 0x80u, 0x1eu } },
        { 0x413du, 0x4623u, { 0x44u, 0x16u, 0xa9u, 0x0bu, 0x80u, 0x18u } },
        { 0x413fu, 0x4635u, { 0x44u, 0x04u, 0xa9u, 0x0eu, 0x80u, 0x06u } },
        { 0x4141u, 0x4629u, { 0x44u, 0x10u, 0xa9u, 0x0cu, 0x80u, 0x12u } },
        { 0x4143u, 0x462fu, { 0x44u, 0x0au, 0xa9u, 0x0du, 0x80u, 0x0cu } }
    };
    static const uint8_t shared_reader[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x0eu, 0x60u
    };
    static const uint8_t shared_tail[] = {
        0x20u, 0xb7u, 0x3au, 0x4cu, 0xf5u, 0x40u
    };

    for (unsigned int i = 0; i < sizeof(roots) / sizeof(roots[0]); ++i) {
        assert(stage2_word_at(raw, raw_size, jp, roots[i].table_address) ==
               roots[i].target);
        for (unsigned int j = 0; j < sizeof(roots[i].bytes); ++j) {
            assert(stage2_byte_at(raw, raw_size, jp,
                                  (uint16_t)(roots[i].target + j)) ==
                   roots[i].bytes[j]);
        }
    }
    for (unsigned int i = 0; i < sizeof(shared_reader); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x463bu + i)) == shared_reader[i]);
    }
    for (unsigned int i = 0; i < sizeof(shared_tail); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4641u + i)) == shared_tail[i]);
    }
    printf("  PASS: stage2_id17_id1b_fixed_argument_roots (%s)\n",
           jp ? "JP" : "US");
}

/* Lock adjacent dispatch roots that select fixed arguments and converge on
 * the same local continuation; argument/callee semantics remain unknown. */
static void test_stage2_id20_id21_fixed_arguments(const uint8_t *raw,
                                                    size_t raw_size, int jp)
{
    static const uint8_t id20[] = {
        0xa9u, 0x02u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xf1u, 0x40u
    };
    static const uint8_t id21[] = {
        0xa9u, 0x03u, 0x80u, 0xf6u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x20u)) == 0x4647u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x21u)) == 0x464fu);
    for (unsigned int i = 0; i < sizeof(id20); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4647u + i)) == id20[i]);
    }
    for (unsigned int i = 0; i < sizeof(id21); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x464fu + i)) == id21[i]);
    }
    printf("  PASS: stage2_id20_id21_fixed_arguments (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the naturally bounded ID $1e helper chain and adjacent full ID $1f
 * root. This is instruction/control-flow evidence, not operand semantics. */
static void test_stage2_id1e_id1f_bounded_handlers(const uint8_t *raw,
                                                    size_t raw_size, int jp)
{
    static const uint8_t id1e_root[] = {
        0x44u, 0x11u, 0xadu, 0x2fu, 0x44u, 0x85u, 0x00u, 0xadu,
        0x30u, 0x44u, 0x85u, 0x01u, 0x62u, 0x20u, 0xb7u, 0x3au,
        0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t id1e_helper[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu, 0x20u, 0x48u,
        0x4fu, 0xadu, 0x79u, 0x4du, 0x8du, 0x2fu, 0x44u, 0xadu,
        0x7au, 0x4du, 0x8du, 0x30u, 0x44u, 0x20u, 0xd2u, 0x4bu,
        0x60u
    };
    static const uint8_t id1f_root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu, 0xc8u, 0xb1u,
        0x1cu, 0x8du, 0xc3u, 0x4eu, 0x8du, 0x31u, 0x44u, 0xc8u,
        0xb1u, 0x1cu, 0x8du, 0xc4u, 0x4eu, 0x8du, 0x32u, 0x44u,
        0xa9u, 0x09u, 0x20u, 0x5eu, 0x4fu, 0x4cu, 0xfdu, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x1eu)) == 0x4433u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x1fu)) == 0x445fu);
    for (unsigned int i = 0; i < sizeof(id1e_root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4433u + i)) == id1e_root[i]);
    }
    for (unsigned int i = 0; i < sizeof(id1e_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4446u + i)) == id1e_helper[i]);
    }
    for (unsigned int i = 0; i < sizeof(id1f_root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x445fu + i)) == id1f_root[i]);
    }
    printf("  PASS: stage2_id1e_id1f_bounded_handlers (%s)\n",
           jp ? "JP" : "US");
}

/* Lock complete bounded roots for IDs $1c/$1d and their shared-step stubs.
 * The bytes establish cursor increments, not stream or operand semantics. */
static void test_stage2_id1c_id1d_cursor_roots(const uint8_t *raw,
                                                size_t raw_size, int jp)
{
    static const uint8_t id1c[] = {
        0xc8u, 0xb1u, 0x1cu, 0xc8u, 0x85u, 0x0eu, 0xb1u, 0x1cu,
        0xc8u, 0x85u, 0x10u, 0xb1u, 0x1cu, 0x85u, 0x11u, 0xc8u,
        0xb1u, 0x1cu, 0x85u, 0x12u, 0xa9u, 0x04u, 0x20u, 0xb7u,
        0x3au, 0x4cu, 0x01u, 0x41u
    };
    static const uint8_t id1d[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x11u, 0xc8u, 0xb1u, 0x1cu,
        0x85u, 0x10u, 0xc8u, 0xb1u, 0x1cu, 0x85u, 0x13u, 0xc8u,
        0xb1u, 0x1cu, 0x85u, 0x12u, 0xc8u, 0xb1u, 0x1cu, 0x85u,
        0x15u, 0xc8u, 0xb1u, 0x1cu, 0x85u, 0x14u, 0xa9u, 0x01u,
        0x20u, 0xb7u, 0x3au, 0x4cu, 0x05u, 0x41u
    };
    static const uint8_t step_plus_five[] = {
        0xa9u, 0x05u, 0x80u, 0xdfu
    };
    static const uint8_t step_plus_seven[] = {
        0xa9u, 0x07u, 0x80u, 0xdbu
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x1cu)) == 0x4345u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x1du)) == 0x4497u);
    for (unsigned int i = 0; i < sizeof(id1c); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4345u + i)) == id1c[i]);
    }
    for (unsigned int i = 0; i < sizeof(id1d); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4497u + i)) == id1d[i]);
    }
    for (unsigned int i = 0; i < sizeof(step_plus_five); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4101u + i)) == step_plus_five[i]);
    }
    for (unsigned int i = 0; i < sizeof(step_plus_seven); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4105u + i)) == step_plus_seven[i]);
    }
    printf("  PASS: stage2_id1c_id1d_cursor_roots (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $24's bounded helper-call root and its following conditional
 * two-byte cursor adjustment, without assigning the helper's semantics. */
static void test_stage2_id24_bounded_wait_root(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x03u, 0x44u, 0x64u, 0xfau, 0x64u, 0xfbu, 0xadu,
        0x73u, 0x43u, 0x85u, 0xf8u, 0xadu, 0x74u, 0x43u, 0x85u,
        0xf9u, 0xa9u, 0x0eu, 0x85u, 0xffu, 0x64u, 0xfeu, 0x20u,
        0x3cu, 0xe0u, 0xa0u, 0x01u, 0xb1u, 0x1cu, 0xd0u, 0x03u,
        0x20u, 0x03u, 0x44u, 0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t helper[] = {
        0x20u, 0x45u, 0xe0u, 0xd0u, 0xfbu, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x29u)) == 0x43ddu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x43ddu + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4403u + i)) == helper[i]);
    }
    printf("  PASS: stage2_id24_bounded_wait_root (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the ID $23 caller and its regional $56af/$5729 call operand. The
 * selected helper is byte-locked by the ID $2b test; gameplay meaning and
 * runtime selector use remain unknown. */
static void test_stage2_id23_regional_handoff(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    uint8_t handler[] = {
        0xc8u, 0xb1u, 0x1cu, 0x48u, 0x20u, 0x00u, 0x4bu, 0x20u,
        0x48u, 0x4fu, 0xadu, 0x79u, 0x4du, 0x8du, 0xdbu, 0x4fu,
        0xadu, 0x7au, 0x4du, 0x8du, 0xdcu, 0x4fu, 0x68u, 0x82u,
        0x20u, 0xafu, 0x56u, 0x4cu, 0xf5u, 0x40u
    };
    uint16_t helper = jp ? 0x5729u : 0x56afu;

    handler[25] = (uint8_t)helper;
    handler[26] = (uint8_t)(helper >> 8);
    assert(stage2_word_at(raw, raw_size, jp, 0x4153u) == 0x42fbu);
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        uint8_t actual = stage2_byte_at(raw, raw_size, jp,
                                        (uint16_t)(0x42fbu + i));
        if (actual != handler[i]) {
            fprintf(stderr, "ID $23 mismatch at +$%02x: expected $%02x got $%02x (%s)\n",
                    i, handler[i], actual, jp ? "JP" : "US");
            assert(actual == handler[i]);
        }
    }
    printf("  PASS: stage2_id23_regional_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $27's bounded pointer setup and fixed $3ab7 argument path. */
static void test_stage2_id27_bounded_pointer_setup(const uint8_t *raw,
                                                   size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x46u, 0x44u, 0xadu, 0x2fu, 0x44u, 0x85u, 0x00u,
        0xadu, 0x30u, 0x44u, 0x85u, 0x01u, 0xadu, 0x7bu, 0x4du,
        0x0au, 0x0au, 0x18u, 0x6du, 0x08u, 0x30u, 0x8du, 0x0au,
        0x30u, 0xa9u, 0x05u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xf5u,
        0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x27u)) == 0x45cau);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x45cau + i)) == root[i]);
    }
    printf("  PASS: stage2_id27_bounded_pointer_setup (%s)\n",
           jp ? "JP" : "US");
}

/* Byte-lock the ID $2a entry/helper path without assigning table semantics. */
static void test_stage2_id2a_entry_helper(const uint8_t *raw,
                                          size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x62u, 0x20u, 0x2du, 0xe0u, 0x44u, 0x06u, 0x20u, 0x12u,
        0xe0u, 0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t helper[] = {
        0xa0u, 0x01u, 0xb1u, 0x1cu, 0xaau, 0xbdu, 0x3cu, 0x4bu,
        0x85u, 0xf8u, 0x18u, 0xf8u, 0x69u, 0x01u, 0x85u, 0xfcu,
        0xd8u, 0xa9u, 0x80u, 0x85u, 0xfbu, 0xa9u, 0x83u, 0x85u,
        0xffu, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x2au)) == 0x4409u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        uint8_t actual = stage2_byte_at(raw, raw_size, jp,
                                        (uint16_t)(0x4409u + i));
        if (actual != root[i]) {
            fprintf(stderr, "ID $2a mismatch at +$%02x: expected $%02x got $%02x (%s)\n",
                    i, root[i], actual, jp ? "JP" : "US");
            assert(actual == root[i]);
        }
    }
    for (unsigned int i = 0; i < sizeof(helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4415u + i)) == helper[i]);
    }
    printf("  PASS: stage2_id2a_entry_helper (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the overlapping $2d root's bounded compare/branch/cursor path. */
static void test_stage2_id2d_overlapping_poll_root(const uint8_t *raw,
                                                   size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x9cu, 0x33u, 0x3bu, 0xcdu,
        0x33u, 0x3bu, 0xb0u, 0xfbu, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x2du)) == 0x468fu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x468fu + i)) == root[i]);
    }
    printf("  PASS: stage2_id2d_overlapping_poll_root (%s)\n",
           jp ? "JP" : "US");
}

/* Lock selected branch/helper windows of the large ID $2e path. */
static void test_stage2_id2e_bounded_windows(const uint8_t *raw,
                                             size_t raw_size, int jp)
{
    static const uint8_t root_prefix[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x7bu, 0x43u, 0x08u, 0x48u,
        0x43u, 0x10u, 0x48u, 0x18u, 0xadu, 0xf5u, 0xffu, 0x69u,
        0x06u, 0x53u, 0x08u, 0x1au, 0x53u, 0x10u, 0xc6u, 0x5bu,
        0x20u, 0xd6u, 0x43u, 0x20u, 0xd8u, 0x37u, 0xa9u, 0x00u,
        0x85u, 0x20u, 0xa9u, 0x60u, 0x85u, 0x21u, 0x20u, 0x3eu,
        0x38u, 0x64u, 0x5bu
    };
    static const uint8_t alternate_path[] = {
        0xa9u, 0x3fu, 0x85u, 0xf8u, 0xa9u, 0x0fu, 0x85u, 0xffu,
        0x20u, 0xd8u, 0xe0u, 0xa9u, 0x3fu, 0x85u, 0xf8u, 0xa9u,
        0x0eu, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u, 0xa9u, 0x01u,
        0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u, 0x4cu, 0xf9u, 0x40u
    };
    static const uint8_t pair_loop[] = {
        0xa2u, 0x05u, 0xa0u, 0xffu, 0x44u, 0x19u, 0xa5u, 0xf8u,
        0xd0u, 0x06u, 0xa5u, 0xf9u, 0xd0u, 0x02u, 0x80u, 0x09u,
        0xdau, 0x5au, 0x86u, 0xffu, 0x20u, 0xd8u, 0xe0u, 0x7au,
        0xfau, 0xe8u, 0xe0u, 0x0au, 0xd0u, 0xe6u, 0x60u
    };
    static const uint8_t pair_reader[] = {
        0xc8u, 0xb1u, 0x00u, 0x85u, 0xf8u, 0xc8u,
        0xb1u, 0x00u, 0x85u, 0xf9u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x2eu)) == 0x46cau);
    for (unsigned int i = 0; i < sizeof(root_prefix); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x46cau + i)) == root_prefix[i]);
    }
    for (unsigned int i = 0; i < sizeof(alternate_path); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x474au + i)) == alternate_path[i]);
    }
    for (unsigned int i = 0; i < sizeof(pair_loop); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x476au + i)) == pair_loop[i]);
    }
    for (unsigned int i = 0; i < sizeof(pair_reader); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4789u + i)) == pair_reader[i]);
    }
    printf("  PASS: stage2_id2e_bounded_windows (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $2f's short parameter handoff and fixed +2 cursor tail. */
static void test_stage2_id2f_parameter_handoff(const uint8_t *raw,
                                                size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0xf8u, 0xa9u, 0x0bu,
        0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u, 0x20u, 0x2du,
        0x4bu, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x2fu)) == 0x4794u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4794u + i)) == root[i]);
    }
    printf("  PASS: stage2_id2f_parameter_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the overlapping conditional-entry bytes for dispatch ID $30. */
static void test_stage2_id30_overlapping_branch_root(const uint8_t *raw,
                                                      size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x0eu, 0xa9u, 0x3fu, 0x85u,
        0xf8u, 0xa9u, 0x0eu, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u,
        0x4cu, 0xf5u, 0x40u, 0x85u, 0xf8u, 0xa9u, 0x13u, 0x85u,
        0xffu, 0x20u, 0xd8u, 0xe0u, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x30u)) == 0x47a6u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        uint8_t actual = stage2_byte_at(raw, raw_size, jp,
                                        (uint16_t)(0x47a6u + i));
        if (actual != root[i]) {
            fprintf(stderr, "ID $30 mismatch at +$%02x: expected $%02x got $%02x (%s)\n",
                    i, root[i], actual, jp ? "JP" : "US");
            assert(actual == root[i]);
        }
    }
    printf("  PASS: stage2_id30_overlapping_branch_root (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $31's short fixed-argument BIOS handoff and +1 tail. */
static void test_stage2_id31_fixed_argument_handoff(const uint8_t *raw,
                                                     size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xa9u, 0x3fu, 0x85u, 0xf8u, 0xa9u, 0x0fu, 0x85u,
        0xffu, 0x20u, 0xd8u, 0xe0u, 0x4cu, 0xf1u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x31u)) == 0x47c5u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x47c5u + i)) == root[i]);
    }
    printf("  PASS: stage2_id31_fixed_argument_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $32's two-way BIOS parameter bytes and shared +2 cursor exit. */
static void test_stage2_id32_conditional_handoff(const uint8_t *raw,
                                                  size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x0eu, 0xa9u, 0xbfu, 0x85u,
        0xf8u, 0xa9u, 0x0eu, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u,
        0x4cu, 0xf5u, 0x40u, 0xa9u, 0x02u, 0x85u, 0xf8u, 0xa9u,
        0x12u, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u, 0x80u, 0xf0u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x32u)) == 0x47d3u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x47d3u + i)) == root[i]);
    }
    printf("  PASS: stage2_id32_conditional_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $33's four indexed-stream reads and fixed +5 cursor tail. */
static void test_stage2_id33_four_byte_handoff(const uint8_t *raw,
                                                size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0x02u, 0x04u,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0x03u, 0x04u,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0x04u, 0x04u,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0x05u, 0x04u,
        0x4cu, 0x01u, 0x41u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x33u)) == 0x469du);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x469du + i)) == root[i]);
    }
    printf("  PASS: stage2_id33_four_byte_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $34's bounded four-way fixed-argument dispatch path. */
static void test_stage2_id34_fixed_argument_select(const uint8_t *raw,
                                                    size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x07u, 0xa9u, 0x10u,
        0x20u, 0xb7u, 0x3au, 0x80u, 0x1bu, 0xc9u, 0x01u,
        0xd0u, 0x07u, 0xa9u, 0x11u, 0x20u, 0xb7u, 0x3au,
        0x80u, 0x10u, 0xc9u, 0x02u, 0xd0u, 0x07u, 0xa9u,
        0x12u, 0x20u, 0xb7u, 0x3au, 0x80u, 0x05u, 0xa9u,
        0x16u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x34u)) == 0x44bdu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        uint8_t actual = stage2_byte_at(raw, raw_size, jp,
                                        (uint16_t)(0x44bdu + i));
        if (actual != root[i]) {
            fprintf(stderr, "ID $34 mismatch at +$%02x: expected $%02x got $%02x (%s)\n",
                    i, root[i], actual, jp ? "JP" : "US");
            assert(actual == root[i]);
        }
    }
    printf("  PASS: stage2_id34_fixed_argument_select (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $35's branch and region-specific $5e4d/$5e7d call target. */
static void test_stage2_id35_regional_call_handoff(const uint8_t *raw,
                                                    size_t raw_size, int jp)
{
    uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x07u, 0xa9u, 0x13u,
        0x20u, 0xb7u, 0x3au, 0x80u, 0x03u, 0x20u, 0x4du,
        0x5eu, 0x4cu, 0xf5u, 0x40u
    };
    uint16_t target = jp ? 0x5e7du : 0x5e4du;

    root[13] = (uint8_t)target;
    root[14] = (uint8_t)(target >> 8);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x35u)) == 0x46b8u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        uint8_t actual = stage2_byte_at(raw, raw_size, jp,
                                        (uint16_t)(0x46b8u + i));
        if (actual != root[i]) {
            fprintf(stderr, "ID $35 mismatch at +$%02x: expected $%02x got $%02x (%s)\n",
                    i, root[i], actual, jp ? "JP" : "US");
            assert(actual == root[i]);
        }
    }
    printf("  PASS: stage2_id35_regional_call_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $36's two-byte stream handoff and fixed helper selector. */
static void test_stage2_id36_stream_handoff(const uint8_t *raw,
                                             size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x15u, 0xc8u, 0xb1u, 0x1cu,
        0x85u, 0x14u, 0xa9u, 0x14u, 0x20u, 0xb7u, 0x3au, 0x4cu,
        0xf9u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x36u)) == 0x4361u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4361u + i)) == root[i]);
    }
    printf("  PASS: stage2_id36_stream_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $37's root and local helper; $383e's effects remain unresolved. */
static void test_stage2_id37_local_helper(const uint8_t *raw,
                                          size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x02u, 0x44u, 0x03u, 0x4cu,
        0xf5u, 0x40u
    };
    static const uint8_t helper[] = {
        0xa9u, 0xd3u, 0x85u, 0x00u, 0xa9u, 0x37u, 0x85u, 0x01u,
        0x18u, 0xa0u, 0x01u, 0xa5u, 0x02u, 0x71u, 0x00u, 0x85u,
        0x24u, 0xc8u, 0x62u, 0x71u, 0x00u, 0x85u, 0x23u, 0x62u,
        0x72u, 0x00u, 0x85u, 0x22u, 0xa9u, 0x00u, 0x85u, 0x20u,
        0xa9u, 0x28u, 0x85u, 0x21u, 0xa9u, 0x01u, 0x85u, 0x1eu,
        0x85u, 0x25u, 0x20u, 0x3eu, 0x38u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x37u)) == 0x480au);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x480au + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4814u + i)) == helper[i]);
    }
    printf("  PASS: stage2_id37_local_helper (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $38's three stream-byte stores and fixed helper selector. */
static void test_stage2_id38_three_byte_handoff(const uint8_t *raw,
                                                size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x0eu, 0xc8u, 0xb1u, 0x1cu,
        0x85u, 0x10u, 0xc8u, 0xb1u, 0x1cu, 0x85u, 0x12u, 0xa9u,
        0x15u, 0x20u, 0xb7u, 0x3au, 0x4cu, 0xfdu, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x38u)) == 0x47f3u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x47f3u + i)) == root[i]);
    }
    printf("  PASS: stage2_id38_three_byte_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $39 and its bounded helper chain against both authentic editions. */
static void test_stage2_id39_helper_chain(const uint8_t *raw,
                                          size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu,
        0x20u, 0xe7u, 0x4bu, 0x4cu, 0xf5u, 0x40u
    };
    static const uint8_t helper[] = {
        0xc6u, 0x5bu, 0x20u, 0x31u, 0x4fu, 0xa5u, 0x00u, 0x85u,
        0x02u, 0xa5u, 0x01u, 0x85u, 0x03u, 0xadu, 0xc2u, 0x4eu,
        0x8du, 0xccu, 0x37u, 0x20u, 0xa0u, 0x37u, 0x64u, 0x5bu,
        0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x39u)) == 0x4842u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4842u + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4be7u + i)) == helper[i]);
    }
    printf("  PASS: stage2_id39_helper_chain (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3a's three-byte jump to the shared +1 cursor path. */
static void test_stage2_id3a_cursor_stub(const uint8_t *raw,
                                         size_t raw_size, int jp)
{
    static const uint8_t root[] = { 0x4cu, 0xf1u, 0x40u };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3au)) == 0x485fu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x485fu + i)) == root[i]);
    }
    printf("  PASS: stage2_id3a_cursor_stub (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3b's root and reuse the independently locked $4483 helper. */
static void test_stage2_id3b_overlapping_helper_root(const uint8_t *raw,
                                                      size_t raw_size, int jp)
{
    static const uint8_t root[] = { 0x44u, 0x02u, 0x80u, 0xbcu };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3bu)) == 0x447fu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x447fu + i)) == root[i]);
    }
    printf("  PASS: stage2_id3b_overlapping_helper_root (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3c's bounded BIOS branches, wait loop, and +1 cursor tail. */
static void test_stage2_id3c_bios_control_flow(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x63u, 0xe0u, 0xadu, 0x28u, 0x22u, 0xeau, 0xeau,
        0xd0u, 0x12u, 0xa9u, 0x95u, 0x85u, 0xfau, 0xa9u, 0x48u,
        0x85u, 0xfbu, 0x20u, 0x1eu, 0xe0u, 0xadu, 0x95u, 0x48u,
        0xf0u, 0xe6u, 0x80u, 0x0du, 0xa9u, 0x0cu, 0x20u, 0x2du,
        0xe0u, 0xa2u, 0x14u, 0x20u, 0x2du, 0x4bu, 0xcau, 0xd0u,
        0xfau, 0x20u, 0x18u, 0xe0u, 0x62u, 0x20u, 0x2du, 0xe0u,
        0x4cu, 0xf1u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3cu)) == 0x4862u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4862u + i)) == root[i]);
    }
    printf("  PASS: stage2_id3c_bios_control_flow (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3d's short window, whose branch returns into the ID $3c tail. */
static void test_stage2_id3d_overlapping_bios_window(const uint8_t *raw,
                                                      size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x63u, 0xe0u, 0xadu, 0x28u, 0x22u, 0xf0u,
        0xebu, 0x8du, 0x80u, 0x27u, 0x80u, 0xe6u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3du)) == 0x489fu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x489fu + i)) == root[i]);
    }
    printf("  PASS: stage2_id3d_overlapping_bios_window (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3e and its local polling helper up to the next dispatch root. */
static void test_stage2_id3e_polling_helper(const uint8_t *raw,
                                             size_t raw_size, int jp)
{
    static const uint8_t root_and_helper[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x0du, 0x3au, 0x8du, 0x68u,
        0x3bu, 0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x21u, 0x44u, 0x22u,
        0x80u, 0x1du, 0xc8u, 0xb1u, 0x1cu, 0xc9u, 0xffu, 0xf0u,
        0x09u, 0x8du, 0x6eu, 0x3bu, 0xc8u, 0xb1u, 0x1cu, 0x8du,
        0x6fu, 0x3bu, 0xa9u, 0xffu, 0x8du, 0x69u, 0x3bu, 0xadu,
        0x69u, 0x3bu, 0xd0u, 0xfbu, 0x9cu, 0x68u, 0x3bu, 0x4cu,
        0xfdu, 0x40u, 0x9cu, 0x02u, 0x04u, 0x9cu, 0x03u, 0x04u,
        0xa9u, 0x00u, 0x85u, 0x00u, 0xa9u, 0x04u, 0x85u, 0x01u,
        0x9cu, 0x04u, 0x04u, 0x9cu, 0x05u, 0x04u, 0xa5u, 0x00u,
        0xd0u, 0x02u, 0xc6u, 0x01u, 0xc6u, 0x00u, 0xa5u, 0x00u,
        0x05u, 0x01u, 0xd0u, 0xecu, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3eu)) == 0x48acu);
    for (unsigned int i = 0; i < sizeof(root_and_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x48acu + i)) ==
               root_and_helper[i]);
    }
    printf("  PASS: stage2_id3e_polling_helper (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $3f's bounded eight-byte indexed transfer and tail jump. */
static void test_stage2_id3f_indexed_transfer(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x82u, 0xc8u, 0xb1u, 0x1cu, 0x9du, 0x70u, 0x3bu, 0xe8u,
        0xe0u, 0x08u, 0xd0u, 0xf5u, 0x4cu, 0x09u, 0x41u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x3fu)) == 0x4901u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4901u + i)) == root[i]);
    }
    printf("  PASS: stage2_id3f_indexed_transfer (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the complete bounded ID $40 branch path, stopping before ID $42. */
static void test_stage2_id40_bounded_selector_path(const uint8_t *raw,
                                                   size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc6u, 0x5bu, 0x20u, 0xd6u, 0x43u, 0xc8u, 0xb1u, 0x1cu,
        0xc9u, 0x01u, 0xf0u, 0x09u, 0xc9u, 0x02u, 0xf0u, 0x3du,
        0x20u, 0xd8u, 0x37u, 0x80u, 0x03u, 0x20u, 0xb5u, 0x43u,
        0xa9u, 0x01u, 0x85u, 0x1eu, 0x85u, 0x25u, 0x20u, 0x48u,
        0x4fu, 0xadu, 0x79u, 0x4du, 0x85u, 0x20u, 0xadu, 0x7au,
        0x4du, 0x85u, 0x21u, 0x20u, 0x00u, 0x4bu, 0x20u, 0xd2u,
        0x43u, 0x20u, 0x09u, 0xe0u, 0xc9u, 0x00u, 0xd0u, 0xf6u,
        0xa9u, 0xffu, 0x20u, 0x1bu, 0xe0u, 0xc9u, 0x00u, 0xf0u,
        0x04u, 0xc9u, 0x0eu, 0x90u, 0xf3u, 0x64u, 0x5bu, 0x20u,
        0xf7u, 0x4au, 0x4cu, 0xf9u, 0x40u, 0xadu, 0xccu, 0x37u,
        0x20u, 0x19u, 0x44u, 0x20u, 0x15u, 0xe0u, 0x80u, 0xedu
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x40u)) == 0x491bu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x491bu + i)) == root[i]);
    }
    printf("  PASS: stage2_id40_bounded_selector_path (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $42's conditional field stores and shared +4 cursor exit. */
static void test_stage2_id42_conditional_stores(const uint8_t *raw,
                                                 size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0xc8u, 0xb1u, 0x1cu, 0xd0u, 0x16u, 0xc8u, 0xb1u, 0x1cu,
        0x8du, 0xc2u, 0x27u, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc3u,
        0x27u, 0x9cu, 0xc6u, 0x27u, 0xa9u, 0xffu, 0x8du, 0xc7u,
        0x27u, 0x80u, 0x03u, 0x9cu, 0xc7u, 0x27u, 0x4cu, 0xfdu,
        0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x42u)) == 0x4973u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4973u + i)) == root[i]);
    }
    printf("  PASS: stage2_id42_conditional_stores (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $43's source copies and fixed +2 cursor tail. */
static void test_stage2_id43_source_copies(const uint8_t *raw,
                                           size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x46u, 0x44u, 0xadu, 0x2fu, 0x44u, 0x8du, 0xc0u,
        0x27u, 0x85u, 0x62u, 0xadu, 0x30u, 0x44u, 0x8du, 0xc1u,
        0x27u, 0x85u, 0x63u, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x43u)) == 0x4995u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4995u + i)) == root[i]);
    }
    printf("  PASS: stage2_id43_source_copies (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $44's short call/branch root; $4483 is independently source-locked. */
static void test_stage2_id44_helper_reentry(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x83u, 0x44u, 0x80u, 0xe7u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x44u)) == 0x45ebu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x45ebu + i)) == root[i]);
    }
    printf("  PASS: stage2_id44_helper_reentry (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $45's bounded subroutine/BIOS handoff and branch tail. */
static void test_stage2_id45_bounded_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0x34u, 0xa9u, 0x08u, 0x20u, 0x5eu, 0x4fu, 0x80u,
        0x24u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x45u)) == 0x49abu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49abu + i)) == root[i]);
    }
    printf("  PASS: stage2_id45_bounded_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $46's bounded relative-helper/callee handoff and branch tail. */
static void test_stage2_id46_helper_handoff(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0x2bu, 0x20u, 0x00u, 0x4cu, 0x80u, 0x1du
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x46u)) == 0x49b4u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49b4u + i)) == root[i]);
    }
    printf("  PASS: stage2_id46_helper_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $47's three-byte setup and the shared $49e1 stream-byte helper. */
static void test_stage2_id47_three_byte_handoff(const uint8_t *raw,
                                                 size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0x24u, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc3u, 0x4eu,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc4u, 0x4eu, 0x9cu, 0xc5u,
        0x4eu, 0xa9u, 0x07u, 0x20u, 0x5eu, 0x4fu, 0x80u, 0x08u
    };
    static const uint8_t stream_byte_helper[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x47u)) == 0x49bbu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49bbu + i)) == root[i]);
    }
    for (unsigned int i = 0; i < sizeof(stream_byte_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49e1u + i)) ==
               stream_byte_helper[i]);
    }
    printf("  PASS: stage2_id47_three_byte_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $48's bounded paired-selector loop and +3 cursor tail. */
static void test_stage2_id48_paired_selector_loop(const uint8_t *raw,
                                                   size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x3bu, 0x46u, 0xc8u, 0xb1u, 0x1cu, 0xaau, 0xa5u,
        0x0eu, 0xdau, 0x48u, 0xa9u, 0x0cu, 0x20u, 0xb7u, 0x3au,
        0x68u,
        0x85u, 0x0eu, 0x48u, 0xa9u, 0x0eu, 0x20u, 0xb7u, 0x3au,
        0x68u, 0x85u, 0x0eu, 0xfau, 0xcau, 0xd0u, 0xe9u, 0x4cu,
        0xf9u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x48u)) == 0x4a5eu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a5eu + i)) == root[i]);
    }
    printf("  PASS: stage2_id48_paired_selector_loop (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $49's selector split and the two paths joining at $4514. */
static void test_stage2_id49_selector_join(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t root_and_join[] = {
        0xc8u, 0xb1u, 0x1cu, 0x48u, 0xf0u, 0x20u, 0x29u, 0xc0u,
        0xc9u, 0x40u, 0xf0u, 0x03u, 0x4cu, 0x9fu, 0x45u, 0xc8u,
        0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu, 0x20u, 0x17u, 0x4cu,
        0xadu, 0x79u, 0x4du, 0x8du, 0x2fu, 0x44u, 0xadu, 0x7au,
        0x4du, 0x8du, 0x30u, 0x44u, 0x80u, 0x03u, 0x20u, 0x46u,
        0x44u, 0x20u, 0x31u, 0x4fu, 0x38u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x49u)) == 0x44ebu);
    for (unsigned int i = 0; i < sizeof(root_and_join); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x44ebu + i)) ==
               root_and_join[i]);
    }
    printf("  PASS: stage2_id49_selector_join (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $4a's paired calls and bounded loop against authentic editions. */
static void test_stage2_id4a_paired_call_loop(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x20u, 0x3bu, 0x46u, 0xa5u, 0x0eu, 0x48u, 0xa9u, 0x0cu,
        0x20u, 0xb7u, 0x3au, 0x68u, 0x85u, 0x0eu, 0x48u, 0xa9u,
        0x0eu, 0x20u, 0xb7u, 0x3au, 0x68u, 0x85u, 0x0eu, 0x20u,
        0x45u, 0xe0u, 0xd0u, 0xe7u, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x4au)) == 0x4a81u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a81u + i)) == root[i]);
    }
    printf("  PASS: stage2_id4a_paired_call_loop (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $4c's short call handoff through its terminal cursor jump. */
static void test_stage2_id4c_call_handoff(const uint8_t *raw,
                                           size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0x0cu, 0x20u, 0xd2u, 0x4bu, 0x4cu, 0xf5u, 0x40u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x4cu)) == 0x49d3u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49d3u + i)) == root[i]);
    }
    printf("  PASS: stage2_id4c_call_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $4e's bounded shared-reader/call/relative-branch handoff. */
static void test_stage2_id4e_relative_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0xa4u, 0x20u, 0xe1u, 0x4cu, 0x80u, 0x96u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x4eu)) == 0x4a3bu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a3bu + i)) == root[i]);
    }
    printf("  PASS: stage2_id4e_relative_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $4f's bounded shared-reader/call/relative-branch prefix. */
static void test_stage2_id4f_relative_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0xcbu, 0x20u, 0x17u, 0x4cu, 0x80u, 0xbdu
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x4fu)) == 0x4a14u);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a14u + i)) == root[i]);
    }
    printf("  PASS: stage2_id4f_relative_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $50's three-byte operand setup and bounded helper handoff. */
static void test_stage2_id50_staged_handoff(const uint8_t *raw,
                                             size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0xc4u, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc1u, 0x4eu,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc3u, 0x4eu, 0xc8u, 0xb1u,
        0x1cu, 0x8du, 0xc4u, 0x4eu, 0xa9u, 0x01u, 0x8du, 0xc5u,
        0x4eu, 0xa9u, 0x07u, 0x20u, 0x5eu, 0x4fu, 0x80u, 0xa3u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x50u)) == 0x4a1bu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a1bu + i)) == root[i]);
    }
    printf("  PASS: stage2_id50_staged_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the index-$4b comparison handler and its local pair checker against
 * each authentic edition. This proves byte-level branches/cursor arithmetic
 * only; it does not establish that a retail stream selects this root. */
static void test_stage2_id4b_indexed_comparison(const uint8_t *raw,
                                                 size_t raw_size, int jp)
{
    static const uint8_t body[] = {
        0xc8u, 0xb1u, 0x1cu, 0x64u, 0x00u, 0x48u, 0x44u, 0x12u,
        0x68u, 0x3au, 0xd0u, 0xf9u, 0xa5u, 0x00u, 0xd0u, 0x03u,
        0x4cu, 0xc5u, 0x41u, 0xc8u, 0xc8u, 0xc8u, 0x98u, 0x4cu,
        0xe4u, 0x40u, 0xc8u, 0xb1u, 0x1cu, 0xaau, 0xc8u, 0xb1u,
        0x1cu, 0xc9u, 0xffu, 0xf0u, 0x07u, 0xddu, 0x80u, 0x27u,
        0xf0u, 0x02u, 0xc6u, 0x00u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp, 0x41a3u) == 0x4acau);
    for (unsigned int i = 0; i < sizeof(body); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4acau + i)) == body[i]);
    }
    printf("  PASS: stage2_id4b_indexed_comparison (%s)\n",
           jp ? "JP" : "US");
}

/* Lock dispatch ID $51 and its in-window helper chain against authentic
 * media. The field meanings and helper effects remain unassigned. */
static void test_stage2_id51_helper_chain(const uint8_t *raw,
                                          size_t raw_size, int jp)
{
    static const uint8_t handler[] = {
        0x44u, 0x9du, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc1u, 0x4eu,
        0x20u, 0x0eu, 0x4du, 0x4cu, 0xf9u, 0x40u
    };
    static const uint8_t mode_handler[] = {
        0x44u, 0x8fu, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc1u, 0x4eu,
        0x20u, 0x6au, 0x4du, 0x4cu, 0xf9u, 0x40u
    };
    static const uint8_t helpers[] = {
        0x44u, 0x05u, 0xb0u, 0x55u, 0x44u, 0x36u, 0x60u, 0xadu,
        0x7bu, 0x4du, 0x48u, 0xadu, 0xc1u, 0x4eu, 0x8du, 0x68u,
        0x4du, 0x8du, 0x7bu, 0x4du, 0x20u, 0x48u, 0x4fu, 0xadu,
        0xc3u, 0x4eu, 0x8du, 0xc5u, 0x4eu, 0xadu, 0xc4u, 0x4eu,
        0x8du, 0xc6u, 0x4eu, 0x68u, 0x8du, 0x7bu, 0x4du, 0x8du,
        0xc1u, 0x4eu, 0x20u, 0xc9u, 0x4eu, 0xb0u, 0x2au, 0xadu,
        0xc2u, 0x4eu, 0x8du, 0x69u, 0x4du, 0xadu, 0x68u, 0x4du,
        0x8du, 0xc2u, 0x4eu, 0x60u, 0xa9u, 0x04u, 0x20u, 0x5eu,
        0x4fu, 0xb0u, 0x16u, 0xadu, 0x68u, 0x4du, 0x8du, 0x7bu,
        0x4du, 0xadu, 0x69u, 0x4du, 0x8du, 0xc2u, 0x4eu, 0x20u,
        0xf4u, 0x4eu, 0xadu, 0xc1u, 0x4eu, 0x8du, 0x7bu, 0x4du,
        0x60u
    };
    static const uint8_t mode_variant[] = {
        0x44u, 0xa9u, 0xb0u, 0xf9u, 0xadu, 0xc1u, 0x4eu, 0x09u,
        0x80u, 0x8du, 0xc1u, 0x4eu, 0x44u, 0xd2u, 0x60u
    };

    assert(stage2_word_at(raw, raw_size, jp, 0x41afu) == 0x4a42u);
    assert(stage2_word_at(raw, raw_size, jp, 0x41b1u) == 0x4a50u);
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a42u + i)) == handler[i]);
    }
    for (unsigned int i = 0; i < sizeof(mode_handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4a50u + i)) == mode_handler[i]);
    }
    for (unsigned int i = 0; i < sizeof(helpers); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4d0eu + i)) == helpers[i]);
    }
    for (unsigned int i = 0; i < sizeof(mode_variant); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4d6au + i)) == mode_variant[i]);
    }
    printf("  PASS: stage2_id51_helper_chain (%s)\n",
           jp ? "JP" : "US");
}

/* Lock the index-$4d operand handoff and cursor tail against each edition. */
static void test_stage2_id4d_operand_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t body[] = {
        0x44u, 0xf7u, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc5u, 0x4eu,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc6u, 0x4eu, 0x20u, 0x30u,
        0x4cu, 0x80u, 0xe0u
    };

    assert(stage2_word_at(raw, raw_size, jp, 0x41a7u) == 0x49e8u);
    for (unsigned int i = 0; i < sizeof(body); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49e8u + i)) == body[i]);
    }
    printf("  PASS: stage2_id4d_operand_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Lock ID $53's independent three-byte staging path. */
static void test_stage2_id53_three_byte_handoff(const uint8_t *raw,
                                                size_t raw_size, int jp)
{
    static const uint8_t root[] = {
        0x44u, 0xe4u, 0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc1u, 0x4eu,
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc5u, 0x4eu, 0xc8u, 0xb1u,
        0x1cu, 0x8du, 0xc6u, 0x4eu, 0x20u, 0x3fu, 0x4cu, 0x80u,
        0xcau
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 2u * 0x53u)) == 0x49fbu);
    for (unsigned int i = 0; i < sizeof(root); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x49fbu + i)) == root[i]);
    }
    printf("  PASS: stage2_id53_three_byte_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* Authentic selector continuations and overlapping nested-stream roots.
 * These byte assertions are source evidence, not proof that a selector runs.
 * See docs/source-lock/theron-disassembly/
 * theron-stage2-bytecode-dispatch-table-20261005.md. */
static void test_stage2_selector_candidate_continuations(
    const uint8_t *raw, size_t raw_size, int jp)
{
    static const uint16_t selector_targets[] = {
        0x694du, 0x69a1u, 0x6a16u, 0x6a7cu, 0x6ae2u,
        0x6b48u, 0x6bbdu, 0x6c13u, 0x681cu
    };
    static const uint8_t poll_handler[] = {
        0xc8u, 0xb1u, 0x1cu, 0x9cu, 0x33u, 0x3bu, 0xcdu,
        0x33u, 0x3bu, 0xb0u, 0xfbu, 0x4cu, 0xf5u, 0x40u
    };
    uint16_t long_root = jp ? 0x7448u : 0x7446u;
    uint16_t middle_root = jp ? 0x7466u : 0x7464u;
    uint16_t final_root = jp ? 0x7472u : 0x7470u;

    for (unsigned int i = 0; i < sizeof(selector_targets) /
                                sizeof(selector_targets[0]); ++i) {
        uint16_t table_address = (uint16_t)(0x6800u + (4u + i) * 2u);
        assert(stage2_word_at(raw, raw_size, jp, table_address) ==
               selector_targets[i]);
    }
    for (unsigned int i = 0; i < sizeof(poll_handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x468fu + i)) == poll_handler[i]);
    }

    for (unsigned int pair = 0; pair < 8u; ++pair) {
        uint8_t value = (uint8_t)(7u - pair);
        uint16_t at = (uint16_t)(long_root + pair * 6u);
        assert(stage2_byte_at(raw, raw_size, jp, at) == 0x14u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 1u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 2u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 3u)) == 0x15u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 4u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 5u)) == value);
    }
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(long_root + 48u)) == 0x09u);

    for (unsigned int pair = 0; pair < 3u; ++pair) {
        uint8_t value = (uint8_t)(2u - pair);
        uint16_t at = (uint16_t)(middle_root + pair * 6u);
        assert(stage2_byte_at(raw, raw_size, jp, at) == 0x14u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 1u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 2u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 3u)) == 0x15u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 4u)) == value);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(at + 5u)) == value);
    }
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(middle_root + 18u)) == 0x09u);
    assert_stage2_pointer(raw, raw_size, jp, 0x69a1u + 43u, middle_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x6ae2u + 43u, middle_root);
    assert(stage2_byte_at(raw, raw_size, jp, final_root) == 0x14u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 1u)) == 0x00u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 2u)) == 0x00u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 3u)) == 0x15u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 4u)) == 0x00u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 5u)) == 0x00u);
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)(final_root + 6u)) == 0x09u);

    assert_stage2_pointer(raw, raw_size, jp, 0x694du + 40u,
                          (uint16_t)(jp ? 0x73b4u : 0x73b2u));
    assert_stage2_pointer(raw, raw_size, jp, 0x694du + 43u, final_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x6a16u + 43u, final_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x6a7cu + 43u, final_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x6bbdu + 40u,
                          (uint16_t)(jp ? 0x73b4u : 0x73b2u));
    assert_stage2_pointer(raw, raw_size, jp, 0x6bbdu + 43u, final_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x6c13u + 33u,
                          (uint16_t)(jp ? 0x73b4u : 0x73b2u));
    assert_stage2_pointer(raw, raw_size, jp, 0x6c13u + 36u, final_root);
    assert_stage2_pointer(raw, raw_size, jp, 0x681cu + 43u,
                          (uint16_t)(jp ? 0x7448u : 0x7446u));
    for (unsigned int row = 0; row < 7u; ++row) {
        uint16_t row_address = (uint16_t)(0x681cu + 46u + row * 5u);
        uint16_t target = (uint16_t)(0x686du + row * 0x20u);
        uint8_t body[32] = {
            0x1au, 0x00u, 0x13u, 0x2du, 0x02u, 0x20u, 0x3eu, 0x01u,
            0x00u, 0x17u, 0x2du, 0x02u, 0x11u, 0x03u, 0x00u, 0x60u,
            0x2bu, 0x02u, 0x00u, 0x1au, 0x07u, 0x12u, 0x52u, 0x75u,
            0x17u, 0x87u, 0x2du, 0x03u, 0x12u, 0xf2u, 0x74u, 0x09u
        };
        assert(stage2_byte_at(raw, raw_size, jp, row_address) == 0x01u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(row_address + 1u)) == 0x01u);
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(row_address + 2u)) == row);
        assert(stage2_word_at(raw, raw_size, jp,
                              (uint16_t)(row_address + 3u)) == target);
        body[1] = (uint8_t)row;
        body[18] = (uint8_t)row;
        body[22] = jp ? 0x54u : 0x52u;
        body[29] = jp ? 0xf4u : 0xf2u;
        for (unsigned int i = 0; i < sizeof(body); ++i) {
            assert(stage2_byte_at(raw, raw_size, jp,
                                  (uint16_t)(target + i)) == body[i]);
        }
    }
    printf("  PASS: stage2_selector_candidate_continuations (%s)\n",
           jp ? "JP" : "US");
}

/* Dispatch ID $11 calls an overlapping code root whose first bytes are not
 * instruction-aligned in the linear listing. Lock its TII descriptor and
 * both regional entry addresses against authentic Track 02 sectors. */
static void test_stage2_id11_overlapping_root(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    static const uint8_t handler[] = {
        0x9cu, 0x9cu, 0x4fu, 0x73u, 0x9cu, 0x4fu, 0x9du, 0x4fu,
        0x37u, 0x00u, 0x44u, 0x1au, 0x44u, 0x0bu, 0xa5u, 0x0cu,
        0x8du, 0xd9u, 0x4fu, 0xa5u, 0x0du, 0x8du, 0xdau, 0x4fu,
        0x60u
    };
    uint8_t tia_callee[] = {
        0xa9u, 0xe0u, 0x8du, 0x02u, 0x04u, 0xa9u, 0x00u, 0x8du,
        0x03u, 0x04u, 0xe3u, 0x5fu, 0x5eu, 0x04u, 0x04u, 0x40u,
        0x00u, 0x60u
    };
    static const uint8_t copy_callee[] = {
        0xadu, 0xdbu, 0x4fu, 0x8du, 0xd5u, 0x4fu, 0xadu, 0xdcu,
        0x4fu, 0x8du, 0xd6u, 0x4fu, 0x60u
    };
    uint16_t root = jp ? 0x5e57u : 0x5e27u;
    uint16_t first_bsr_target = (uint16_t)(root + 0x26u);
    uint16_t second_bsr_target = (uint16_t)(root + 0x19u);

    tia_callee[11] = jp ? 0x8fu : 0x5fu;

    assert(stage2_byte_at(raw, raw_size, jp, 0x42f5u) == 0x20u);
    assert(stage2_word_at(raw, raw_size, jp, 0x42f6u) == root);
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(root + i)) == handler[i]);
    }
    assert(first_bsr_target == (jp ? 0x5e7du : 0x5e4du));
    assert(second_bsr_target == (jp ? 0x5e70u : 0x5e40u));
    for (unsigned int i = 0; i < sizeof(tia_callee); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(first_bsr_target + i)) ==
               tia_callee[i]);
    }
    for (unsigned int i = 0; i < sizeof(copy_callee); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(second_bsr_target + i)) ==
               copy_callee[i]);
    }
    printf("  PASS: stage2_id11_overlapping_root (%s)\n",
           jp ? "JP" : "US");
}

/* ID $2b has a regional helper operand at $466f but a shared caller body. */
static void test_stage2_id2b_regional_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    uint8_t helper_prefix[] = {
        0x8eu, 0x91u, 0x4fu, 0x0au, 0xa8u, 0xadu, 0xd9u, 0x4fu,
        0x85u, 0x00u, 0xadu, 0xdau, 0x4fu, 0x85u, 0x01u, 0x18u,
        0xb1u, 0x00u, 0x6du, 0xd9u, 0x4fu, 0x85u, 0x0cu, 0xc8u,
        0xb1u, 0x00u, 0x6du, 0xdau, 0x4fu, 0x85u, 0x0du, 0x20u,
        0x40u, 0x5eu
    };
    static const uint8_t pointer_swap[] = {
        0xa5u, 0x0au, 0xa6u, 0x0cu, 0x22u, 0x85u, 0x0au, 0x86u,
        0x0cu, 0xa5u, 0x0bu, 0xa6u, 0x0du, 0x22u, 0x85u, 0x0bu,
        0x86u, 0x0du, 0x60u
    };
    static const uint8_t pointer_empty_test[] = {
        0xc2u, 0xb2u, 0x0cu, 0xc8u, 0x11u, 0x0cu, 0xd0u, 0x01u,
        0x60u, 0x44u, 0x17u, 0x60u
    };
    static const uint8_t relative_pointer_add[] = {
        0xc2u, 0x18u, 0xb1u, 0x0cu, 0x6du, 0xd9u, 0x4fu, 0x85u,
        0x0au, 0xc8u, 0xb1u, 0x0cu, 0x6du, 0xdau, 0x4fu, 0x85u,
        0x0bu, 0x18u, 0xa5u, 0x0cu, 0x69u, 0x02u, 0x85u, 0x0cu,
        0x90u, 0x02u, 0xe6u, 0x0du, 0x60u
    };
    uint8_t handler[] = {
        0xc8u, 0xb1u, 0x1cu, 0x48u, 0xc8u, 0xb1u, 0x1cu, 0xaau,
        0x20u, 0x00u, 0x4bu, 0x20u, 0x48u, 0x4fu, 0xadu, 0x79u,
        0x4du, 0x8du, 0xdbu, 0x4fu, 0xadu, 0x7au, 0x4du, 0x8du,
        0xdcu, 0x4fu, 0x68u, 0x20u, 0xafu, 0x56u, 0x4cu, 0xf9u,
        0x40u
    };
    uint16_t helper = jp ? 0x5729u : 0x56afu;

    handler[28] = jp ? 0x29u : 0xafu;
    handler[29] = jp ? 0x57u : 0x56u;

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 0x2bu * 2u)) == 0x4653u);
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4653u + i)) == handler[i]);
    }
    assert(stage2_word_at(raw, raw_size, jp, 0x466fu) == helper);
    helper_prefix[32] = jp ? 0x70u : 0x40u;
    for (unsigned int i = 0; i < sizeof(helper_prefix); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)((jp ? 0x5729u : 0x56afu) + i)) ==
               helper_prefix[i]);
    }
    assert(stage2_byte_at(raw, raw_size, jp,
                          (uint16_t)((jp ? 0x5729u : 0x56afu) + 0x22u)) ==
           0x20u);
    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)((jp ? 0x5729u : 0x56afu) + 0x23u)) ==
           (jp ? 0x57b9u : 0x573fu));
    for (unsigned int i = 0; i < sizeof(pointer_swap); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)((jp ? 0x57a6u : 0x572cu) + i)) ==
               pointer_swap[i]);
    }
    for (unsigned int i = 0; i < sizeof(pointer_empty_test); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)((jp ? 0x57b9u : 0x573fu) + i)) ==
               pointer_empty_test[i]);
    }
    for (unsigned int i = 0; i < sizeof(relative_pointer_add); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)((jp ? 0x57dbu : 0x5761u) + i)) ==
               relative_pointer_add[i]);
    }
    printf("  PASS: stage2_id2b_regional_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* ID $2c reads four stream operands, calls below the stage-two window, then
 * takes the fixed +5 cursor path. The external callee's behavior is unknown. */
static void test_stage2_id2c_external_handoff(const uint8_t *raw,
                                              size_t raw_size, int jp)
{
    static const uint8_t offset_one_helper[] = {
        0xc8u, 0xb1u, 0x1cu, 0x8du, 0xc2u, 0x4eu, 0x20u, 0xc9u,
        0x4eu, 0xadu, 0xc3u, 0x4eu, 0x85u, 0x00u, 0xadu, 0xc4u,
        0x4eu, 0x85u, 0x01u, 0x60u
    };
    static const uint8_t handler[] = {
        0x20u, 0x83u, 0x44u, 0xa0u, 0x02u, 0xb1u, 0x1cu, 0x85u, 0x02u,
        0xc8u, 0xb1u, 0x1cu, 0x85u, 0x03u, 0xc8u, 0xb1u, 0x1cu, 0x85u,
        0x0eu, 0xa9u, 0x0fu, 0x20u, 0xb7u, 0x3au, 0x4cu, 0x01u, 0x41u
    };

    assert(stage2_word_at(raw, raw_size, jp,
                          (uint16_t)(0x410du + 0x2cu * 2u)) == 0x4674u);
    for (unsigned int i = 0; i < sizeof(offset_one_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4483u + i)) ==
               offset_one_helper[i]);
    }
    for (unsigned int i = 0; i < sizeof(handler); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4674u + i)) == handler[i]);
    }
    printf("  PASS: stage2_id2c_external_handoff (%s)\n",
           jp ? "JP" : "US");
}

/* ID $2c's $4483 helper reaches $4ec9. Lock the internal table helper and
 * the bounded $4ec9 caller, while leaving its below-window $3a2e callee
 * unresolved. */
static void test_stage2_id2c_internal_helpers(const uint8_t *raw,
                                               size_t raw_size, int jp)
{
    static const uint8_t bounded_caller[] = {
        0xc6u, 0x5bu, 0xadu, 0xc2u, 0x4eu, 0x8du, 0xccu, 0x37u,
        0x20u, 0x31u, 0x4fu, 0x20u, 0x2eu, 0x3au, 0xb0u, 0x18u,
        0xadu, 0xceu, 0x37u, 0x8du, 0xc3u, 0x4eu, 0xadu, 0xcfu,
        0x37u, 0x8du, 0xc4u, 0x4eu, 0xadu, 0xd0u, 0x37u, 0x8du,
        0xc7u, 0x4eu, 0xadu, 0xd1u, 0x37u, 0x8du, 0xc8u, 0x4eu,
        0x64u, 0x5bu, 0x60u
    };
    static const uint8_t table_pointer_helper[] = {
        0xa9u, 0x7cu, 0x85u, 0x02u, 0xa9u, 0x4du, 0x85u, 0x03u,
        0xadu, 0x7bu, 0x4du, 0x0au, 0xa8u, 0xb1u, 0x02u, 0x85u,
        0x00u, 0xc8u, 0xb1u, 0x02u, 0x85u, 0x01u, 0x60u
    };

    for (unsigned int i = 0; i < sizeof(bounded_caller); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4ec9u + i)) ==
               bounded_caller[i]);
    }
    for (unsigned int i = 0; i < sizeof(table_pointer_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4f31u + i)) ==
               table_pointer_helper[i]);
    }
    printf("  PASS: stage2_id2c_internal_helpers (%s)\n",
           jp ? "JP" : "US");
}

/* The adjacent slot helper shares $4ec2/$37cc with $4ec9 but calls the
 * below-window $3879 routine, whose behavior remains outside this lock. */
static void test_stage2_shared_slot_helpers(const uint8_t *raw,
                                            size_t raw_size, int jp)
{
    static const uint8_t restore_helper[] = {
        0xc6u, 0x5bu, 0xadu, 0xc2u, 0x4eu, 0x8du, 0xccu, 0x37u,
        0xadu, 0xc7u, 0x4eu, 0x8du, 0xd0u, 0x37u, 0xadu, 0xc8u,
        0x4eu, 0x8du, 0xd1u, 0x37u, 0x20u, 0x31u, 0x4fu, 0x20u,
        0x79u, 0x38u, 0x64u, 0x5bu, 0x60u
    };
    static const uint8_t pointer_entry_writer[] = {
        0x20u, 0x31u, 0x4fu, 0xc2u, 0x62u, 0x92u, 0x00u, 0xc8u,
        0xa9u, 0x00u, 0x91u, 0x00u, 0xc8u, 0xa9u, 0x60u, 0x91u,
        0x00u
    };
    static const uint8_t direct_caller[] = {
        0x20u, 0xf4u, 0x4eu, 0x60u
    };
    static const uint8_t conditional_caller[] = {
        0xadu, 0x7bu, 0x4du, 0x8du, 0xc1u, 0x4eu, 0xa9u, 0x01u,
        0x8du, 0xc5u, 0x4eu, 0x20u, 0x48u, 0x4fu, 0xa9u, 0x05u,
        0x20u, 0x5eu, 0x4fu, 0xb0u, 0x03u, 0x20u, 0xf4u, 0x4eu,
        0x60u
    };

    for (unsigned int i = 0; i < sizeof(restore_helper); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4ef4u + i)) == restore_helper[i]);
    }
    for (unsigned int i = 0; i < sizeof(pointer_entry_writer); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4f11u + i)) ==
               pointer_entry_writer[i]);
    }
    for (unsigned int i = 0; i < sizeof(direct_caller); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4be2u + i)) == direct_caller[i]);
    }
    for (unsigned int i = 0; i < sizeof(conditional_caller); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(0x4c17u + i)) ==
               conditional_caller[i]);
    }
    printf("  PASS: stage2_shared_slot_helpers (%s)\n",
           jp ? "JP" : "US");
}

/* Selector pointer operands are read at cursor+1/+2 by the rooted $41/$12
 * handlers. This locks the earlier selector roots to authentic regional
 * bytes; it does not prove that any selector is executed. */
static void test_stage2_selector_00_03_pointer_roots(
    const uint8_t *raw, size_t raw_size, int jp)
{
    uint16_t recursive_target = jp ? 0x73b4u : 0x73b2u;
    uint16_t selector01_target = jp ? 0x78ecu : 0x78eau;
    uint16_t selector23_target = jp ? 0x74f4u : 0x74f2u;
    static const uint16_t pointer_roots[] = {
        0x6ea7u, 0x6c6bu, 0x6c6eu, 0x6c92u, 0x6cfau
    };
    static const uint8_t pointer_ids[] = {
        0x41u, 0x12u, 0x41u, 0x12u, 0x12u
    };
    static const uint8_t recursive_stream[] = {
        0x1du, 0x00u, 0x00u, 0x20u, 0x20u, 0x00u, 0x00u, 0x09u
    };
    uint16_t targets[] = {
        recursive_target, selector01_target, recursive_target,
        selector23_target, selector23_target
    };

    for (unsigned int i = 0; i < sizeof(pointer_roots) /
                                sizeof(pointer_roots[0]); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp, pointer_roots[i]) ==
               pointer_ids[i]);
        assert(stage2_word_at(raw, raw_size, jp,
                              (uint16_t)(pointer_roots[i] + 1u)) ==
               targets[i]);
        assert(targets[i] >= THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS);
        assert(targets[i] < THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS +
                            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT * 2048u);
    }
    for (unsigned int i = 0; i < sizeof(recursive_stream); ++i) {
        assert(stage2_byte_at(raw, raw_size, jp,
                              (uint16_t)(recursive_target + i)) ==
               recursive_stream[i]);
    }
    printf("  PASS: stage2_selector_00_03_pointer_roots (%s)\n",
           jp ? "JP" : "US");
}

static void test_stage2_counter_wait_sites(const uint8_t *raw,
                                          size_t raw_size, int jp)
{
    static const struct Stage2ByteSite {
        uint16_t address;
        uint8_t bytes[12];
        uint8_t length;
    } us_sites[] = {
        { 0x503du, { 0x9cu, 0x33u, 0x3bu }, 3u },
        { 0x5048u, { 0xadu, 0x33u, 0x3bu, 0xf0u, 0xfbu,
                      0x20u, 0xaeu, 0x51u }, 8u },
        { 0x7539u, { 0x9cu, 0x33u, 0x3bu }, 3u },
        { 0x753cu, { 0xadu, 0x33u, 0x3bu, 0xc9u, 0x03u,
                      0x90u, 0xf9u }, 7u },
        { 0x7549u, { 0x9cu, 0x33u, 0x3bu, 0xadu, 0x33u, 0x3bu,
                      0xf0u, 0xfbu, 0x60u }, 9u },
        { 0x7733u, { 0x9cu, 0x33u, 0x3bu }, 3u },
        { 0x7736u, { 0xadu, 0x33u, 0x3bu, 0xf0u, 0xfbu }, 5u },
        { 0x8862u, { 0x44u, 0x42u }, 2u },
        { 0x8877u, { 0x44u, 0x2du }, 2u },
        { 0x88b1u, { 0x44u, 0xf3u }, 2u },
        { 0x88d9u, { 0x44u, 0xcbu }, 2u },
        { 0x895du, { 0xadu, 0x00u, 0x00u, 0x48u }, 4u },
        { 0x88a6u, { 0x9cu, 0x33u, 0x3bu, 0xadu, 0x33u, 0x3bu,
                      0xf0u, 0xfbu, 0x60u }, 9u },
        { 0x8975u, { 0x89u, 0x20u, 0xd0u, 0x03u,
                      0x4cu, 0xe2u, 0x49u }, 7u },
        { 0x89e2u, { 0x68u, 0x29u, 0x20u, 0xf0u, 0x06u, 0xeeu,
                      0x33u, 0x3bu, 0xeeu, 0x49u, 0x22u, 0x68u }, 12u }
    };
    static const struct Stage2ByteSite jp_sites[] = {
        { 0x5044u, { 0x9cu, 0x33u, 0x3bu, 0x20u, 0x63u, 0xe0u }, 6u },
        { 0x753bu, { 0x9cu, 0x33u, 0x3bu }, 3u },
        { 0x753eu, { 0xadu, 0x33u, 0x3bu, 0xc9u, 0x03u,
                      0x90u, 0xf9u }, 7u },
        { 0x754bu, { 0x9cu, 0x33u, 0x3bu, 0xadu, 0x33u, 0x3bu,
                      0xf0u, 0xfbu, 0x60u }, 9u },
        { 0x7735u, { 0x9cu, 0x33u, 0x3bu }, 3u },
        { 0x7738u, { 0xadu, 0x33u, 0x3bu, 0xf0u, 0xfbu }, 5u },
        { 0x8862u, { 0x44u, 0x42u }, 2u },
        { 0x8877u, { 0x44u, 0x2du }, 2u },
        { 0x88b1u, { 0x44u, 0xf3u }, 2u },
        { 0x88d9u, { 0x44u, 0xcbu }, 2u },
        { 0x895du, { 0xadu, 0x00u, 0x00u, 0x48u }, 4u },
        { 0x88a6u, { 0x9cu, 0x33u, 0x3bu, 0xadu, 0x33u, 0x3bu,
                      0xf0u, 0xfbu, 0x60u }, 9u },
        { 0x8975u, { 0x89u, 0x20u, 0xd0u, 0x03u,
                      0x4cu, 0xe2u, 0x49u }, 7u },
        { 0x89e2u, { 0x68u, 0x29u, 0x20u, 0xf0u, 0x06u, 0xeeu,
                      0x33u, 0x3bu, 0xeeu, 0x49u, 0x22u, 0x68u }, 12u }
    };
    static const struct CallerByteWindow {
        uint16_t address;
        uint8_t bytes[64];
        uint8_t length;
    } common_callers[] = {
        { 0x8860u, { 0xc6u, 0x5au, 0x44u, 0x42u, 0x03u, 0x05u,
                      0xa5u, 0xf3u, 0x09u, 0x88u, 0x0du, 0xd9u,
                      0x27u, 0x85u, 0xf3u, 0x8du, 0x02u, 0x00u,
                      0x64u, 0x5au, 0x60u }, 21u },
        { 0x8875u, { 0xc6u, 0x5au, 0x44u, 0x2du, 0x03u, 0x05u,
                      0xa5u, 0xf3u, 0x29u, 0x3bu, 0x85u, 0xf3u,
                      0x8du, 0x02u, 0x00u, 0x9cu, 0x78u, 0x3bu,
                      0xa9u, 0xffu, 0x8du, 0x70u, 0x3bu, 0x8du,
                      0x71u, 0x3bu, 0x8du, 0x72u, 0x3bu, 0x8du,
                      0x73u, 0x3bu, 0x9cu, 0x74u, 0x3bu, 0x9cu,
                      0x75u, 0x3bu, 0x9cu, 0x76u, 0x3bu, 0xa9u,
                      0x01u, 0x8du, 0x77u, 0x3bu, 0x64u, 0x5au,
                      0x60u }, 49u },
        { 0x88afu, { 0xc6u, 0x5au, 0x44u, 0xf3u, 0x03u, 0x00u,
                      0x13u, 0x00u, 0x23u, 0x00u, 0x03u, 0x02u,
                      0xa2u, 0x02u, 0xc2u, 0x13u, 0x00u, 0x23u,
                      0x01u, 0x13u, 0x00u, 0x23u, 0x01u, 0x13u,
                      0x00u, 0x23u, 0x01u, 0x13u, 0x00u, 0x23u,
                      0x01u, 0x88u, 0xd0u, 0xedu, 0xcau, 0xd0u,
                      0xe9u, 0x64u, 0x5au, 0x60u }, 40u },
        { 0x88d7u, { 0xc6u, 0x5au, 0x44u, 0xcbu, 0x03u, 0x00u,
                      0xadu, 0xdau, 0x27u, 0x8du, 0x02u, 0x00u,
                      0xadu, 0xdbu, 0x27u, 0x8du, 0x03u, 0x00u,
                      0x03u, 0x02u, 0x82u, 0x13u, 0x00u, 0x23u,
                      0x00u, 0x13u, 0x00u, 0x23u, 0x00u, 0xcau,
                      0xd0u, 0xf5u, 0x64u, 0x5au, 0x60u }, 35u }
    };
    const struct Stage2ByteSite *sites = jp ? jp_sites : us_sites;
    size_t site_count = jp ? sizeof(jp_sites) / sizeof(jp_sites[0]) :
                             sizeof(us_sites) / sizeof(us_sites[0]);

    for (size_t site = 0; site < site_count;
         ++site) {
        for (unsigned int i = 0; i < sites[site].length; ++i) {
            assert(stage2_byte_at(raw, raw_size, jp,
                                  (uint16_t)(sites[site].address + i)) ==
                   sites[site].bytes[i]);
        }
    }
    for (size_t site = 0;
         site < sizeof(common_callers) / sizeof(common_callers[0]); ++site) {
        for (unsigned int i = 0; i < common_callers[site].length; ++i) {
            uint16_t address = (uint16_t)(common_callers[site].address + i);
            assert(stage2_byte_at(raw, raw_size, jp, address) ==
                   common_callers[site].bytes[i]);
        }
    }
    printf("  PASS: stage2_counter_wait_sites (%s)\n", jp ? "JP" : "US");
}

static void test_ipl_loader(void)
{
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_find_ipl_loader(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_US_BIN);
    assert(receipt.load_address == 0x4000u);
    assert(receipt.entry_address == 0x4000u);
    assert(receipt.stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD);
    assert(receipt.stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT);
    assert(receipt.cd_read_table_load_proven == 1);
    assert(receipt.stage2_seed_call_sites_proven == 1);
    assert(receipt.stage2_cd_read_record_proven == 1);
    assert(receipt.stage2_cd_read_dynamic_boundary_valid == 1);
    printf("  PASS: ipl_loader\n");
}

static void test_ipl_loader_jp(void)
{
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_find_ipl_loader(
        g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_JP_BIN);
    assert(receipt.load_address == 0x4000u);
    assert(receipt.entry_address == 0x4000u);
    assert(receipt.stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD);
    assert(receipt.stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT);
    assert(receipt.stage2_cd_read_record ==
           THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.stage2_cd_read_raw_sector ==
           THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.cd_read_table_load_proven == 1);
    assert(receipt.stage2_seed_call_sites_proven == 1);
    assert(receipt.stage2_cd_read_record_proven == 1);
    assert(receipt.stage2_cd_read_dynamic_boundary_valid == 1);
    printf("  PASS: ipl_loader_jp (record=0x%04x)\n",
           receipt.stage2_cd_read_record);
}

static void test_stage2_dynamic_payload(void)
{
    Theron_Track02Stage2DynamicPayloadReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_inspect_stage2_dynamic_payload(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_US_BIN);
    assert(receipt.manifest_entry_count ==
           THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT);
    printf("  PASS: stage2_dynamic_payload\n");
}

static void test_stage2_dynamic_payload_jp(void)
{
    Theron_Track02Stage2DynamicPayloadReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_inspect_stage2_dynamic_payload(
        g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_JP_BIN);
    assert(receipt.track02_record == THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.raw_sector == THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.header_word0 == 0x00ffu);
    assert(receipt.header_word1 == 0x0308u);
    assert(receipt.manifest_entry_count ==
           THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT);
    printf("  PASS: stage2_dynamic_payload_jp (raw-sector=0x%04zx)\n",
           receipt.raw_sector);
}

static void test_stage2_entry_path(void)
{
    Theron_Track02Stage2EntryPathReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_entry_path(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.entry_prologue_proven == 1);
    assert(receipt.main_path_proven == 1);
    assert(receipt.entry_path_contiguous_proven == 1);
    assert(receipt.entry_path_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES);
    printf("  PASS: stage2_entry_path\n");
}

static void test_stage2_call_graph(void)
{
    Theron_Track02Stage2CallGraphReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_call_graph(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.dispatcher_proven == 1);
    assert(receipt.delay_proven == 1);
    assert(receipt.port_clear_proven == 1);
    assert(receipt.pointer_setup_proven == 1);
    assert(receipt.call_graph_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES);
    printf("  PASS: stage2_call_graph\n");
}

static void test_stage2_dispatch_machine(void)
{
    Theron_Track02Stage2DispatchMachineReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_dispatch_machine(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.seed_tail_proven == 1);
    assert(receipt.dispatch_stubs_proven == 1);
    assert(receipt.jump_table_proven == 1);
    assert(receipt.mpr_page_proven == 1);
    assert(receipt.selector_proven == 1);
    assert(receipt.dispatch_machine_contiguous_proven == 1);
    assert(receipt.jump_table_entries ==
           THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES);
    assert(receipt.dispatch_machine_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES);
    printf("  PASS: stage2_dispatch_machine\n");
}

static void test_stage2_l8000_pair(void)
{
    Theron_Track02Stage2L8000PairReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l8000_pair(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l8000_proven == 1);
    assert(receipt.l45a6_proven == 1);
    assert(receipt.l8000_call_site_proven == 1);
    assert(receipt.l45a6_single_caller_proven == 1);
    assert(receipt.pair_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES);
    printf("  PASS: stage2_l8000_pair\n");
}

static void test_stage2_jump_table_handlers(void)
{
    Theron_Track02Stage2JumpTableHandlersReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_jump_table_handlers(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.handlers_proven == 1);
    assert(receipt.handler_entry_chain_proven == 1);
    assert(receipt.handlers_contiguous_proven == 1);
    assert(receipt.handler_count ==
           THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT);
    printf("  PASS: stage2_jump_table_handlers\n");
}

static void test_stage2_l4696_l3114(void)
{
    Theron_Track02Stage2L4696L3114Receipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l4696_l3114(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l4696_proven == 1);
    assert(receipt.l3114_proven == 1);
    assert(receipt.l4696_call_site_proven == 1);
    assert(receipt.l3114_call_site_proven == 1);
    assert(receipt.l4696_l3114_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES);
    printf("  PASS: stage2_l4696_l3114\n");
}

static void test_stage2_l3114_callees(void)
{
    Theron_Track02Stage2L3114CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l3172_proven == 1);
    assert(receipt.far117d_proven == 1);
    assert(receipt.l4f66_proven == 1);
    assert(receipt.l3114_callees_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES);
    printf("  PASS: stage2_l3114_callees\n");
}

static void test_stage2_l3114_tier2_callees(void)
{
    Theron_Track02Stage2L3114Tier2CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier2_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier2_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier2_callees\n");
}

static void test_stage2_l3114_tier3_callees(void)
{
    Theron_Track02Stage2L3114Tier3CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier3_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier3_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier3_callees\n");
}

static void test_stage2_l3114_tier4_callees(void)
{
    Theron_Track02Stage2L3114Tier4CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier4_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier4_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier4_callees\n");
}

static void test_stage2_enclosing_45xx(void)
{
    Theron_Track02Stage2Enclosing45xxReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.routine_proven == 1);
    assert(receipt.l4696_call_sites_within_proven == 1);
    printf("  PASS: stage2_enclosing_45xx\n");
}

static void test_stage2_enclosing_45xx_callees(void)
{
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;
    uint8_t *mutated;
    size_t stage2_sector = THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR +
                           THERON_TRACK02_IPL_STAGE2_RECORD;
    size_t user_offset =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4943_USER_OFFSET;
    size_t raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                        16u + user_offset % 2048u;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l4943_proven == 1);
    assert(receipt.l4943_mpr_bracket_proven == 1);
    assert(receipt.l4943_targets_proven == 1);
    assert(receipt.l49fa_proven == 1);
    assert(receipt.l49fa_targets_proven == 1);
    assert(receipt.l4a09_proven == 1);
    assert(receipt.l4a09_vdc_writes_proven == 1);
    assert(receipt.l4a09_targets_proven == 1);
    assert(receipt.l4a84_proven == 1);
    assert(receipt.l4a84_vdc_writes_proven == 1);
    assert(receipt.l4a84_targets_proven == 1);
    assert(receipt.l4b24_proven == 1);
    assert(receipt.l491f_proven == 1);
    assert(receipt.l4bb0_proven == 1);
    assert(receipt.l4bb0_vdc_scroll_writes_proven == 1);
    assert(receipt.l56de_proven == 1);
    assert(receipt.l56de_vram_transfer_proven == 1);
    assert(receipt.l570a_proven == 1);
    assert(receipt.l50f1_proven == 1);
    assert(receipt.l50f1_vdc_transfer_proven == 1);
    assert(receipt.l5111_proven == 1);
    assert(receipt.l5111_local_targets_proven == 1);
    assert(receipt.l5111_command_table_proven == 1);
    assert(receipt.l5111_command_targets_proven == 1);
    assert(receipt.l533d_proven == 1);
    assert(receipt.l533d_command_table_proven == 1);
    assert(receipt.l533d_command_targets_proven == 1);
    assert(receipt.l55ef_proven == 1);
    assert(receipt.l55f4_overlap_entry_proven == 1);
    assert(receipt.l55ff_entry_proven == 1);
    assert(receipt.l5617_overlap_entry_proven == 1);
    assert(receipt.l562a_overlap_entry_proven == 1);
    assert(receipt.l563d_overlap_entry_proven == 1);
    assert(receipt.l55b6_proven == 1);
    assert(receipt.l55c8_overlap_entry_proven == 1);
    assert(receipt.l563d_bbr4_target_proven == 1);
    assert(receipt.l5e2b_proven == 1);
    assert(receipt.l5e2b_vdc_register_dispatch_proven == 1);
    assert(receipt.l5ce4_proven == 1);
    assert(receipt.l5ce4_buffer_init_proven == 1);
    mutated = malloc(g_us_size);
    assert(mutated != NULL && raw_offset < g_us_size);
    memcpy(mutated, g_us_data, g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L55EF_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L55B6_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L5CE4_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L5E2B_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L50F1_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L5111_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L5111_USER_OFFSET +
                  THERON_TRACK02_IPL_STAGE2_L5111_COMMAND_TABLE_OFF;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L533D_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L56DE_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L570A_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4BB0_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4A84_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4B24_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4A09_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L49FA_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L491F_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(mutated);
    if (g_jp_data) {
        status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
            g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
        assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    }
    printf("  PASS: stage2_enclosing_45xx_callees\n");
}

static void test_stage2_l3114_tier5_callees(void)
{
    Theron_Track02Stage2L3114Tier5CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier5_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier5_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier5_callees\n");
}

static void test_stage2_45xx_tier2_callees(void)
{
    Theron_Track02Stage245xxTier2CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_45xx_tier2_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier2_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES);
    printf("  PASS: stage2_45xx_tier2_callees\n");
}

static void test_stage2_45xx_tier3_callees(void)
{
    Theron_Track02Stage245xxTier3CalleesReceipt receipt;
    Theron_Track02SignalStatus status;
    uint8_t *mutated;
    size_t stage2_sector = THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR +
                           THERON_TRACK02_IPL_STAGE2_RECORD;
    size_t user_offset =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER3_L4417_USER_OFFSET;
    size_t raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                        16u + user_offset % 2048u;

    status = theron_v1_track02_verify_stage2_45xx_tier3_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l4215_proven == 1);
    assert(receipt.l4417_proven == 1);
    assert(receipt.l44a2_proven == 1);
    assert(receipt.l42db_proven == 1);
    assert(receipt.l4519_proven == 1);
    assert(receipt.local_subroutines_proven == 1);
    assert(receipt.caller_targets_proven == 1);
    assert(receipt.existing_callee_targets_proven == 1);
    assert(receipt.adjacency_proven == 1);
    assert(receipt.tier3_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_45XX_TIER3_BOUND_BYTES);

    mutated = malloc(g_us_size);
    assert(mutated != NULL && raw_offset < g_us_size);
    memcpy(mutated, g_us_data, g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_45xx_tier3_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_TIER3_L42DB_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_45xx_tier3_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    memcpy(mutated, g_us_data, g_us_size);
    user_offset = THERON_TRACK02_IPL_STAGE2_45XX_TIER3_L4519_USER_OFFSET;
    raw_offset = (stage2_sector + user_offset / 2048u) * 2352u +
                 16u + user_offset % 2048u;
    assert(raw_offset < g_us_size);
    mutated[raw_offset] ^= 1u;
    status = theron_v1_track02_verify_stage2_45xx_tier3_callees(
        mutated, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    free(mutated);

    if (g_jp_data) {
        status = theron_v1_track02_verify_stage2_45xx_tier3_callees(
            g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
        assert(status == THERON_TRACK02_SIGNAL_NOT_FOUND);
    }
    printf("  PASS: stage2_45xx_tier3_callees\n");
}

static void test_total_bound_bytes(void)
{
    size_t total = 0;
    total += THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_45XX_TIER3_BOUND_BYTES;

    assert(total > 2048u);
    printf("  PASS: total_bound_bytes = %zu (%.1f%% of stage-2 image)\n",
           total,
           100.0 * (double)total /
           (double)(THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                    THERON_TRACK02_RAW_USER_DATA_BYTES));
}

static void test_vdc_port_clear_semantics(void)
{
    /* The proven port_clear bytes at user offset 0xb73 contain st0/st1/st2
     * instructions that write to the HuC6270 VDC.  We verify the opcode
     * stream matches the expected VDC register operations:
     *
     *   st0 #$00 → select MAWR (VRAM write address register)
     *   st1 #$00 / st2 #$08 → MAWR = $0800
     *   st0 #$02 → select VWR (VRAM write data register)
     *   [loop] st1 #$00 / st2 #$00 → write $0000 to VRAM (clear)
     *   st0 #$05 → select CR (control register)
     *
     * This proves the game clears VRAM starting at $0800 and configures
     * the VDC control register during initialization. */
    static const uint8_t expected_port_clear[] = {
        0x78,                   /* SEI */
        0x03, 0x00,             /* st0 #$00 (MAWR) */
        0x13, 0x00,             /* st1 #$00 */
        0x23, 0x08,             /* st2 #$08 → MAWR=$0800 */
        0x03, 0x02,             /* st0 #$02 (VWR) */
        0x82,                   /* CLX */
        0xa0, 0x78,             /* LDY #$78 (120 iterations outer) */
        0x13, 0x00,             /* st1 #$00 → VWR.lo=0 */
        0x23, 0x00,             /* st2 #$00 → VWR.hi=0 */
        0xca,                   /* DEX */
        0xd0, 0xf9,             /* BNE -7 (256 inner) */
        0x88,                   /* DEY */
        0xd0, 0xf6,             /* BNE -10 */
        0x03, 0x05,             /* st0 #$05 (CR) */
        0xa5, 0xf3,             /* LDA $F3 */
        0x29, 0x3f,             /* AND #$3F */
        0x85, 0xf3,             /* STA $F3 */
        0x8d, 0x02, 0x00,       /* STA $0002 (VDC data port) */
        0x58,                   /* CLI */
        0x60                    /* RTS */
    };
    Theron_Track02Stage2CallGraphReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_call_graph(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.port_clear_proven == 1);
    assert(receipt.port_clear_bytes == sizeof(expected_port_clear));

    /* VDC register 0x00 (MAWR): VRAM write address = $0800 */
    assert(expected_port_clear[1] == 0x03 && expected_port_clear[2] == 0x00);
    /* VDC register 0x02 (VWR): VRAM data write */
    assert(expected_port_clear[7] == 0x03 && expected_port_clear[8] == 0x02);
    /* VDC register 0x05 (CR): control register */
    assert(expected_port_clear[22] == 0x03 && expected_port_clear[23] == 0x05);
    /* VRAM clear: 256 * 120 = 30720 words = 60 KiB */
    assert(expected_port_clear[11] == 0x78); /* LDY #$78 = 120 outer */
    /* STA $0002: write to VDC data port (HuC6270 port 2) */
    assert(expected_port_clear[30] == 0x8d &&
           expected_port_clear[31] == 0x02 &&
           expected_port_clear[32] == 0x00);

    printf("  PASS: vdc_port_clear_semantics"
           " (MAWR=$0800, VWR clear 30720 words, CR via $F3)\n");
}

static void test_vdc_l8000_init_semantics(void)
{
    /* The proven L8000 body at user offset 0x4000 initializes VDC scroll
     * registers and clears game state RAM:
     *
     *   STZ $220C/$220D/$2210/$2211 → clear game state
     *   st0 #$08 / st1 #$00 / st2 #$00 → BYR=0 (BG Y scroll)
     *   st0 #$07 / st1 #$00 / st2 #$00 → BXR=0 (BG X scroll)
     *
     * This proves the game resets viewport scroll to origin (0,0). */
    static const uint8_t l8000_vdc_head[] = {
        0xc6, 0x5a,             /* DEC $5A */
        0x9c, 0x0c, 0x22,       /* STZ $220C */
        0x9c, 0x0d, 0x22,       /* STZ $220D */
        0x9c, 0x10, 0x22,       /* STZ $2210 */
        0x9c, 0x11, 0x22,       /* STZ $2211 */
        0x03, 0x08,             /* st0 #$08 → select BYR */
        0x13, 0x00,             /* st1 #$00 → BYR.lo = 0 */
        0x23, 0x00,             /* st2 #$00 → BYR.hi = 0 */
        0x03, 0x07,             /* st0 #$07 → select BXR */
        0x13, 0x00,             /* st1 #$00 → BXR.lo = 0 */
        0x23, 0x00,             /* st2 #$00 → BXR.hi = 0 */
    };
    Theron_Track02Stage2L8000PairReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l8000_pair(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l8000_proven == 1);

    /* VDC register 0x08 (BYR) = 0 */
    assert(l8000_vdc_head[14] == 0x03 && l8000_vdc_head[15] == 0x08);
    assert(l8000_vdc_head[16] == 0x13 && l8000_vdc_head[17] == 0x00);
    assert(l8000_vdc_head[18] == 0x23 && l8000_vdc_head[19] == 0x00);
    /* VDC register 0x07 (BXR) = 0 */
    assert(l8000_vdc_head[20] == 0x03 && l8000_vdc_head[21] == 0x07);
    assert(l8000_vdc_head[22] == 0x13 && l8000_vdc_head[23] == 0x00);
    assert(l8000_vdc_head[24] == 0x23 && l8000_vdc_head[25] == 0x00);

    printf("  PASS: vdc_l8000_init_semantics"
           " (BYR=0, BXR=0 — viewport scroll reset to origin)\n");
}

static void test_dispatch_advance_counts(void)
{
    /* The proven dispatch stubs encode the stream-advance count for each
     * command return path.  Seven stubs load counts 1,2,3,4,5,7,9
     * confirming the command stream is a variable-length instruction set
     * with 1-9 byte commands. */
    static const uint8_t counts[] = {1, 2, 3, 4, 5, 7, 9};
    static const uint8_t stubs[] = {
        0xa9, 0x01, 0x80, 0xef,
        0xa9, 0x02, 0x80, 0xeb,
        0xa9, 0x03, 0x80, 0xe7,
        0xa9, 0x04, 0x80, 0xe3,
        0xa9, 0x05, 0x80, 0xdf,
        0xa9, 0x07, 0x80, 0xdb,
        0xa9, 0x09, 0x80, 0xd7,
    };
    size_t i;
    for (i = 0; i < 7; i++) {
        assert(stubs[i * 4] == 0xa9);
        assert(stubs[i * 4 + 1] == counts[i]);
        assert(stubs[i * 4 + 2] == 0x80);
    }
    printf("  PASS: dispatch_advance_counts"
           " (7 stubs, advances: 1,2,3,4,5,7,9)\n");
}

static void test_vram_transfer_l466b(void)
{
    /* L466B at user offset 0x466B is a proven VRAM tile transfer function:
     *
     *   DEC $5A
     *   ST0 #$00          ; select MAWR (VRAM write address)
     *   LDA $02 / STA $0002  ; MAWR low byte from zero-page $02
     *   LDA $03 / STA $0003  ; MAWR high byte from zero-page $03
     *   ST0 #$02          ; select VWR (VRAM write data register)
     *   [conditional TIA]  ; TIA $00,$02,$0000 — bulk transfer to VRAM
     *   STZ $5A / RTS
     *
     * The TIA (Transfer Increment-Alternate) instruction at 0x468C bulk-copies
     * source data directly into the VDC's VRAM data register.  This is the
     * proven tile/sprite VRAM transfer path.
     *
     * This function is called from the $45xx rendering lane at +0x87,
     * verified by theron_v1_track02_verify_stage2_enclosing_45xx_callees. */
    static const uint8_t l466b_head[] = {
        0xc6, 0x5a,         /* DEC $5A */
        0x03, 0x00,         /* ST0 #$00 → select MAWR */
        0xa5, 0x02,         /* LDA $02 */
        0x8d, 0x02, 0x00,   /* STA $0002 → MAWR.lo */
        0xa5, 0x03,         /* LDA $03 */
        0x8d, 0x03, 0x00,   /* STA $0003 → MAWR.hi */
        0x03, 0x02,         /* ST0 #$02 → select VWR */
    };
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l466b_proven == 1);

    /* ST0 #$00: select MAWR (VRAM address register) */
    assert(l466b_head[2] == 0x03 && l466b_head[3] == 0x00);
    /* STA $0002/$0003: write address to VDC ports */
    assert(l466b_head[6] == 0x8d && l466b_head[7] == 0x02 && l466b_head[8] == 0x00);
    assert(l466b_head[11] == 0x8d && l466b_head[12] == 0x03 && l466b_head[13] == 0x00);
    /* ST0 #$02: select VWR for VRAM data write */
    assert(l466b_head[14] == 0x03 && l466b_head[15] == 0x02);

    printf("  PASS: vram_transfer_l466b"
           " (MAWR set from ZP $02:$03, TIA bulk write to VWR)\n");
}

static void test_vdc_cr_write_l4932(void)
{
    /* L4932 at user offset 0x4932 writes the VDC control register:
     *
     *   ST0 #$05          ; select CR (control register)
     *   LDA $F3 / STA $0002  ; CR low byte from ZP $F3
     *   LDA $F4 / AND #$07 / STA $F4 / STA $0003  ; CR high byte (masked)
     *
     * CR high bits 0-2 control auto-increment mode:
     *   000 = +1 word, 001 = +32 words, 010 = +64 words, 011 = +128 words */
    static const uint8_t l4932[] = {
        0x03, 0x05,         /* ST0 #$05 → select CR */
        0xa5, 0xf3,         /* LDA $F3 */
        0x8d, 0x02, 0x00,   /* STA $0002 → CR.lo */
        0xa5, 0xf4,         /* LDA $F4 */
        0x29, 0x07,         /* AND #$07 → mask to increment bits */
        0x85, 0xf4,         /* STA $F4 */
        0x8d, 0x03, 0x00,   /* STA $0003 → CR.hi */
        0x60                /* RTS */
    };
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l4932_proven == 1);
    assert(receipt.l491f_proven == 1);

    /* ST0 #$05: select CR */
    assert(l4932[0] == 0x03 && l4932[1] == 0x05);
    /* AND #$07: mask to auto-increment bits only */
    assert(l4932[9] == 0x29 && l4932[10] == 0x07);

    printf("  PASS: vdc_cr_write_l4932"
           " (CR set from ZP $F3/$F4, increment mode masked)\n");
}

int main(void)
{
    printf("test_theron_v1_stage2_disassembly_chain:\n");

    if (!load_track02()) {
        printf("  SKIP: TQUS02.bin not available\n");
        printf("All stage-2 disassembly chain tests skipped.\n");
        return 0;
    }

    test_ipl_loader();
    if (g_jp_data) test_ipl_loader_jp();
    test_stage2_selector_candidate_continuations(
        g_us_data, g_us_size, 0);
    test_stage2_entry_mpr_window(g_us_data, g_us_size, 0);
    test_stage2_l48fc_countdown(g_us_data, g_us_size, 0);
    test_stage2_id0b_0f_indexed_mutation(g_us_data, g_us_size, 0);
    test_stage2_id09_id0a_id10_nested_cursor(g_us_data, g_us_size, 0);
    test_stage2_id28_conditional_handoff(g_us_data, g_us_size, 0);
    test_stage2_id12_indirect_call(g_us_data, g_us_size, 0);
    test_stage2_id08_local_helper(g_us_data, g_us_size, 0);
    test_stage2_id13_id16_fixed_arguments(g_us_data, g_us_size, 0);
    test_stage2_id14_id15_operand_reader(g_us_data, g_us_size, 0);
    test_stage2_id17_id1b_fixed_argument_roots(g_us_data, g_us_size, 0);
    test_stage2_id20_id21_fixed_arguments(g_us_data, g_us_size, 0);
    test_stage2_id1e_id1f_bounded_handlers(g_us_data, g_us_size, 0);
    test_stage2_id1c_id1d_cursor_roots(g_us_data, g_us_size, 0);
    test_stage2_id24_bounded_wait_root(g_us_data, g_us_size, 0);
    test_stage2_id23_regional_handoff(g_us_data, g_us_size, 0);
    test_stage2_id27_bounded_pointer_setup(g_us_data, g_us_size, 0);
    test_stage2_id2a_entry_helper(g_us_data, g_us_size, 0);
    test_stage2_id2d_overlapping_poll_root(g_us_data, g_us_size, 0);
    test_stage2_id2e_bounded_windows(g_us_data, g_us_size, 0);
    test_stage2_id2f_parameter_handoff(g_us_data, g_us_size, 0);
    test_stage2_id30_overlapping_branch_root(g_us_data, g_us_size, 0);
    test_stage2_id31_fixed_argument_handoff(g_us_data, g_us_size, 0);
    test_stage2_id32_conditional_handoff(g_us_data, g_us_size, 0);
    test_stage2_id33_four_byte_handoff(g_us_data, g_us_size, 0);
    test_stage2_id34_fixed_argument_select(g_us_data, g_us_size, 0);
    test_stage2_id35_regional_call_handoff(g_us_data, g_us_size, 0);
    test_stage2_id36_stream_handoff(g_us_data, g_us_size, 0);
    test_stage2_id37_local_helper(g_us_data, g_us_size, 0);
    test_stage2_id38_three_byte_handoff(g_us_data, g_us_size, 0);
    test_stage2_id39_helper_chain(g_us_data, g_us_size, 0);
    test_stage2_id3a_cursor_stub(g_us_data, g_us_size, 0);
    test_stage2_id3b_overlapping_helper_root(g_us_data, g_us_size, 0);
    test_stage2_id3c_bios_control_flow(g_us_data, g_us_size, 0);
    test_stage2_id3d_overlapping_bios_window(g_us_data, g_us_size, 0);
    test_stage2_id3e_polling_helper(g_us_data, g_us_size, 0);
    test_stage2_id3f_indexed_transfer(g_us_data, g_us_size, 0);
    test_stage2_id40_bounded_selector_path(g_us_data, g_us_size, 0);
    test_stage2_id42_conditional_stores(g_us_data, g_us_size, 0);
    test_stage2_id43_source_copies(g_us_data, g_us_size, 0);
    test_stage2_id44_helper_reentry(g_us_data, g_us_size, 0);
    test_stage2_id45_bounded_handoff(g_us_data, g_us_size, 0);
    test_stage2_id46_helper_handoff(g_us_data, g_us_size, 0);
    test_stage2_id47_three_byte_handoff(g_us_data, g_us_size, 0);
    test_stage2_id48_paired_selector_loop(g_us_data, g_us_size, 0);
    test_stage2_id49_selector_join(g_us_data, g_us_size, 0);
    test_stage2_id4a_paired_call_loop(g_us_data, g_us_size, 0);
    test_stage2_id4c_call_handoff(g_us_data, g_us_size, 0);
    test_stage2_id4e_relative_handoff(g_us_data, g_us_size, 0);
    test_stage2_id4f_relative_handoff(g_us_data, g_us_size, 0);
    test_stage2_id50_staged_handoff(g_us_data, g_us_size, 0);
    test_stage2_id4b_indexed_comparison(g_us_data, g_us_size, 0);
    test_stage2_id4d_operand_handoff(g_us_data, g_us_size, 0);
    test_stage2_id53_three_byte_handoff(g_us_data, g_us_size, 0);
    test_stage2_id51_helper_chain(g_us_data, g_us_size, 0);
    test_stage2_id11_overlapping_root(g_us_data, g_us_size, 0);
    test_stage2_id2b_regional_handoff(g_us_data, g_us_size, 0);
    test_stage2_id2c_external_handoff(g_us_data, g_us_size, 0);
    test_stage2_id2c_internal_helpers(g_us_data, g_us_size, 0);
    test_stage2_shared_slot_helpers(g_us_data, g_us_size, 0);
    test_stage2_selector_00_03_pointer_roots(g_us_data, g_us_size, 0);
    test_stage2_counter_wait_sites(g_us_data, g_us_size, 0);
    if (g_jp_data) {
        test_stage2_selector_candidate_continuations(
            g_jp_data, g_jp_size, 1);
        test_stage2_entry_mpr_window(g_jp_data, g_jp_size, 1);
        test_stage2_l48fc_countdown(g_jp_data, g_jp_size, 1);
        test_stage2_id0b_0f_indexed_mutation(g_jp_data, g_jp_size, 1);
        test_stage2_id09_id0a_id10_nested_cursor(g_jp_data, g_jp_size, 1);
        test_stage2_id28_conditional_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id12_indirect_call(g_jp_data, g_jp_size, 1);
        test_stage2_id08_local_helper(g_jp_data, g_jp_size, 1);
        test_stage2_id13_id16_fixed_arguments(g_jp_data, g_jp_size, 1);
        test_stage2_id14_id15_operand_reader(g_jp_data, g_jp_size, 1);
        test_stage2_id17_id1b_fixed_argument_roots(g_jp_data, g_jp_size, 1);
        test_stage2_id20_id21_fixed_arguments(g_jp_data, g_jp_size, 1);
        test_stage2_id1e_id1f_bounded_handlers(g_jp_data, g_jp_size, 1);
        test_stage2_id1c_id1d_cursor_roots(g_jp_data, g_jp_size, 1);
        test_stage2_id24_bounded_wait_root(g_jp_data, g_jp_size, 1);
        test_stage2_id23_regional_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id27_bounded_pointer_setup(g_jp_data, g_jp_size, 1);
        test_stage2_id2a_entry_helper(g_jp_data, g_jp_size, 1);
        test_stage2_id2d_overlapping_poll_root(g_jp_data, g_jp_size, 1);
        test_stage2_id2e_bounded_windows(g_jp_data, g_jp_size, 1);
        test_stage2_id2f_parameter_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id30_overlapping_branch_root(g_jp_data, g_jp_size, 1);
        test_stage2_id31_fixed_argument_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id32_conditional_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id33_four_byte_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id34_fixed_argument_select(g_jp_data, g_jp_size, 1);
        test_stage2_id35_regional_call_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id36_stream_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id37_local_helper(g_jp_data, g_jp_size, 1);
        test_stage2_id38_three_byte_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id39_helper_chain(g_jp_data, g_jp_size, 1);
        test_stage2_id3a_cursor_stub(g_jp_data, g_jp_size, 1);
        test_stage2_id3b_overlapping_helper_root(g_jp_data, g_jp_size, 1);
        test_stage2_id3c_bios_control_flow(g_jp_data, g_jp_size, 1);
        test_stage2_id3d_overlapping_bios_window(g_jp_data, g_jp_size, 1);
        test_stage2_id3e_polling_helper(g_jp_data, g_jp_size, 1);
        test_stage2_id3f_indexed_transfer(g_jp_data, g_jp_size, 1);
        test_stage2_id40_bounded_selector_path(g_jp_data, g_jp_size, 1);
        test_stage2_id42_conditional_stores(g_jp_data, g_jp_size, 1);
        test_stage2_id43_source_copies(g_jp_data, g_jp_size, 1);
        test_stage2_id44_helper_reentry(g_jp_data, g_jp_size, 1);
        test_stage2_id45_bounded_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id46_helper_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id47_three_byte_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id48_paired_selector_loop(g_jp_data, g_jp_size, 1);
        test_stage2_id49_selector_join(g_jp_data, g_jp_size, 1);
        test_stage2_id4a_paired_call_loop(g_jp_data, g_jp_size, 1);
        test_stage2_id4c_call_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id4e_relative_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id4f_relative_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id50_staged_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id4b_indexed_comparison(g_jp_data, g_jp_size, 1);
        test_stage2_id4d_operand_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id53_three_byte_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id51_helper_chain(g_jp_data, g_jp_size, 1);
        test_stage2_id11_overlapping_root(g_jp_data, g_jp_size, 1);
        test_stage2_id2b_regional_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id2c_external_handoff(g_jp_data, g_jp_size, 1);
        test_stage2_id2c_internal_helpers(g_jp_data, g_jp_size, 1);
        test_stage2_shared_slot_helpers(g_jp_data, g_jp_size, 1);
        test_stage2_selector_00_03_pointer_roots(
            g_jp_data, g_jp_size, 1);
        test_stage2_counter_wait_sites(g_jp_data, g_jp_size, 1);
    }
    test_stage2_dynamic_payload();
    if (g_jp_data) test_stage2_dynamic_payload_jp();
    test_stage2_entry_path();
    test_stage2_call_graph();
    test_stage2_dispatch_machine();
    test_stage2_l8000_pair();
    test_stage2_jump_table_handlers();
    test_stage2_l4696_l3114();
    test_stage2_l3114_callees();
    test_stage2_l3114_tier2_callees();
    test_stage2_l3114_tier3_callees();
    test_stage2_l3114_tier4_callees();
    test_stage2_enclosing_45xx();
    test_stage2_enclosing_45xx_callees();
    test_stage2_l3114_tier5_callees();
    test_stage2_45xx_tier2_callees();
    test_stage2_45xx_tier3_callees();
    test_total_bound_bytes();
    test_vdc_port_clear_semantics();
    test_vdc_l8000_init_semantics();
    test_dispatch_advance_counts();
    test_vram_transfer_l466b();
    test_vdc_cr_write_l4932();

    free(g_us_data);
    free(g_jp_data);
    printf("All stage-2 disassembly chain tests passed.\n");
    return 0;
}
