/*
 * Original Saturn PRS3 version-1 branch-flow receipt.
 *
 * This is deliberately not a decoder. It MD5-locks the Japanese DM.BIN and
 * records one selected SH-2 branch outcome: after R11 refill/control setup,
 * `TST R11,R3` with R3=1 sends a zero low-bit to a separate target. Its
 * nonzero fallthrough bounds a byte read through R12 and a byte store through
 * R13/R0 before returning to the control loop. The receipt proves neither
 * payload identity, command semantics, output format, nor frame completion.
 */

#include "firestaff_x68k_media_receipt.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEXUS_PRS3_DM_MD5 "e88d60859f65f08fa622e1992b02280f"
#define NEXUS_PRS3_DM_SIZE 555144U
#define NEXUS_PRS3_VERSION1_CALLEE_OFFSET 85376U

typedef struct {
    int valid;
    size_t low_bit_immediate_offset;
    unsigned int low_bit_register;
    int low_bit_immediate;
    size_t low_bit_test_offset;
    unsigned int low_bit_test_source_register;
    unsigned int low_bit_test_destination_register;
    size_t zero_bit_branch_offset;
    size_t zero_bit_branch_target;
    size_t fallthrough_remaining_test_offset;
    size_t fallthrough_exhausted_branch_offset;
    size_t fallthrough_exhausted_target;
    size_t fallthrough_byte_load_offset;
    unsigned int byte_load_cursor_register;
    unsigned int byte_load_value_register;
    size_t output_index_copy_offset;
    unsigned int output_index_source_register;
    unsigned int output_index_destination_register;
    size_t output_byte_store_offset;
    unsigned int output_byte_source_register;
    unsigned int output_byte_base_register;
    unsigned int output_byte_index_register;
    size_t loop_back_branch_offset;
    size_t loop_back_target;
} Nexus_Prs3V1BranchFlowReceipt;

/* Read-only continuation of the zero-low-bit branch. After its R14 gate
 * accepts, the original code consumes two post-increment R12 bytes and
 * zero-extends each. This does not assign those bytes a token, offset,
 * palette, or output meaning. */
typedef struct {
    int valid;
    size_t counter_decrement_offset;
    unsigned int counter_register;
    int counter_decrement;
    size_t counter_gate_test_offset;
    size_t counter_rejection_branch_offset;
    size_t counter_rejection_target;
    size_t first_byte_load_offset;
    unsigned int first_byte_cursor_register;
    unsigned int first_byte_value_register;
    size_t first_byte_extend_offset;
    size_t second_byte_load_offset;
    unsigned int second_byte_cursor_register;
    unsigned int second_byte_value_register;
    size_t second_byte_extend_offset;
} Nexus_Prs3V1ZeroSideReadReceipt;

/* Direct R14 guard branches in the selected v1 route converge on one shared
 * failure epilogue. The receipt records only that architecture-level failure
 * return shape; it does not classify successful PRS3 data or decode it. */
typedef struct {
    int valid;
    size_t refill_guard_branch_offset;
    size_t refill_guard_target;
    size_t fallthrough_guard_branch_offset;
    size_t fallthrough_guard_target;
    size_t zero_side_guard_branch_offset;
    size_t zero_side_guard_target;
    size_t failure_result_offset;
    unsigned int failure_result_register;
    int failure_result_immediate;
    size_t return_offset;
} Nexus_Prs3V1TerminationReceipt;

/* This joins the R11 low-bit test with the two directly observed branch-local
 * R14/read shapes. It records control-dependent consumption only; neither
 * branch is named as a PRS3 command or an output format. */
typedef struct {
    int valid;
    size_t low_bit_test_offset;
    unsigned int low_bit_test_source_register;
    unsigned int low_bit_test_destination_register;
    size_t zero_bit_branch_offset;
    size_t zero_bit_branch_target;
    size_t nonzero_counter_decrement_offset;
    int nonzero_counter_decrement;
    size_t nonzero_byte_load_offset;
    unsigned int nonzero_byte_value_register;
    size_t zero_counter_decrement_offset;
    int zero_counter_decrement;
    size_t zero_first_byte_load_offset;
    unsigned int zero_first_byte_value_register;
    size_t zero_second_byte_load_offset;
    unsigned int zero_second_byte_value_register;
} Nexus_Prs3V1LowBitConsumptionReceipt;

static int failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static int sh2_conditional_branch_target(size_t instruction_offset,
                                         uint16_t instruction,
                                         size_t size,
                                         size_t *out_target) {
    int displacement;
    long target;
    uint16_t opcode = instruction & 0xff00U;

    if (!out_target || (opcode != 0x8900U && opcode != 0x8b00U &&
                        opcode != 0x8d00U && opcode != 0x8f00U)) return 0;
    displacement = (int)(int8_t)(instruction & 0x00ffU) * 2;
    target = (long)instruction_offset + 4L + (long)displacement;
    if (target < 0L || (size_t)target + 2U > size) return 0;
    *out_target = (size_t)target;
    return 1;
}

static int sh2_bra_target(size_t instruction_offset, uint16_t instruction,
                          size_t size, size_t *out_target) {
    int displacement;
    long target;

    if (!out_target || (instruction & 0xf000U) != 0xa000U) return 0;
    displacement = (int)(instruction & 0x0fffU);
    if ((displacement & 0x0800) != 0) displacement -= 0x1000;
    target = (long)instruction_offset + 4L + (long)displacement * 2L;
    if (target < 0L || (size_t)target + 2U > size) return 0;
    *out_target = (size_t)target;
    return 1;
}

static int sh2_mov_immediate_fields(uint16_t instruction,
                                    unsigned int *out_register,
                                    int *out_immediate) {
    if ((instruction & 0xf000U) != 0xe000U || !out_register ||
        !out_immediate) return 0;
    *out_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_immediate = (int)(int8_t)(instruction & 0x00ffU);
    return 1;
}

/* SH-2 ADD #imm,Rn is 0111nnnniiiiiiii. */
static int sh2_add_immediate_fields(uint16_t instruction,
                                    unsigned int *out_register,
                                    int *out_immediate) {
    if ((instruction & 0xf000U) != 0x7000U || !out_register ||
        !out_immediate) return 0;
    *out_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_immediate = (int)(int8_t)(instruction & 0x00ffU);
    return 1;
}

static int sh2_tst_register_fields(uint16_t instruction,
                                   unsigned int *out_source_register,
                                   unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x2008U || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

static int sh2_movb_postinc_fields(uint16_t instruction,
                                   unsigned int *out_cursor_register,
                                   unsigned int *out_value_register) {
    if ((instruction & 0xf00fU) != 0x6004U || !out_cursor_register ||
        !out_value_register) return 0;
    *out_value_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_cursor_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 EXTU.B Rm,Rn is 0110nnnnmmmm1100. */
static int sh2_extub_fields(uint16_t instruction,
                            unsigned int *out_source_register,
                            unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x600cU || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

static int sh2_mov_register_fields(uint16_t instruction,
                                   unsigned int *out_source_register,
                                   unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x6003U || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 MOV.B Rm,@(R0,Rn) is 0000nnnnmmmm0100. */
static int sh2_movb_r0_index_store_fields(uint16_t instruction,
                                          unsigned int *out_source_register,
                                          unsigned int *out_base_register) {
    if ((instruction & 0xf00fU) != 0x0004U || !out_source_register ||
        !out_base_register) return 0;
    *out_base_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

static int prs3_v1_branch_flow_receipt(const uint8_t *data, size_t size,
                                       Nexus_Prs3V1BranchFlowReceipt *out) {
    Nexus_Prs3V1BranchFlowReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 164U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.low_bit_immediate_offset = entry + 72U;
    receipt.low_bit_test_offset = entry + 74U;
    receipt.zero_bit_branch_offset = entry + 76U;
    receipt.fallthrough_remaining_test_offset = entry + 78U;
    receipt.fallthrough_exhausted_branch_offset = entry + 80U;
    receipt.fallthrough_byte_load_offset = entry + 84U;
    receipt.output_index_copy_offset = entry + 86U;
    receipt.output_byte_store_offset = entry + 88U;
    receipt.loop_back_branch_offset = entry + 96U;

    if (!sh2_mov_immediate_fields(
            read_be16(data + receipt.low_bit_immediate_offset),
            &receipt.low_bit_register, &receipt.low_bit_immediate) ||
        !sh2_tst_register_fields(read_be16(data + receipt.low_bit_test_offset),
                                 &receipt.low_bit_test_source_register,
                                 &receipt.low_bit_test_destination_register) ||
        !sh2_conditional_branch_target(
            receipt.zero_bit_branch_offset,
            read_be16(data + receipt.zero_bit_branch_offset), size,
            &receipt.zero_bit_branch_target) ||
        read_be16(data + receipt.fallthrough_remaining_test_offset) != 0x2ee8U ||
        !sh2_conditional_branch_target(
            receipt.fallthrough_exhausted_branch_offset,
            read_be16(data + receipt.fallthrough_exhausted_branch_offset), size,
            &receipt.fallthrough_exhausted_target) ||
        read_be16(data + receipt.fallthrough_byte_load_offset - 2U) != 0x7effU ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.fallthrough_byte_load_offset),
                                 &receipt.byte_load_cursor_register,
                                 &receipt.byte_load_value_register) ||
        !sh2_mov_register_fields(read_be16(data + receipt.output_index_copy_offset),
                                 &receipt.output_index_source_register,
                                 &receipt.output_index_destination_register) ||
        !sh2_movb_r0_index_store_fields(read_be16(data + receipt.output_byte_store_offset),
                                        &receipt.output_byte_source_register,
                                        &receipt.output_byte_base_register) ||
        !sh2_bra_target(receipt.loop_back_branch_offset,
                        read_be16(data + receipt.loop_back_branch_offset), size,
                        &receipt.loop_back_target)) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.output_byte_index_register = 0U;
    if (receipt.low_bit_register != 3U || receipt.low_bit_immediate != 1 ||
        receipt.low_bit_test_source_register != 11U ||
        receipt.low_bit_test_destination_register != 3U ||
        receipt.zero_bit_branch_target != entry + 100U ||
        receipt.fallthrough_exhausted_target != entry + 166U ||
        receipt.byte_load_cursor_register != 12U ||
        receipt.byte_load_value_register != 2U ||
        receipt.output_index_source_register != 6U ||
        receipt.output_index_destination_register != 0U ||
        receipt.output_byte_source_register != 2U ||
        receipt.output_byte_base_register != 13U ||
        receipt.loop_back_target != entry + 52U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_zero_side_read_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1ZeroSideReadReceipt *out) {
    Nexus_Prs3V1ZeroSideReadReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 168U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.counter_decrement_offset = entry + 100U;
    receipt.counter_gate_test_offset = entry + 102U;
    receipt.counter_rejection_branch_offset = entry + 104U;
    receipt.first_byte_load_offset = entry + 108U;
    receipt.first_byte_extend_offset = entry + 110U;
    receipt.second_byte_load_offset = entry + 112U;
    receipt.second_byte_extend_offset = entry + 114U;
    if (!sh2_add_immediate_fields(
            read_be16(data + receipt.counter_decrement_offset),
            &receipt.counter_register, &receipt.counter_decrement) ||
        read_be16(data + receipt.counter_gate_test_offset) != 0x4e11U ||
        !sh2_conditional_branch_target(
            receipt.counter_rejection_branch_offset,
            read_be16(data + receipt.counter_rejection_branch_offset), size,
            &receipt.counter_rejection_target) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.first_byte_load_offset),
                                 &receipt.first_byte_cursor_register,
                                 &receipt.first_byte_value_register) ||
        !sh2_extub_fields(read_be16(data + receipt.first_byte_extend_offset),
                          &receipt.first_byte_value_register,
                          &receipt.first_byte_value_register) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.second_byte_load_offset),
                                 &receipt.second_byte_cursor_register,
                                 &receipt.second_byte_value_register) ||
        !sh2_extub_fields(read_be16(data + receipt.second_byte_extend_offset),
                          &receipt.second_byte_value_register,
                          &receipt.second_byte_value_register)) {
        if (out) *out = receipt;
        return 0;
    }
    if (receipt.counter_register != 14U || receipt.counter_decrement != -2 ||
        receipt.counter_rejection_target != entry + 166U ||
        receipt.first_byte_cursor_register != 12U ||
        receipt.first_byte_value_register != 4U ||
        receipt.second_byte_cursor_register != 12U ||
        receipt.second_byte_value_register != 7U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_termination_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1TerminationReceipt *out) {
    Nexus_Prs3V1TerminationReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 190U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.refill_guard_branch_offset = entry + 62U;
    receipt.fallthrough_guard_branch_offset = entry + 80U;
    receipt.zero_side_guard_branch_offset = entry + 104U;
    receipt.failure_result_offset = entry + 174U;
    receipt.return_offset = entry + 188U;
    if (!sh2_conditional_branch_target(
            receipt.refill_guard_branch_offset,
            read_be16(data + receipt.refill_guard_branch_offset), size,
            &receipt.refill_guard_target) ||
        !sh2_conditional_branch_target(
            receipt.fallthrough_guard_branch_offset,
            read_be16(data + receipt.fallthrough_guard_branch_offset), size,
            &receipt.fallthrough_guard_target) ||
        !sh2_conditional_branch_target(
            receipt.zero_side_guard_branch_offset,
            read_be16(data + receipt.zero_side_guard_branch_offset), size,
            &receipt.zero_side_guard_target) ||
        !sh2_mov_immediate_fields(read_be16(data + receipt.failure_result_offset),
                                  &receipt.failure_result_register,
                                  &receipt.failure_result_immediate) ||
        read_be16(data + receipt.return_offset) != 0x000bU ||
        receipt.refill_guard_target != entry + 166U ||
        receipt.fallthrough_guard_target != entry + 166U ||
        receipt.zero_side_guard_target != entry + 166U ||
        receipt.failure_result_register != 0U || receipt.failure_result_immediate != 0) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_low_bit_consumption_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1LowBitConsumptionReceipt *out) {
    Nexus_Prs3V1LowBitConsumptionReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int counter_register = 0U;
    unsigned int cursor_register = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 116U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.low_bit_test_offset = entry + 74U;
    receipt.zero_bit_branch_offset = entry + 76U;
    receipt.nonzero_counter_decrement_offset = entry + 82U;
    receipt.nonzero_byte_load_offset = entry + 84U;
    receipt.zero_counter_decrement_offset = entry + 100U;
    receipt.zero_first_byte_load_offset = entry + 108U;
    receipt.zero_second_byte_load_offset = entry + 112U;
    if (read_be16(data + entry + 72U) != 0xe301U ||
        !sh2_tst_register_fields(read_be16(data + receipt.low_bit_test_offset),
                                 &receipt.low_bit_test_source_register,
                                 &receipt.low_bit_test_destination_register) ||
        read_be16(data + receipt.zero_bit_branch_offset) != 0x890aU ||
        !sh2_conditional_branch_target(
            receipt.zero_bit_branch_offset,
            read_be16(data + receipt.zero_bit_branch_offset), size,
            &receipt.zero_bit_branch_target) ||
        !sh2_add_immediate_fields(
            read_be16(data + receipt.nonzero_counter_decrement_offset),
            &counter_register, &receipt.nonzero_counter_decrement) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.nonzero_byte_load_offset),
                                 &cursor_register,
                                 &receipt.nonzero_byte_value_register) ||
        !sh2_add_immediate_fields(
            read_be16(data + receipt.zero_counter_decrement_offset),
            &counter_register, &receipt.zero_counter_decrement) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.zero_first_byte_load_offset),
                                 &cursor_register,
                                 &receipt.zero_first_byte_value_register) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.zero_second_byte_load_offset),
                                 &cursor_register,
                                 &receipt.zero_second_byte_value_register) ||
        receipt.low_bit_test_source_register != 11U ||
        receipt.low_bit_test_destination_register != 3U ||
        receipt.zero_bit_branch_target != entry + 100U ||
        counter_register != 14U || receipt.nonzero_counter_decrement != -1 ||
        receipt.nonzero_byte_value_register != 2U ||
        receipt.zero_counter_decrement != -2 ||
        receipt.zero_first_byte_value_register != 4U ||
        receipt.zero_second_byte_value_register != 7U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static void test_synthetic_branch_flow(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1BranchFlowReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 72U] = 0xe3U; fixture[entry + 73U] = 0x01U;
    fixture[entry + 74U] = 0x23U; fixture[entry + 75U] = 0xb8U;
    fixture[entry + 76U] = 0x89U; fixture[entry + 77U] = 0x0aU;
    fixture[entry + 78U] = 0x2eU; fixture[entry + 79U] = 0xe8U;
    fixture[entry + 80U] = 0x89U; fixture[entry + 81U] = 0x29U;
    fixture[entry + 82U] = 0x7eU; fixture[entry + 83U] = 0xffU;
    fixture[entry + 84U] = 0x62U; fixture[entry + 85U] = 0xc4U;
    fixture[entry + 86U] = 0x60U; fixture[entry + 87U] = 0x63U;
    fixture[entry + 88U] = 0x0dU; fixture[entry + 89U] = 0x24U;
    fixture[entry + 96U] = 0xafU; fixture[entry + 97U] = 0xe8U;
    check(prs3_v1_branch_flow_receipt(fixture, sizeof(fixture), &receipt) &&
              receipt.valid && receipt.zero_bit_branch_target == entry + 100U &&
              receipt.output_byte_source_register == 2U &&
              receipt.output_byte_base_register == 13U &&
              receipt.loop_back_target == entry + 52U,
          "SH-2 PRS3 branch-flow receipt locks the low-bit fallthrough path");
    fixture[entry + 89U] = 0x20U;
    check(!prs3_v1_branch_flow_receipt(fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 branch-flow receipt rejects a non-indexed byte store");
}

static void test_synthetic_zero_side_read(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1ZeroSideReadReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 100U] = 0x7eU; fixture[entry + 101U] = 0xfeU;
    fixture[entry + 102U] = 0x4eU; fixture[entry + 103U] = 0x11U;
    fixture[entry + 104U] = 0x8bU; fixture[entry + 105U] = 0x1dU;
    fixture[entry + 108U] = 0x64U; fixture[entry + 109U] = 0xc4U;
    fixture[entry + 110U] = 0x64U; fixture[entry + 111U] = 0x4cU;
    fixture[entry + 112U] = 0x67U; fixture[entry + 113U] = 0xc4U;
    fixture[entry + 114U] = 0x67U; fixture[entry + 115U] = 0x7cU;
    check(prs3_v1_zero_side_read_receipt(fixture, sizeof(fixture), &receipt) &&
              receipt.valid && receipt.counter_decrement == -2 &&
              receipt.first_byte_cursor_register == 12U &&
              receipt.first_byte_value_register == 4U &&
              receipt.second_byte_cursor_register == 12U &&
              receipt.second_byte_value_register == 7U,
          "SH-2 PRS3 zero side receipt locks two post-increment byte reads");
    fixture[entry + 115U] = 0x70U;
    check(!prs3_v1_zero_side_read_receipt(fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 zero side receipt rejects a non-byte-extension path");
}

static void test_synthetic_termination(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1TerminationReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 62U] = 0x89U; fixture[entry + 63U] = 0x32U;
    fixture[entry + 80U] = 0x89U; fixture[entry + 81U] = 0x29U;
    fixture[entry + 104U] = 0x8bU; fixture[entry + 105U] = 0x1dU;
    fixture[entry + 174U] = 0xe0U; fixture[entry + 175U] = 0x00U;
    fixture[entry + 188U] = 0x00U; fixture[entry + 189U] = 0x0bU;
    check(prs3_v1_termination_receipt(fixture, sizeof(fixture), &receipt) &&
              receipt.valid && receipt.refill_guard_target == entry + 166U &&
              receipt.fallthrough_guard_target == entry + 166U &&
              receipt.zero_side_guard_target == entry + 166U &&
              receipt.failure_result_register == 0U &&
              receipt.failure_result_immediate == 0,
          "SH-2 PRS3 termination receipt locks converged failure return");
    fixture[entry + 175U] = 0x01U;
    check(!prs3_v1_termination_receipt(fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 termination receipt rejects a nonzero failure result");
}

static void test_synthetic_low_bit_consumption(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1LowBitConsumptionReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 72U] = 0xe3U; fixture[entry + 73U] = 0x01U;
    fixture[entry + 74U] = 0x23U; fixture[entry + 75U] = 0xb8U;
    fixture[entry + 76U] = 0x89U; fixture[entry + 77U] = 0x0aU;
    fixture[entry + 82U] = 0x7eU; fixture[entry + 83U] = 0xffU;
    fixture[entry + 84U] = 0x62U; fixture[entry + 85U] = 0xc4U;
    fixture[entry + 100U] = 0x7eU; fixture[entry + 101U] = 0xfeU;
    fixture[entry + 108U] = 0x64U; fixture[entry + 109U] = 0xc4U;
    fixture[entry + 112U] = 0x67U; fixture[entry + 113U] = 0xc4U;
    check(prs3_v1_low_bit_consumption_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.nonzero_counter_decrement == -1 &&
              receipt.nonzero_byte_value_register == 2U &&
              receipt.zero_counter_decrement == -2 &&
              receipt.zero_first_byte_value_register == 4U &&
              receipt.zero_second_byte_value_register == 7U,
          "SH-2 PRS3 low-bit receipt locks the two branch-local read shapes");
    fixture[entry + 101U] = 0xffU;
    check(!prs3_v1_low_bit_consumption_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 low-bit receipt rejects a changed zero-side decrement");
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *data;

    if (!out_data || !out_size) return 0;
    *out_data = NULL;
    *out_size = 0U;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0L, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0L ||
        fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1U, (size_t)file_size, fp) != (size_t)file_size) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)file_size;
    return 1;
}

int main(int argc, char **argv) {
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    char default_dir[1024];
    char dm_path[1200];
    char md5[33];
    const char *home;
    uint8_t *data = NULL;
    size_t size = 0U;
    Nexus_Prs3V1BranchFlowReceipt receipt;
    Nexus_Prs3V1ZeroSideReadReceipt zero_side_receipt;
    Nexus_Prs3V1TerminationReceipt termination_receipt;
    Nexus_Prs3V1LowBitConsumptionReceipt low_bit_receipt;

    test_synthetic_branch_flow();
    test_synthetic_zero_side_read();
    test_synthetic_termination();
    test_synthetic_low_bit_consumption();
    if (!data_dir) {
        home = getenv("HOME");
        if (!home || snprintf(default_dir, sizeof(default_dir),
                              "%s/.firestaff/data/nexus", home) <= 0) {
            puts("SKIP: no Nexus data directory argument or HOME");
            return failures == 0 ? 0 : 1;
        }
        data_dir = default_dir;
    }
    if (snprintf(dm_path, sizeof(dm_path), "%s/DM.BIN", data_dir) <= 0 ||
        !read_file(dm_path, &data, &size)) {
        puts("SKIP: hash-verified DM.BIN is not available");
        return failures == 0 ? 0 : 1;
    }
    check(size == NEXUS_PRS3_DM_SIZE,
          "DM.BIN has the locked Japanese Track 1 size");
    check(firestaff_x68k_media_receipt_md5_hex(data, size, md5, sizeof(md5)) == 0 &&
              strcmp(md5, NEXUS_PRS3_DM_MD5) == 0,
          "DM.BIN matches its locked MD5");
    if (failures == 0) {
        check(prs3_v1_branch_flow_receipt(data, size, &receipt) && receipt.valid,
              "DM.BIN locks the PRS3 v1 low-bit branch and byte-store fallthrough");
        check(prs3_v1_zero_side_read_receipt(data, size, &zero_side_receipt) &&
                  zero_side_receipt.valid,
              "DM.BIN locks the PRS3 v1 zero-side two-byte continuation");
        check(prs3_v1_termination_receipt(data, size, &termination_receipt) &&
                  termination_receipt.valid,
              "DM.BIN locks the PRS3 v1 converged failure return");
        check(prs3_v1_low_bit_consumption_receipt(
                  data, size, &low_bit_receipt) &&
                  low_bit_receipt.valid,
              "DM.BIN locks the PRS3 v1 low-bit branch-local read shapes");
        if (receipt.valid) {
            printf("SH-2 PRS3 v1 branch flow: test=R%u&R%u branch=%zu->%zu "
                   "fallthrough-load=%zu @R%u+->R%u store=%zu R%u->@(R%u+R%u) "
                   "loop=%zu->%zu; codec/termination-proof=0\n",
                   receipt.low_bit_test_source_register,
                   receipt.low_bit_test_destination_register,
                   receipt.zero_bit_branch_offset, receipt.zero_bit_branch_target,
                   receipt.fallthrough_byte_load_offset,
                   receipt.byte_load_cursor_register, receipt.byte_load_value_register,
                   receipt.output_byte_store_offset, receipt.output_byte_source_register,
                   receipt.output_byte_base_register, receipt.output_byte_index_register,
                   receipt.loop_back_branch_offset, receipt.loop_back_target);
        }
        if (zero_side_receipt.valid) {
            printf("SH-2 PRS3 v1 zero-side read: counter=R%u%+d reject=%zu->%zu "
                   "reads=%zu @R%u+->R%u,%zu @R%u+->R%u; "
                   "token/termination-proof=0\n",
                   zero_side_receipt.counter_register,
                   zero_side_receipt.counter_decrement,
                   zero_side_receipt.counter_rejection_branch_offset,
                   zero_side_receipt.counter_rejection_target,
                   zero_side_receipt.first_byte_load_offset,
                   zero_side_receipt.first_byte_cursor_register,
                   zero_side_receipt.first_byte_value_register,
                   zero_side_receipt.second_byte_load_offset,
                   zero_side_receipt.second_byte_cursor_register,
                   zero_side_receipt.second_byte_value_register);
        }
        if (termination_receipt.valid) {
            printf("SH-2 PRS3 v1 termination: guards=%zu,%zu,%zu->%zu "
                   "result=%zu R%u=%d return=%zu; codec-success-proof=0\n",
                   termination_receipt.refill_guard_branch_offset,
                   termination_receipt.fallthrough_guard_branch_offset,
                   termination_receipt.zero_side_guard_branch_offset,
                   termination_receipt.failure_result_offset,
                   termination_receipt.failure_result_offset,
                   termination_receipt.failure_result_register,
                   termination_receipt.failure_result_immediate,
                   termination_receipt.return_offset);
        }
        if (low_bit_receipt.valid) {
            printf("SH-2 PRS3 v1 low-bit reads: test=R%u&R%u zero=%zu->%zu "
                   "nonzero=R14%+d @R12+->R%u zero=R14%+d @R12+->R%u,R%u; "
                   "codec/termination-proof=0\n",
                   low_bit_receipt.low_bit_test_source_register,
                   low_bit_receipt.low_bit_test_destination_register,
                   low_bit_receipt.zero_bit_branch_offset,
                   low_bit_receipt.zero_bit_branch_target,
                   low_bit_receipt.nonzero_counter_decrement,
                   low_bit_receipt.nonzero_byte_value_register,
                   low_bit_receipt.zero_counter_decrement,
                   low_bit_receipt.zero_first_byte_value_register,
                   low_bit_receipt.zero_second_byte_value_register);
        }
    }
    free(data);
    return failures == 0 ? 0 : 1;
}
