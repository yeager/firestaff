#include "dm1_v1_f0079_cpsc_checksum_add_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB Toolchains/Common/Source/COPYPRO1.C:14-73 "
    "F0079_CPSC_GetChecksumAdd; LOADSAVE.C:2091-2103 calls the checksum "
    "only in source builds without NOCOPYPROTECTION. Atari ST sums 68k "
    "words with the A6 UNLK/RTS sentinel; Amiga uses the A5 sentinel. The "
    "G14ED/G20E/G21E path sums caller-selected bytes from a bounded skip "
    "table. This adapter requires caller-authenticated executable bytes and "
    "does not provide a copy-protection fallback.";

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) | bytes[3];
}

static int code_offset(const DM1_V1_F0079CpscRequestPc34 *request,
                       size_t *outStart)
{
    uint64_t image_end;
    uint64_t entry = request->functionAddress;
    size_t offset;

    if (!request->codeBytes || !outStart || request->codeByteCount < 6u)
        return 0;
    image_end = (uint64_t)request->codeImageAddress +
                (uint64_t)request->codeByteCount;
    if (entry < request->codeImageAddress || entry >= image_end)
        return 0;
    offset = (size_t)(entry - request->codeImageAddress);
    if (offset + 2u > request->codeByteCount) return 0;

    /* COPYPRO1.C:16-19 redirects a jump-table JMP absolute-long entry. */
    if (read_be16(request->codeBytes + offset) == 0x4ef9u) {
        uint32_t target;
        uint64_t target_offset;
        if (offset + 6u > request->codeByteCount) return 0;
        target = read_be32(request->codeBytes + offset + 2u);
        target_offset = (uint64_t)target - request->codeImageAddress;
        if ((uint64_t)target < request->codeImageAddress ||
            target_offset >= request->codeByteCount)
            return 0;
        offset = (size_t)target_offset;
    }
    *outStart = offset;
    return 1;
}

static int checksum_terminated_code(
    const DM1_V1_F0079CpscRequestPc34 *request, uint16_t terminator,
    uint16_t *outChecksum, size_t *outConsumed)
{
    size_t start;
    size_t offset;
    uint16_t sum = 100u;
    uint32_t marker;

    if (!outChecksum || !outConsumed || !code_offset(request, &start))
        return 0;
    offset = start;
    /* COPYPRO1.C:25-30 and :40-45 start at 100, add one big-endian word,
     * then stop when the following bytes are UNLK A6/A5 followed by RTS. */
    for (;;) {
        if (offset > request->codeByteCount ||
            request->codeByteCount - offset < 6u)
            return 0;
        sum = (uint16_t)(sum + read_be16(request->codeBytes + offset));
        offset += 2u;
        marker = read_be32(request->codeBytes + offset);
        if (marker == (((uint32_t)terminator << 16) | 0x4e75u)) {
            *outChecksum = sum;
            *outConsumed = offset - start;
            return 1;
        }
    }
}

static int checksum_skip_table(
    const DM1_V1_F0079CpscRequestPc34 *request,
    uint16_t *outChecksum, size_t *outConsumed)
{
    uint16_t byte_limit;
    uint16_t offset = 0u;
    uint16_t sum = 0u;
    size_t table_offset = 2u;

    if (!request->codeBytes || !request->skipTableBytes || !outChecksum ||
        !outConsumed || request->skipTableByteCount < 4u)
        return 0;
    byte_limit = read_be16(request->skipTableBytes);
    if (byte_limit == 0u || (size_t)byte_limit > request->codeByteCount)
        return 0;

    /* COPYPRO1.C:61-72 reads one offset/count pair whenever an excluded
     * byte range starts, then adds 100 after reaching the caller's limit. */
    while (offset < byte_limit) {
        uint16_t skip_at;
        if (table_offset + 2u > request->skipTableByteCount) return 0;
        skip_at = read_be16(request->skipTableBytes + table_offset);
        if (skip_at == offset) {
            uint16_t skip_count;
            if (table_offset + 4u > request->skipTableByteCount) return 0;
            skip_count = read_be16(request->skipTableBytes + table_offset + 2u);
            if ((uint32_t)offset + skip_count > byte_limit) return 0;
            table_offset += 4u;
            offset = (uint16_t)(offset + skip_count);
        } else {
            sum = (uint16_t)(sum + request->codeBytes[offset]);
            ++offset;
        }
    }
    *outChecksum = (uint16_t)(sum + 100u);
    *outConsumed = byte_limit;
    return 1;
}

int dm1_v1_f0079_cpsc_checksum_add_pc34(
    const DM1_V1_F0079CpscRequestPc34 *request,
    DM1_V1_F0079CpscReceiptPc34 *outReceipt)
{
    uint16_t checksum = 0u;
    size_t consumed = 0u;
    int accepted;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !request->mediaSourceVerified) return 0;

    switch (request->variant) {
        case DM1_V1_F0079_CPSC_TERMINATED_ATARI_ST_PC34:
            accepted = checksum_terminated_code(
                request, 0x4e5eu, &checksum, &consumed);
            break;
        case DM1_V1_F0079_CPSC_TERMINATED_AMIGA_PC34:
            accepted = checksum_terminated_code(
                request, 0x4e5du, &checksum, &consumed);
            break;
        case DM1_V1_F0079_CPSC_SKIP_TABLE_PC34:
            accepted = checksum_skip_table(request, &checksum, &consumed);
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

const char *dm1_v1_f0079_cpsc_checksum_add_source_evidence_pc34(void)
{
    return s_source_evidence;
}
