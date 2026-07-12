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
#define NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET 85252U
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

/* The accepted zero-side path combines the two already proven byte registers
 * through a bounded SH-2 shift/mask/OR sequence. This records register
 * algebra only, never a PRS3 token, offset, palette, or output interpretation. */
typedef struct {
    int valid;
    size_t upper_mask_literal_load_offset;
    size_t upper_mask_literal_offset;
    uint16_t upper_mask_word;
    size_t low_mask_load_offset;
    int low_mask_immediate;
    size_t second_byte_copy_offset;
    size_t first_shift_offset;
    size_t second_shift_offset;
    size_t upper_mask_and_offset;
    size_t merge_or_offset;
    size_t low_mask_and_offset;
} Nexus_Prs3V1ZeroSideMergeReceipt;

/* The merged R4 value then reaches a bounded comparison/control branch. The
 * receipt names only the observed registers and branch target, not a token
 * class, length, or decoded output. */
typedef struct {
    int valid;
    size_t low_fragment_increment_offset;
    int low_fragment_increment;
    size_t merged_value_add_offset;
    unsigned int merged_value_source_register;
    unsigned int merged_value_destination_register;
    size_t merged_value_compare_offset;
    unsigned int compare_source_register;
    unsigned int compare_destination_register;
    size_t control_branch_offset;
    size_t control_branch_target;
} Nexus_Prs3V1ZeroSideMergeBranchReceipt;

/* The merged R4 value next becomes a bounded R13-indexed byte-read offset.
 * This is only an instruction/dataflow receipt; R13's allocation and the
 * byte's PRS3 role remain outside this probe. */
typedef struct {
    int valid;
    size_t index_mask_literal_load_offset;
    size_t index_mask_literal_offset;
    uint16_t index_mask_word;
    size_t merged_value_copy_offset;
    unsigned int merged_value_copy_source_register;
    unsigned int merged_value_copy_destination_register;
    size_t index_mask_and_offset;
    size_t indexed_byte_load_offset;
    unsigned int indexed_byte_base_register;
    unsigned int indexed_byte_destination_register;
} Nexus_Prs3V1MergedValueReadReceipt;

/* After the R13-indexed byte read, the next R4/R7 comparison is followed by
 * two R1-based comparisons before the delayed branch. This receipt preserves
 * that overwrite boundary so the R4/R7 comparison is not incorrectly claimed
 * to own the branch condition. */
typedef struct {
    int valid;
    size_t merged_compare_offset;
    unsigned int merged_compare_source_register;
    unsigned int merged_compare_destination_register;
    size_t first_r1_compare_offset;
    unsigned int first_r1_compare_source_register;
    unsigned int first_r1_compare_destination_register;
    size_t branch_condition_compare_offset;
    unsigned int branch_condition_source_register;
    unsigned int branch_condition_destination_register;
    size_t delayed_branch_offset;
    size_t delayed_branch_target;
    size_t outer_loop_branch_offset;
    size_t outer_loop_target;
} Nexus_Prs3V1PostReadControlReceipt;

/* Exact continuation of the post-read control decision. The final R1/R10
 * comparison, increment, BF/S delay slot, local-repeat target, and outer
 * loop fallthrough are retained as control-flow facts only. */
typedef struct {
    int valid;
    size_t compare_offset;
    unsigned int compare_source_register;
    unsigned int compare_destination_register;
    size_t counter_increment_offset;
    unsigned int counter_register;
    int counter_increment;
    size_t delayed_branch_offset;
    size_t delayed_branch_target;
    size_t delay_slot_offset;
    unsigned int delay_slot_source_register;
    unsigned int delay_slot_destination_register;
    size_t outer_loop_branch_offset;
    size_t outer_loop_target;
} Nexus_Prs3V1PostReadBranchReceipt;

/* The local-repeat and outer-loop paths share an R6 increment/mask sequence.
 * This is a bounded register-state receipt only; it does not classify R6 as
 * a decoded output length or buffer cursor. */
typedef struct {
    int valid;
    size_t mask_literal_load_offset;
    size_t mask_literal_offset;
    uint16_t mask_word;
    size_t local_entry_offset;
    size_t local_r6_copy_offset;
    size_t r6_increment_offset;
    int r6_increment;
    size_t delayed_branch_offset;
    size_t delay_slot_mask_offset;
    unsigned int delay_slot_mask_source_register;
    unsigned int delay_slot_mask_destination_register;
    size_t local_repeat_target;
    size_t outer_loop_target;
} Nexus_Prs3V1RepeatR6MaskReceipt;

/* The outer fallthrough from the R6 repeat block re-enters the shared R11
 * control block. This proves only its exact SH-2 control/refill shape; it
 * does not identify a PRS3 bit order, payload, token, or output operation. */
typedef struct {
    int valid;
    size_t outer_loop_branch_offset;
    size_t reentry_target;
    size_t sentinel_literal_load_offset;
    size_t sentinel_literal_offset;
    uint16_t sentinel_word;
    size_t control_shift_offset;
    unsigned int control_shift_register;
    size_t control_test_offset;
    unsigned int control_test_source_register;
    unsigned int control_test_destination_register;
    size_t refill_skip_branch_offset;
    size_t refill_skip_target;
    size_t refill_guard_offset;
    size_t refill_failure_branch_offset;
    size_t refill_failure_target;
    size_t refill_byte_load_offset;
    unsigned int refill_cursor_register;
    unsigned int refill_value_register;
    size_t refill_merge_offset;
    unsigned int refill_merge_source_register;
    unsigned int refill_merge_destination_register;
} Nexus_Prs3V1OuterLoopReentryReceipt;

/* Both observed outer-loop re-entry continuations converge at the same R11
 * low-bit test. This is a control-flow join only; it says nothing about the
 * meaning or ordering of the control bits. */
typedef struct {
    int valid;
    size_t skip_branch_offset;
    size_t skip_target;
    size_t refill_merge_offset;
    unsigned int refill_merge_source_register;
    unsigned int refill_merge_destination_register;
    size_t low_bit_immediate_offset;
    unsigned int low_bit_register;
    int low_bit_immediate;
    size_t low_bit_test_offset;
    unsigned int low_bit_test_source_register;
    unsigned int low_bit_test_destination_register;
    size_t zero_bit_branch_offset;
    size_t zero_bit_target;
} Nexus_Prs3V1OuterLoopLowBitJoinReceipt;

/* The directly observed failure target begins with a literal-fed indirect
 * call. The pointer is retained as raw original data: no memory-map or callee
 * meaning is inferred here. */
typedef struct {
    int valid;
    size_t failure_target;
    size_t literal_load_offset;
    size_t literal_offset;
    uint32_t literal_word;
    unsigned int literal_register;
    size_t indirect_call_offset;
    unsigned int indirect_call_register;
    size_t call_delay_offset;
    unsigned int call_delay_source_register;
    unsigned int call_delay_destination_register;
    size_t failure_result_offset;
    size_t return_offset;
} Nexus_Prs3V1FailureCallReceipt;

/* The selected callee's entry establishes registers used by the bounded loop.
 * Literal pointers and indirect calls remain raw facts; this receipt does not
 * name either callee or assign an ABI/payload role to any register. */
typedef struct {
    int valid;
    size_t r4_to_r12_offset;
    size_t r5_to_r10_offset;
    size_t first_call_literal_load_offset;
    size_t first_call_literal_offset;
    uint32_t first_call_literal_word;
    unsigned int first_call_literal_register;
    size_t first_indirect_call_offset;
    unsigned int first_indirect_call_register;
    size_t first_call_delay_offset;
    unsigned int first_call_delay_source_register;
    unsigned int first_call_delay_destination_register;
    size_t post_call_copy_offset;
    unsigned int post_call_copy_source_register;
    unsigned int post_call_copy_destination_register;
    size_t post_call_test_offset;
    size_t post_call_branch_offset;
    size_t post_call_branch_target;
    size_t r11_zero_offset;
} Nexus_Prs3V1EntryRegisterReceipt;

/* The entry's R13 test either bypasses or falls through a second literal-fed
 * indirect call. Both routes reach the same R11 initialization. Literal words
 * are raw original data only; no callee, ABI, or decoder role is inferred. */
typedef struct {
    int valid;
    size_t bypass_branch_offset;
    size_t bypass_target;
    size_t second_r4_literal_load_offset;
    size_t second_r4_literal_offset;
    uint32_t second_r4_literal_word;
    size_t second_r3_literal_load_offset;
    size_t second_r3_literal_offset;
    uint32_t second_r3_literal_word;
    size_t second_indirect_call_offset;
    unsigned int second_indirect_call_register;
    size_t second_call_delay_offset;
    unsigned int second_call_delay_register;
    int second_call_delay_immediate;
    size_t r11_zero_offset;
} Nexus_Prs3V1EntryBypassReceipt;

/* Crosses only the selected original version-1 call boundary. The caller's
 * R6+12 word reaches callee R14 through delayed register moves; it is not
 * named as a descriptor, payload pointer, counter, or decode input. */
typedef struct {
    int valid;
    size_t caller_word_load_offset;
    unsigned int caller_word_base_register;
    unsigned int caller_word_value_register;
    unsigned int caller_word_byte_displacement;
    size_t version1_branch_offset;
    size_t version1_branch_target;
    size_t version1_branch_delay_offset;
    size_t version1_call_offset;
    size_t version1_call_target;
    size_t version1_call_delay_offset;
    size_t callee_r6_to_r14_offset;
} Nexus_Prs3V1CallerToCalleeReceipt;

static int failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
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

/* SH-2 MOV.W @(disp,PC),Rn is 1001nnnndddddddd. */
static int sh2_movw_pc_literal_target(size_t instruction_offset,
                                      uint16_t instruction,
                                      size_t *out_target) {
    size_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0x9000U ||
        instruction_offset > SIZE_MAX - 4U) return 0;
    displacement = (size_t)(instruction & 0x00ffU) * 2U;
    if (displacement > SIZE_MAX - (instruction_offset + 4U)) return 0;
    *out_target = instruction_offset + 4U + displacement;
    return 1;
}

/* SH-2 MOV.L @(disp,PC),Rn is 1101nnnndddddddd. */
static int sh2_movl_pc_literal_target(size_t instruction_offset,
                                      uint16_t instruction,
                                      size_t *out_target) {
    size_t base;
    size_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0xd000U) return 0;
    base = (instruction_offset & ~(size_t)3U) + 4U;
    displacement = (size_t)(instruction & 0x00ffU) * 4U;
    if (displacement > SIZE_MAX - base) return 0;
    *out_target = base + displacement;
    return 1;
}

/* SH-2 JSR @Rn is 0100nnnn00001011. */
static int sh2_jsr_register(uint16_t instruction, unsigned int *out_register) {
    if ((instruction & 0xf0ffU) != 0x400bU || !out_register) return 0;
    *out_register = (unsigned int)((instruction >> 8) & 0x0fU);
    return 1;
}

/* SH-2 MOV.L @(disp,Rm),Rn is 0101nnnnmmmmdddd. */
static int sh2_movl_disp_register_fields(uint16_t instruction,
                                         unsigned int *out_base_register,
                                         unsigned int *out_value_register,
                                         unsigned int *out_byte_displacement) {
    if ((instruction & 0xf000U) != 0x5000U || !out_base_register ||
        !out_value_register || !out_byte_displacement) return 0;
    *out_value_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_base_register = (unsigned int)((instruction >> 4) & 0x0fU);
    *out_byte_displacement = (unsigned int)(instruction & 0x000fU) * 4U;
    return 1;
}

/* SH-2 BSR disp12 is 1011dddddddddddd. */
static int sh2_bsr_target(size_t instruction_offset, uint16_t instruction,
                          size_t size, size_t *out_target) {
    int displacement;
    long target;

    if (!out_target || (instruction & 0xf000U) != 0xb000U) return 0;
    displacement = (int)(instruction & 0x0fffU);
    if ((displacement & 0x0800) != 0) displacement -= 0x1000;
    target = (long)instruction_offset + 4L + (long)displacement * 2L;
    if (target < 0L || (size_t)target + 2U > size) return 0;
    *out_target = (size_t)target;
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

/* SH-2 AND/OR Rm,Rn are 0010nnnnmmmm1001/1011. */
static int sh2_logic_register_fields(uint16_t instruction, uint16_t opcode,
                                     unsigned int *out_source_register,
                                     unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != opcode || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 CMP/GT Rm,Rn is 0011nnnnmmmm0111. */
static int sh2_cmp_gt_register_fields(uint16_t instruction,
                                      unsigned int *out_source_register,
                                      unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x3007U || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 CMP/EQ Rm,Rn is 0010nnnnmmmm0000. */
static int sh2_cmp_eq_register_fields(uint16_t instruction,
                                      unsigned int *out_source_register,
                                      unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x2000U || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 ADD Rm,Rn is 0011nnnnmmmm1100. */
static int sh2_add_register_fields(uint16_t instruction,
                                   unsigned int *out_source_register,
                                   unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x300cU || !out_source_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 MOV.B @(R0,Rm),Rn is 0000nnnnmmmm1100. */
static int sh2_movb_r0_index_load_fields(uint16_t instruction,
                                         unsigned int *out_base_register,
                                         unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x000cU || !out_base_register ||
        !out_destination_register) return 0;
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_base_register = (unsigned int)((instruction >> 4) & 0x0fU);
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

static int prs3_v1_zero_side_merge_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1ZeroSideMergeReceipt *out) {
    Nexus_Prs3V1ZeroSideMergeReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int source = 0U, destination = 0U;
    unsigned int low_mask_register = 0U;
    unsigned int copy_source = 0U, copy_destination = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 130U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.upper_mask_literal_load_offset = entry + 44U;
    receipt.low_mask_load_offset = entry + 106U;
    receipt.second_byte_copy_offset = entry + 116U;
    receipt.first_shift_offset = entry + 118U;
    receipt.second_shift_offset = entry + 120U;
    receipt.upper_mask_and_offset = entry + 122U;
    receipt.merge_or_offset = entry + 124U;
    receipt.low_mask_and_offset = entry + 126U;
    if (read_be16(data + receipt.upper_mask_literal_load_offset) != 0x986cU ||
        !sh2_movw_pc_literal_target(
            receipt.upper_mask_literal_load_offset,
            read_be16(data + receipt.upper_mask_literal_load_offset),
            &receipt.upper_mask_literal_offset) ||
        receipt.upper_mask_literal_offset + 2U > size ||
        !sh2_mov_immediate_fields(read_be16(data + receipt.low_mask_load_offset),
                                  &low_mask_register, &receipt.low_mask_immediate) ||
        !sh2_mov_register_fields(read_be16(data + receipt.second_byte_copy_offset),
                                 &copy_source, &copy_destination) ||
        read_be16(data + receipt.first_shift_offset) != 0x4308U ||
        read_be16(data + receipt.second_shift_offset) != 0x4308U ||
        !sh2_logic_register_fields(read_be16(data + receipt.upper_mask_and_offset),
                                   0x2009U, &source, &destination) ||
        source != 8U || destination != 3U ||
        !sh2_logic_register_fields(read_be16(data + receipt.merge_or_offset),
                                   0x200bU, &source, &destination) ||
        source != 3U || destination != 4U ||
        !sh2_logic_register_fields(read_be16(data + receipt.low_mask_and_offset),
                                   0x2009U, &source, &destination) ||
        source != 2U || destination != 7U ||
        low_mask_register != 2U || copy_source != 7U || copy_destination != 3U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.upper_mask_word = read_be16(data + receipt.upper_mask_literal_offset);
    if (receipt.upper_mask_word != 0x0f00U ||
        receipt.low_mask_immediate != 15) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_zero_side_merge_branch_receipt(
    const uint8_t *data, size_t size,
    Nexus_Prs3V1ZeroSideMergeBranchReceipt *out) {
    Nexus_Prs3V1ZeroSideMergeBranchReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int increment_register = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 136U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.low_fragment_increment_offset = entry + 128U;
    receipt.merged_value_add_offset = entry + 130U;
    receipt.merged_value_compare_offset = entry + 132U;
    receipt.control_branch_offset = entry + 134U;
    if (!sh2_add_immediate_fields(
            read_be16(data + receipt.low_fragment_increment_offset),
            &increment_register, &receipt.low_fragment_increment) ||
        !sh2_add_register_fields(read_be16(data + receipt.merged_value_add_offset),
                                 &receipt.merged_value_source_register,
                                 &receipt.merged_value_destination_register) ||
        !sh2_cmp_gt_register_fields(read_be16(data + receipt.merged_value_compare_offset),
                                    &receipt.compare_source_register,
                                    &receipt.compare_destination_register) ||
        read_be16(data + receipt.control_branch_offset) != 0x89d5U ||
        !sh2_conditional_branch_target(
            receipt.control_branch_offset,
            read_be16(data + receipt.control_branch_offset), size,
            &receipt.control_branch_target) ||
        increment_register != 7U || receipt.low_fragment_increment != 2 ||
        receipt.merged_value_source_register != 4U ||
        receipt.merged_value_destination_register != 7U ||
        receipt.compare_source_register != 7U ||
        receipt.compare_destination_register != 4U ||
        receipt.control_branch_target != entry + 52U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_merged_value_read_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1MergedValueReadReceipt *out) {
    Nexus_Prs3V1MergedValueReadReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int source = 0U, destination = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 150U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.index_mask_literal_load_offset = entry + 50U;
    receipt.merged_value_copy_offset = entry + 142U;
    receipt.index_mask_and_offset = entry + 144U;
    receipt.indexed_byte_load_offset = entry + 148U;
    if (read_be16(data + receipt.index_mask_literal_load_offset) != 0x956aU ||
        !sh2_movw_pc_literal_target(
            receipt.index_mask_literal_load_offset,
            read_be16(data + receipt.index_mask_literal_load_offset),
            &receipt.index_mask_literal_offset) ||
        receipt.index_mask_literal_offset + 2U > size ||
        !sh2_mov_register_fields(read_be16(data + receipt.merged_value_copy_offset),
                                 &receipt.merged_value_copy_source_register,
                                 &receipt.merged_value_copy_destination_register) ||
        !sh2_logic_register_fields(read_be16(data + receipt.index_mask_and_offset),
                                   0x2009U, &source, &destination) ||
        source != 5U || destination != 0U ||
        !sh2_movb_r0_index_load_fields(read_be16(data + receipt.indexed_byte_load_offset),
                                       &receipt.indexed_byte_base_register,
                                       &receipt.indexed_byte_destination_register) ||
        receipt.merged_value_copy_source_register != 4U ||
        receipt.merged_value_copy_destination_register != 0U ||
        receipt.indexed_byte_base_register != 13U ||
        receipt.indexed_byte_destination_register != 1U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.index_mask_word = read_be16(data + receipt.index_mask_literal_offset);
    if (receipt.index_mask_word != 0x0fffU) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_post_read_control_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1PostReadControlReceipt *out) {
    Nexus_Prs3V1PostReadControlReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 164U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.merged_compare_offset = entry + 150U;
    receipt.first_r1_compare_offset = entry + 152U;
    receipt.branch_condition_compare_offset = entry + 154U;
    receipt.delayed_branch_offset = entry + 158U;
    receipt.outer_loop_branch_offset = entry + 162U;
    if (!sh2_cmp_gt_register_fields(read_be16(data + receipt.merged_compare_offset),
                                    &receipt.merged_compare_source_register,
                                    &receipt.merged_compare_destination_register) ||
        !sh2_cmp_eq_register_fields(read_be16(data + receipt.first_r1_compare_offset),
                                    &receipt.first_r1_compare_source_register,
                                    &receipt.first_r1_compare_destination_register) ||
        !sh2_cmp_eq_register_fields(
            read_be16(data + receipt.branch_condition_compare_offset),
            &receipt.branch_condition_source_register,
            &receipt.branch_condition_destination_register) ||
        read_be16(data + receipt.branch_condition_compare_offset + 2U) != 0x7a01U ||
        read_be16(data + receipt.delayed_branch_offset) != 0x8ff3U ||
        !sh2_conditional_branch_target(
            receipt.delayed_branch_offset,
            read_be16(data + receipt.delayed_branch_offset), size,
            &receipt.delayed_branch_target) ||
        !sh2_bra_target(receipt.outer_loop_branch_offset,
                        read_be16(data + receipt.outer_loop_branch_offset), size,
                        &receipt.outer_loop_target) ||
        receipt.merged_compare_source_register != 7U ||
        receipt.merged_compare_destination_register != 4U ||
        receipt.first_r1_compare_source_register != 1U ||
        receipt.first_r1_compare_destination_register != 3U ||
        receipt.branch_condition_source_register != 1U ||
        receipt.branch_condition_destination_register != 10U ||
        receipt.delayed_branch_target != entry + 136U ||
        receipt.outer_loop_target != entry + 52U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_post_read_branch_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1PostReadBranchReceipt *out) {
    Nexus_Prs3V1PostReadBranchReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 164U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.compare_offset = entry + 154U;
    receipt.counter_increment_offset = entry + 156U;
    receipt.delayed_branch_offset = entry + 158U;
    receipt.delay_slot_offset = entry + 160U;
    receipt.outer_loop_branch_offset = entry + 162U;
    if (!sh2_cmp_eq_register_fields(read_be16(data + receipt.compare_offset),
                                    &receipt.compare_source_register,
                                    &receipt.compare_destination_register) ||
        !sh2_add_immediate_fields(
            read_be16(data + receipt.counter_increment_offset),
            &receipt.counter_register, &receipt.counter_increment) ||
        read_be16(data + receipt.delayed_branch_offset) != 0x8ff3U ||
        !sh2_conditional_branch_target(
            receipt.delayed_branch_offset,
            read_be16(data + receipt.delayed_branch_offset), size,
            &receipt.delayed_branch_target) ||
        !sh2_logic_register_fields(read_be16(data + receipt.delay_slot_offset),
                                   0x2009U,
                                   &receipt.delay_slot_source_register,
                                   &receipt.delay_slot_destination_register) ||
        !sh2_bra_target(receipt.outer_loop_branch_offset,
                        read_be16(data + receipt.outer_loop_branch_offset), size,
                        &receipt.outer_loop_target) ||
        receipt.compare_source_register != 1U ||
        receipt.compare_destination_register != 10U ||
        receipt.counter_register != 10U || receipt.counter_increment != 1 ||
        receipt.delay_slot_source_register != 5U ||
        receipt.delay_slot_destination_register != 6U ||
        receipt.delayed_branch_target != entry + 136U ||
        receipt.outer_loop_target != entry + 52U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_repeat_r6_mask_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1RepeatR6MaskReceipt *out) {
    Nexus_Prs3V1RepeatR6MaskReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int copy_source = 0U, copy_destination = 0U;
    unsigned int increment_register = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 164U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.mask_literal_load_offset = entry + 50U;
    receipt.local_entry_offset = entry + 136U;
    receipt.local_r6_copy_offset = entry + 136U;
    receipt.r6_increment_offset = entry + 138U;
    receipt.delayed_branch_offset = entry + 158U;
    receipt.delay_slot_mask_offset = entry + 160U;
    if (read_be16(data + receipt.mask_literal_load_offset) != 0x956aU ||
        !sh2_movw_pc_literal_target(
            receipt.mask_literal_load_offset,
            read_be16(data + receipt.mask_literal_load_offset),
            &receipt.mask_literal_offset) ||
        receipt.mask_literal_offset + 2U > size ||
        !sh2_mov_register_fields(read_be16(data + receipt.local_r6_copy_offset),
                                 &copy_source, &copy_destination) ||
        !sh2_add_immediate_fields(read_be16(data + receipt.r6_increment_offset),
                                  &increment_register, &receipt.r6_increment) ||
        read_be16(data + receipt.delayed_branch_offset) != 0x8ff3U ||
        !sh2_conditional_branch_target(
            receipt.delayed_branch_offset,
            read_be16(data + receipt.delayed_branch_offset), size,
            &receipt.local_repeat_target) ||
        !sh2_logic_register_fields(read_be16(data + receipt.delay_slot_mask_offset),
                                   0x2009U,
                                   &receipt.delay_slot_mask_source_register,
                                   &receipt.delay_slot_mask_destination_register) ||
        !sh2_bra_target(entry + 162U, read_be16(data + entry + 162U), size,
                        &receipt.outer_loop_target) ||
        copy_source != 6U || copy_destination != 3U ||
        increment_register != 6U || receipt.r6_increment != 1 ||
        receipt.delay_slot_mask_source_register != 5U ||
        receipt.delay_slot_mask_destination_register != 6U ||
        receipt.local_repeat_target != receipt.local_entry_offset ||
        receipt.outer_loop_target != entry + 52U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.mask_word = read_be16(data + receipt.mask_literal_offset);
    if (receipt.mask_word != 0x0fffU) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_outer_loop_reentry_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1OuterLoopReentryReceipt *out) {
    Nexus_Prs3V1OuterLoopReentryReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 270U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.outer_loop_branch_offset = entry + 162U;
    receipt.sentinel_literal_load_offset = entry + 52U;
    receipt.control_shift_offset = entry + 54U;
    receipt.control_test_offset = entry + 56U;
    receipt.refill_skip_branch_offset = entry + 58U;
    receipt.refill_guard_offset = entry + 60U;
    receipt.refill_failure_branch_offset = entry + 62U;
    receipt.refill_byte_load_offset = entry + 64U;
    receipt.refill_merge_offset = entry + 70U;
    if (!sh2_bra_target(receipt.outer_loop_branch_offset,
                        read_be16(data + receipt.outer_loop_branch_offset), size,
                        &receipt.reentry_target) ||
        read_be16(data + receipt.sentinel_literal_load_offset) != 0x926aU ||
        !sh2_movw_pc_literal_target(
            receipt.sentinel_literal_load_offset,
            read_be16(data + receipt.sentinel_literal_load_offset),
            &receipt.sentinel_literal_offset) ||
        receipt.sentinel_literal_offset + 2U > size ||
        read_be16(data + receipt.control_shift_offset) != 0x4b21U ||
        !sh2_tst_register_fields(read_be16(data + receipt.control_test_offset),
                                 &receipt.control_test_source_register,
                                 &receipt.control_test_destination_register) ||
        !sh2_conditional_branch_target(
            receipt.refill_skip_branch_offset,
            read_be16(data + receipt.refill_skip_branch_offset), size,
            &receipt.refill_skip_target) ||
        read_be16(data + receipt.refill_guard_offset) != 0x2ee8U ||
        !sh2_conditional_branch_target(
            receipt.refill_failure_branch_offset,
            read_be16(data + receipt.refill_failure_branch_offset), size,
            &receipt.refill_failure_target) ||
        !sh2_movb_postinc_fields(read_be16(data + receipt.refill_byte_load_offset),
                                 &receipt.refill_cursor_register,
                                 &receipt.refill_value_register) ||
        read_be16(data + receipt.refill_byte_load_offset + 2U) != 0x7effU ||
        read_be16(data + receipt.refill_byte_load_offset + 4U) != 0x6bbcU ||
        !sh2_logic_register_fields(read_be16(data + receipt.refill_merge_offset),
                                   0x200bU,
                                   &receipt.refill_merge_source_register,
                                   &receipt.refill_merge_destination_register) ||
        receipt.reentry_target != receipt.sentinel_literal_load_offset ||
        receipt.control_test_source_register != 11U ||
        receipt.control_test_destination_register != 2U ||
        receipt.refill_skip_target != entry + 72U ||
        receipt.refill_failure_target != entry + 166U ||
        receipt.refill_cursor_register != 12U ||
        receipt.refill_value_register != 11U ||
        receipt.refill_merge_source_register != 9U ||
        receipt.refill_merge_destination_register != 11U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.sentinel_word = read_be16(data + receipt.sentinel_literal_offset);
    if (receipt.sentinel_word != 0x0100U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.control_shift_register = 11U;
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_outer_loop_low_bit_join_receipt(
    const uint8_t *data, size_t size,
    Nexus_Prs3V1OuterLoopLowBitJoinReceipt *out) {
    Nexus_Prs3V1OuterLoopLowBitJoinReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 100U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.skip_branch_offset = entry + 58U;
    receipt.refill_merge_offset = entry + 70U;
    receipt.low_bit_immediate_offset = entry + 72U;
    receipt.low_bit_test_offset = entry + 74U;
    receipt.zero_bit_branch_offset = entry + 76U;
    if (read_be16(data + receipt.skip_branch_offset) != 0x8b05U ||
        !sh2_conditional_branch_target(
            receipt.skip_branch_offset,
            read_be16(data + receipt.skip_branch_offset), size,
            &receipt.skip_target) ||
        !sh2_logic_register_fields(read_be16(data + receipt.refill_merge_offset),
                                   0x200bU,
                                   &receipt.refill_merge_source_register,
                                   &receipt.refill_merge_destination_register) ||
        !sh2_mov_immediate_fields(
            read_be16(data + receipt.low_bit_immediate_offset),
            &receipt.low_bit_register, &receipt.low_bit_immediate) ||
        !sh2_tst_register_fields(read_be16(data + receipt.low_bit_test_offset),
                                 &receipt.low_bit_test_source_register,
                                 &receipt.low_bit_test_destination_register) ||
        !sh2_conditional_branch_target(
            receipt.zero_bit_branch_offset,
            read_be16(data + receipt.zero_bit_branch_offset), size,
            &receipt.zero_bit_target) ||
        receipt.skip_target != receipt.low_bit_immediate_offset ||
        receipt.refill_merge_source_register != 9U ||
        receipt.refill_merge_destination_register != 11U ||
        receipt.low_bit_register != 3U || receipt.low_bit_immediate != 1 ||
        receipt.low_bit_test_source_register != 11U ||
        receipt.low_bit_test_destination_register != 3U ||
        receipt.zero_bit_target != entry + 100U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_failure_call_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1FailureCallReceipt *out) {
    Nexus_Prs3V1FailureCallReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 290U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.failure_target = entry + 166U;
    receipt.literal_load_offset = receipt.failure_target;
    receipt.indirect_call_offset = entry + 168U;
    receipt.call_delay_offset = entry + 170U;
    receipt.failure_result_offset = entry + 174U;
    receipt.return_offset = entry + 188U;
    if (read_be16(data + entry + 62U) != 0x8932U ||
        !sh2_conditional_branch_target(entry + 62U,
                                       read_be16(data + entry + 62U), size,
                                       &receipt.failure_target) ||
        read_be16(data + receipt.literal_load_offset) != 0xd31eU ||
        !sh2_movl_pc_literal_target(
            receipt.literal_load_offset,
            read_be16(data + receipt.literal_load_offset),
            &receipt.literal_offset) ||
        receipt.literal_offset + 4U > size ||
        !sh2_jsr_register(read_be16(data + receipt.indirect_call_offset),
                          &receipt.indirect_call_register) ||
        !sh2_mov_register_fields(read_be16(data + receipt.call_delay_offset),
                                 &receipt.call_delay_source_register,
                                 &receipt.call_delay_destination_register) ||
        read_be16(data + receipt.failure_result_offset) != 0xe000U ||
        read_be16(data + receipt.return_offset) != 0x000bU ||
        receipt.failure_target != entry + 166U ||
        receipt.literal_offset != entry + 288U ||
        receipt.indirect_call_register != 3U ||
        receipt.call_delay_source_register != 13U ||
        receipt.call_delay_destination_register != 4U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.literal_register = 3U;
    receipt.literal_word = read_be32(data + receipt.literal_offset);
    if (receipt.literal_word != 0x060284e0U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_entry_register_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1EntryRegisterReceipt *out) {
    Nexus_Prs3V1EntryRegisterReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    unsigned int source = 0U, destination = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 276U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.r4_to_r12_offset = entry + 6U;
    receipt.first_call_literal_load_offset = entry + 8U;
    receipt.r5_to_r10_offset = entry + 14U;
    receipt.first_indirect_call_offset = entry + 24U;
    receipt.first_call_delay_offset = entry + 26U;
    receipt.post_call_copy_offset = entry + 28U;
    receipt.post_call_test_offset = entry + 30U;
    receipt.post_call_branch_offset = entry + 32U;
    receipt.r11_zero_offset = entry + 42U;
    if (!sh2_mov_register_fields(read_be16(data + receipt.r4_to_r12_offset),
                                 &source, &destination) ||
        source != 4U || destination != 12U ||
        read_be16(data + receipt.first_call_literal_load_offset) != 0xd341U ||
        !sh2_movl_pc_literal_target(
            receipt.first_call_literal_load_offset,
            read_be16(data + receipt.first_call_literal_load_offset),
            &receipt.first_call_literal_offset) ||
        receipt.first_call_literal_offset + 4U > size ||
        !sh2_mov_register_fields(read_be16(data + receipt.r5_to_r10_offset),
                                 &source, &destination) ||
        source != 5U || destination != 10U ||
        !sh2_jsr_register(read_be16(data + receipt.first_indirect_call_offset),
                          &receipt.first_indirect_call_register) ||
        !sh2_mov_register_fields(read_be16(data + receipt.first_call_delay_offset),
                                 &receipt.first_call_delay_source_register,
                                 &receipt.first_call_delay_destination_register) ||
        !sh2_mov_register_fields(read_be16(data + receipt.post_call_copy_offset),
                                 &receipt.post_call_copy_source_register,
                                 &receipt.post_call_copy_destination_register) ||
        !sh2_tst_register_fields(read_be16(data + receipt.post_call_test_offset),
                                 &source, &destination) ||
        source != 13U || destination != 13U ||
        !sh2_conditional_branch_target(
            receipt.post_call_branch_offset,
            read_be16(data + receipt.post_call_branch_offset), size,
            &receipt.post_call_branch_target) ||
        read_be16(data + receipt.r11_zero_offset) != 0xeb00U ||
        receipt.first_indirect_call_register != 3U ||
        receipt.first_call_delay_source_register != 6U ||
        receipt.first_call_delay_destination_register != 14U ||
        receipt.post_call_copy_source_register != 0U ||
        receipt.post_call_copy_destination_register != 13U ||
        receipt.post_call_branch_target != receipt.r11_zero_offset) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.first_call_literal_register = 3U;
    receipt.first_call_literal_word =
        read_be32(data + receipt.first_call_literal_offset);
    if (receipt.first_call_literal_word != 0x06028490U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_entry_bypass_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1EntryBypassReceipt *out) {
    Nexus_Prs3V1EntryBypassReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || entry + 284U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.bypass_branch_offset = entry + 32U;
    receipt.second_r4_literal_load_offset = entry + 34U;
    receipt.second_r3_literal_load_offset = entry + 36U;
    receipt.second_indirect_call_offset = entry + 38U;
    receipt.second_call_delay_offset = entry + 40U;
    receipt.r11_zero_offset = entry + 42U;
    if (read_be16(data + receipt.bypass_branch_offset) != 0x8b03U ||
        !sh2_conditional_branch_target(
            receipt.bypass_branch_offset,
            read_be16(data + receipt.bypass_branch_offset), size,
            &receipt.bypass_target) ||
        read_be16(data + receipt.second_r4_literal_load_offset) != 0xd43cU ||
        !sh2_movl_pc_literal_target(
            receipt.second_r4_literal_load_offset,
            read_be16(data + receipt.second_r4_literal_load_offset),
            &receipt.second_r4_literal_offset) ||
        receipt.second_r4_literal_offset + 4U > size ||
        read_be16(data + receipt.second_r3_literal_load_offset) != 0xd33cU ||
        !sh2_movl_pc_literal_target(
            receipt.second_r3_literal_load_offset,
            read_be16(data + receipt.second_r3_literal_load_offset),
            &receipt.second_r3_literal_offset) ||
        receipt.second_r3_literal_offset + 4U > size ||
        !sh2_jsr_register(read_be16(data + receipt.second_indirect_call_offset),
                          &receipt.second_indirect_call_register) ||
        !sh2_mov_immediate_fields(
            read_be16(data + receipt.second_call_delay_offset),
            &receipt.second_call_delay_register,
            &receipt.second_call_delay_immediate) ||
        read_be16(data + receipt.r11_zero_offset) != 0xeb00U ||
        receipt.bypass_target != receipt.r11_zero_offset ||
        receipt.second_indirect_call_register != 3U ||
        receipt.second_call_delay_register != 5U ||
        receipt.second_call_delay_immediate != 70) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.second_r4_literal_word = read_be32(data + receipt.second_r4_literal_offset);
    receipt.second_r3_literal_word = read_be32(data + receipt.second_r3_literal_offset);
    if (receipt.second_r4_literal_word != 0x06047048U ||
        receipt.second_r3_literal_word != 0x06024084U) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out) *out = receipt;
    return 1;
}

static int prs3_v1_caller_to_callee_receipt(
    const uint8_t *data, size_t size, Nexus_Prs3V1CallerToCalleeReceipt *out) {
    Nexus_Prs3V1CallerToCalleeReceipt receipt;
    unsigned int move_source = 0U, move_destination = 0U;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 28U > size) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.caller_word_load_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET - 6U;
    receipt.version1_branch_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 2U;
    receipt.version1_branch_delay_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 4U;
    receipt.version1_call_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 26U;
    receipt.version1_call_delay_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 28U;
    receipt.callee_r6_to_r14_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 26U;
    if (!sh2_movl_disp_register_fields(
            read_be16(data + receipt.caller_word_load_offset),
            &receipt.caller_word_base_register,
            &receipt.caller_word_value_register,
            &receipt.caller_word_byte_displacement) ||
        read_be16(data + receipt.version1_branch_offset) != 0x8d0aU ||
        !sh2_conditional_branch_target(
            receipt.version1_branch_offset,
            read_be16(data + receipt.version1_branch_offset), size,
            &receipt.version1_branch_target) ||
        !sh2_mov_register_fields(
            read_be16(data + receipt.version1_branch_delay_offset),
            &move_source, &move_destination) ||
        move_source != 3U || move_destination != 11U ||
        !sh2_bsr_target(receipt.version1_call_offset,
                        read_be16(data + receipt.version1_call_offset), size,
                        &receipt.version1_call_target) ||
        !sh2_mov_register_fields(
            read_be16(data + receipt.version1_call_delay_offset),
            &move_source, &move_destination) ||
        move_source != 11U || move_destination != 6U ||
        !sh2_mov_register_fields(
            read_be16(data + receipt.callee_r6_to_r14_offset),
            &move_source, &move_destination) ||
        receipt.caller_word_base_register != 6U ||
        receipt.caller_word_value_register != 3U ||
        receipt.caller_word_byte_displacement != 12U ||
        receipt.version1_branch_target != receipt.version1_call_offset ||
        receipt.version1_call_target != NEXUS_PRS3_VERSION1_CALLEE_OFFSET ||
        move_source != 6U || move_destination != 14U) {
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

static void test_synthetic_zero_side_merge(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1ZeroSideMergeReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 44U] = 0x98U; fixture[entry + 45U] = 0x6cU;
    fixture[entry + 264U] = 0x0fU; fixture[entry + 265U] = 0x00U;
    fixture[entry + 106U] = 0xe2U; fixture[entry + 107U] = 0x0fU;
    fixture[entry + 116U] = 0x63U; fixture[entry + 117U] = 0x73U;
    fixture[entry + 118U] = 0x43U; fixture[entry + 119U] = 0x08U;
    fixture[entry + 120U] = 0x43U; fixture[entry + 121U] = 0x08U;
    fixture[entry + 122U] = 0x23U; fixture[entry + 123U] = 0x89U;
    fixture[entry + 124U] = 0x24U; fixture[entry + 125U] = 0x3bU;
    fixture[entry + 126U] = 0x27U; fixture[entry + 127U] = 0x29U;
    check(prs3_v1_zero_side_merge_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.upper_mask_word == 0x0f00U &&
              receipt.low_mask_immediate == 15,
          "SH-2 PRS3 zero-side receipt locks shift/mask/OR register algebra");
    fixture[entry + 125U] = 0x39U;
    check(!prs3_v1_zero_side_merge_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 zero-side receipt rejects a non-R4 merge operation");
}

static void test_synthetic_zero_side_merge_branch(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1ZeroSideMergeBranchReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 128U] = 0x77U; fixture[entry + 129U] = 0x02U;
    fixture[entry + 130U] = 0x37U; fixture[entry + 131U] = 0x4cU;
    fixture[entry + 132U] = 0x34U; fixture[entry + 133U] = 0x77U;
    fixture[entry + 134U] = 0x89U; fixture[entry + 135U] = 0xd5U;
    check(prs3_v1_zero_side_merge_branch_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.merged_value_source_register == 4U &&
              receipt.merged_value_destination_register == 7U &&
              receipt.compare_source_register == 7U &&
              receipt.compare_destination_register == 4U &&
              receipt.control_branch_target == entry + 52U,
          "SH-2 PRS3 zero-side receipt locks merged-value comparison branch");
    fixture[entry + 135U] = 0xd4U;
    check(!prs3_v1_zero_side_merge_branch_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 zero-side receipt rejects a changed merged-value branch");
}

static void test_synthetic_merged_value_read(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1MergedValueReadReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 50U] = 0x95U; fixture[entry + 51U] = 0x6aU;
    fixture[entry + 266U] = 0x0fU; fixture[entry + 267U] = 0xffU;
    fixture[entry + 142U] = 0x60U; fixture[entry + 143U] = 0x43U;
    fixture[entry + 144U] = 0x20U; fixture[entry + 145U] = 0x59U;
    fixture[entry + 148U] = 0x01U; fixture[entry + 149U] = 0xdcU;
    check(prs3_v1_merged_value_read_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.index_mask_word == 0x0fffU &&
              receipt.merged_value_copy_source_register == 4U &&
              receipt.merged_value_copy_destination_register == 0U &&
              receipt.indexed_byte_base_register == 13U &&
              receipt.indexed_byte_destination_register == 1U,
          "SH-2 PRS3 merged-value receipt locks masked R13-indexed byte read");
    fixture[entry + 149U] = 0xccU;
    check(!prs3_v1_merged_value_read_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 merged-value receipt rejects a changed indexed byte read");
}

static void test_synthetic_post_read_control(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1PostReadControlReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 150U] = 0x34U; fixture[entry + 151U] = 0x77U;
    fixture[entry + 152U] = 0x23U; fixture[entry + 153U] = 0x10U;
    fixture[entry + 154U] = 0x2aU; fixture[entry + 155U] = 0x10U;
    fixture[entry + 156U] = 0x7aU; fixture[entry + 157U] = 0x01U;
    fixture[entry + 158U] = 0x8fU; fixture[entry + 159U] = 0xf3U;
    fixture[entry + 162U] = 0xafU; fixture[entry + 163U] = 0xc7U;
    check(prs3_v1_post_read_control_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.merged_compare_source_register == 7U &&
              receipt.merged_compare_destination_register == 4U &&
              receipt.branch_condition_source_register == 1U &&
              receipt.branch_condition_destination_register == 10U &&
              receipt.delayed_branch_target == entry + 136U,
          "SH-2 PRS3 post-read receipt locks comparison overwrite and repeat branch");
    fixture[entry + 155U] = 0x00U;
    check(!prs3_v1_post_read_control_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 post-read receipt rejects a changed branch-condition compare");
}

static void test_synthetic_post_read_branch(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1PostReadBranchReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 154U] = 0x2aU; fixture[entry + 155U] = 0x10U;
    fixture[entry + 156U] = 0x7aU; fixture[entry + 157U] = 0x01U;
    fixture[entry + 158U] = 0x8fU; fixture[entry + 159U] = 0xf3U;
    fixture[entry + 160U] = 0x26U; fixture[entry + 161U] = 0x59U;
    fixture[entry + 162U] = 0xafU; fixture[entry + 163U] = 0xc7U;
    check(prs3_v1_post_read_branch_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.compare_source_register == 1U &&
              receipt.compare_destination_register == 10U &&
              receipt.delay_slot_source_register == 5U &&
              receipt.delay_slot_destination_register == 6U &&
              receipt.delayed_branch_target == entry + 136U &&
              receipt.outer_loop_target == entry + 52U,
          "SH-2 PRS3 post-read branch receipt locks BF/S delay and both targets");
    fixture[entry + 161U] = 0x58U;
    check(!prs3_v1_post_read_branch_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 post-read branch receipt rejects a changed delay-slot operation");
}

static void test_synthetic_repeat_r6_mask(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1RepeatR6MaskReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 50U] = 0x95U; fixture[entry + 51U] = 0x6aU;
    fixture[entry + 266U] = 0x0fU; fixture[entry + 267U] = 0xffU;
    fixture[entry + 136U] = 0x63U; fixture[entry + 137U] = 0x63U;
    fixture[entry + 138U] = 0x76U; fixture[entry + 139U] = 0x01U;
    fixture[entry + 158U] = 0x8fU; fixture[entry + 159U] = 0xf3U;
    fixture[entry + 160U] = 0x26U; fixture[entry + 161U] = 0x59U;
    fixture[entry + 162U] = 0xafU; fixture[entry + 163U] = 0xc7U;
    check(prs3_v1_repeat_r6_mask_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.mask_word == 0x0fffU && receipt.r6_increment == 1 &&
              receipt.delay_slot_mask_source_register == 5U &&
              receipt.delay_slot_mask_destination_register == 6U &&
              receipt.local_repeat_target == entry + 136U &&
              receipt.outer_loop_target == entry + 52U,
          "SH-2 PRS3 repeat receipt locks shared R6 increment/mask paths");
    fixture[entry + 161U] = 0x58U;
    check(!prs3_v1_repeat_r6_mask_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 repeat receipt rejects a changed R6 delay-slot mask");
}

static void test_synthetic_outer_loop_reentry(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1OuterLoopReentryReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 52U] = 0x92U; fixture[entry + 53U] = 0x6aU;
    fixture[entry + 268U] = 0x01U; fixture[entry + 269U] = 0x00U;
    fixture[entry + 54U] = 0x4bU; fixture[entry + 55U] = 0x21U;
    fixture[entry + 56U] = 0x22U; fixture[entry + 57U] = 0xb8U;
    fixture[entry + 58U] = 0x8bU; fixture[entry + 59U] = 0x05U;
    fixture[entry + 60U] = 0x2eU; fixture[entry + 61U] = 0xe8U;
    fixture[entry + 62U] = 0x89U; fixture[entry + 63U] = 0x32U;
    fixture[entry + 64U] = 0x6bU; fixture[entry + 65U] = 0xc4U;
    fixture[entry + 66U] = 0x7eU; fixture[entry + 67U] = 0xffU;
    fixture[entry + 68U] = 0x6bU; fixture[entry + 69U] = 0xbcU;
    fixture[entry + 70U] = 0x2bU; fixture[entry + 71U] = 0x9bU;
    fixture[entry + 162U] = 0xafU; fixture[entry + 163U] = 0xc7U;
    check(prs3_v1_outer_loop_reentry_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.reentry_target == entry + 52U &&
              receipt.sentinel_word == 0x0100U &&
              receipt.refill_skip_target == entry + 72U &&
              receipt.refill_failure_target == entry + 166U &&
              receipt.refill_cursor_register == 12U &&
              receipt.refill_value_register == 11U &&
              receipt.refill_merge_source_register == 9U &&
              receipt.refill_merge_destination_register == 11U,
          "SH-2 PRS3 outer-loop receipt locks R11 control/refill reentry");
    fixture[entry + 71U] = 0x99U;
    check(!prs3_v1_outer_loop_reentry_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 outer-loop receipt rejects a changed R11 merge");
}

static void test_synthetic_outer_loop_low_bit_join(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1OuterLoopLowBitJoinReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 58U] = 0x8bU; fixture[entry + 59U] = 0x05U;
    fixture[entry + 70U] = 0x2bU; fixture[entry + 71U] = 0x9bU;
    fixture[entry + 72U] = 0xe3U; fixture[entry + 73U] = 0x01U;
    fixture[entry + 74U] = 0x23U; fixture[entry + 75U] = 0xb8U;
    fixture[entry + 76U] = 0x89U; fixture[entry + 77U] = 0x0aU;
    check(prs3_v1_outer_loop_low_bit_join_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.skip_target == entry + 72U &&
              receipt.refill_merge_source_register == 9U &&
              receipt.refill_merge_destination_register == 11U &&
              receipt.zero_bit_target == entry + 100U,
          "SH-2 PRS3 outer-loop receipt locks the low-bit control join");
    fixture[entry + 73U] = 0x02U;
    check(!prs3_v1_outer_loop_low_bit_join_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 outer-loop receipt rejects a changed low-bit setup");
}

static void test_synthetic_failure_call(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1FailureCallReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 62U] = 0x89U; fixture[entry + 63U] = 0x32U;
    fixture[entry + 166U] = 0xd3U; fixture[entry + 167U] = 0x1eU;
    fixture[entry + 168U] = 0x43U; fixture[entry + 169U] = 0x0bU;
    fixture[entry + 170U] = 0x64U; fixture[entry + 171U] = 0xd3U;
    fixture[entry + 174U] = 0xe0U; fixture[entry + 175U] = 0x00U;
    fixture[entry + 188U] = 0x00U; fixture[entry + 189U] = 0x0bU;
    fixture[entry + 288U] = 0x06U; fixture[entry + 289U] = 0x02U;
    fixture[entry + 290U] = 0x84U; fixture[entry + 291U] = 0xe0U;
    check(prs3_v1_failure_call_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.failure_target == entry + 166U &&
              receipt.literal_word == 0x060284e0U &&
              receipt.indirect_call_register == 3U &&
              receipt.call_delay_source_register == 13U &&
              receipt.call_delay_destination_register == 4U,
          "SH-2 PRS3 failure receipt locks literal-fed indirect-call shape");
    fixture[entry + 169U] = 0x09U;
    check(!prs3_v1_failure_call_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 failure receipt rejects a non-JSR failure call");
}

static void test_synthetic_entry_registers(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1EntryRegisterReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 6U] = 0x6cU; fixture[entry + 7U] = 0x43U;
    fixture[entry + 8U] = 0xd3U; fixture[entry + 9U] = 0x41U;
    fixture[entry + 14U] = 0x6aU; fixture[entry + 15U] = 0x53U;
    fixture[entry + 24U] = 0x43U; fixture[entry + 25U] = 0x0bU;
    fixture[entry + 26U] = 0x6eU; fixture[entry + 27U] = 0x63U;
    fixture[entry + 28U] = 0x6dU; fixture[entry + 29U] = 0x03U;
    fixture[entry + 30U] = 0x2dU; fixture[entry + 31U] = 0xd8U;
    fixture[entry + 32U] = 0x8bU; fixture[entry + 33U] = 0x03U;
    fixture[entry + 42U] = 0xebU; fixture[entry + 43U] = 0x00U;
    fixture[entry + 272U] = 0x06U; fixture[entry + 273U] = 0x02U;
    fixture[entry + 274U] = 0x84U; fixture[entry + 275U] = 0x90U;
    check(prs3_v1_entry_register_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.first_call_literal_word == 0x06028490U &&
              receipt.first_call_delay_source_register == 6U &&
              receipt.first_call_delay_destination_register == 14U &&
              receipt.post_call_copy_source_register == 0U &&
              receipt.post_call_copy_destination_register == 13U &&
              receipt.post_call_branch_target == entry + 42U,
          "SH-2 PRS3 entry receipt locks loop-register setup dataflow");
    fixture[entry + 27U] = 0x62U;
    check(!prs3_v1_entry_register_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 entry receipt rejects a changed R6-to-R14 delay copy");
}

static void test_synthetic_entry_bypass(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Nexus_Prs3V1EntryBypassReceipt receipt;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 32U] = 0x8bU; fixture[entry + 33U] = 0x03U;
    fixture[entry + 34U] = 0xd4U; fixture[entry + 35U] = 0x3cU;
    fixture[entry + 36U] = 0xd3U; fixture[entry + 37U] = 0x3cU;
    fixture[entry + 38U] = 0x43U; fixture[entry + 39U] = 0x0bU;
    fixture[entry + 40U] = 0xe5U; fixture[entry + 41U] = 0x46U;
    fixture[entry + 42U] = 0xebU; fixture[entry + 43U] = 0x00U;
    fixture[entry + 276U] = 0x06U; fixture[entry + 277U] = 0x04U;
    fixture[entry + 278U] = 0x70U; fixture[entry + 279U] = 0x48U;
    fixture[entry + 280U] = 0x06U; fixture[entry + 281U] = 0x02U;
    fixture[entry + 282U] = 0x40U; fixture[entry + 283U] = 0x84U;
    check(prs3_v1_entry_bypass_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.bypass_target == entry + 42U &&
              receipt.second_r4_literal_word == 0x06047048U &&
              receipt.second_r3_literal_word == 0x06024084U &&
              receipt.second_indirect_call_register == 3U &&
              receipt.second_call_delay_register == 5U &&
              receipt.second_call_delay_immediate == 70,
          "SH-2 PRS3 entry receipt locks the alternate call/bypass dataflow");
    fixture[entry + 41U] = 0x45U;
    check(!prs3_v1_entry_bypass_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 entry receipt rejects a changed alternate-call delay");
}

static void test_synthetic_caller_to_callee(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Nexus_Prs3V1CallerToCalleeReceipt receipt;

    memset(fixture, 0, sizeof(fixture));
    fixture[85246U] = 0x53U; fixture[85247U] = 0x63U;
    fixture[85254U] = 0x8dU; fixture[85255U] = 0x0aU;
    fixture[85256U] = 0x6bU; fixture[85257U] = 0x33U;
    fixture[85278U] = 0xb0U; fixture[85279U] = 0x2fU;
    fixture[85280U] = 0x66U; fixture[85281U] = 0xb3U;
    fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 26U] = 0x6eU;
    fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 27U] = 0x63U;
    check(prs3_v1_caller_to_callee_receipt(
              fixture, sizeof(fixture), &receipt) && receipt.valid &&
              receipt.caller_word_base_register == 6U &&
              receipt.caller_word_value_register == 3U &&
              receipt.caller_word_byte_displacement == 12U &&
              receipt.version1_branch_target == 85278U &&
              receipt.version1_call_target == NEXUS_PRS3_VERSION1_CALLEE_OFFSET,
          "SH-2 PRS3 caller receipt locks R6+12 through callee R14");
    fixture[85281U] = 0xb2U;
    check(!prs3_v1_caller_to_callee_receipt(
              fixture, sizeof(fixture), &receipt),
          "SH-2 PRS3 caller receipt rejects a changed callee-argument move");
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
    Nexus_Prs3V1ZeroSideMergeReceipt zero_side_merge_receipt;
    Nexus_Prs3V1ZeroSideMergeBranchReceipt zero_side_merge_branch_receipt;
    Nexus_Prs3V1MergedValueReadReceipt merged_value_read_receipt;
    Nexus_Prs3V1PostReadControlReceipt post_read_control_receipt;
    Nexus_Prs3V1PostReadBranchReceipt post_read_branch_receipt;
    Nexus_Prs3V1RepeatR6MaskReceipt repeat_r6_mask_receipt;
    Nexus_Prs3V1OuterLoopReentryReceipt outer_loop_reentry_receipt;
    Nexus_Prs3V1OuterLoopLowBitJoinReceipt outer_loop_low_bit_join_receipt;
    Nexus_Prs3V1FailureCallReceipt failure_call_receipt;
    Nexus_Prs3V1EntryRegisterReceipt entry_register_receipt;
    Nexus_Prs3V1EntryBypassReceipt entry_bypass_receipt;
    Nexus_Prs3V1CallerToCalleeReceipt caller_to_callee_receipt;

    test_synthetic_branch_flow();
    test_synthetic_zero_side_read();
    test_synthetic_termination();
    test_synthetic_low_bit_consumption();
    test_synthetic_zero_side_merge();
    test_synthetic_zero_side_merge_branch();
    test_synthetic_merged_value_read();
    test_synthetic_post_read_control();
    test_synthetic_post_read_branch();
    test_synthetic_repeat_r6_mask();
    test_synthetic_outer_loop_reentry();
    test_synthetic_outer_loop_low_bit_join();
    test_synthetic_failure_call();
    test_synthetic_entry_registers();
    test_synthetic_entry_bypass();
    test_synthetic_caller_to_callee();
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
        check(prs3_v1_zero_side_merge_receipt(
                  data, size, &zero_side_merge_receipt) &&
                  zero_side_merge_receipt.valid,
              "DM.BIN locks the PRS3 v1 zero-side shift/mask/OR operation");
        check(prs3_v1_zero_side_merge_branch_receipt(
                  data, size, &zero_side_merge_branch_receipt) &&
                  zero_side_merge_branch_receipt.valid,
              "DM.BIN locks the PRS3 v1 merged-value control branch");
        check(prs3_v1_merged_value_read_receipt(
                  data, size, &merged_value_read_receipt) &&
                  merged_value_read_receipt.valid,
              "DM.BIN locks the PRS3 v1 masked R13-indexed byte read");
        check(prs3_v1_post_read_control_receipt(
                  data, size, &post_read_control_receipt) &&
                  post_read_control_receipt.valid,
              "DM.BIN locks the PRS3 v1 post-read comparison overwrite and repeat branch");
        check(prs3_v1_post_read_branch_receipt(
                  data, size, &post_read_branch_receipt) &&
                  post_read_branch_receipt.valid,
              "DM.BIN locks the PRS3 v1 post-read BF/S delay and branch targets");
        check(prs3_v1_repeat_r6_mask_receipt(
                  data, size, &repeat_r6_mask_receipt) &&
                  repeat_r6_mask_receipt.valid,
              "DM.BIN locks the PRS3 v1 shared R6 increment/mask continuation");
        check(prs3_v1_outer_loop_reentry_receipt(
                  data, size, &outer_loop_reentry_receipt) &&
                  outer_loop_reentry_receipt.valid,
              "DM.BIN locks the PRS3 v1 outer-loop R11 control/refill reentry");
        check(prs3_v1_outer_loop_low_bit_join_receipt(
                  data, size, &outer_loop_low_bit_join_receipt) &&
                  outer_loop_low_bit_join_receipt.valid,
              "DM.BIN locks the PRS3 v1 outer-loop low-bit control join");
        check(prs3_v1_failure_call_receipt(data, size, &failure_call_receipt) &&
                  failure_call_receipt.valid,
              "DM.BIN locks the PRS3 v1 failure literal/call dataflow");
        check(prs3_v1_entry_register_receipt(data, size, &entry_register_receipt) &&
                  entry_register_receipt.valid,
              "DM.BIN locks the PRS3 v1 entry loop-register dataflow");
        check(prs3_v1_entry_bypass_receipt(data, size, &entry_bypass_receipt) &&
                  entry_bypass_receipt.valid,
              "DM.BIN locks the PRS3 v1 entry alternate-call/bypass dataflow");
        check(prs3_v1_caller_to_callee_receipt(
                  data, size, &caller_to_callee_receipt) &&
                  caller_to_callee_receipt.valid,
              "DM.BIN locks the PRS3 v1 caller R6+12-to-R14 dataflow");
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
        if (zero_side_merge_receipt.valid) {
            printf("SH-2 PRS3 v1 zero-side merge: R7->R3 shifts=%zu,%zu "
                   "and-mask=%04x or-R4=%zu low-mask=%d; "
                   "token/output-proof=0\n",
                   zero_side_merge_receipt.first_shift_offset,
                   zero_side_merge_receipt.second_shift_offset,
                   (unsigned int)zero_side_merge_receipt.upper_mask_word,
                   zero_side_merge_receipt.merge_or_offset,
                   zero_side_merge_receipt.low_mask_immediate);
        }
        if (zero_side_merge_branch_receipt.valid) {
            printf("SH-2 PRS3 v1 merged-value branch: increment=%zu R7%+d "
                   "add=%zu R%u->R%u compare=%zu R%u,R%u branch=%zu->%zu; "
                   "token/output-proof=0\n",
                   zero_side_merge_branch_receipt.low_fragment_increment_offset,
                   zero_side_merge_branch_receipt.low_fragment_increment,
                   zero_side_merge_branch_receipt.merged_value_add_offset,
                   zero_side_merge_branch_receipt.merged_value_source_register,
                   zero_side_merge_branch_receipt.merged_value_destination_register,
                   zero_side_merge_branch_receipt.merged_value_compare_offset,
                   zero_side_merge_branch_receipt.compare_source_register,
                   zero_side_merge_branch_receipt.compare_destination_register,
                   zero_side_merge_branch_receipt.control_branch_offset,
                   zero_side_merge_branch_receipt.control_branch_target);
        }
        if (merged_value_read_receipt.valid) {
            printf("SH-2 PRS3 v1 merged-value read: R4->R0 mask=%04x "
                   "and=%zu load=%zu @(R0,R%u)->R%u; token/output-proof=0\n",
                   (unsigned int)merged_value_read_receipt.index_mask_word,
                   merged_value_read_receipt.index_mask_and_offset,
                   merged_value_read_receipt.indexed_byte_load_offset,
                   merged_value_read_receipt.indexed_byte_base_register,
                   merged_value_read_receipt.indexed_byte_destination_register);
        }
        if (post_read_control_receipt.valid) {
            printf("SH-2 PRS3 v1 post-read control: R%u,R%u -> R%u,R%u -> R%u,R%u "
                   "delayed=%zu->%zu outer=%zu->%zu; token/output-proof=0\n",
                   post_read_control_receipt.merged_compare_source_register,
                   post_read_control_receipt.merged_compare_destination_register,
                   post_read_control_receipt.first_r1_compare_source_register,
                   post_read_control_receipt.first_r1_compare_destination_register,
                   post_read_control_receipt.branch_condition_source_register,
                   post_read_control_receipt.branch_condition_destination_register,
                   post_read_control_receipt.delayed_branch_offset,
                   post_read_control_receipt.delayed_branch_target,
                   post_read_control_receipt.outer_loop_branch_offset,
                   post_read_control_receipt.outer_loop_target);
        }
        if (post_read_branch_receipt.valid) {
            printf("SH-2 PRS3 v1 post-read branch: cmp=R%u,R%u increment=R%u%+d "
                   "bf/s=%zu->%zu delay=R%u,R%u outer=%zu->%zu; "
                   "token/output-proof=0\n",
                   post_read_branch_receipt.compare_source_register,
                   post_read_branch_receipt.compare_destination_register,
                   post_read_branch_receipt.counter_register,
                   post_read_branch_receipt.counter_increment,
                   post_read_branch_receipt.delayed_branch_offset,
                   post_read_branch_receipt.delayed_branch_target,
                   post_read_branch_receipt.delay_slot_source_register,
                   post_read_branch_receipt.delay_slot_destination_register,
                   post_read_branch_receipt.outer_loop_branch_offset,
                   post_read_branch_receipt.outer_loop_target);
        }
        if (repeat_r6_mask_receipt.valid) {
            printf("SH-2 PRS3 v1 repeat R6: local=%zu R6->R3 increment=R6%+d "
                   "mask=%04x delay=R%u,R%u repeat=%zu outer=%zu; "
                   "token/output-proof=0\n",
                   repeat_r6_mask_receipt.local_entry_offset,
                   repeat_r6_mask_receipt.r6_increment,
                   (unsigned int)repeat_r6_mask_receipt.mask_word,
                   repeat_r6_mask_receipt.delay_slot_mask_source_register,
                   repeat_r6_mask_receipt.delay_slot_mask_destination_register,
                   repeat_r6_mask_receipt.local_repeat_target,
                   repeat_r6_mask_receipt.outer_loop_target);
        }
        if (outer_loop_reentry_receipt.valid) {
            printf("SH-2 PRS3 v1 outer reentry: branch=%zu->%zu shift=R%u "
                   "test=R%u,R%u skip=%zu->%zu guard=%zu->%zu "
                   "refill=@R%u+->R%u merge=R%u,R%u; token/output-proof=0\n",
                   outer_loop_reentry_receipt.outer_loop_branch_offset,
                   outer_loop_reentry_receipt.reentry_target,
                   outer_loop_reentry_receipt.control_shift_register,
                   outer_loop_reentry_receipt.control_test_source_register,
                   outer_loop_reentry_receipt.control_test_destination_register,
                   outer_loop_reentry_receipt.refill_skip_branch_offset,
                   outer_loop_reentry_receipt.refill_skip_target,
                   outer_loop_reentry_receipt.refill_failure_branch_offset,
                   outer_loop_reentry_receipt.refill_failure_target,
                   outer_loop_reentry_receipt.refill_cursor_register,
                   outer_loop_reentry_receipt.refill_value_register,
                   outer_loop_reentry_receipt.refill_merge_source_register,
                   outer_loop_reentry_receipt.refill_merge_destination_register);
        }
        if (outer_loop_low_bit_join_receipt.valid) {
            printf("SH-2 PRS3 v1 outer join: skip=%zu->%zu refill=R%u,R%u "
                   "low-test=R%u,R%u zero=%zu->%zu; token/output-proof=0\n",
                   outer_loop_low_bit_join_receipt.skip_branch_offset,
                   outer_loop_low_bit_join_receipt.skip_target,
                   outer_loop_low_bit_join_receipt.refill_merge_source_register,
                   outer_loop_low_bit_join_receipt.refill_merge_destination_register,
                   outer_loop_low_bit_join_receipt.low_bit_test_source_register,
                   outer_loop_low_bit_join_receipt.low_bit_test_destination_register,
                   outer_loop_low_bit_join_receipt.zero_bit_branch_offset,
                   outer_loop_low_bit_join_receipt.zero_bit_target);
        }
        if (failure_call_receipt.valid) {
            printf("SH-2 PRS3 v1 failure call: target=%zu literal=%zu R%u=%08x "
                   "jsr=@R%u delay=R%u->R%u result=%zu return=%zu; "
                   "callee-proof=0\n",
                   failure_call_receipt.failure_target,
                   failure_call_receipt.literal_offset,
                   failure_call_receipt.literal_register,
                   (unsigned int)failure_call_receipt.literal_word,
                   failure_call_receipt.indirect_call_register,
                   failure_call_receipt.call_delay_source_register,
                   failure_call_receipt.call_delay_destination_register,
                   failure_call_receipt.failure_result_offset,
                   failure_call_receipt.return_offset);
        }
        if (entry_register_receipt.valid) {
            printf("SH-2 PRS3 v1 entry: R4->R12 R5->R10 literal=%zu R%u=%08x "
                   "jsr=@R%u delay=R%u->R%u post=R%u->R%u branch=%zu->%zu "
                   "R11=0; callee/ABI-proof=0\n",
                   entry_register_receipt.first_call_literal_offset,
                   entry_register_receipt.first_call_literal_register,
                   (unsigned int)entry_register_receipt.first_call_literal_word,
                   entry_register_receipt.first_indirect_call_register,
                   entry_register_receipt.first_call_delay_source_register,
                   entry_register_receipt.first_call_delay_destination_register,
                   entry_register_receipt.post_call_copy_source_register,
                   entry_register_receipt.post_call_copy_destination_register,
                   entry_register_receipt.post_call_branch_offset,
                   entry_register_receipt.post_call_branch_target);
        }
        if (entry_bypass_receipt.valid) {
            printf("SH-2 PRS3 v1 entry bypass: branch=%zu->%zu R4=%08x R3=%08x "
                   "jsr=@R%u delay=R%u=%d R11=0; callee/ABI-proof=0\n",
                   entry_bypass_receipt.bypass_branch_offset,
                   entry_bypass_receipt.bypass_target,
                   (unsigned int)entry_bypass_receipt.second_r4_literal_word,
                   (unsigned int)entry_bypass_receipt.second_r3_literal_word,
                   entry_bypass_receipt.second_indirect_call_register,
                   entry_bypass_receipt.second_call_delay_register,
                   entry_bypass_receipt.second_call_delay_immediate);
        }
        if (caller_to_callee_receipt.valid) {
            printf("SH-2 PRS3 v1 caller: @R%u+%u->R%u branch=%zu->%zu "
                   "call=%zu->%zu delay=R11->R6 callee=%zu R6->R14; "
                   "descriptor/payload-proof=0\n",
                   caller_to_callee_receipt.caller_word_base_register,
                   caller_to_callee_receipt.caller_word_byte_displacement,
                   caller_to_callee_receipt.caller_word_value_register,
                   caller_to_callee_receipt.version1_branch_offset,
                   caller_to_callee_receipt.version1_branch_target,
                   caller_to_callee_receipt.version1_call_offset,
                   caller_to_callee_receipt.version1_call_target,
                   caller_to_callee_receipt.callee_r6_to_r14_offset);
        }
    }
    free(data);
    return failures == 0 ? 0 : 1;
}
