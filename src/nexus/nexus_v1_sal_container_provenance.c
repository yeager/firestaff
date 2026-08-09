#include "nexus_v1_sal_container_provenance.h"

#include <stdio.h>
#include <string.h>

static const uint8_t sal_magic[NEXUS_V1_SAL_CONTAINER_HEADER_BYTES] = {
    'd', 's', 'p', '0', '1', '.', 'E', 'X'
};

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_sal_container_provenance_parse(
    const uint8_t *bytes, uint64_t byte_count, uint64_t expected_fnv1a64,
    Nexus_V1_SalContainerProvenanceReceipt *out_receipt)
{
    Nexus_V1_SalContainerProvenanceReceipt receipt;
    uint64_t hash;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!bytes || byte_count <= NEXUS_V1_SAL_CONTAINER_HEADER_BYTES ||
        byte_count > (uint64_t)SIZE_MAX || !expected_fnv1a64 ||
        memcmp(bytes, sal_magic, NEXUS_V1_SAL_CONTAINER_HEADER_BYTES) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    hash = fnv1a64_update(UINT64_C(1469598103934665603), bytes,
                          (size_t)byte_count);
    if (hash != expected_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    memcpy(receipt.magic, sal_magic, sizeof(receipt.magic));
    receipt.source_fnv1a64 = hash;
    receipt.source_byte_count = byte_count;
    receipt.descriptor_offset = NEXUS_V1_SAL_CONTAINER_HEADER_BYTES;
    receipt.descriptor_length = byte_count - NEXUS_V1_SAL_CONTAINER_HEADER_BYTES;
    receipt.descriptor_fnv1a64 = fnv1a64_update(
        UINT64_C(1469598103934665603),
        bytes + NEXUS_V1_SAL_CONTAINER_HEADER_BYTES,
        (size_t)receipt.descriptor_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_sal_container_provenance_from_direct_identity(
    const Nexus_V1_SlevSalDirectIdentity *identity,
    Nexus_V1_SalContainerProvenanceReceipt *out_receipt)
{
    Nexus_V1_SalContainerProvenanceReceipt receipt;
    uint8_t buffer[4096], header[NEXUS_V1_SAL_CONTAINER_HEADER_BYTES];
    uint64_t total = 0U, full = UINT64_C(1469598103934665603);
    uint64_t payload = UINT64_C(1469598103934665603);
    FILE *file;
    size_t count, header_count = 0U;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !identity->valid || !identity->byte_count ||
        !identity->fnv1a64 || !nexus_v1_slev_sal_direct_identity_still_matches(identity) ||
        !(file = fopen(identity->direct_path, "rb"))) {
        *out_receipt = receipt;
        return 0;
    }
    while ((count = fread(buffer, 1U, sizeof(buffer), file)) != 0U) {
        size_t start = 0U;
        if (UINT64_MAX - total < count) { fclose(file); *out_receipt = receipt; return 0; }
        while (header_count < sizeof(header) && start < count)
            header[header_count++] = buffer[start++];
        full = fnv1a64_update(full, buffer, count);
        if (start < count) payload = fnv1a64_update(payload, buffer + start, count - start);
        total += count;
    }
    {
        /* Close before evaluating the rest: a read error would otherwise
         * short-circuit past fclose() and leak a descriptor per SAL file. */
        int io_failed = ferror(file) != 0;
        if (fclose(file) != 0) io_failed = 1;
        if (io_failed || total != identity->byte_count ||
            full != identity->fnv1a64 || header_count != sizeof(header) ||
            total <= sizeof(header) ||
            memcmp(header, sal_magic, sizeof(header)) != 0) {
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.valid = 1;
    memcpy(receipt.magic, header, sizeof(header));
    receipt.source_fnv1a64 = full;
    receipt.source_byte_count = total;
    receipt.descriptor_offset = sizeof(header);
    receipt.descriptor_length = total - sizeof(header);
    receipt.descriptor_fnv1a64 = payload;
    *out_receipt = receipt;
    return 1;
}
