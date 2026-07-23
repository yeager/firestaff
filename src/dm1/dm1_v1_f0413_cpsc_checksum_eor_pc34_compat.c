#include "dm1_v1_f0413_cpsc_checksum_eor_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB Toolchains/Common/Source/COPYPRO8.C F0413 "
    "CPSC_GetChecksumEor. MEDIA626 uses add.b/eor.b over an address span; "
    "MEDIA137/MEDIA413 uses initial D1=10, add.w/eor.w until 4E5E4E75 or "
    "4E5D4E75; MEDIA351 uses its first uint16 length and BytesToSkip table. "
    "This helper is raw-media only and does not own spell or save state.";

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) | bytes[3];
}

static int checksum_pair_span(const uint8_t *bytes, size_t byteCount,
                              uint16_t *outChecksum)
{
    uint8_t sum = 0;
    uint8_t eor = 0;
    size_t offset;

    if (!bytes || !outChecksum || byteCount == 0 || (byteCount & 1u) != 0u) {
        return 0;
    }
    for (offset = 0; offset < byteCount; offset += 2) {
        sum = (uint8_t)(sum + bytes[offset]);
        eor = (uint8_t)(eor ^ bytes[offset + 1]);
    }
    *outChecksum = (uint16_t)(((uint16_t)sum << 8) | eor);
    return 1;
}

static int checksum_terminated_function(const uint8_t *bytes, size_t byteCount,
                                        uint16_t *outChecksum,
                                        size_t *outConsumed)
{
    uint16_t running = 10;
    uint16_t checksum = 0;
    size_t offset = 0;

    if (!bytes || !outChecksum || !outConsumed || byteCount < 6) return 0;
    while (offset + 4 <= byteCount) {
        uint32_t marker = read_be32(bytes + offset);
        if (marker == 0x4e5e4e75u || marker == 0x4e5d4e75u) {
            if (offset == 0) return 0;
            *outChecksum = checksum;
            *outConsumed = offset;
            return 1;
        }
        if (offset + 2 > byteCount) return 0;
        running = (uint16_t)(running + read_be16(bytes + offset));
        checksum = (uint16_t)(checksum ^ running);
        offset += 2;
    }
    return 0;
}

static int checksum_skip_table(const uint8_t *bytes, size_t byteCount,
                               const uint8_t *table, size_t tableByteCount,
                               uint16_t *outChecksum, size_t *outConsumed)
{
    uint16_t byteLimit;
    uint16_t checksum = 0;
    uint16_t offset = 0;
    size_t tableOffset = 2;

    if (!bytes || !table || !outChecksum || !outConsumed ||
        tableByteCount < 4) {
        return 0;
    }
    byteLimit = read_be16(table);
    if (byteLimit == 0 || byteLimit > byteCount) return 0;
    while (offset < byteLimit) {
        uint16_t skipAt;
        if (tableOffset + 2 > tableByteCount) return 0;
        skipAt = read_be16(table + tableOffset);
        if (skipAt == offset) {
            uint16_t skipCount;
            tableOffset += 2;
            if (tableOffset + 2 > tableByteCount) return 0;
            skipCount = read_be16(table + tableOffset);
            tableOffset += 2;
            if (skipCount == 0 || (uint32_t)offset + skipCount > byteLimit) {
                return 0;
            }
            offset = (uint16_t)(offset + skipCount);
            continue;
        }
        checksum = (uint16_t)(checksum + bytes[offset]);
        ++offset;
        checksum = (uint16_t)(checksum ^ offset);
    }
    *outChecksum = checksum;
    *outConsumed = byteLimit;
    return 1;
}

int dm1_v1_f0413_cpsc_checksum_eor_pc34(
    const DM1_V1_F0413CpscRequestPc34 *request,
    DM1_V1_F0413CpscReceiptPc34 *outReceipt)
{
    uint16_t checksum = 0;
    size_t consumed = 0;
    int accepted = 0;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !request->mediaSourceVerified || !request->functionBytes ||
        request->functionByteCount == 0) {
        return 0;
    }
    switch (request->variant) {
        case DM1_V1_F0413_CPSC_PAIR_SPAN_PC34:
            accepted = checksum_pair_span(request->functionBytes,
                                         request->functionByteCount, &checksum);
            consumed = request->functionByteCount;
            break;
        case DM1_V1_F0413_CPSC_TERMINATED_FUNCTION_PC34:
            accepted = checksum_terminated_function(
                request->functionBytes, request->functionByteCount,
                &checksum, &consumed);
            break;
        case DM1_V1_F0413_CPSC_SKIP_TABLE_PC34:
            accepted = checksum_skip_table(
                request->functionBytes, request->functionByteCount,
                request->skipTableBytes, request->skipTableByteCount,
                &checksum, &consumed);
            break;
        default:
            return 0;
    }
    if (!accepted) return 0;

    outReceipt->accepted = 1;
    outReceipt->variant = request->variant;
    outReceipt->checksum = checksum;
    outReceipt->consumedByteCount = consumed;
    outReceipt->suppressSyntheticFallback = 1;
    return 1;
}

const char *dm1_v1_f0413_cpsc_checksum_eor_source_evidence_pc34(void)
{
    return s_source_evidence;
}
