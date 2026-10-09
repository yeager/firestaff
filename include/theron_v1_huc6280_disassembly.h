#ifndef THERON_V1_HUC6280_DISASSEMBLY_H
#define THERON_V1_HUC6280_DISASSEMBLY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    THERON_V1_HUC6280_DISASSEMBLY_UNAVAILABLE = 0,
    THERON_V1_HUC6280_DISASSEMBLY_REJECTED,
    THERON_V1_HUC6280_DISASSEMBLY_READY
} Theron_V1Huc6280DisassemblyStatus;

/* Static, source-backed support fragment at bank-$1f address $243e. This is
 * the authenticated byte/bitstream helper around the later loader. It is not
 * the post-CD RAM-loaded $2600 consumer and grants no tile/object semantics. */
typedef struct {
    Theron_V1Huc6280DisassemblyStatus status;
    int source_file_identity_verified;
    int bank_window_verified;
    /* Contiguous HuC6280 $2386..$2502 code/data window. Its exact bytes are
     * shared by the authenticated US and JP images; this is static source
     * evidence only, not a claim about the post-CD RAM consumer. */
    int bank1f_disassembly_window_verified;
    uint16_t bank1f_disassembly_window_address;
    uint16_t bank1f_disassembly_window_bytes;
    uint32_t bank1f_disassembly_window_file_offset;
    uint32_t bank1f_disassembly_window_fnv1a;
    int forward_byte_step_verified;
    int bank_switch_table_verified;
    int reverse_byte_read_verified;
    int level_decompressor_fragment_verified;
    int level_decompressor_caller_verified;
    int stage2_resource_handler_verified;
    int stage2_resource_bank_table_population_verified;
    int stage2_resource_destination_registers_verified;
    /* Stage-two interpreter jump table at $410d. This proves exact static
     * dispatch addresses in each raw retail BIN, not opcode/game semantics. */
    int stage2_dispatch_table_verified;
    uint16_t stage2_dispatch_table_address;
    uint16_t stage2_dispatch_table_bytes;
    uint16_t stage2_dispatch_table_entries;
    uint32_t stage2_dispatch_table_file_offset;
    uint32_t stage2_dispatch_table_fnv1a;
    /* US raw-BIN regular-spawn helper at HuC6280 $4667.  This is a static
     * call-contract receipt only; its RAM-loaded $5d64/$5d6a callees and
     * runtime RNG state remain unresolved. */
    int spawn_rng_helper_verified;
    uint16_t spawn_rng_helper_address;
    uint16_t spawn_rng_helper_bytes;
    uint32_t spawn_rng_helper_file_offset;
    uint32_t spawn_rng_helper_fnv1a;
    int spawn_rng_preconsumer_verified;
    uint16_t spawn_rng_preconsumer_address;
    uint16_t spawn_rng_preconsumer_bytes;
    uint32_t spawn_rng_preconsumer_file_offset;
    uint32_t spawn_rng_preconsumer_fnv1a;
    /* Static US-BIN bodies reached by the $4644 preconsumer. Their bytes and
     * RTS-bounded spans are verified; this does not prove the bank-switched
     * runtime state or semantic RNG return contract. */
    int spawn_rng_c96b_verified;
    uint16_t spawn_rng_c96b_address;
    uint16_t spawn_rng_c96b_bytes;
    uint32_t spawn_rng_c96b_file_offset;
    uint32_t spawn_rng_c96b_fnv1a;
    int spawn_rng_cc4c_verified;
    uint16_t spawn_rng_cc4c_address;
    uint16_t spawn_rng_cc4c_bytes;
    uint32_t spawn_rng_cc4c_file_offset;
    uint32_t spawn_rng_cc4c_fnv1a;
    /* JP static $4661 helper entry and $C414 preconsumer bytes; this does not
     * establish runtime mapping, execution, or helper return semantics. */
    int spawn_rng_helper_jp_verified;
    uint16_t spawn_rng_helper_jp_address;
    uint16_t spawn_rng_helper_jp_bytes;
    uint32_t spawn_rng_helper_jp_file_offset;
    uint32_t spawn_rng_helper_jp_fnv1a;
    int spawn_rng_preconsumer_jp_verified;
    uint16_t spawn_rng_preconsumer_jp_address;
    uint16_t spawn_rng_preconsumer_jp_bytes;
    uint32_t spawn_rng_preconsumer_jp_file_offset;
    uint32_t spawn_rng_preconsumer_jp_fnv1a;
    /* JP raw-window candidates for the caller's static $C95D/$CC3E targets;
     * byte identity does not establish runtime bank mapping or execution. */
    int spawn_rng_c95d_jp_candidate_verified;
    uint16_t spawn_rng_c95d_jp_candidate_address;
    uint16_t spawn_rng_c95d_jp_candidate_bytes;
    uint32_t spawn_rng_c95d_jp_candidate_file_offset;
    uint32_t spawn_rng_c95d_jp_candidate_fnv1a;
    int spawn_rng_cc3e_jp_candidate_verified;
    uint16_t spawn_rng_cc3e_jp_candidate_address;
    uint16_t spawn_rng_cc3e_jp_candidate_bytes;
    uint32_t spawn_rng_cc3e_jp_candidate_file_offset;
    uint32_t spawn_rng_cc3e_jp_candidate_fnv1a;
    /* Additional US Track 02 caller window at $c3a0.  It is a byte-backed
     * source-consumer reference only; the pointed RAM tables remain
     * semantically unidentified. */
    int spawn_runtime_c3a0_verified;
    uint16_t spawn_runtime_c3a0_address;
    uint16_t spawn_runtime_c3a0_bytes;
    uint32_t spawn_runtime_c3a0_file_offset;
    uint32_t spawn_runtime_c3a0_fnv1a;
    /* Static JP Track 02 counterpart candidate at the region-shifted source
     * offset. The corresponding runtime bank mapping remains unproven. */
    int spawn_runtime_c3a0_jp_verified;
    uint16_t spawn_runtime_c3a0_jp_address;
    uint16_t spawn_runtime_c3a0_jp_bytes;
    uint32_t spawn_runtime_c3a0_jp_file_offset;
    uint32_t spawn_runtime_c3a0_jp_fnv1a;
    /* Static palette consumer from the retail HuC6280 bank. The routine
     * proves the VCE write contract only; its dynamic $27c4/$27c5 source
     * pointer is not a Track 02 palette binding by itself. */
    int vce_palette_consumer_verified;
    uint16_t vce_palette_consumer_address;
    uint16_t vce_palette_consumer_bytes;
    uint32_t vce_palette_consumer_file_offset;
    uint32_t vce_palette_consumer_fnv1a;
    /* Immediate source caller which reads $27c4/$27c5/$27c6 through ($62),y
     * before BSR L96A5. This is a static source contract; it does not
     * establish which runtime initializer or source data owns $62/$63. */
    int vce_palette_caller_verified;
    uint16_t vce_palette_caller_address;
    uint16_t vce_palette_caller_bytes;
    uint32_t vce_palette_caller_file_offset;
    uint32_t vce_palette_caller_fnv1a;
    int semantic_publication_allowed;
    uint32_t source_file_size;
    uint32_t bank_file_offset;
    uint16_t fragment_address;
    uint16_t fragment_bytes;
    uint32_t fragment_fnv1a;
    uint16_t level_decompressor_address;
    uint16_t level_decompressor_bytes;
    uint32_t level_decompressor_fnv1a;
    uint16_t level_decompressor_caller_address;
    uint16_t level_decompressor_caller_bytes;
    uint32_t level_decompressor_caller_fnv1a;
    uint16_t stage2_resource_handler_address;
    uint16_t stage2_resource_handler_bytes;
    uint32_t stage2_resource_handler_fnv1a;
    char source_md5[33];
} Theron_V1Huc6280DisassemblyReceipt;

/* Reads a direct retail Track 02 BIN or ISO projection and verifies the exact
 * source-owned US or JP bank-$1f fragment. Missing input is UNAVAILABLE;
 * mismatched size, identity, bytes, or path type is REJECTED. */
int theron_v1_huc6280_disassembly_read_file(
    const char *path,
    int track02_variant,
    Theron_V1Huc6280DisassemblyReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_HUC6280_DISASSEMBLY_H */
