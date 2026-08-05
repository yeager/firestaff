/* test_dm2_v1_ccm_source_alignment_pc34_compat.c — DM2-005 follow-up:
 * legacy interpreter opcode numbering vs the source b_1a matrix.
 *
 * Cross-checks dm2_v1_ccm.c's opcode table against the verbatim
 * DM2_PROCEED_CCM compare chain bound in
 * dm2_v1_ccm_dispatch_pc34_compat.c (skproject/SKULLWIN/
 * c_creature.cpp:2930-3212):
 *
 *  1. Every legacy table row's source_group matches
 *     dm2_v1_ccm_dispatch_source_group(row.opcode).
 *  2. Key source mappings hold: 0x17=PLACE_MERCHANDISE,
 *     0x27/0x28=CAST_SPELL, 0x08/0x26=ATTACKS_PARTY,
 *     0x0E/0x0F=SHOOT_ITEM, 0x13=KILL_ON_TIMER_POSITION.
 *  3. Every source byte the compare chain routes to NO handler
 *     (NONE) is absent from the legacy table (fail-closed).
 *  4. Every non-NONE source byte 0x00-0x55 has a legacy table row.
 *  5. Row names equal the dispatch module's group names.
 *  6. HALT (0xFF) is the only row without a source group.
 *  7. Every source-owned row is marked implemented in the public table;
 *     actual execution remains callback-bound and fails closed without a
 *     live command stream/record owner.
 *  8. Source evidence is present in both modules.
 */

#include "dm2_v1_ccm.h"
#include "dm2_v1_ccm_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-58s", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        printf("  FAIL\n"); \
    } \
} while (0)

/* All b_1a bytes the legacy table implements or stubs. */
static const int g_table_bytes[DM2_CCM_MAX_OPCODES] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x13,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x55, 0xFF
};

static int test_every_row_group_matches_dispatch(void) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        int byte = g_table_bytes[i];
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(byte);
        DM2_V1_CcmSourceHandler group;
        if (!def) return 0;
        if (byte == 0xFF) continue;  /* Firestaff-internal control byte */
        group = dm2_v1_ccm_dispatch_source_group((uint8_t)byte);
        if (group == DM2_V1_CCM_SRC_NONE) return 0;
        if (def->source_group != (int)group) return 0;
    }
    return 1;
}

static int test_key_source_mappings(void) {
    const DM2_V1_CCMOpcodeDef *def;
    def = dm2_v1_ccm_get_opcode_def(0x17);
    if (!def || def->source_group != DM2_V1_CCM_SRC_PLACE_MERCHANDISE) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x27);
    if (!def || def->source_group != DM2_V1_CCM_SRC_CAST_SPELL) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x28);
    if (!def || def->source_group != DM2_V1_CCM_SRC_CAST_SPELL) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x08);
    if (!def || def->source_group != DM2_V1_CCM_SRC_ATTACKS_PARTY) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x26);
    if (!def || def->source_group != DM2_V1_CCM_SRC_ATTACKS_PARTY) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x0E);
    if (!def || def->source_group != DM2_V1_CCM_SRC_SHOOT_ITEM) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x0F);
    if (!def || def->source_group != DM2_V1_CCM_SRC_SHOOT_ITEM) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x13);
    if (!def || def->source_group != DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION) return 0;
    def = dm2_v1_ccm_get_opcode_def(0x3D);
    if (!def || def->source_group != DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON) return 0;
    return 1;
}

static int test_no_handler_bytes_absent(void) {
    /* Sweep the whole byte range: every byte the source chain routes to
     * NONE must be absent from the legacy table. */
    for (int byte = 0x00; byte <= 0xFE; byte++) {
        if (dm2_v1_ccm_dispatch_source_group((uint8_t)byte) ==
            DM2_V1_CCM_SRC_NONE) {
            if (dm2_v1_ccm_get_opcode_def(byte) != NULL) return 0;
        }
    }
    return 1;
}

static int test_every_handled_source_byte_has_row(void) {
    /* Within the proven table1d613a span (0x00-0x55), every byte the
     * source chain routes to a handler must have a legacy table row. */
    for (int byte = 0x00; byte <= 0x55; byte++) {
        if (dm2_v1_ccm_dispatch_source_group((uint8_t)byte) !=
            DM2_V1_CCM_SRC_NONE) {
            if (dm2_v1_ccm_get_opcode_def(byte) == NULL) return 0;
        }
    }
    return 1;
}

static int test_row_names_match_group_names(void) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        int byte = g_table_bytes[i];
        const DM2_V1_CCMOpcodeDef *def;
        const char *group_name;
        if (byte == 0xFF) continue;
        def = dm2_v1_ccm_get_opcode_def(byte);
        if (!def || !def->name) return 0;
        group_name = dm2_v1_ccm_dispatch_group_name(
            dm2_v1_ccm_dispatch_source_group((uint8_t)byte));
        if (!group_name || strcmp(def->name, group_name) != 0) return 0;
    }
    return 1;
}

static int test_halt_is_only_internal_row(void) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        int byte = g_table_bytes[i];
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(byte);
        if (!def) return 0;
        if (byte == 0xFF) {
            if (def->source_group != -1) return 0;
        } else {
            if (def->source_group < 0) return 0;
        }
    }
    return 1;
}

static int test_source_rows_are_not_stale_stubs(void) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        int byte = g_table_bytes[i];
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(byte);
        if (!def) return 0;
        if (byte != 0xFF && def->stubbed != 0) return 0;
    }
    return 1;
}

static int test_source_evidence_present(void) {
    const char *a = dm2_v1_ccm_source_evidence();
    const char *b = dm2_v1_ccm_dispatch_source_evidence();
    return a && strstr(a, "c_creature.cpp:2930-3212") != NULL
        && b && strstr(b, "c_creature.cpp:2930-3212") != NULL;
}

int main(void) {
    printf("DM2 V1 CCM legacy interpreter vs source b_1a matrix alignment\n");
    printf("Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM)\n\n");

    TEST(every_row_group_matches_dispatch);
    TEST(key_source_mappings);
    TEST(no_handler_bytes_absent);
    TEST(every_handled_source_byte_has_row);
    TEST(row_names_match_group_names);
    TEST(halt_is_only_internal_row);
    TEST(source_rows_are_not_stale_stubs);
    TEST(source_evidence_present);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
