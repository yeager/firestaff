#include "nexus_v1_saturn_bkr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SATURN_BKR_MAGIC "BackUpRam Format"
#define SATURN_BKR_MAGIC_BYTES 16U
#define SATURN_BKR_SAVE_TAG UINT32_C(0x80000000)

typedef struct {
    uint32_t block;
    uint32_t payload_offset;
    uint32_t payload_bytes;
    uint32_t allocated_blocks;
    uint8_t language;
    uint32_t timestamp;
    char name[12];
    char comment[11];
} SaturnBkrEntryWork;

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
        ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) | bytes[3];
}

static int32_t read_be_s16(const uint8_t *bytes)
{
    uint32_t value = read_be16(bytes);
    return (value & 0x8000U) != 0U ? (int32_t)value - 0x10000 : (int32_t)value;
}

static int block_range_valid(uint32_t block, uint32_t block_count)
{
    return block >= 2U && block < block_count;
}

static int parse_entry(const uint8_t *image, uint32_t block_bytes,
                       uint32_t block_count, uint32_t first_block,
                       uint8_t *claimed, uint32_t payload_capacity,
                       SaturnBkrEntryWork *work, uint8_t *payload)
{
    uint16_t *references = NULL;
    uint8_t *visited_list_blocks = NULL;
    uint8_t *reference_seen = NULL;
    uint32_t reference_count = 0U;
    uint32_t list_block = first_block;
    uint32_t visited_count = 1U;
    uint32_t cursor;
    uint32_t terminator_at = 0U;
    uint32_t term_block_index = 0U;
    uint32_t stream_index;
    uint32_t output_bytes = 0U;
    uint32_t payload_size;
    uint32_t first_base;
    uint32_t i;
    int ok = 0;

    references = (uint16_t *)malloc((size_t)block_count * sizeof(*references));
    visited_list_blocks = (uint8_t *)calloc(block_count, 1U);
    reference_seen = (uint8_t *)calloc(block_count, 1U);
    if (!references || !visited_list_blocks || !reference_seen) goto done;
    first_base = first_block * block_bytes;
    if (read_be32(image + first_base) != SATURN_BKR_SAVE_TAG) goto done;
    payload_size = read_be32(image + first_base + 30U);
    if (!payload_size || payload_size > payload_capacity) goto done;

    visited_list_blocks[first_block] = 1U;
    cursor = first_base + 34U;
    while (reference_count < block_count) {
        uint32_t block_end = (list_block + 1U) * block_bytes;
        uint32_t value;
        if (cursor + 2U > block_end) {
            uint32_t next_reference = visited_count - 1U;
            uint32_t next_block;
            if (next_reference >= reference_count) goto done;
            next_block = references[next_reference];
            if (!block_range_valid(next_block, block_count) ||
                visited_list_blocks[next_block] ||
                read_be32(image + next_block * block_bytes) != 0U) {
                goto done;
            }
            visited_list_blocks[next_block] = 1U;
            ++visited_count;
            list_block = next_block;
            cursor = next_block * block_bytes + 4U;
            continue;
        }
        value = read_be16(image + cursor);
        cursor += 2U;
        if (value == 0U) {
            terminator_at = cursor;
            break;
        }
        if (!block_range_valid(value, block_count) || value == first_block ||
            claimed[value] || visited_list_blocks[value] ||
            reference_seen[value]) goto done;
        reference_seen[value] = 1U;
        references[reference_count++] = (uint16_t)value;
    }
    if (!terminator_at) goto done;

    /* The payload begins immediately after the block-list terminator, then
     * continues in referenced blocks after the directory's final block. */
    term_block_index = visited_count - 1U;
    if (term_block_index >= reference_count && term_block_index != 0U) goto done;
    {
        uint32_t term_block = term_block_index == 0U
            ? first_block : references[term_block_index - 1U];
        uint32_t term_end = (term_block + 1U) * block_bytes;
        uint32_t amount = term_end - terminator_at;
        if (amount > payload_size) amount = payload_size;
        if (amount) {
            memcpy(payload, image + terminator_at, amount);
            output_bytes = amount;
        }
        stream_index = term_block_index;
        while (output_bytes < payload_size && stream_index < reference_count) {
            uint32_t source_block = references[stream_index++];
            uint32_t source_offset = source_block * block_bytes + 4U;
            uint32_t amount = block_bytes - 4U;
            if (amount > payload_size - output_bytes)
                amount = payload_size - output_bytes;
            memcpy(payload + output_bytes, image + source_offset, amount);
            output_bytes += amount;
        }
    }
    if (output_bytes != payload_size) goto done;

    claimed[first_block] = 1U;
    for (i = 0U; i < reference_count; ++i) claimed[references[i]] = 1U;
    memset(work, 0, sizeof(*work));
    work->block = first_block;
    work->payload_bytes = payload_size;
    work->allocated_blocks = reference_count + 1U;
    work->language = image[first_base + 15U];
    work->timestamp = read_be32(image + first_base + 26U);
    memcpy(work->name, image + first_base + 4U, 11U);
    memcpy(work->comment, image + first_base + 16U, 10U);
    ok = 1;

done:
    free(references);
    free(visited_list_blocks);
    free(reference_seen);
    return ok;
}

int nexus_v1_saturn_bkr_parse(
    const uint8_t *image, size_t image_size,
    Nexus_V1_SaturnBkrEntry *entries, size_t entry_capacity,
    uint8_t *payload_storage, size_t payload_capacity,
    Nexus_V1_SaturnBkrReceipt *out_receipt)
{
    Nexus_V1_SaturnBkrReceipt receipt;
    SaturnBkrEntryWork work[NEXUS_V1_SATURN_BKR_MAX_ENTRIES];
    uint8_t *claimed = NULL;
    uint32_t repeats = 0U;
    uint32_t block_bytes;
    uint32_t block_count;
    uint32_t payload_used = 0U;
    uint32_t entry_count = 0U;
    uint32_t first_block;
    int ok = 0;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!image || !out_receipt || image_size < 128U ||
        image_size > NEXUS_V1_SATURN_BKR_MAX_IMAGE_BYTES) return 0;
    while ((size_t)(repeats + 1U) * SATURN_BKR_MAGIC_BYTES <= image_size &&
           memcmp(image + (size_t)repeats * SATURN_BKR_MAGIC_BYTES,
                  SATURN_BKR_MAGIC, SATURN_BKR_MAGIC_BYTES) == 0) {
        ++repeats;
    }
    block_bytes = repeats * SATURN_BKR_MAGIC_BYTES;
    if ((block_bytes != 64U && block_bytes != 512U) ||
        image_size % block_bytes != 0U) return 0;
    block_count = (uint32_t)(image_size / block_bytes);
    if (block_count < 2U || (entries && entry_capacity == 0U) ||
        (payload_storage && payload_capacity < image_size)) return 0;
    claimed = (uint8_t *)calloc(block_count, 1U);
    if (!claimed) return 0;
    memset(work, 0, sizeof(work));

    for (first_block = 2U; first_block < block_count; ++first_block) {
        uint32_t payload_offset;
        uint8_t *destination;
        if (claimed[first_block]) continue;
        if (read_be32(image + first_block * block_bytes) != SATURN_BKR_SAVE_TAG)
            continue;
        if (entry_count >= NEXUS_V1_SATURN_BKR_MAX_ENTRIES ||
            (entries && entry_count >= entry_capacity) ||
            payload_used > image_size) goto done;
        payload_offset = payload_used;
        destination = payload_storage ? payload_storage + payload_used : NULL;
        if (!destination) goto done;
        if (!parse_entry(image, block_bytes, block_count, first_block,
                         claimed, (uint32_t)(image_size - payload_used),
                         &work[entry_count], destination)) goto done;
        work[entry_count].payload_offset = payload_offset;
        payload_used += work[entry_count].payload_bytes;
        ++entry_count;
    }

    if (entries) {
        uint32_t i;
        memset(entries, 0, entry_capacity * sizeof(*entries));
        for (i = 0U; i < entry_count; ++i) {
            Nexus_V1_SaturnBkrEntry *entry = &entries[i];
            memcpy(entry->name, work[i].name, sizeof(entry->name));
            memcpy(entry->comment, work[i].comment, sizeof(entry->comment));
            entry->language = work[i].language;
            entry->timestamp_minutes = work[i].timestamp;
            entry->payload_bytes = work[i].payload_bytes;
            entry->payload_offset = work[i].payload_offset;
            entry->first_block = work[i].block;
            entry->allocated_block_count = work[i].allocated_blocks;
        }
    }
    receipt.valid = 1;
    receipt.image_bytes = (uint32_t)image_size;
    receipt.block_bytes = block_bytes;
    receipt.block_count = block_count;
    receipt.entry_count = entry_count;
    receipt.payload_bytes = payload_used;
    ok = 1;

done:
    if (!ok) {
        if (payload_storage)
            memset(payload_storage, 0, image_size);
        if (entries && entry_capacity)
            memset(entries, 0, entry_capacity * sizeof(*entries));
    }
    free(claimed);
    *out_receipt = receipt;
    return ok;
}

int nexus_v1_saturn_bkr_nexus_payload_probe(
    const Nexus_V1_SaturnBkrEntry *entry,
    const uint8_t *payload, size_t payload_size,
    Nexus_V1_SaturnBkrNexusPayloadReceipt *out_receipt)
{
    Nexus_V1_SaturnBkrNexusPayloadReceipt receipt;
    uint32_t champion_count;
    int32_t party_leader_index;
    uint32_t records_offset = 0x0CCU + 0x50U;
    uint32_t records_stride = 0x0E0U;
    uint32_t declared_span;
    uint32_t records_bytes;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!entry || !payload || !out_receipt ||
        memcmp(entry->name, "DMNEXUS__01", sizeof("DMNEXUS__01")) != 0 ||
        entry->payload_bytes != 20480U || payload_size != entry->payload_bytes ||
        read_be32(payload + 0x54U) != 0x10U ||
        read_be32(payload + 0x58U) != 0xCCU) {
        return 0;
    }

    champion_count = payload[0x62U];
    party_leader_index = (int8_t)payload[0x61U];
    declared_span = read_be16(payload + 0x0AU);
    if (champion_count == 0U || champion_count > 4U ||
        party_leader_index < 0 ||
        (uint32_t)party_leader_index >= champion_count ||
        declared_span != 0x274U + champion_count * records_stride) {
        return 0;
    }
    records_bytes = champion_count * records_stride;
    if (records_offset > payload_size ||
        records_bytes > payload_size - records_offset ||
        records_offset + records_bytes > declared_span ||
        declared_span > payload_size) {
        return 0;
    }

    receipt.valid = 1;
    receipt.payload_bytes = (uint32_t)payload_size;
    receipt.champion_count = champion_count;
    receipt.party_leader_index = (uint32_t)party_leader_index;
    {
        uint32_t champion;
        for (champion = 0U; champion < champion_count; ++champion) {
            uint32_t record_offset = records_offset + champion * records_stride;
            receipt.champion_health_current[champion] =
                read_be_s16(payload + record_offset + 0x38U);
            receipt.champion_max_health[champion] =
                read_be_s16(payload + record_offset + 0x3AU);
            receipt.champion_stamina_current[champion] =
                read_be_s16(payload + record_offset + 0x3CU);
            receipt.champion_stamina_max[champion] =
                read_be_s16(payload + record_offset + 0x3EU);
            receipt.champion_mana_current[champion] =
                read_be_s16(payload + record_offset + 0x40U);
            receipt.champion_max_mana[champion] =
                read_be_s16(payload + record_offset + 0x42U);
        }
    }
    receipt.champion_records_offset = records_offset;
    receipt.champion_record_stride = records_stride;
    receipt.champion_records_bytes = records_bytes;
    receipt.declared_payload_span = declared_span;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_saturn_bkr_probe_nexus_file(
    const char *path,
    Nexus_V1_SaturnBkrNexusFileReceipt *out_receipt)
{
    Nexus_V1_SaturnBkrNexusFileReceipt receipt;
    Nexus_V1_SaturnBkrEntry entries[NEXUS_V1_SATURN_BKR_MAX_ENTRIES];
    FILE *file = NULL;
    uint8_t *image = NULL;
    uint8_t *payload_storage = NULL;
    long file_bytes;
    size_t image_size;
    uint32_t nexus_entry_index = 0U;
    uint32_t nexus_entry_count = 0U;
    int ok = 0;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!path || !path[0] || !out_receipt || !(file = fopen(path, "rb")))
        goto done;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_bytes = ftell(file)) <= 0 ||
        (unsigned long)file_bytes > NEXUS_V1_SATURN_BKR_MAX_IMAGE_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0) {
        goto done;
    }
    image_size = (size_t)file_bytes;
    image = (uint8_t *)malloc(image_size);
    payload_storage = (uint8_t *)malloc(image_size);
    if (!image || !payload_storage ||
        fread(image, 1U, image_size, file) != image_size ||
        fgetc(file) != EOF || ferror(file)) {
        goto done;
    }
    if (!nexus_v1_saturn_bkr_parse(
            image, image_size, entries, NEXUS_V1_SATURN_BKR_MAX_ENTRIES,
            payload_storage, image_size, &receipt.card) ||
        !receipt.card.valid) {
        goto done;
    }
    {
        uint32_t i;
        for (i = 0U; i < receipt.card.entry_count; ++i) {
            if (memcmp(entries[i].name, "DMNEXUS__01",
                       sizeof("DMNEXUS__01")) == 0) {
                nexus_entry_index = i;
                ++nexus_entry_count;
            }
        }
    }
    if (nexus_entry_count != 1U) goto done;
    receipt.entry = entries[nexus_entry_index];
    if (receipt.entry.payload_offset > receipt.card.payload_bytes ||
        receipt.entry.payload_bytes >
            receipt.card.payload_bytes - receipt.entry.payload_offset ||
        !nexus_v1_saturn_bkr_nexus_payload_probe(
            &receipt.entry,
            payload_storage + receipt.entry.payload_offset,
            receipt.entry.payload_bytes, &receipt.nexus_payload)) {
        goto done;
    }
    receipt.valid = 1;
    *out_receipt = receipt;
    ok = 1;

done:
    if (file) fclose(file);
    free(payload_storage);
    free(image);
    if (!ok && out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    return ok;
}
