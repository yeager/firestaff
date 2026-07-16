#include "dm2_v1_memory_mement_helpers.h"

#include <string.h>

static uint32_t dm2_memory_fnv1a(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!data && size != 0u) {
        return 0u;
    }
    for (i = 0u; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static void dm2_zero_memory_begin(DM2_V1_ZeroMemoryReceipt *receipt)
{
    dm2_v1_zero_memory_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = "ZERO_MEMORY";
    receipt->source_path = "SKWIN/SkWinCore.cpp:2166";
}

static void dm2_validate_mements_begin(
    DM2_V1_ValidateMementsReceipt *receipt)
{
    dm2_v1_validate_mements_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = "ValidateMements";
    receipt->source_path = "SKWIN/SkWinCore.cpp:3908";
    receipt->first_invalid_index = DM2_V1_VALIDATE_MEMENTS_NULL_REF;
}

void dm2_v1_zero_memory_receipt_clear(DM2_V1_ZeroMemoryReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_validate_mements_receipt_clear(
    DM2_V1_ValidateMementsReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->first_invalid_index = DM2_V1_VALIDATE_MEMENTS_NULL_REF;
}

int dm2_v1_ZERO_MEMORY(
    void *buffer,
    size_t byte_count,
    DM2_V1_ZeroMemoryReceipt *out_receipt)
{
    dm2_zero_memory_begin(out_receipt);
    if (out_receipt) {
        out_receipt->requested_bytes = (uint32_t)byte_count;
    }
    if (byte_count > UINT32_MAX || (!buffer && byte_count != 0u)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_receipt) {
        out_receipt->before_hash =
            dm2_memory_fnv1a((const uint8_t *)buffer, byte_count);
    }
    if (byte_count != 0u) {
        memset(buffer, 0, byte_count);
    }
    if (out_receipt) {
        out_receipt->zeroed_bytes = (uint32_t)byte_count;
        out_receipt->after_hash =
            dm2_memory_fnv1a((const uint8_t *)buffer, byte_count);
        out_receipt->valid = 1;
    }
    return 1;
}

static int dm2_mement_span_overlaps(
    const DM2_V1_MementDescriptor *a,
    const DM2_V1_MementDescriptor *b)
{
    uint64_t a_end = (uint64_t)a->offset + (uint64_t)a->length;
    uint64_t b_end = (uint64_t)b->offset + (uint64_t)b->length;

    return (uint64_t)a->offset < b_end && (uint64_t)b->offset < a_end;
}

int dm2_v1_ValidateMements(
    const DM2_V1_MementDescriptor *entries,
    size_t entry_count,
    size_t buffer_size,
    DM2_V1_ValidateMementsReceipt *out_receipt)
{
    size_t i;
    size_t j;

    dm2_validate_mements_begin(out_receipt);
    if (out_receipt) {
        out_receipt->entry_count = (uint16_t)entry_count;
        out_receipt->buffer_size = (uint32_t)buffer_size;
    }
    if (entry_count > DM2_V1_VALIDATE_MEMENTS_MAX_ENTRIES ||
        buffer_size > UINT32_MAX) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->blocked_too_many_entries = 1;
        }
        return 0;
    }
    if (!entries && entry_count != 0u) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->blocked_null_entries = 1;
        }
        return 0;
    }
    if (out_receipt) {
        out_receipt->table_hash =
            dm2_memory_fnv1a((const uint8_t *)entries,
                             entry_count * sizeof(entries[0]));
    }
    for (i = 0u; i < entry_count; ++i) {
        const DM2_V1_MementDescriptor *entry = &entries[i];
        uint64_t end;

        if (!entry->active) {
            if (out_receipt) {
                ++out_receipt->inactive_count;
            }
            continue;
        }
        if (entry->id == DM2_V1_VALIDATE_MEMENTS_NULL_REF) {
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->blocked_bad_id = 1;
                out_receipt->first_invalid_index = (uint16_t)i;
            }
            return 0;
        }
        end = (uint64_t)entry->offset + (uint64_t)entry->length;
        if (entry->length == 0u || end > (uint64_t)buffer_size) {
            if (out_receipt) {
                out_receipt->blocked = 1;
                out_receipt->blocked_bad_span = 1;
                out_receipt->first_invalid_index = (uint16_t)i;
            }
            return 0;
        }
        for (j = i + 1u; j < entry_count; ++j) {
            if (entries[j].active &&
                dm2_mement_span_overlaps(entry, &entries[j])) {
                if (out_receipt) {
                    out_receipt->blocked = 1;
                    out_receipt->blocked_overlap = 1;
                    out_receipt->first_invalid_index = (uint16_t)j;
                }
                return 0;
            }
        }
        if (out_receipt) {
            ++out_receipt->active_count;
            out_receipt->used_bytes += entry->length;
        }
    }
    if (out_receipt) {
        out_receipt->valid = 1;
    }
    return 1;
}

const char *dm2_v1_memory_mement_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp ZERO_MEMORY:2166 "
           "ValidateMements:3908; bounded caller-owned memory/mement "
           "receipts only.";
}
