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
    int forward_byte_step_verified;
    int bank_switch_table_verified;
    int reverse_byte_read_verified;
    int level_decompressor_fragment_verified;
    int level_decompressor_caller_verified;
    int stage2_resource_handler_verified;
    int stage2_resource_bank_table_population_verified;
    int stage2_resource_destination_registers_verified;
    /* Static palette consumer from the retail HuC6280 bank. The routine
     * proves the VCE write contract only; its dynamic $27c4/$27c5 source
     * pointer is not a Track 02 palette binding by itself. */
    int vce_palette_consumer_verified;
    uint16_t vce_palette_consumer_address;
    uint16_t vce_palette_consumer_bytes;
    uint32_t vce_palette_consumer_file_offset;
    uint32_t vce_palette_consumer_fnv1a;
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
