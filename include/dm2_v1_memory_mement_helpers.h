#ifndef FIRESTAFF_DM2_V1_MEMORY_MEMENT_HELPERS_H
#define FIRESTAFF_DM2_V1_MEMORY_MEMENT_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_VALIDATE_MEMENTS_MAX_ENTRIES 256u
#define DM2_V1_VALIDATE_MEMENTS_NULL_REF 0xffffu

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint32_t requested_bytes;
    uint32_t zeroed_bytes;
    uint32_t before_hash;
    uint32_t after_hash;
    const char *symbol;
    const char *source_path;
} DM2_V1_ZeroMemoryReceipt;

typedef struct {
    uint16_t id;
    uint16_t cache_index;
    uint16_t raw_index;
    uint32_t offset;
    uint32_t length;
    uint8_t active;
} DM2_V1_MementDescriptor;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint16_t entry_count;
    uint16_t active_count;
    uint16_t inactive_count;
    uint16_t first_invalid_index;
    uint32_t buffer_size;
    uint32_t used_bytes;
    uint32_t table_hash;
    int blocked_too_many_entries;
    int blocked_null_entries;
    int blocked_bad_id;
    int blocked_bad_span;
    int blocked_overlap;
    const char *symbol;
    const char *source_path;
} DM2_V1_ValidateMementsReceipt;

void dm2_v1_zero_memory_receipt_clear(DM2_V1_ZeroMemoryReceipt *receipt);
void dm2_v1_validate_mements_receipt_clear(
    DM2_V1_ValidateMementsReceipt *receipt);

int dm2_v1_ZERO_MEMORY(
    void *buffer,
    size_t byte_count,
    DM2_V1_ZeroMemoryReceipt *out_receipt);

int dm2_v1_ValidateMements(
    const DM2_V1_MementDescriptor *entries,
    size_t entry_count,
    size_t buffer_size,
    DM2_V1_ValidateMementsReceipt *out_receipt);

const char *dm2_v1_memory_mement_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MEMORY_MEMENT_HELPERS_H */
