/*
 * Original Saturn PRS3 loader-media evidence probe.
 *
 * This is deliberately not a decoder. It hash-gates the verified Japanese
 * Track 1 DM.BIN, inventories executable-side PRS3 markers, and records only
 * SH-2 executable-layout evidence. The original executable currently proves
 * that PRS3 is known to the game binary. This
 * probe additionally decodes the conditional branches immediately following
 * the locked version comparisons, so the selected version-1 route can be
 * recorded from the real executable. The selected callee's direct basic-block
 * edges are also locked. The selected caller-side R6+12 slot is now traced
 * through its caller-stack preservation and selected-call register lifetime, but no
 * register/dataflow trace has connected either value to
 * a payload-bit reader, opcode reader, completion condition, or palette
 * handoff. The first real indirect call is additionally traced only far
 * enough to prove its post-call R0 value becomes the byte-store base in R13. Keep
 * decoder promotion false until independent bit-reader and completion proof
 * exists.
 *
 * ReDMCSB has no Saturn/Nexus implementation. Provenance is the hash-locked
 * Track 1 corpus in docs/VERIFIED_HASHES.md and
 * docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md.
 */

#include "firestaff_x68k_media_receipt.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEXUS_PRS3_LOADER_DM_MD5 "e88d60859f65f08fa622e1992b02280f"
#define NEXUS_PRS3_LOADER_DM_SIZE 555144U

/* DM.BIN begins with a real SH-2 bootstrap that jumps to 0x06010014.  Its
 * file-relative entry offset derives a 0x06010000 initial map.  Separately,
 * the version-1 callee's 0x06028490 literal aligns with file offset 0x8490
 * only under a 0x06020000 candidate map.  The flat file contains no relocation
 * record, vector table, or indirect-dispatch entry that proves the transition
 * between these maps, so neither is a universal image base. */
#define NEXUS_PRS3_SH2_BOOTSTRAP_IMAGE_BASE 0x06010000U
#define NEXUS_PRS3_SH2_MASTER_VECTOR_BASE 0x06000000U
#define NEXUS_PRS3_SH2_CALLEE_LITERAL_IMAGE_BASE 0x06020000U
#define NEXUS_PRS3_BOOTSTRAP_ENTRY_OFFSET 0x14U
#define NEXUS_PRS3_BOOTSTRAP_ENTRY_ADDRESS 0x06010014U
#define NEXUS_PRS3_VERSION1_FIRST_CALL_ADDRESS 0x06028490U
#define NEXUS_PRS3_VERSION1_FIRST_CALL_IMAGE_OFFSET 0x00008490U

/* These are byte locations in the MD5-verified Japanese DM.BIN only. */
#define NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET 85356U
#define NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET 231668U

/* These file offsets are an instruction-level receipt for the MD5-locked
 * DM.BIN only. They are decoded from the SH-2 instruction encodings in the
 * Renesas/Hitachi SH-2 manual: MOV.L @(disp,PC),Rn, CMP/EQ, BT/BT/S and
 * CMP/EQ #imm,R0. The receipt proves only header recognition and the version
 * dispatcher. It must not be extended into a PRS3 payload-bit interpretation
 * until a selected route is traced to its bit reader and completion branch. */
#define NEXUS_PRS3_MAGIC_PREDICATE_OFFSET 85156U
#define NEXUS_PRS3_DISPATCH_MAGIC_LOAD_OFFSET 85192U
#define NEXUS_PRS3_DISPATCH_MAGIC_COMPARE_OFFSET 85214U
#define NEXUS_PRS3_DISPATCH_MAGIC_BRANCH_OFFSET 85216U
#define NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET 85252U
#define NEXUS_PRS3_VERSION1_CALLEE_OFFSET 85376U
#define NEXUS_PRS3_DISPATCH_ENTRY_OFFSET 85188U

static int g_failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* SH-2 MOV.L @(disp,PC),Rn is 1101nnnndddddddd.  This scan uses only the
 * file-relative form of that documented PC-relative address calculation;
 * it does not establish DM.BIN's load address, reachability, or a function
 * boundary.  A matching row is therefore a candidate literal reference,
 * not loader control-flow proof. */
typedef struct {
    size_t literal_references;
    size_t nearby_indirect_call_candidates;
    size_t recorded_reference_count;
    size_t reference_offsets[4];
} Sh2FlatLiteralEvidence;

typedef struct {
    int valid;
    size_t compare_offsets[5];
    size_t branch_offsets[5];
    size_t branch_targets[5];
    uint16_t branch_instructions[5];
    size_t version1_entry_offset;
    size_t version1_call_offset;
    size_t version1_call_target;
    uint16_t version1_delay_instruction;
} Sh2Prs3VersionDispatchEvidence;

/* This is a control-flow receipt for the selected version-1 callee, not an
 * interpretation of its register state.  The exact instruction words and
 * direct targets are locked so later work can extend the proven trace one
 * basic block at a time without turning a plausible SH-2 disassembly into a
 * PRS3 opcode claim. */
typedef struct {
    int valid;
    size_t entry_offset;
    size_t conditional_call_test_offset;
    size_t conditional_call_bypass_target;
    size_t first_decision_offset;
    size_t first_decision_target;
    size_t early_exit_decision_offset;
    size_t early_exit_target;
    size_t second_decision_offset;
    size_t second_decision_target;
    size_t loop_decision_offset;
    size_t loop_decision_target;
    size_t failure_decision_offset;
    size_t failure_target;
    size_t delayed_backward_branch_offset;
    size_t delayed_backward_branch_target;
    size_t loop_branch_offset;
    size_t loop_target;
    size_t return_offset;
} Sh2Prs3Version1CalleeEvidence;

/* This locks a prerequisite for the callee's literal-output path. It does
 * not name the indirect target, infer allocation, or interpret an output
 * byte as a decoded PRS3 symbol. The SH-2 dataflow facts are limited to:
 *   JSR @R3 -> post-call R0 -> R13 -> MOV.B R0,@(R0,R13).
 * Therefore a caller of this path needs a writable post-call base before its
 * first observed byte store can happen. */
typedef struct {
    int valid;
    size_t first_call_literal_load_offset;
    size_t first_call_offset;
    uint32_t first_call_target_address;
    size_t post_call_base_copy_offset;
    unsigned int post_call_base_source_register;
    unsigned int output_base_register;
    size_t first_output_byte_store_offset;
    unsigned int output_index_register;
    unsigned int output_value_register;
} Sh2Prs3Version1OutputBaseEvidence;

/* The literal target at 0x06028490 maps to DM.BIN+0x8490 under the separately
 * evidenced 0x06020000 candidate map. It is a bounded in-place variable-shift block: it
 * conditionally shifts selected R4-rooted 32-bit fields by R5.  This gives us
 * a concrete bit-field output primitive, but the block contains no payload
 * stream load, so it is explicitly not a PRS3 input-bit reader. */
typedef struct {
    int valid;
    uint32_t image_base_address;
    uint32_t target_address;
    size_t target_image_offset;
    size_t first_control_branch_offset;
    size_t first_field_load_offset;
    size_t first_shift_offset;
    size_t first_store_offset;
    size_t conditional_field_count;
    unsigned int conditional_field_offsets[4];
} Sh2Prs3Version1BitfieldOutputEvidence;

/* This records only the register moves which construct the selected
 * version-1 call argument, plus the adjacent version-2 entry.  The loaded
 * word is deliberately not named as a compressed-input pointer or an output
 * descriptor: its owning object and pointed-to bytes are still untraced. */
typedef struct {
    int valid;
    size_t pre_dispatch_word_load_offset;
    unsigned int pre_dispatch_base_register;
    unsigned int pre_dispatch_value_register;
    unsigned int pre_dispatch_byte_displacement;
    size_t version1_branch_delay_offset;
    unsigned int version1_branch_delay_source_register;
    unsigned int version1_branch_delay_destination_register;
    size_t version1_call_delay_offset;
    unsigned int version1_call_delay_source_register;
    unsigned int version1_call_delay_destination_register;
    size_t version2_entry_offset;
    size_t version2_literal_load_offset;
    size_t version2_indirect_call_offset;
    unsigned int version2_indirect_call_register;
    uint16_t version2_indirect_call_delay_instruction;
} Sh2Prs3Version1ArgumentEvidence;

/* This is metadata for the untyped R6+12 word only.  The first load at
 * 85246 is preserved to the caller stack while R3 remains live through the
 * version comparison and then reaches R11/R6 on the selected path. The
 * caller's R6 is incoming over this bounded setup. PRS3 magic is guarded through R4,
 * and this receipt finds no R6-to-R4 transfer or descriptor-field address
 * calculation.  It therefore must not name the word as a PRS3 header field,
 * input, output, decode state, or palette state. */
typedef struct {
    int valid;
    size_t caller_entry_offset;
    size_t first_slot_load_offset;
    size_t first_slot_stack_store_offset;
    size_t r3_live_compare_offset;
    size_t dispatch_r11_copy_offset;
    size_t selected_call_offset;
    size_t selected_call_r6_copy_offset;
    size_t prs3_magic_load_offset;
    unsigned int slot_base_register;
    unsigned int slot_value_register;
    unsigned int slot_byte_displacement;
    unsigned int stack_base_register;
    unsigned int stack_value_register;
    unsigned int stack_byte_displacement;
    int r6_is_incoming_over_setup;
    int canonical_prs3_descriptor_field_correlation;
} Sh2Prs3Version1WordLifetimeEvidence;

/* This is the first concrete byte-consumption evidence inside the selected
 * version-1 callee. The branch-controlled refill path shifts R11, tests it,
 * and, only when that test falls through, reads a byte through R12 with
 * post-increment while decrementing R14. R12 originates from R4 at callee
 * entry. These instruction facts prove a bounded R4-origin byte reader, but
 * not that R4 points at a MENU.BPK payload, which bit means literal/backref,
 * the refill bit order, the completion condition, or an output pixel format.
 * Keep all PRS3 materialization blocked until those independent facts exist. */
typedef struct {
    int valid;
    size_t control_shift_offset;
    unsigned int control_register;
    size_t control_test_offset;
    unsigned int control_test_other_register;
    size_t refill_branch_offset;
    size_t refill_branch_target;
    size_t refill_remaining_test_offset;
    unsigned int remaining_register;
    size_t refill_exhausted_target;
    size_t byte_load_offset;
    unsigned int byte_cursor_register;
    unsigned int byte_value_register;
    size_t remaining_decrement_offset;
    int remaining_decrement;
    size_t byte_extend_offset;
    unsigned int byte_extend_source_register;
    unsigned int byte_extend_destination_register;
    size_t next_control_test_offset;
} Sh2Prs3Version1StreamReadEvidence;

/* The selected callee's R11 state has an original-code sentinel protocol:
 * a PC-relative word loads R2 with 0x0100, a PC-relative long loads R9 with
 * 0x0000ff00, and refill ORs R9 into the byte-expanded R11 state before the
 * loop returns to the R11 shift/test. This proves control-state framing, not
 * a PRS3 command grammar, the meaning of either test outcome, payload ABI,
 * frame termination, or pixel output. */
typedef struct {
    int valid;
    size_t sentinel_literal_load_offset;
    size_t sentinel_literal_offset;
    uint16_t sentinel_word;
    unsigned int sentinel_register;
    size_t refill_marker_literal_load_offset;
    size_t refill_marker_literal_offset;
    uint32_t refill_marker_word;
    unsigned int refill_marker_register;
    size_t refill_marker_or_offset;
    unsigned int refill_marker_or_source_register;
    unsigned int refill_marker_or_destination_register;
    size_t loop_back_branch_offset;
    size_t loop_back_target;
} Sh2Prs3Version1ControlSentinelEvidence;

/* The zero-low-bit side branch has a separate bounded gate: it subtracts two
 * from R14, checks R14 with CMP/PZ, and takes a direct rejection edge before
 * the remaining side-path body when that check fails. This proves only an
 * R14 counter transition and branch shape, not a PRS3 token length, command,
 * payload boundary, or frame completion condition. */
typedef struct {
    int valid;
    size_t zero_bit_branch_offset;
    size_t zero_bit_branch_target;
    size_t counter_decrement_offset;
    unsigned int counter_register;
    int counter_decrement;
    size_t counter_nonnegative_test_offset;
    size_t rejection_branch_offset;
    size_t rejection_target;
} Sh2Prs3Version1ZeroBitGateEvidence;

/* After the accepted zero-bit side path has read its bounded R12 bytes, the
 * original code compares R1 with R10 and uses a delayed BF/S to repeat a
 * local block or a following BRA to return to the outer control loop. This
 * proves a local compare/repeat relation only; it is not a token length,
 * payload boundary, opcode, or completion interpretation. */
typedef struct {
    int valid;
    size_t compare_offset;
    unsigned int compare_source_register;
    unsigned int compare_destination_register;
    size_t counter_increment_offset;
    unsigned int counter_register;
    int counter_increment;
    size_t delayed_repeat_branch_offset;
    size_t delayed_repeat_target;
    size_t outer_loop_branch_offset;
    size_t outer_loop_target;
} Sh2Prs3Version1SideRepeatEvidence;

/* The available flat DM.BIN image has no encoded direct predecessor for the
 * dispatcher entry and no PC-relative literal that materializes its work-RAM
 * address. This is a negative locator receipt only: an external, indirect, or
 * fall-through route could still exist, and no R6 object construction or ABI
 * descriptor field may be inferred from these zero counts. */
typedef struct {
    int valid;
    size_t dispatcher_entry_offset;
    uint32_t dispatcher_entry_address;
    size_t direct_in_image_entry_edges;
    size_t direct_in_image_call_edges;
    size_t pc_literal_entry_address_materializations;
    int caller_callsite_found;
    int r6_object_construction_found;
} Sh2Prs3DispatcherCallerEvidence;

/* The initial transfer is the only direct entry route encoded at the start of
 * the real flat file. Saturn's documented master vector base is 0x06000000
 * (Sega System Program User's Manual, section 1/3,
 * https://www.infochunk.com/saturn/segahtml_en/prgg/sysg/prog/hon/p01_10.htm),
 * before this loaded image; DM.BIN does not contain it. This receipt therefore records the
 * bootstrap transfer, not a vector or a route to the PRS3 dispatcher. */
typedef struct {
    int valid;
    size_t bootstrap_jump_offset;
    size_t bootstrap_delay_offset;
    size_t entry_literal_offset;
    uint32_t entry_address;
    uint32_t derived_initial_image_base;
    uint32_t master_vector_base_address;
    int vector_table_precedes_initial_image;
    int dispatcher_is_in_initial_image_range;
} Sh2Prs3BootstrapEvidence;

/* This locks the mapping boundary that prevents a false in-image caller
 * claim.  The first callee literal is byte-compatible with the 0x06020000
 * candidate map, but incompatible with the bootstrap's 0x06010000 map.
 * It proves neither copying nor relocation; those require evidence outside
 * the flat DM.BIN. */
typedef struct {
    int valid;
    uint32_t callee_literal_address;
    size_t literal_candidate_offset;
    size_t bootstrap_map_offset;
    uint16_t candidate_map_word;
    uint16_t bootstrap_map_word;
    int candidate_map_matches_known_block;
    int bootstrap_map_matches_known_block;
} Sh2Prs3MapBoundaryEvidence;

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static int sh2_movl_pc_literal_target(size_t instruction_offset,
                                      uint16_t instruction,
                                      size_t *out_target);

static int sh2_work_ram_image_offset(uint32_t address, size_t image_size,
                                     size_t *out_offset) {
    uint32_t offset;

    if (!out_offset || address < NEXUS_PRS3_SH2_CALLEE_LITERAL_IMAGE_BASE) return 0;
    offset = address - NEXUS_PRS3_SH2_CALLEE_LITERAL_IMAGE_BASE;
    if ((uint64_t)offset >= (uint64_t)image_size) return 0;
    *out_offset = (size_t)offset;
    return 1;
}

static int sh2_prs3_bootstrap_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3BootstrapEvidence *out_evidence) {
    Sh2Prs3BootstrapEvidence evidence;
    size_t literal_target;

    memset(&evidence, 0, sizeof(evidence));
    evidence.bootstrap_jump_offset = 6U;
    evidence.bootstrap_delay_offset = 8U;
    if (!data || data_size < NEXUS_PRS3_BOOTSTRAP_ENTRY_OFFSET + 2U ||
        read_be16(data) != 0xd003U || read_be16(data + 2U) != 0x6f03U ||
        read_be16(data + 4U) != 0xd001U ||
        read_be16(data + evidence.bootstrap_jump_offset) != 0x402bU ||
        read_be16(data + evidence.bootstrap_delay_offset) != 0x0009U ||
        !sh2_movl_pc_literal_target(4U, read_be16(data + 4U), &literal_target) ||
        literal_target + 4U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.entry_literal_offset = literal_target;
    evidence.entry_address = read_be32(data + literal_target);
    if (evidence.entry_address < NEXUS_PRS3_BOOTSTRAP_ENTRY_OFFSET ||
        evidence.entry_address != NEXUS_PRS3_BOOTSTRAP_ENTRY_ADDRESS) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.derived_initial_image_base =
        evidence.entry_address - NEXUS_PRS3_BOOTSTRAP_ENTRY_OFFSET;
    evidence.master_vector_base_address = NEXUS_PRS3_SH2_MASTER_VECTOR_BASE;
    evidence.vector_table_precedes_initial_image =
        evidence.master_vector_base_address < evidence.derived_initial_image_base;
    evidence.dispatcher_is_in_initial_image_range =
        NEXUS_PRS3_DISPATCH_ENTRY_OFFSET < data_size;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_map_boundary_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3MapBoundaryEvidence *out_evidence) {
    Sh2Prs3MapBoundaryEvidence evidence;
    uint32_t bootstrap_offset;

    memset(&evidence, 0, sizeof(evidence));
    evidence.callee_literal_address = NEXUS_PRS3_VERSION1_FIRST_CALL_ADDRESS;
    if (!data || !sh2_work_ram_image_offset(evidence.callee_literal_address,
                                             data_size,
                                             &evidence.literal_candidate_offset) ||
        evidence.literal_candidate_offset != NEXUS_PRS3_VERSION1_FIRST_CALL_IMAGE_OFFSET ||
        evidence.callee_literal_address < NEXUS_PRS3_SH2_BOOTSTRAP_IMAGE_BASE) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    bootstrap_offset = evidence.callee_literal_address -
                       NEXUS_PRS3_SH2_BOOTSTRAP_IMAGE_BASE;
    if ((uint64_t)bootstrap_offset + 2U > (uint64_t)data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.bootstrap_map_offset = (size_t)bootstrap_offset;
    evidence.candidate_map_word = read_be16(data + evidence.literal_candidate_offset);
    evidence.bootstrap_map_word = read_be16(data + evidence.bootstrap_map_offset);
    evidence.candidate_map_matches_known_block =
        evidence.candidate_map_word == 0x8902U;
    evidence.bootstrap_map_matches_known_block =
        evidence.bootstrap_map_word == 0x8902U;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_movl_pc_literal_target(size_t instruction_offset,
                                      uint16_t instruction,
                                      size_t *out_target) {
    size_t base;
    size_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0xd000U) return 0;
    base = (instruction_offset + 4U) & ~(size_t)3U;
    displacement = (size_t)(instruction & 0x00ffU) * 4U;
    if (displacement > SIZE_MAX - base) return 0;
    *out_target = base + displacement;
    return 1;
}

/* SH-2 MOV.W @(disp,PC),Rn is 1001nnnndddddddd. The displacement is a
 * word count from PC + 4 and does not carry a DM.BIN image-base claim. */
static int sh2_movw_pc_literal_target(size_t instruction_offset,
                                      uint16_t instruction,
                                      size_t *out_target) {
    size_t base;
    size_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0x9000U) return 0;
    if (instruction_offset > SIZE_MAX - 4U) return 0;
    base = instruction_offset + 4U;
    displacement = (size_t)(instruction & 0x00ffU) * 2U;
    if (displacement > SIZE_MAX - base) return 0;
    *out_target = base + displacement;
    return 1;
}

/* SH-2 MOV.L @(disp,Rm),Rn is 0101nnnnmmmmdddd.  This exposes only the
 * instruction's register/displacement fields; it does not dereference the
 * resulting address or assign a type to the loaded word. */
static int sh2_movl_disp_register_fields(uint16_t instruction,
                                         unsigned int *out_base_register,
                                         unsigned int *out_value_register,
                                         unsigned int *out_byte_displacement) {
    if ((instruction & 0xf000U) != 0x5000U || !out_base_register ||
        !out_value_register || !out_byte_displacement) {
        return 0;
    }
    *out_value_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_base_register = (unsigned int)((instruction >> 4) & 0x0fU);
    *out_byte_displacement = (unsigned int)(instruction & 0x000fU) * 4U;
    return 1;
}

/* SH-2 MOV Rm,Rn is 0110nnnnmmmm0011. */
static int sh2_mov_register_fields(uint16_t instruction,
                                   unsigned int *out_source_register,
                                   unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x6003U || !out_source_register ||
        !out_destination_register) {
        return 0;
    }
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 MOV.L Rm,@(disp,Rn) is 0001nnnnmmmmdddd.  As with the load helper,
 * this reports only encoded register/displacement fields. */
static int sh2_movl_store_disp_register_fields(
    uint16_t instruction, unsigned int *out_base_register,
    unsigned int *out_value_register, unsigned int *out_byte_displacement) {
    if ((instruction & 0xf000U) != 0x1000U || !out_base_register ||
        !out_value_register || !out_byte_displacement) {
        return 0;
    }
    *out_base_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_value_register = (unsigned int)((instruction >> 4) & 0x0fU);
    *out_byte_displacement = (unsigned int)(instruction & 0x000fU) * 4U;
    return 1;
}

/* SH-2 conditional branch format is 1000 1001/1011/1101/1111 dddddddd
 * (BT, BF, BT/S, BF/S). The signed byte displacement is measured in words
 * from PC + 4. This helper establishes only a file-relative branch target. */
static int sh2_conditional_branch_target(size_t instruction_offset,
                                         uint16_t instruction,
                                         size_t data_size,
                                         size_t *out_target) {
    int displacement;
    size_t base;
    long target;
    uint16_t opcode = instruction & 0xff00U;

    if (!out_target || (opcode != 0x8900U && opcode != 0x8b00U &&
                        opcode != 0x8d00U && opcode != 0x8f00U)) return 0;
    if (instruction_offset > SIZE_MAX - 4U) return 0;
    base = instruction_offset + 4U;
    displacement = (int)(int8_t)(instruction & 0x00ffU) * 2;
    target = (long)base + (long)displacement;
    if (target < 0L || (size_t)target + 2U > data_size) return 0;
    *out_target = (size_t)target;
    return 1;
}

/* SH-2 BSR is 1011dddddddddddd. Its signed 12-bit displacement is measured
 * in words from PC + 4. The target is recorded as a call edge only; this
 * probe does not infer a callee name or its register contract. */
static int sh2_bsr_target(size_t instruction_offset, uint16_t instruction,
                          size_t data_size, size_t *out_target) {
    int displacement;
    size_t base;
    long target;

    if (!out_target || (instruction & 0xf000U) != 0xb000U ||
        instruction_offset > SIZE_MAX - 4U) return 0;
    base = instruction_offset + 4U;
    displacement = (int)(instruction & 0x0fffU);
    if ((displacement & 0x0800) != 0) displacement -= 0x1000;
    target = (long)base + (long)displacement * 2L;
    if (target < 0L || (size_t)target + 2U > data_size) return 0;
    *out_target = (size_t)target;
    return 1;
}

/* SH-2 BRA is 1010dddddddddddd.  Like BSR, its signed 12-bit displacement
 * is measured in words from PC + 4.  We use it only for a direct local edge;
 * no execution or data interpretation is implied. */
static int sh2_bra_target(size_t instruction_offset, uint16_t instruction,
                          size_t data_size, size_t *out_target) {
    int displacement;
    size_t base;
    long target;

    if (!out_target || (instruction & 0xf000U) != 0xa000U ||
        instruction_offset > SIZE_MAX - 4U) return 0;
    base = instruction_offset + 4U;
    displacement = (int)(instruction & 0x0fffU);
    if ((displacement & 0x0800) != 0) displacement -= 0x1000;
    target = (long)base + (long)displacement * 2L;
    if (target < 0L || (size_t)target + 2U > data_size) return 0;
    *out_target = (size_t)target;
    return 1;
}

static int sh2_direct_control_flow_target(size_t instruction_offset,
                                          uint16_t instruction,
                                          size_t data_size,
                                          size_t *out_target,
                                          int *out_is_call) {
    if (!out_target || !out_is_call) return 0;
    if ((instruction & 0xf000U) == 0xb000U) {
        *out_is_call = 1;
        return sh2_bsr_target(instruction_offset, instruction, data_size,
                              out_target);
    }
    *out_is_call = 0;
    if ((instruction & 0xf000U) == 0xa000U) {
        return sh2_bra_target(instruction_offset, instruction, data_size,
                              out_target);
    }
    return sh2_conditional_branch_target(instruction_offset, instruction,
                                         data_size, out_target);
}

static int sh2_prs3_dispatcher_caller_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3DispatcherCallerEvidence *out_evidence) {
    Sh2Prs3DispatcherCallerEvidence evidence;
    size_t offset;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_DISPATCH_ENTRY_OFFSET + 2U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.dispatcher_entry_offset = NEXUS_PRS3_DISPATCH_ENTRY_OFFSET;
    evidence.dispatcher_entry_address = NEXUS_PRS3_SH2_CALLEE_LITERAL_IMAGE_BASE +
                                        (uint32_t)evidence.dispatcher_entry_offset;
    for (offset = 0U; offset + 2U <= data_size; offset += 2U) {
        uint16_t instruction = read_be16(data + offset);
        size_t target;
        int is_call;

        if (sh2_direct_control_flow_target(offset, instruction, data_size,
                                           &target, &is_call) &&
            target == evidence.dispatcher_entry_offset) {
            ++evidence.direct_in_image_entry_edges;
            if (is_call) ++evidence.direct_in_image_call_edges;
        }
        if (sh2_movl_pc_literal_target(offset, instruction, &target) &&
            target + 4U <= data_size &&
            read_be32(data + target) == evidence.dispatcher_entry_address) {
            ++evidence.pc_literal_entry_address_materializations;
        }
    }
    evidence.caller_callsite_found =
        evidence.direct_in_image_entry_edges != 0U ||
        evidence.pc_literal_entry_address_materializations != 0U;
    /* No predecessor establishes the incoming R6 object, and zero encoded
     * predecessors establish no replacement construction site. */
    evidence.r6_object_construction_found = 0;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_callee_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1CalleeEvidence *out_evidence) {
    Sh2Prs3Version1CalleeEvidence evidence;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 194U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.entry_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;
    /* Entry saves register state, copies R4/R5, then makes its first indirect
     * call. The TST/BF immediately after it conditionally bypasses the second
     * indirect call at 85412; its non-delayed branch bypasses that call. */
    if (read_be16(data + evidence.entry_offset) != 0x2fe6U ||
        read_be16(data + evidence.entry_offset + 2U) != 0x2fd6U ||
        read_be16(data + evidence.entry_offset + 4U) != 0x2fc6U ||
        read_be16(data + evidence.entry_offset + 6U) != 0x6c43U ||
        read_be16(data + evidence.entry_offset + 8U) != 0xd341U ||
        read_be16(data + evidence.entry_offset + 24U) != 0x430bU ||
        read_be16(data + evidence.entry_offset + 30U) != 0x2dd8U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.conditional_call_test_offset = evidence.entry_offset + 32U;
    if (read_be16(data + evidence.conditional_call_test_offset) != 0x8b03U ||
        read_be16(data + evidence.entry_offset + 38U) != 0x430bU ||
        !sh2_conditional_branch_target(
            evidence.conditional_call_test_offset,
            read_be16(data + evidence.conditional_call_test_offset), data_size,
            &evidence.conditional_call_bypass_target) ||
        evidence.conditional_call_bypass_target != evidence.entry_offset + 42U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.first_decision_offset = evidence.entry_offset + 58U;
    evidence.early_exit_decision_offset = evidence.entry_offset + 62U;
    evidence.second_decision_offset = evidence.entry_offset + 76U;
    evidence.loop_decision_offset = evidence.entry_offset + 80U;
    evidence.failure_decision_offset = evidence.entry_offset + 104U;
    evidence.delayed_backward_branch_offset = evidence.entry_offset + 158U;
    evidence.loop_branch_offset = evidence.entry_offset + 162U;
    if (read_be16(data + evidence.first_decision_offset) != 0x8b05U ||
        read_be16(data + evidence.early_exit_decision_offset) != 0x8932U ||
        read_be16(data + evidence.second_decision_offset) != 0x890aU ||
        read_be16(data + evidence.loop_decision_offset) != 0x8929U ||
        read_be16(data + evidence.failure_decision_offset) != 0x8b1dU ||
        read_be16(data + evidence.delayed_backward_branch_offset) != 0x8ff3U ||
        read_be16(data + evidence.loop_branch_offset) != 0xafc7U ||
        !sh2_conditional_branch_target(
            evidence.first_decision_offset,
            read_be16(data + evidence.first_decision_offset), data_size,
            &evidence.first_decision_target) ||
        !sh2_conditional_branch_target(
            evidence.early_exit_decision_offset,
            read_be16(data + evidence.early_exit_decision_offset), data_size,
            &evidence.early_exit_target) ||
        !sh2_conditional_branch_target(
            evidence.second_decision_offset,
            read_be16(data + evidence.second_decision_offset), data_size,
            &evidence.second_decision_target) ||
        !sh2_conditional_branch_target(
            evidence.loop_decision_offset,
            read_be16(data + evidence.loop_decision_offset), data_size,
            &evidence.loop_decision_target) ||
        !sh2_conditional_branch_target(
            evidence.failure_decision_offset,
            read_be16(data + evidence.failure_decision_offset), data_size,
            &evidence.failure_target) ||
        !sh2_conditional_branch_target(
            evidence.delayed_backward_branch_offset,
            read_be16(data + evidence.delayed_backward_branch_offset), data_size,
            &evidence.delayed_backward_branch_target) ||
        !sh2_bra_target(evidence.loop_branch_offset,
                        read_be16(data + evidence.loop_branch_offset), data_size,
                        &evidence.loop_target) ||
        evidence.first_decision_target != evidence.entry_offset + 72U ||
        evidence.early_exit_target != evidence.entry_offset + 166U ||
        evidence.second_decision_target != evidence.entry_offset + 100U ||
        evidence.loop_decision_target != evidence.entry_offset + 166U ||
        evidence.failure_target != evidence.entry_offset + 166U ||
        evidence.delayed_backward_branch_target != evidence.entry_offset + 136U ||
        evidence.loop_target != evidence.entry_offset + 52U ||
        read_be16(data + evidence.entry_offset + 188U) != 0x000bU) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.return_offset = evidence.entry_offset + 188U;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_output_base_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1OutputBaseEvidence *out_evidence) {
    Sh2Prs3Version1OutputBaseEvidence evidence;
    size_t literal_target;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 90U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.first_call_literal_load_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 8U;
    evidence.first_call_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 24U;
    evidence.post_call_base_copy_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 28U;
    evidence.first_output_byte_store_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 88U;
    if (read_be16(data + evidence.first_call_literal_load_offset) != 0xd341U ||
        !sh2_movl_pc_literal_target(evidence.first_call_literal_load_offset,
                                    read_be16(data + evidence.first_call_literal_load_offset),
                                    &literal_target) ||
        literal_target + 4U > data_size ||
        read_be16(data + evidence.first_call_offset) != 0x430bU ||
        read_be16(data + evidence.post_call_base_copy_offset) != 0x6d03U ||
        read_be16(data + evidence.first_output_byte_store_offset) != 0x0d24U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.first_call_target_address = read_be32(data + literal_target);
    evidence.post_call_base_source_register = 0U;
    evidence.output_base_register = 13U;
    evidence.output_index_register = 0U;
    evidence.output_value_register = 0U;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_bitfield_output_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1BitfieldOutputEvidence *out_evidence) {
    static const uint16_t conditional_fields[][3] = {
        { 0x5044U, 0x5e44U, 0x14e4U }, /* R4+16 */
        { 0x5045U, 0x5e45U, 0x14e5U }, /* R4+20 */
        { 0x5047U, 0x5e47U, 0x14e7U }, /* R4+28 */
        { 0x5049U, 0x5e49U, 0x14e9U }  /* R4+36 */
    };
    static const unsigned int field_offsets[] = { 16U, 20U, 28U, 36U };
    Sh2Prs3Version1BitfieldOutputEvidence evidence;
    size_t target_offset;
    size_t i;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || !sh2_work_ram_image_offset(
                     NEXUS_PRS3_VERSION1_FIRST_CALL_ADDRESS, data_size,
                     &target_offset) ||
        target_offset != NEXUS_PRS3_VERSION1_FIRST_CALL_IMAGE_OFFSET ||
        target_offset + 60U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.image_base_address = NEXUS_PRS3_SH2_CALLEE_LITERAL_IMAGE_BASE;
    evidence.target_address = NEXUS_PRS3_VERSION1_FIRST_CALL_ADDRESS;
    evidence.target_image_offset = target_offset;
    evidence.first_control_branch_offset = target_offset;
    evidence.first_field_load_offset = target_offset + 2U;
    evidence.first_shift_offset = target_offset + 4U;
    evidence.first_store_offset = target_offset + 6U;
    if (read_be16(data + evidence.first_control_branch_offset) != 0x8902U ||
        read_be16(data + evidence.first_field_load_offset) != 0x5e43U ||
        read_be16(data + evidence.first_shift_offset) != 0x3e5cU ||
        read_be16(data + evidence.first_store_offset) != 0x14e3U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    for (i = 0U; i < sizeof(field_offsets) / sizeof(field_offsets[0]); ++i) {
        size_t offset = target_offset + 8U + i * 12U;
        if (read_be16(data + offset) != conditional_fields[i][0] ||
            read_be16(data + offset + 2U) != 0x2008U ||
            read_be16(data + offset + 4U) != 0x8902U ||
            read_be16(data + offset + 6U) != conditional_fields[i][1] ||
            read_be16(data + offset + 8U) != 0x3e5cU ||
            read_be16(data + offset + 10U) != conditional_fields[i][2]) {
            if (out_evidence) *out_evidence = evidence;
            return 0;
        }
        evidence.conditional_field_offsets[i] = field_offsets[i];
    }
    evidence.conditional_field_count =
        sizeof(field_offsets) / sizeof(field_offsets[0]);
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version_dispatch_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3VersionDispatchEvidence *out_evidence) {
    static const size_t compare_offsets[] = { 0U, 6U, 10U, 14U, 18U };
    Sh2Prs3VersionDispatchEvidence evidence;
    size_t i;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 20U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    for (i = 0U; i < sizeof(compare_offsets) / sizeof(compare_offsets[0]); ++i) {
        size_t compare_offset = NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET +
                                compare_offsets[i];
        size_t branch_offset = compare_offset + 2U;
        uint16_t branch_instruction;

        if (read_be16(data + compare_offset) != (uint16_t)(0x8800U + i + 1U) ||
            branch_offset + 2U > data_size) {
            if (out_evidence) *out_evidence = evidence;
            return 0;
        }
        branch_instruction = read_be16(data + branch_offset);
        /* A matching version must take the following true branch. */
        if (((branch_instruction & 0xff00U) != 0x8900U &&
             (branch_instruction & 0xff00U) != 0x8d00U) ||
            !sh2_conditional_branch_target(branch_offset, branch_instruction,
                                           data_size, &evidence.branch_targets[i])) {
            if (out_evidence) *out_evidence = evidence;
            return 0;
        }
        evidence.compare_offsets[i] = compare_offset;
        evidence.branch_offsets[i] = branch_offset;
        evidence.branch_instructions[i] = branch_instruction;
    }
    evidence.version1_entry_offset = evidence.branch_targets[0];
    evidence.version1_call_offset = evidence.version1_entry_offset;
    if (evidence.version1_call_offset + 4U > data_size ||
        !sh2_bsr_target(evidence.version1_call_offset,
                        read_be16(data + evidence.version1_call_offset), data_size,
                        &evidence.version1_call_target)) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.version1_delay_instruction =
        read_be16(data + evidence.version1_call_offset + 2U);
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_argument_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1ArgumentEvidence *out_evidence) {
    Sh2Prs3Version1ArgumentEvidence evidence;
    size_t version2_branch_offset;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 40U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    /* The selected version-1 branch is delayed: its delay slot preserves the
     * word loaded from the caller-owned R6+12 slot in R11.  The BSR at its
     * target then moves R11 into the callee argument register R6. */
    evidence.pre_dispatch_word_load_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET - 6U;
    evidence.version1_branch_delay_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 4U;
    evidence.version1_call_delay_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 28U;
    version2_branch_offset = NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 8U;
    evidence.version2_entry_offset = NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 34U;
    evidence.version2_literal_load_offset = evidence.version2_entry_offset;
    evidence.version2_indirect_call_offset = evidence.version2_entry_offset + 2U;

    if (read_be16(data + NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 2U) !=
            0x8d0aU ||
        read_be16(data + evidence.version1_branch_delay_offset) != 0x6b33U ||
        read_be16(data + evidence.version1_call_delay_offset) != 0x66b3U ||
        read_be16(data + evidence.version2_literal_load_offset) != 0x951eU ||
        read_be16(data + evidence.version2_indirect_call_offset) != 0x4d0bU ||
        !sh2_movl_disp_register_fields(
            read_be16(data + evidence.pre_dispatch_word_load_offset),
            &evidence.pre_dispatch_base_register,
            &evidence.pre_dispatch_value_register,
            &evidence.pre_dispatch_byte_displacement) ||
        !sh2_mov_register_fields(
            read_be16(data + evidence.version1_branch_delay_offset),
            &evidence.version1_branch_delay_source_register,
            &evidence.version1_branch_delay_destination_register) ||
        !sh2_mov_register_fields(
            read_be16(data + evidence.version1_call_delay_offset),
            &evidence.version1_call_delay_source_register,
            &evidence.version1_call_delay_destination_register) ||
        !sh2_conditional_branch_target(
            version2_branch_offset, read_be16(data + version2_branch_offset), data_size,
            &evidence.version2_entry_offset) ||
        evidence.version2_entry_offset != evidence.version2_literal_load_offset) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.version2_indirect_call_register =
        (unsigned int)((read_be16(data + evidence.version2_indirect_call_offset) >> 8) & 0x0fU);
    evidence.version2_indirect_call_delay_instruction =
        read_be16(data + evidence.version2_indirect_call_offset + 2U);
    if (evidence.pre_dispatch_base_register != 6U ||
        evidence.pre_dispatch_value_register != 3U ||
        evidence.pre_dispatch_byte_displacement != 12U ||
        evidence.version1_branch_delay_source_register != 3U ||
        evidence.version1_branch_delay_destination_register != 11U ||
        evidence.version1_call_delay_source_register != 11U ||
        evidence.version1_call_delay_destination_register != 6U ||
        evidence.version2_indirect_call_register != 13U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_word_lifetime_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1WordLifetimeEvidence *out_evidence) {
    Sh2Prs3Version1WordLifetimeEvidence evidence;
    unsigned int reload_source = 0U, reload_destination = 0U;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 30U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.caller_entry_offset = NEXUS_PRS3_DISPATCH_MAGIC_LOAD_OFFSET - 4U;
    evidence.prs3_magic_load_offset = NEXUS_PRS3_MAGIC_PREDICATE_OFFSET + 4U;
    evidence.first_slot_load_offset =
        NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET - 6U;
    evidence.first_slot_stack_store_offset = evidence.first_slot_load_offset + 2U;
    evidence.r3_live_compare_offset = NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET;
    evidence.dispatch_r11_copy_offset = evidence.r3_live_compare_offset + 4U;
    evidence.selected_call_offset = evidence.r3_live_compare_offset + 26U;
    evidence.selected_call_r6_copy_offset = evidence.selected_call_offset + 2U;

    /* The exact caller window establishes the bounded lifetime receipt. It
     * has no R6-destination instruction before the selected call delay slot;
     * R6 is consequently only an incoming base for the recorded slot load. */
    if (read_be16(data + evidence.caller_entry_offset) != 0xd72aU ||
        read_be16(data + evidence.caller_entry_offset + 2U) != 0xe300U ||
        read_be16(data + evidence.prs3_magic_load_offset) != 0x6342U ||
        read_be16(data + evidence.first_slot_load_offset) != 0x5363U ||
        read_be16(data + evidence.first_slot_stack_store_offset) != 0x1f31U ||
        read_be16(data + evidence.r3_live_compare_offset) != 0x8801U ||
        read_be16(data + evidence.dispatch_r11_copy_offset) != 0x6b33U ||
        read_be16(data + evidence.selected_call_offset) != 0xb02fU ||
        read_be16(data + evidence.selected_call_r6_copy_offset) != 0x66b3U ||
        !sh2_movl_disp_register_fields(
            read_be16(data + evidence.first_slot_load_offset),
            &evidence.slot_base_register, &evidence.slot_value_register,
            &evidence.slot_byte_displacement) ||
        !sh2_movl_store_disp_register_fields(
            read_be16(data + evidence.first_slot_stack_store_offset),
            &evidence.stack_base_register, &evidence.stack_value_register,
            &evidence.stack_byte_displacement) ||
        !sh2_mov_register_fields(read_be16(data + evidence.dispatch_r11_copy_offset),
                                 &reload_source, &reload_destination) ||
        evidence.slot_base_register != 6U || evidence.slot_value_register != 3U ||
        evidence.slot_byte_displacement != 12U ||
        evidence.stack_base_register != 15U || evidence.stack_value_register != 3U ||
        evidence.stack_byte_displacement != 4U ||
        reload_source != 3U || reload_destination != 11U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.r6_is_incoming_over_setup = 1;
    /* The PRS3 guard's source is @R4. No encoded R6-to-R4 bridge exists in
     * this receipt window, so the canonical 12-byte PRS3 header ABI cannot
     * type the R6+12 word. */
    evidence.canonical_prs3_descriptor_field_correlation = 0;
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

/* SH-2 MOV.B @Rm+,Rn is 0110nnnnmmmm0100. */
static int sh2_movb_postinc_register_fields(uint16_t instruction,
                                            unsigned int *out_base_register,
                                            unsigned int *out_value_register) {
    if ((instruction & 0xf00fU) != 0x6004U || !out_base_register ||
        !out_value_register) {
        return 0;
    }
    *out_value_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_base_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 EXTU.B Rm,Rn is 0110nnnnmmmm1100. */
static int sh2_extub_register_fields(uint16_t instruction,
                                     unsigned int *out_source_register,
                                     unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x600cU || !out_source_register ||
        !out_destination_register) {
        return 0;
    }
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 ADD #imm,Rn is 0111nnnniiiiiiii. */
static int sh2_add_immediate_fields(uint16_t instruction,
                                    unsigned int *out_register,
                                    int *out_immediate) {
    if ((instruction & 0xf000U) != 0x7000U || !out_register ||
        !out_immediate) {
        return 0;
    }
    *out_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_immediate = (int)(int8_t)(instruction & 0x00ffU);
    return 1;
}

/* SH-2 OR Rm,Rn is 0010nnnnmmmm1011. */
static int sh2_or_register_fields(uint16_t instruction,
                                  unsigned int *out_source_register,
                                  unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x200bU || !out_source_register ||
        !out_destination_register) {
        return 0;
    }
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

/* SH-2 CMP/EQ Rm,Rn is 0010nnnnmmmm0000. */
static int sh2_cmp_eq_register_fields(uint16_t instruction,
                                      unsigned int *out_source_register,
                                      unsigned int *out_destination_register) {
    if ((instruction & 0xf00fU) != 0x2000U || !out_source_register ||
        !out_destination_register) {
        return 0;
    }
    *out_destination_register = (unsigned int)((instruction >> 8) & 0x0fU);
    *out_source_register = (unsigned int)((instruction >> 4) & 0x0fU);
    return 1;
}

static int sh2_prs3_version1_stream_read_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1StreamReadEvidence *out_evidence) {
    Sh2Prs3Version1StreamReadEvidence evidence;
    unsigned int test_left = 0U, test_right = 0U;
    unsigned int decrement_register = 0U;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 76U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.control_shift_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 54U;
    evidence.control_test_offset = evidence.control_shift_offset + 2U;
    evidence.refill_branch_offset = evidence.control_shift_offset + 4U;
    evidence.refill_remaining_test_offset = evidence.control_shift_offset + 6U;
    evidence.byte_load_offset = evidence.control_shift_offset + 10U;
    evidence.remaining_decrement_offset = evidence.byte_load_offset + 2U;
    evidence.byte_extend_offset = evidence.byte_load_offset + 4U;
    evidence.next_control_test_offset = evidence.byte_load_offset + 10U;

    if (read_be16(data + evidence.control_shift_offset) != 0x4b21U ||
        read_be16(data + evidence.control_test_offset) != 0x22b8U ||
        read_be16(data + evidence.refill_remaining_test_offset) != 0x2ee8U ||
        !sh2_conditional_branch_target(
            evidence.refill_branch_offset,
            read_be16(data + evidence.refill_branch_offset), data_size,
            &evidence.refill_branch_target) ||
        !sh2_conditional_branch_target(
            evidence.refill_remaining_test_offset + 2U,
            read_be16(data + evidence.refill_remaining_test_offset + 2U), data_size,
            &evidence.refill_exhausted_target) ||
        evidence.refill_branch_target != evidence.control_shift_offset + 18U ||
        evidence.refill_exhausted_target !=
            NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 166U ||
        !sh2_movb_postinc_register_fields(
            read_be16(data + evidence.byte_load_offset),
            &evidence.byte_cursor_register, &evidence.byte_value_register) ||
        !sh2_add_immediate_fields(
            read_be16(data + evidence.remaining_decrement_offset),
            &decrement_register, &evidence.remaining_decrement) ||
        !sh2_extub_register_fields(
            read_be16(data + evidence.byte_extend_offset),
            &evidence.byte_extend_source_register,
            &evidence.byte_extend_destination_register) ||
        !sh2_mov_register_fields(read_be16(data + evidence.control_shift_offset - 48U),
                                 &test_left, &test_right)) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    /* 4b21 is SHAR R11. The preceding callee prologue copies R4 to R12. */
    evidence.control_register = 11U;
    evidence.control_test_other_register = 2U;
    evidence.remaining_register = decrement_register;
    if (test_left != 4U || test_right != 12U ||
        evidence.byte_cursor_register != 12U ||
        evidence.byte_value_register != 11U ||
        evidence.remaining_register != 14U || evidence.remaining_decrement != -1 ||
        evidence.byte_extend_source_register != 11U ||
        evidence.byte_extend_destination_register != 11U ||
        read_be16(data + evidence.next_control_test_offset) != 0x23b8U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_control_sentinel_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1ControlSentinelEvidence *out_evidence) {
    Sh2Prs3Version1ControlSentinelEvidence evidence;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 164U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.refill_marker_literal_load_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 48U;
    evidence.sentinel_literal_load_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 52U;
    evidence.refill_marker_or_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 70U;
    evidence.loop_back_branch_offset =
        NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 162U;

    if (read_be16(data + evidence.refill_marker_literal_load_offset) != 0xd93aU ||
        !sh2_movl_pc_literal_target(
            evidence.refill_marker_literal_load_offset,
            read_be16(data + evidence.refill_marker_literal_load_offset),
            &evidence.refill_marker_literal_offset) ||
        evidence.refill_marker_literal_offset + 4U > data_size ||
        read_be16(data + evidence.sentinel_literal_load_offset) != 0x926aU ||
        !sh2_movw_pc_literal_target(
            evidence.sentinel_literal_load_offset,
            read_be16(data + evidence.sentinel_literal_load_offset),
            &evidence.sentinel_literal_offset) ||
        evidence.sentinel_literal_offset + 2U > data_size ||
        !sh2_or_register_fields(
            read_be16(data + evidence.refill_marker_or_offset),
            &evidence.refill_marker_or_source_register,
            &evidence.refill_marker_or_destination_register) ||
        !sh2_bra_target(evidence.loop_back_branch_offset,
                        read_be16(data + evidence.loop_back_branch_offset),
                        data_size, &evidence.loop_back_target)) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }

    evidence.refill_marker_word =
        read_be32(data + evidence.refill_marker_literal_offset);
    evidence.sentinel_word = read_be16(data + evidence.sentinel_literal_offset);
    evidence.refill_marker_register = 9U;
    evidence.sentinel_register = 2U;
    if (evidence.refill_marker_word != 0x0000ff00U ||
        evidence.sentinel_word != 0x0100U ||
        evidence.refill_marker_or_source_register != evidence.refill_marker_register ||
        evidence.refill_marker_or_destination_register != 11U ||
        evidence.loop_back_target != NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 52U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_zero_bit_gate_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1ZeroBitGateEvidence *out_evidence) {
    Sh2Prs3Version1ZeroBitGateEvidence evidence;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 168U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.zero_bit_branch_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 76U;
    evidence.counter_decrement_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 100U;
    evidence.counter_nonnegative_test_offset = evidence.counter_decrement_offset + 2U;
    evidence.rejection_branch_offset = evidence.counter_decrement_offset + 4U;
    if (!sh2_conditional_branch_target(
            evidence.zero_bit_branch_offset,
            read_be16(data + evidence.zero_bit_branch_offset), data_size,
            &evidence.zero_bit_branch_target) ||
        !sh2_add_immediate_fields(
            read_be16(data + evidence.counter_decrement_offset),
            &evidence.counter_register, &evidence.counter_decrement) ||
        read_be16(data + evidence.counter_nonnegative_test_offset) != 0x4e11U ||
        !sh2_conditional_branch_target(
            evidence.rejection_branch_offset,
            read_be16(data + evidence.rejection_branch_offset), data_size,
            &evidence.rejection_target) ||
        evidence.zero_bit_branch_target != evidence.counter_decrement_offset ||
        evidence.counter_register != 14U || evidence.counter_decrement != -2 ||
        evidence.rejection_target != NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 166U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_version1_side_repeat_evidence(
    const uint8_t *data, size_t data_size,
    Sh2Prs3Version1SideRepeatEvidence *out_evidence) {
    Sh2Prs3Version1SideRepeatEvidence evidence;

    memset(&evidence, 0, sizeof(evidence));
    if (!data || NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 164U > data_size) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.compare_offset = NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 154U;
    evidence.counter_increment_offset = evidence.compare_offset + 2U;
    evidence.delayed_repeat_branch_offset = evidence.compare_offset + 4U;
    evidence.outer_loop_branch_offset = evidence.compare_offset + 8U;
    if (!sh2_cmp_eq_register_fields(
            read_be16(data + evidence.compare_offset),
            &evidence.compare_source_register,
            &evidence.compare_destination_register) ||
        !sh2_add_immediate_fields(
            read_be16(data + evidence.counter_increment_offset),
            &evidence.counter_register, &evidence.counter_increment) ||
        !sh2_conditional_branch_target(
            evidence.delayed_repeat_branch_offset,
            read_be16(data + evidence.delayed_repeat_branch_offset), data_size,
            &evidence.delayed_repeat_target) ||
        !sh2_bra_target(evidence.outer_loop_branch_offset,
                        read_be16(data + evidence.outer_loop_branch_offset), data_size,
                        &evidence.outer_loop_target) ||
        evidence.compare_source_register != 1U ||
        evidence.compare_destination_register != 10U ||
        evidence.counter_register != 10U || evidence.counter_increment != 1 ||
        evidence.delayed_repeat_target != NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 136U ||
        evidence.outer_loop_target != NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 52U) {
        if (out_evidence) *out_evidence = evidence;
        return 0;
    }
    evidence.valid = 1;
    if (out_evidence) *out_evidence = evidence;
    return 1;
}

static int sh2_prs3_header_guard_evidence(const uint8_t *data,
                                          size_t data_size) {
    static const uint16_t predicate[] = {
        0xd131U, /* MOV.L @(49,PC),R1 -> 0x50525333 */
        0x7ffcU, /* ADD #-4,R15 */
        0x6342U, /* MOV.L @R4,R3 */
        0x2f32U, /* MOV.L R3,@R15 */
        0x6233U, /* MOV R3,R2 */
        0x3210U, /* CMP/EQ R1,R2 */
        0x8902U, /* BT equal-return */
        0xe000U, /* unequal -> return 0 */
        0x000bU, /* RTS */
        0x7f04U, /* delay: restore R15 */
        0xe001U, /* equal -> return 1 */
        0x000bU,
        0x7f04U
    };
    size_t literal_target;
    static const size_t version_compare_offsets[] = { 0U, 6U, 10U, 14U, 18U };
    size_t i;

    if (!data || NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET + 20U > data_size ||
        !sh2_movl_pc_literal_target(NEXUS_PRS3_MAGIC_PREDICATE_OFFSET,
                                    read_be16(data + NEXUS_PRS3_MAGIC_PREDICATE_OFFSET),
                                    &literal_target) ||
        literal_target != NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET) {
        return 0;
    }
    for (i = 0U; i < sizeof(predicate) / sizeof(predicate[0]); ++i) {
        if (read_be16(data + NEXUS_PRS3_MAGIC_PREDICATE_OFFSET + i * 2U) !=
            predicate[i]) {
            return 0;
        }
    }
    if (!sh2_movl_pc_literal_target(NEXUS_PRS3_DISPATCH_MAGIC_LOAD_OFFSET,
                                    read_be16(data + NEXUS_PRS3_DISPATCH_MAGIC_LOAD_OFFSET),
                                    &literal_target) ||
        literal_target != NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET ||
        read_be16(data + NEXUS_PRS3_DISPATCH_MAGIC_COMPARE_OFFSET) != 0x3310U ||
        read_be16(data + NEXUS_PRS3_DISPATCH_MAGIC_BRANCH_OFFSET) != 0x8d04U) {
        return 0;
    }
    for (i = 1U; i <= 5U; ++i) {
        size_t offset = NEXUS_PRS3_DISPATCH_VERSION_COMPARE_OFFSET +
                        version_compare_offsets[i - 1U];
        if (read_be16(data + offset) != (uint16_t)(0x8800U | i)) return 0;
    }
    return 1;
}

static int sh2_has_nearby_jsr_same_register(const uint8_t *data,
                                             size_t data_size,
                                             size_t instruction_offset,
                                             unsigned int reg) {
    size_t next = instruction_offset + 2U;
    size_t limit = next + 24U; /* Twelve following SH-2 instructions. */

    if (limit < next || limit > data_size) limit = data_size;
    while (next + 2U <= limit) {
        uint16_t instruction = read_be16(data + next);
        if ((instruction & 0xf0ffU) == 0x400bU &&
            ((instruction >> 8) & 0x0fU) == reg) {
            return 1;
        }
        next += 2U;
    }
    return 0;
}

static Sh2FlatLiteralEvidence sh2_flat_literal_evidence(
    const uint8_t *data, size_t data_size, size_t target_offset) {
    Sh2FlatLiteralEvidence evidence = {0};
    size_t offset;

    if (!data || target_offset + 4U > data_size) return evidence;
    for (offset = 0U; offset + 2U <= data_size; offset += 2U) {
        uint16_t instruction = read_be16(data + offset);
        size_t target;
        unsigned int reg;

        if (!sh2_movl_pc_literal_target(offset, instruction, &target) ||
            target != target_offset) {
            continue;
        }
        ++evidence.literal_references;
        if (evidence.recorded_reference_count <
            sizeof(evidence.reference_offsets) / sizeof(evidence.reference_offsets[0])) {
            evidence.reference_offsets[evidence.recorded_reference_count++] = offset;
        }
        reg = (unsigned int)((instruction >> 8) & 0x0fU);
        if (sh2_has_nearby_jsr_same_register(data, data_size, offset, reg)) {
            ++evidence.nearby_indirect_call_candidates;
        }
    }
    return evidence;
}

static void test_sh2_flat_literal_scanner(void) {
    uint8_t fixture[64];
    Sh2FlatLiteralEvidence evidence;

    memset(fixture, 0, sizeof(fixture));
    /* MOV.L @(3,PC),R1 at 0 targets 16; JSR @R1 follows at 2. */
    fixture[0] = 0xd1U;
    fixture[1] = 0x03U;
    fixture[2] = 0x41U;
    fixture[3] = 0x0bU;
    memcpy(fixture + 16U, "PRS3", 4U);
    evidence = sh2_flat_literal_evidence(fixture, sizeof(fixture), 16U);
    check(evidence.literal_references == 1U,
          "SH-2 flat scanner finds the synthetic PC-relative literal reference");
    check(evidence.nearby_indirect_call_candidates == 1U,
          "SH-2 flat scanner identifies only a synthetic nearby JSR candidate");
    check(evidence.recorded_reference_count == 1U &&
              evidence.reference_offsets[0] == 0U,
          "SH-2 flat scanner records the synthetic instruction offset");
}

static void test_sh2_conditional_branch_decoder(void) {
    uint8_t fixture[64];
    size_t target = 0U;

    memset(fixture, 0, sizeof(fixture));
    /* BT/S +3 at 8 branches from PC+4 to file offset 18. */
    fixture[8] = 0x8dU;
    fixture[9] = 0x03U;
    check(sh2_conditional_branch_target(8U, read_be16(fixture + 8U),
                                        sizeof(fixture), &target) && target == 18U,
          "SH-2 conditional branch decoder preserves BT/S word displacement");
    /* BF -4 at 12 branches from PC+4 back to file offset 8. */
    fixture[12] = 0x8bU;
    fixture[13] = 0xfcU;
    check(sh2_conditional_branch_target(12U, read_be16(fixture + 12U),
                                        sizeof(fixture), &target) && target == 8U,
          "SH-2 conditional branch decoder preserves signed backward displacement");
    check(!sh2_conditional_branch_target(8U, 0x8a00U, sizeof(fixture), &target),
          "SH-2 conditional branch decoder rejects non-conditional opcodes");
}

static void test_sh2_bsr_decoder(void) {
    uint8_t fixture[64];
    size_t target = 0U;

    memset(fixture, 0, sizeof(fixture));
    /* BSR +3 at 8 branches from PC+4 to file offset 18. */
    fixture[8] = 0xb0U;
    fixture[9] = 0x03U;
    check(sh2_bsr_target(8U, read_be16(fixture + 8U), sizeof(fixture), &target) &&
              target == 18U,
          "SH-2 BSR decoder preserves forward signed-12 displacement");
    /* BSR -4 at 12 branches from PC+4 back to file offset 8. */
    fixture[12] = 0xbfU;
    fixture[13] = 0xfcU;
    check(sh2_bsr_target(12U, read_be16(fixture + 12U), sizeof(fixture), &target) &&
              target == 8U,
          "SH-2 BSR decoder preserves backward signed-12 displacement");
    check(!sh2_bsr_target(8U, 0xa003U, sizeof(fixture), &target),
          "SH-2 BSR decoder rejects non-call opcodes");
}

static void test_sh2_bra_decoder(void) {
    uint8_t fixture[64];
    size_t target = 0U;

    memset(fixture, 0, sizeof(fixture));
    /* BRA +3 at 8 branches from PC+4 to file offset 18. */
    fixture[8] = 0xa0U;
    fixture[9] = 0x03U;
    check(sh2_bra_target(8U, read_be16(fixture + 8U), sizeof(fixture), &target) &&
              target == 18U,
          "SH-2 BRA decoder preserves forward signed-12 displacement");
    /* BRA -4 at 12 branches from PC+4 back to file offset 8. */
    fixture[12] = 0xafU;
    fixture[13] = 0xfcU;
    check(sh2_bra_target(12U, read_be16(fixture + 12U), sizeof(fixture), &target) &&
              target == 8U,
          "SH-2 BRA decoder preserves backward signed-12 displacement");
    check(!sh2_bra_target(8U, 0xb003U, sizeof(fixture), &target),
          "SH-2 BRA decoder rejects non-branch opcodes");
}

static void test_sh2_work_ram_image_mapper(void) {
    size_t offset = 0U;

    check(sh2_work_ram_image_offset(0x06028490U, 0x9000U, &offset) &&
              offset == 0x8490U,
          "SH-2 candidate mapper resolves the PRS3 literal target into its matching file block");
    check(!sh2_work_ram_image_offset(0x0601ffffU, 0x9000U, &offset),
          "SH-2 work-RAM mapper rejects addresses below the documented image base");
    check(!sh2_work_ram_image_offset(0x06028490U, 0x8490U, &offset),
          "SH-2 work-RAM mapper rejects a target outside the available image");
}

static void test_sh2_argument_field_decoders(void) {
    unsigned int base = 0U, value = 0U, displacement = 0U;
    unsigned int source = 0U, destination = 0U;

    check(sh2_movl_disp_register_fields(0x5363U, &base, &value, &displacement) &&
              base == 6U && value == 3U && displacement == 12U,
          "SH-2 MOV.L @(disp,Rm),Rn decoder preserves the R6+12 to R3 fields");
    check(sh2_mov_register_fields(0x6b33U, &source, &destination) &&
              source == 3U && destination == 11U,
          "SH-2 MOV Rm,Rn decoder preserves the R3 to R11 fields");
    check(!sh2_movl_disp_register_fields(0x6b33U, &base, &value, &displacement) &&
              !sh2_mov_register_fields(0x5363U, &source, &destination),
          "SH-2 argument-field decoders reject the other instruction family");
    check(sh2_movl_store_disp_register_fields(0x1f31U, &base, &value, &displacement) &&
              base == 15U && value == 3U && displacement == 4U,
          "SH-2 MOV.L store decoder preserves the R3 to R15+4 fields");
    check(!sh2_movl_store_disp_register_fields(0x5363U, &base, &value, &displacement),
          "SH-2 MOV.L store decoder rejects the load instruction family");
}

static void test_sh2_dispatcher_caller_scanner(void) {
    uint8_t fixture[64];
    size_t target = 20U;
    int is_call = 0;

    memset(fixture, 0, sizeof(fixture));
    /* BSR +4 at 8 reaches 20; MOV.L @(4,PC),R1 at 0 reads the same
     * synthetic entry address from 20. */
    fixture[0] = 0xd1U;
    fixture[1] = 0x04U;
    fixture[8] = 0xb0U;
    fixture[9] = 0x04U;
    fixture[20] = 0x06U;
    fixture[21] = 0x02U;
    fixture[22] = 0x00U;
    fixture[23] = 0x14U;
    check(sh2_direct_control_flow_target(8U, read_be16(fixture + 8U),
                                         sizeof(fixture), &target,
                                         &is_call) && target == 20U && is_call,
          "SH-2 direct-edge scanner decodes the synthetic BSR predecessor");
    check(read_be32(fixture + 20U) == 0x06020014U,
          "SH-2 caller scanner fixture retains its synthetic entry address");
}

static void test_sh2_version1_stream_read_evidence(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Sh2Prs3Version1StreamReadEvidence evidence;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    /* Callee prologue: MOV R4,R12. */
    fixture[entry + 6U] = 0x6cU;
    fixture[entry + 7U] = 0x43U;
    /* Shift/test/refill branch, then the R14 exhaustion branch. */
    fixture[entry + 54U] = 0x4bU;
    fixture[entry + 55U] = 0x21U;
    fixture[entry + 56U] = 0x22U;
    fixture[entry + 57U] = 0xb8U;
    fixture[entry + 58U] = 0x8bU;
    fixture[entry + 59U] = 0x05U;
    fixture[entry + 60U] = 0x2eU;
    fixture[entry + 61U] = 0xe8U;
    fixture[entry + 62U] = 0x89U;
    fixture[entry + 63U] = 0x32U;
    /* MOV.B @R12+,R11; ADD #-1,R14; EXTU.B R11,R11. */
    fixture[entry + 64U] = 0x6bU;
    fixture[entry + 65U] = 0xc4U;
    fixture[entry + 66U] = 0x7eU;
    fixture[entry + 67U] = 0xffU;
    fixture[entry + 68U] = 0x6bU;
    fixture[entry + 69U] = 0xbcU;
    fixture[entry + 74U] = 0x23U;
    fixture[entry + 75U] = 0xb8U;

    check(sh2_prs3_version1_stream_read_evidence(
              fixture, sizeof(fixture), &evidence) && evidence.valid &&
              evidence.byte_cursor_register == 12U &&
              evidence.byte_value_register == 11U &&
              evidence.remaining_register == 14U &&
              evidence.remaining_decrement == -1 &&
              evidence.refill_branch_target == entry + 72U &&
              evidence.refill_exhausted_target == entry + 166U,
          "SH-2 stream reader evidence locks the selected refill byte load");
    fixture[entry + 65U] = 0xc0U;
    check(!sh2_prs3_version1_stream_read_evidence(
              fixture, sizeof(fixture), &evidence),
          "SH-2 stream reader evidence rejects a non-post-increment refill load");
}

static void test_sh2_version1_control_sentinel_evidence(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 512U];
    Sh2Prs3Version1ControlSentinelEvidence evidence;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    /* MOV.L @(0x3a,PC),R9 -> entry+284 = 0x0000ff00. */
    fixture[entry + 48U] = 0xd9U;
    fixture[entry + 49U] = 0x3aU;
    fixture[entry + 284U] = 0x00U;
    fixture[entry + 285U] = 0x00U;
    fixture[entry + 286U] = 0xffU;
    fixture[entry + 287U] = 0x00U;
    /* MOV.W @(0x6a,PC),R2 -> entry+268 = 0x0100. */
    fixture[entry + 52U] = 0x92U;
    fixture[entry + 53U] = 0x6aU;
    fixture[entry + 268U] = 0x01U;
    fixture[entry + 269U] = 0x00U;
    fixture[entry + 70U] = 0x2bU;
    fixture[entry + 71U] = 0x9bU; /* OR R9,R11 */
    fixture[entry + 162U] = 0xafU;
    fixture[entry + 163U] = 0xc7U; /* BRA back to entry+52 */

    check(sh2_prs3_version1_control_sentinel_evidence(
              fixture, sizeof(fixture), &evidence) && evidence.valid &&
              evidence.sentinel_word == 0x0100U &&
              evidence.refill_marker_word == 0x0000ff00U &&
              evidence.refill_marker_or_source_register == 9U &&
              evidence.refill_marker_or_destination_register == 11U &&
              evidence.loop_back_target == entry + 52U,
          "SH-2 control-sentinel evidence locks the refill marker protocol");
    fixture[entry + 71U] = 0x9aU;
    check(!sh2_prs3_version1_control_sentinel_evidence(
              fixture, sizeof(fixture), &evidence),
          "SH-2 control-sentinel evidence rejects a non-OR refill operation");
}

static void test_sh2_version1_zero_bit_gate_evidence(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Sh2Prs3Version1ZeroBitGateEvidence evidence;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 76U] = 0x89U;
    fixture[entry + 77U] = 0x0aU; /* BT -> entry + 100 */
    fixture[entry + 100U] = 0x7eU;
    fixture[entry + 101U] = 0xfeU; /* ADD #-2,R14 */
    fixture[entry + 102U] = 0x4eU;
    fixture[entry + 103U] = 0x11U; /* CMP/PZ R14 */
    fixture[entry + 104U] = 0x8bU;
    fixture[entry + 105U] = 0x1dU; /* BF -> entry + 166 */
    check(sh2_prs3_version1_zero_bit_gate_evidence(
              fixture, sizeof(fixture), &evidence) && evidence.valid &&
              evidence.zero_bit_branch_target == entry + 100U &&
              evidence.counter_register == 14U && evidence.counter_decrement == -2 &&
              evidence.rejection_target == entry + 166U,
          "SH-2 zero-bit gate evidence locks the R14 decrement and rejection edge");
    fixture[entry + 101U] = 0xffU;
    check(!sh2_prs3_version1_zero_bit_gate_evidence(
              fixture, sizeof(fixture), &evidence),
          "SH-2 zero-bit gate evidence rejects a different counter decrement");
}

static void test_sh2_version1_side_repeat_evidence(void) {
    uint8_t fixture[NEXUS_PRS3_VERSION1_CALLEE_OFFSET + 256U];
    Sh2Prs3Version1SideRepeatEvidence evidence;
    size_t entry = NEXUS_PRS3_VERSION1_CALLEE_OFFSET;

    memset(fixture, 0, sizeof(fixture));
    fixture[entry + 154U] = 0x2aU;
    fixture[entry + 155U] = 0x10U; /* CMP/EQ R1,R10 */
    fixture[entry + 156U] = 0x7aU;
    fixture[entry + 157U] = 0x01U; /* ADD #1,R10 */
    fixture[entry + 158U] = 0x8fU;
    fixture[entry + 159U] = 0xf3U; /* BF/S -> entry + 136 */
    fixture[entry + 162U] = 0xafU;
    fixture[entry + 163U] = 0xc7U; /* BRA -> entry + 52 */
    check(sh2_prs3_version1_side_repeat_evidence(
              fixture, sizeof(fixture), &evidence) && evidence.valid &&
              evidence.compare_source_register == 1U &&
              evidence.compare_destination_register == 10U &&
              evidence.delayed_repeat_target == entry + 136U &&
              evidence.outer_loop_target == entry + 52U,
          "SH-2 side-repeat evidence locks compare, delayed repeat, and outer loop");
    fixture[entry + 159U] = 0xf2U;
    check(!sh2_prs3_version1_side_repeat_evidence(
              fixture, sizeof(fixture), &evidence),
          "SH-2 side-repeat evidence rejects a changed delayed branch target");
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *data;
    *out_data = NULL;
    *out_size = 0U;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0L, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0L ||
        fseek(fp, 0L, SEEK_SET) != 0) { fclose(fp); return 0; }
    data = (uint8_t *)malloc((size_t)file_size);
    if (!data) { fclose(fp); return 0; }
    if (fread(data, 1U, (size_t)file_size, fp) != (size_t)file_size) {
        free(data); fclose(fp); return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)file_size;
    return 1;
}

static size_t count_magic(const uint8_t *data, size_t size, const char magic[4]) {
    size_t count = 0U, i;
    if (!data || size < 4U) return 0U;
    for (i = 0U; i + 4U <= size; ++i)
        if (memcmp(data + i, magic, 4U) == 0) ++count;
    return count;
}

int main(int argc, char **argv) {
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    char default_dir[1024], dm_path[1200];
    char dm_md5[33];
    const char *home;
    uint8_t *dm = NULL;
    size_t dm_size = 0U;
    Sh2FlatLiteralEvidence code_evidence;
    Sh2FlatLiteralEvidence embedded_evidence;
    Sh2Prs3VersionDispatchEvidence dispatch_evidence;
    Sh2Prs3Version1CalleeEvidence callee_evidence;
    Sh2Prs3Version1OutputBaseEvidence output_base_evidence;
    Sh2Prs3Version1BitfieldOutputEvidence bitfield_output_evidence;
    Sh2Prs3Version1ArgumentEvidence argument_evidence;
    Sh2Prs3Version1WordLifetimeEvidence word_lifetime_evidence;
    Sh2Prs3Version1StreamReadEvidence stream_read_evidence;
    Sh2Prs3Version1ControlSentinelEvidence control_sentinel_evidence;
    Sh2Prs3Version1ZeroBitGateEvidence zero_bit_gate_evidence;
    Sh2Prs3Version1SideRepeatEvidence side_repeat_evidence;
    Sh2Prs3DispatcherCallerEvidence dispatcher_caller_evidence;
    Sh2Prs3BootstrapEvidence bootstrap_evidence;
    Sh2Prs3MapBoundaryEvidence map_boundary_evidence;

    test_sh2_flat_literal_scanner();
    test_sh2_conditional_branch_decoder();
    test_sh2_bsr_decoder();
    test_sh2_bra_decoder();
    test_sh2_work_ram_image_mapper();
    test_sh2_argument_field_decoders();
    test_sh2_dispatcher_caller_scanner();
    test_sh2_version1_stream_read_evidence();
    test_sh2_version1_control_sentinel_evidence();
    test_sh2_version1_zero_bit_gate_evidence();
    test_sh2_version1_side_repeat_evidence();

    if (!data_dir) {
        home = getenv("HOME");
        if (!home || snprintf(default_dir, sizeof(default_dir),
                              "%s/.firestaff/data/nexus", home) <= 0) {
            puts("SKIP: no Nexus data directory argument or HOME");
            return 0;
        }
        data_dir = default_dir;
    }
    if (snprintf(dm_path, sizeof(dm_path), "%s/DM.BIN", data_dir) <= 0) {
        fprintf(stderr, "FAIL: Nexus data path is too long\n");
        return 1;
    }
    if (!read_file(dm_path, &dm, &dm_size)) {
        puts("SKIP: hash-verified DM.BIN is not available");
        free(dm); return 0;
    }

    check(dm_size == NEXUS_PRS3_LOADER_DM_SIZE,
          "DM.BIN has the locked Japanese Track 1 size");
    check(firestaff_x68k_media_receipt_md5_hex(dm, dm_size, dm_md5, sizeof(dm_md5)) == 0 &&
              strcmp(dm_md5, NEXUS_PRS3_LOADER_DM_MD5) == 0,
          "DM.BIN matches its locked MD5");
    if (g_failures == 0) {
        check(count_magic(dm, dm_size, "PRS3") == 2U,
              "DM.BIN contains exactly two PRS3 markers");
        check(NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET + 4U <= dm_size &&
                  memcmp(dm + NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET, "PRS3", 4U) == 0,
              "DM.BIN code-region PRS3 marker remains at its locked offset");
        check(sh2_prs3_header_guard_evidence(dm, dm_size),
              "DM.BIN SH-2 code proves PRS3 magic guarding and version-1..5 dispatch");
        check(sh2_prs3_bootstrap_evidence(dm, dm_size, &bootstrap_evidence) &&
                  bootstrap_evidence.valid &&
                  bootstrap_evidence.derived_initial_image_base ==
                      NEXUS_PRS3_SH2_BOOTSTRAP_IMAGE_BASE &&
                  bootstrap_evidence.vector_table_precedes_initial_image &&
                  bootstrap_evidence.dispatcher_is_in_initial_image_range,
              "DM.BIN bootstrap proves only the 0x06010000 initial map and direct entry route");
        check(sh2_prs3_map_boundary_evidence(dm, dm_size, &map_boundary_evidence) &&
                  map_boundary_evidence.valid &&
                  map_boundary_evidence.candidate_map_matches_known_block &&
                  !map_boundary_evidence.bootstrap_map_matches_known_block,
              "DM.BIN keeps the callee literal map distinct from the bootstrap map without relocation promotion");
        check(sh2_prs3_version_dispatch_evidence(dm, dm_size, &dispatch_evidence) &&
                  dispatch_evidence.valid,
              "DM.BIN SH-2 version comparisons lead through direct true branches");
        check(sh2_prs3_version1_callee_evidence(dm, dm_size, &callee_evidence) &&
                  callee_evidence.valid,
              "DM.BIN version-1 callee has the locked direct basic-block graph");
        check(sh2_prs3_version1_output_base_evidence(
                  dm, dm_size, &output_base_evidence) && output_base_evidence.valid,
              "DM.BIN version-1 callee locks its post-call output-base prerequisite");
        check(sh2_prs3_version1_bitfield_output_evidence(
                  dm, dm_size, &bitfield_output_evidence) &&
                  bitfield_output_evidence.valid,
              "DM.BIN first indirect target maps to its bounded bit-field output block");
        check(sh2_prs3_version1_argument_evidence(
                  dm, dm_size, &argument_evidence) && argument_evidence.valid,
              "DM.BIN version-1 branch locks its R6 argument construction and version-2 neighbor");
        check(sh2_prs3_version1_word_lifetime_evidence(
                  dm, dm_size, &word_lifetime_evidence) && word_lifetime_evidence.valid &&
                  word_lifetime_evidence.r6_is_incoming_over_setup &&
                  !word_lifetime_evidence.canonical_prs3_descriptor_field_correlation,
              "DM.BIN preserves the untyped R6+12 word through the selected call without descriptor-field promotion");
        check(sh2_prs3_version1_stream_read_evidence(
                  dm, dm_size, &stream_read_evidence) && stream_read_evidence.valid &&
                  stream_read_evidence.byte_cursor_register == 12U &&
                  stream_read_evidence.byte_value_register == 11U &&
                  stream_read_evidence.remaining_register == 14U &&
                  stream_read_evidence.remaining_decrement == -1,
              "DM.BIN version-1 callee proves a bounded R4-origin byte refill path");
        check(sh2_prs3_version1_control_sentinel_evidence(
                  dm, dm_size, &control_sentinel_evidence) &&
                  control_sentinel_evidence.valid &&
                  control_sentinel_evidence.sentinel_word == 0x0100U &&
                  control_sentinel_evidence.refill_marker_word == 0x0000ff00U,
              "DM.BIN version-1 callee proves its R11 refill sentinel protocol");
        check(sh2_prs3_version1_zero_bit_gate_evidence(
                  dm, dm_size, &zero_bit_gate_evidence) &&
                  zero_bit_gate_evidence.valid &&
                  zero_bit_gate_evidence.counter_register == 14U &&
                  zero_bit_gate_evidence.counter_decrement == -2,
              "DM.BIN version-1 zero-bit side route proves its bounded R14 gate");
        check(sh2_prs3_version1_side_repeat_evidence(
                  dm, dm_size, &side_repeat_evidence) && side_repeat_evidence.valid &&
                  side_repeat_evidence.compare_source_register == 1U &&
                  side_repeat_evidence.compare_destination_register == 10U,
              "DM.BIN version-1 side route proves its local delayed repeat relation");
        check(sh2_prs3_dispatcher_caller_evidence(
                  dm, dm_size, &dispatcher_caller_evidence) &&
                  dispatcher_caller_evidence.valid &&
                  dispatcher_caller_evidence.direct_in_image_entry_edges == 0U &&
                  dispatcher_caller_evidence.direct_in_image_call_edges == 0U &&
                  dispatcher_caller_evidence.pc_literal_entry_address_materializations == 0U &&
                  !dispatcher_caller_evidence.caller_callsite_found &&
                  !dispatcher_caller_evidence.r6_object_construction_found,
              "DM.BIN exposes no in-image caller or R6-object construction site for the dispatcher");
        code_evidence = sh2_flat_literal_evidence(
            dm, dm_size, NEXUS_PRS3_LOADER_CODE_MARKER_OFFSET);
        embedded_evidence = sh2_flat_literal_evidence(
            dm, dm_size, NEXUS_PRS3_LOADER_EMBEDDED_FRAME_OFFSET);
        printf("SH-2 flat literal candidates: code-marker refs=%zu nearby-jsr=%zu; "
               "embedded-record refs=%zu nearby-jsr=%zu\n",
               code_evidence.literal_references,
               code_evidence.nearby_indirect_call_candidates,
               embedded_evidence.literal_references,
               embedded_evidence.nearby_indirect_call_candidates);
        printf("SH-2 code-marker candidate offsets:");
        if (code_evidence.recorded_reference_count == 0U) {
            printf(" none");
        } else {
            size_t i;
            for (i = 0U; i < code_evidence.recorded_reference_count; ++i) {
                printf(" %zu", code_evidence.reference_offsets[i]);
            }
        }
        putchar('\n');
        if (dispatch_evidence.valid) {
            size_t i;
            for (i = 0U; i < 5U; ++i) {
                printf("SH-2 PRS3 version-%zu branch: compare=%zu branch=%zu "
                       "instruction=%04x target=%zu\n",
                       i + 1U, dispatch_evidence.compare_offsets[i],
                       dispatch_evidence.branch_offsets[i],
                       (unsigned int)dispatch_evidence.branch_instructions[i],
                       dispatch_evidence.branch_targets[i]);
            }
            printf("SH-2 PRS3 version-1 call: entry=%zu call=%zu delay=%04x target=%zu\n",
                   dispatch_evidence.version1_entry_offset,
                   dispatch_evidence.version1_call_offset,
                   (unsigned int)dispatch_evidence.version1_delay_instruction,
                   dispatch_evidence.version1_call_target);
        }
        if (callee_evidence.valid) {
            printf("SH-2 PRS3 version-1 callee graph: entry=%zu bypass=%zu "
                   "decisions=%zu->%zu,%zu->%zu,%zu->%zu,%zu->%zu,%zu->%zu "
                   "backward=%zu->%zu loop=%zu->%zu return=%zu\n",
                   callee_evidence.entry_offset,
                   callee_evidence.conditional_call_bypass_target,
                   callee_evidence.first_decision_offset,
                   callee_evidence.first_decision_target,
                   callee_evidence.early_exit_decision_offset,
                   callee_evidence.early_exit_target,
                   callee_evidence.second_decision_offset,
                   callee_evidence.second_decision_target,
                   callee_evidence.loop_decision_offset,
                   callee_evidence.loop_decision_target,
                   callee_evidence.failure_decision_offset,
                   callee_evidence.failure_target,
                   callee_evidence.delayed_backward_branch_offset,
                   callee_evidence.delayed_backward_branch_target,
                   callee_evidence.loop_branch_offset,
                   callee_evidence.loop_target,
                   callee_evidence.return_offset);
        }
        if (output_base_evidence.valid) {
            printf("SH-2 PRS3 version-1 output-base: literal-load=%zu call=%zu "
                   "target=%08x R%u->R%u store=%zu @R%u+R%u value=R%u\n",
                   output_base_evidence.first_call_literal_load_offset,
                   output_base_evidence.first_call_offset,
                   (unsigned int)output_base_evidence.first_call_target_address,
                   output_base_evidence.post_call_base_source_register,
                   output_base_evidence.output_base_register,
                   output_base_evidence.first_output_byte_store_offset,
                   output_base_evidence.output_base_register,
                   output_base_evidence.output_index_register,
                   output_base_evidence.output_value_register);
        }
        if (bitfield_output_evidence.valid) {
            size_t i;
            printf("SH-2 PRS3 target bit-field output: address=%08x base=%08x "
                   "image-offset=%zu branch=%zu load=%zu shad=%zu store=%zu fields=",
                   (unsigned int)bitfield_output_evidence.target_address,
                   (unsigned int)bitfield_output_evidence.image_base_address,
                   bitfield_output_evidence.target_image_offset,
                   bitfield_output_evidence.first_control_branch_offset,
                   bitfield_output_evidence.first_field_load_offset,
                   bitfield_output_evidence.first_shift_offset,
                   bitfield_output_evidence.first_store_offset);
            for (i = 0U; i < bitfield_output_evidence.conditional_field_count; ++i) {
                printf("%sR4+%u", i == 0U ? "" : ",",
                       bitfield_output_evidence.conditional_field_offsets[i]);
            }
            putchar('\n');
        }
        if (argument_evidence.valid) {
            printf("SH-2 PRS3 version-1 argument path: load=%zu @R%u+%u->R%u "
                   "branch-delay=%zu R%u->R%u call-delay=%zu R%u->R%u; "
                   "version-2 entry=%zu literal-load=%zu jsr@R%u=%zu delay=%04x\n",
                   argument_evidence.pre_dispatch_word_load_offset,
                   argument_evidence.pre_dispatch_base_register,
                   argument_evidence.pre_dispatch_byte_displacement,
                   argument_evidence.pre_dispatch_value_register,
                   argument_evidence.version1_branch_delay_offset,
                   argument_evidence.version1_branch_delay_source_register,
                   argument_evidence.version1_branch_delay_destination_register,
                   argument_evidence.version1_call_delay_offset,
                   argument_evidence.version1_call_delay_source_register,
                   argument_evidence.version1_call_delay_destination_register,
                   argument_evidence.version2_entry_offset,
                   argument_evidence.version2_literal_load_offset,
                   argument_evidence.version2_indirect_call_register,
                   argument_evidence.version2_indirect_call_offset,
                   (unsigned int)argument_evidence.version2_indirect_call_delay_instruction);
        }
        if (word_lifetime_evidence.valid) {
            printf("SH-2 PRS3 version-1 word lifetime: entry=%zu first-load=%zu "
                   "R%u+%u->R%u stack-store=%zu R%u+%u<-R%u compare=%zu "
                   "r11-copy=%zu call=%zu r6-copy=%zu magic-load=%zu; "
                   "r6-incoming=%d canonical-descriptor-field=%d",
                   word_lifetime_evidence.caller_entry_offset,
                   word_lifetime_evidence.first_slot_load_offset,
                   word_lifetime_evidence.slot_base_register,
                   word_lifetime_evidence.slot_byte_displacement,
                   word_lifetime_evidence.slot_value_register,
                   word_lifetime_evidence.first_slot_stack_store_offset,
                   word_lifetime_evidence.stack_base_register,
                   word_lifetime_evidence.stack_byte_displacement,
                   word_lifetime_evidence.stack_value_register,
                   word_lifetime_evidence.r3_live_compare_offset,
                   word_lifetime_evidence.dispatch_r11_copy_offset,
                   word_lifetime_evidence.selected_call_offset,
                   word_lifetime_evidence.selected_call_r6_copy_offset,
                   word_lifetime_evidence.prs3_magic_load_offset,
                   word_lifetime_evidence.r6_is_incoming_over_setup,
                   word_lifetime_evidence.canonical_prs3_descriptor_field_correlation);
            putchar('\n');
        }
        if (stream_read_evidence.valid) {
            printf("SH-2 PRS3 version-1 stream refill: shift=%zu R%u test=%zu R%u "
                   "branch=%zu->%zu remaining-test=%zu R%u exhausted=%zu "
                   "load=%zu @R%u+->R%u decrement=%zu R%u%+d extend=%zu R%u->R%u "
                   "next-test=%zu; payload/opcode/termination-proof=0\n",
                   stream_read_evidence.control_shift_offset,
                   stream_read_evidence.control_register,
                   stream_read_evidence.control_test_offset,
                   stream_read_evidence.control_test_other_register,
                   stream_read_evidence.refill_branch_offset,
                   stream_read_evidence.refill_branch_target,
                   stream_read_evidence.refill_remaining_test_offset,
                   stream_read_evidence.remaining_register,
                   stream_read_evidence.refill_exhausted_target,
                   stream_read_evidence.byte_load_offset,
                   stream_read_evidence.byte_cursor_register,
                   stream_read_evidence.byte_value_register,
                   stream_read_evidence.remaining_decrement_offset,
                   stream_read_evidence.remaining_register,
                   stream_read_evidence.remaining_decrement,
                   stream_read_evidence.byte_extend_offset,
                   stream_read_evidence.byte_extend_source_register,
                   stream_read_evidence.byte_extend_destination_register,
                   stream_read_evidence.next_control_test_offset);
        }
        if (control_sentinel_evidence.valid) {
            printf("SH-2 PRS3 version-1 control sentinel: R%u word=%04x load=%zu->%zu "
                   "R%u marker=%08x load=%zu->%zu or=%zu R%u->R%u "
                   "loop=%zu->%zu; opcode/payload/termination-proof=0\n",
                   control_sentinel_evidence.sentinel_register,
                   (unsigned int)control_sentinel_evidence.sentinel_word,
                   control_sentinel_evidence.sentinel_literal_load_offset,
                   control_sentinel_evidence.sentinel_literal_offset,
                   control_sentinel_evidence.refill_marker_register,
                   (unsigned int)control_sentinel_evidence.refill_marker_word,
                   control_sentinel_evidence.refill_marker_literal_load_offset,
                   control_sentinel_evidence.refill_marker_literal_offset,
                   control_sentinel_evidence.refill_marker_or_offset,
                   control_sentinel_evidence.refill_marker_or_source_register,
                   control_sentinel_evidence.refill_marker_or_destination_register,
                   control_sentinel_evidence.loop_back_branch_offset,
                   control_sentinel_evidence.loop_back_target);
        }
        if (zero_bit_gate_evidence.valid) {
            printf("SH-2 PRS3 version-1 zero-bit gate: branch=%zu->%zu "
                   "counter=%zu R%u%+d cmp-pz=%zu reject=%zu->%zu; "
                   "token/termination-proof=0\n",
                   zero_bit_gate_evidence.zero_bit_branch_offset,
                   zero_bit_gate_evidence.zero_bit_branch_target,
                   zero_bit_gate_evidence.counter_decrement_offset,
                   zero_bit_gate_evidence.counter_register,
                   zero_bit_gate_evidence.counter_decrement,
                   zero_bit_gate_evidence.counter_nonnegative_test_offset,
                   zero_bit_gate_evidence.rejection_branch_offset,
                   zero_bit_gate_evidence.rejection_target);
        }
        if (side_repeat_evidence.valid) {
            printf("SH-2 PRS3 version-1 side repeat: cmp=%zu R%u,R%u "
                   "increment=%zu R%u%+d delayed=%zu->%zu outer=%zu->%zu; "
                   "token/termination-proof=0\n",
                   side_repeat_evidence.compare_offset,
                   side_repeat_evidence.compare_source_register,
                   side_repeat_evidence.compare_destination_register,
                   side_repeat_evidence.counter_increment_offset,
                   side_repeat_evidence.counter_register,
                   side_repeat_evidence.counter_increment,
                   side_repeat_evidence.delayed_repeat_branch_offset,
                   side_repeat_evidence.delayed_repeat_target,
                   side_repeat_evidence.outer_loop_branch_offset,
                   side_repeat_evidence.outer_loop_target);
        }
        if (dispatcher_caller_evidence.valid) {
            printf("SH-2 PRS3 dispatcher caller scan: entry=%zu address=%08x "
                   "direct-edges=%zu direct-calls=%zu pc-literals=%zu "
                   "caller=%d r6-construction=%d\n",
                   dispatcher_caller_evidence.dispatcher_entry_offset,
                   (unsigned int)dispatcher_caller_evidence.dispatcher_entry_address,
                   dispatcher_caller_evidence.direct_in_image_entry_edges,
                   dispatcher_caller_evidence.direct_in_image_call_edges,
                   dispatcher_caller_evidence.pc_literal_entry_address_materializations,
                   dispatcher_caller_evidence.caller_callsite_found,
                   dispatcher_caller_evidence.r6_object_construction_found);
        }
        if (bootstrap_evidence.valid) {
            printf("SH-2 bootstrap map: jump=%zu delay=%zu literal=%zu entry=%08x "
                   "initial-base=%08x vector-base=%08x vector-before-image=%d "
                   "dispatcher-in-image=%d\n",
                   bootstrap_evidence.bootstrap_jump_offset,
                   bootstrap_evidence.bootstrap_delay_offset,
                   bootstrap_evidence.entry_literal_offset,
                   (unsigned int)bootstrap_evidence.entry_address,
                   (unsigned int)bootstrap_evidence.derived_initial_image_base,
                   (unsigned int)bootstrap_evidence.master_vector_base_address,
                   bootstrap_evidence.vector_table_precedes_initial_image,
                   bootstrap_evidence.dispatcher_is_in_initial_image_range);
        }
        if (map_boundary_evidence.valid) {
            printf("SH-2 map boundary: literal=%08x candidate-offset=%zu word=%04x "
                   "bootstrap-offset=%zu word=%04x; relocation-proof=0\n",
                   (unsigned int)map_boundary_evidence.callee_literal_address,
                   map_boundary_evidence.literal_candidate_offset,
                   (unsigned int)map_boundary_evidence.candidate_map_word,
                   map_boundary_evidence.bootstrap_map_offset,
                   (unsigned int)map_boundary_evidence.bootstrap_map_word);
        }
    }
    puts("RECEIPT: DM.BIN proves a bootstrap entry map and a separate callee-literal map, but no relocation, vector, direct predecessor, literal materialization, or indirect-dispatch metadata routes either map to the PRS3 v1 dispatcher.");
    puts("RECEIPT: the dispatcher remains unreachable from flat-image evidence alone; no runtime route is promoted.");
    free(dm);
    return g_failures == 0 ? 0 : 1;
}
