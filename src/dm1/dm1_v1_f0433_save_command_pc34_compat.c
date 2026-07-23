#include "dm1_v1_f0433_save_command_pc34_compat.h"

#include "memory_savegame_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

#define DM1_F0433_HEADER_FORMAT_OFFSET 299u
#define DM1_F0433_HEADER_GAME_ID_OFFSET 306u
#define DM1_F0433_HEADER_KEYS_OFFSET 310u
#define DM1_F0433_HEADER_CHECKSUMS_OFFSET 342u
#define DM1_F0433_HEADER_PLATFORM_OFFSET 374u
#define DM1_F0433_HEADER_DUNGEON_OFFSET 376u
#define DM1_F0433_FORMAT_DM_PC34 5u
#define DM1_F0433_PLATFORM_PC 1u
#define DM1_F0433_DUNGEON_DM 0u

static const char s_source_evidence[] =
    "ReDMCSB Toolchains/Common/Source/LOADSAVE.C F0433:1573-1682; "
    "READWRIT.C F0419/F0420; SAVEHEAD.C F0429/F0430. The helper ends before "
    "F0434 dungeon-tail serialization and owns no menu or filesystem route.";

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static uint32_t fingerprint(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0u; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int header_is_dm_pc34(const uint8_t *header)
{
    return header && header[DM1_F0433_HEADER_FORMAT_OFFSET] ==
            DM1_F0433_FORMAT_DM_PC34 &&
        read_le16(header + DM1_F0433_HEADER_PLATFORM_OFFSET) ==
            DM1_F0433_PLATFORM_PC &&
        read_le16(header + DM1_F0433_HEADER_DUNGEON_OFFSET) ==
            DM1_F0433_DUNGEON_DM &&
        (header[DM1_F0433_HEADER_GAME_ID_OFFSET] != 0u ||
         header[DM1_F0433_HEADER_GAME_ID_OFFSET + 1u] != 0u ||
         header[DM1_F0433_HEADER_GAME_ID_OFFSET + 2u] != 0u ||
         header[DM1_F0433_HEADER_GAME_ID_OFFSET + 3u] != 0u);
}

int dm1_v1_f0433_save_command_write_pc34(
    const DM1_V1_F0433SaveCommandRequestPc34 *request,
    uint8_t *destination,
    size_t destination_size,
    DM1_V1_F0433SaveCommandReceiptPc34 *out_receipt)
{
    uint8_t header[DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES];
    uint8_t decoded_header[DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES];
    DM1_V1_F0433SaveCommandReceiptPc34 receipt;
    size_t total = DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES;
    size_t part_cursor;
    size_t header_cursor = 0u;
    size_t decoded_cursor = 0u;
    size_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!request || !destination || !request->sourceHeaderReceiptValid ||
        !request->sourceRandomWords ||
        request->sourceRandomWordCount !=
            DM1_ORIGINAL_SAVE_PC34_HEADER_RANDOM_WORD_COUNT ||
        !header_is_dm_pc34(request->headerPlain)) {
        *out_receipt = receipt;
        return 0;
    }
    for (index = 0u; index < DM1_V1_F0433_PART_COUNT; ++index) {
        const DM1_V1_F0433SavePartPc34 *part = &request->parts[index];
        if (!part->sourceReceiptValid || !part->plainBytes ||
            part->byteCount == 0u || (part->byteCount & 1u) != 0u ||
            read_le16(request->headerPlain + DM1_F0433_HEADER_KEYS_OFFSET +
                      index * 2u) != part->key ||
            part->byteCount > UINT16_MAX ||
            total > SIZE_MAX - 2u - part->byteCount) {
            *out_receipt = receipt;
            return 0;
        }
        total += 2u + part->byteCount;
    }
    if (total > destination_size) {
        *out_receipt = receipt;
        return 0;
    }

    /* LOADSAVE.C computes all five checksums, writes the obfuscated header,
     * then writes the five source-order F0420 parts. */
    memcpy(header, request->headerPlain, sizeof(header));
    for (index = 0u; index < DM1_V1_F0433_PART_COUNT; ++index) {
        const DM1_V1_F0433SavePartPc34 *part = &request->parts[index];
        receipt.partChecksums[index] = F0418_SAVEUTIL_GetChecksumPC34_Compat(
            part->plainBytes, part->byteCount / 2u, part->key);
        receipt.partFingerprints[index] = fingerprint(part->plainBytes,
                                                       part->byteCount);
        write_le16(header + DM1_F0433_HEADER_CHECKSUMS_OFFSET + index * 2u,
                   receipt.partChecksums[index]);
    }
    if (!dm1_v1_original_save_pc34_f0430_write_obfuscated_header(
            destination, destination_size, &header_cursor, header,
            sizeof(header), request->sourceRandomWords,
            request->sourceRandomWordCount) ||
        header_cursor != DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES ||
        !dm1_v1_original_save_pc34_f0429_read_header(
            destination, total, &decoded_cursor, decoded_header,
            sizeof(decoded_header)) ||
        memcmp(decoded_header + DM1_F0433_HEADER_KEYS_OFFSET,
               header + DM1_F0433_HEADER_KEYS_OFFSET, 32u) != 0 ||
        memcmp(decoded_header + DM1_F0433_HEADER_CHECKSUMS_OFFSET,
               header + DM1_F0433_HEADER_CHECKSUMS_OFFSET, 32u) != 0) {
        memset(destination, 0, total);
        *out_receipt = receipt;
        return 0;
    }

    part_cursor = DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES;
    for (index = 0u; index < DM1_V1_F0433_PART_COUNT; ++index) {
        const DM1_V1_F0433SavePartPc34 *part = &request->parts[index];
        uint16_t written_checksum = 0u;
        int written = dm1_v1_original_save_pc34_write_part_f0420(
            destination + part_cursor, destination_size - part_cursor,
            part->plainBytes, part->byteCount, part->key, &written_checksum);
        if (written != (int)(2u + part->byteCount) ||
            written_checksum != receipt.partChecksums[index]) {
            memset(destination, 0, total);
            *out_receipt = receipt;
            return 0;
        }
        receipt.partOffsets[index] = part_cursor;
        part_cursor += (size_t)written;
    }

    part_cursor = DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES;
    for (index = 0u; index < DM1_V1_F0433_PART_COUNT; ++index) {
        const DM1_V1_F0433SavePartPc34 *part = &request->parts[index];
        uint8_t *decoded = (uint8_t *)malloc(part->byteCount);
        size_t decoded_size = 0u;
        uint16_t actual_checksum = 0u;
        int part_ok = 0;
        if (decoded) {
            part_ok = dm1_v1_original_save_pc34_read_part_f0419(
                destination, total, &part_cursor, part->key,
                receipt.partChecksums[index], decoded, part->byteCount,
                &decoded_size, &actual_checksum) == SAVEGAME_OK &&
                decoded_size == part->byteCount &&
                actual_checksum == receipt.partChecksums[index] &&
                memcmp(decoded, part->plainBytes, part->byteCount) == 0;
        }
        free(decoded);
        if (!part_ok) {
            memset(destination, 0, total);
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.accepted = 1;
    receipt.bytesWritten = total;
    receipt.headerFingerprint = fingerprint(header, sizeof(header));
    receipt.suppressSyntheticFallback = 1;
    *out_receipt = receipt;
    return 1;
}

const char *dm1_v1_f0433_save_command_source_evidence_pc34(void)
{
    return s_source_evidence;
}
