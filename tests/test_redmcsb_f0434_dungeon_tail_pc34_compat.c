#include "redmcsb_f0434_dungeon_tail_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t cursor;
    unsigned int calls;
    unsigned int fail_at_call;
} Reader;

static int read_exact(void *context, uint8_t *destination, size_t byte_count)
{
    Reader *reader = (Reader *)context;

    ++reader->calls;
    if (reader->calls == reader->fail_at_call ||
        byte_count > reader->byte_count - reader->cursor) {
        return 0;
    }
    memcpy(destination, reader->bytes + reader->cursor, byte_count);
    reader->cursor += byte_count;
    return 1;
}

static uint16_t initialize_parts(RedmcsbF0434DungeonTailPartPc34 *parts,
                                 uint8_t *bytes)
{
    uint16_t checksum = 0U;
    unsigned int part;

    for (part = 0U; part < REDMCSB_F7063_DUNGEON_PART_COUNT; ++part) {
        bytes[part] = (uint8_t)(part + 1U);
        parts[part].bytes = &bytes[part];
        parts[part].byte_count = 1U;
        checksum = (uint16_t)(checksum + bytes[part]);
    }
    return checksum;
}

int main(void)
{
    RedmcsbF0434DungeonTailPartPc34
        parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t part_bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t stream[REDMCSB_F7063_DUNGEON_PART_COUNT + 2U];
    RedmcsbF0434DungeonTailReceiptPc34 receipt;
    Reader reader;
    uint16_t checksum;
    unsigned int part;
    size_t zero_stream_size;
    int result;

    checksum = initialize_parts(parts, part_bytes);
    memcpy(stream, part_bytes, sizeof(part_bytes));
    stream[sizeof(part_bytes)] = (uint8_t)checksum;
    stream[sizeof(part_bytes) + 1U] = (uint8_t)(checksum >> 8);
    reader.bytes = stream;
    reader.byte_count = sizeof(stream);
    reader.cursor = 0U;
    reader.calls = 0U;
    reader.fail_at_call = 0U;

    result = redmcsb_f0434_load_dungeon_tail_pc34(read_exact, &reader, parts,
                                                   &receipt);
    CHECK("F0434 reads all 22 source-owned tail parts then checksum",
          result == REDMCSB_F0434_PC34_RESULT_OK &&
          reader.calls == REDMCSB_F7063_DUNGEON_PART_COUNT + 1U &&
          reader.cursor == sizeof(stream));
    CHECK("F0421 byte accumulator agrees with stored trailing word",
          receipt.parts_loaded == REDMCSB_F7063_DUNGEON_PART_COUNT &&
          receipt.calculated_checksum == checksum &&
          receipt.stored_checksum == checksum);
    CHECK("F0434 preserves exact source bytes in each caller-owned span",
          memcmp(part_bytes, stream, sizeof(part_bytes)) == 0);

    (void)initialize_parts(parts, part_bytes);
    memcpy(stream, part_bytes, sizeof(part_bytes));
    stream[sizeof(part_bytes)] = (uint8_t)checksum;
    stream[sizeof(part_bytes) + 1U] = (uint8_t)(checksum >> 8);
    reader.cursor = 0U;
    reader.calls = 0U;
    reader.fail_at_call = REDMCSB_F7063_PART_THING_DATA_0 + 1U;
    result = redmcsb_f0434_load_dungeon_tail_pc34(read_exact, &reader, parts,
                                                   &receipt);
    CHECK("failed ThingData transport stops before later tail parts",
          result == REDMCSB_F0434_PC34_RESULT_PART_READ_FAILED &&
          receipt.parts_loaded == REDMCSB_F7063_PART_THING_DATA_0 &&
          receipt.failed_part == REDMCSB_F7063_PART_THING_DATA_0 &&
          reader.calls == REDMCSB_F7063_PART_THING_DATA_0 + 1U);

    (void)initialize_parts(parts, part_bytes);
    memcpy(stream, part_bytes, sizeof(part_bytes));
    stream[sizeof(part_bytes)] = (uint8_t)(checksum + 1U);
    stream[sizeof(part_bytes) + 1U] = (uint8_t)(checksum >> 8);
    reader.cursor = 0U;
    reader.calls = 0U;
    reader.fail_at_call = 0U;
    result = redmcsb_f0434_load_dungeon_tail_pc34(read_exact, &reader, parts,
                                                   &receipt);
    CHECK("bad trailing checksum rejects after complete source read",
          result == REDMCSB_F0434_PC34_RESULT_CHECKSUM_MISMATCH &&
          receipt.parts_loaded == REDMCSB_F7063_DUNGEON_PART_COUNT &&
          reader.calls == REDMCSB_F7063_DUNGEON_PART_COUNT + 1U);

    (void)initialize_parts(parts, part_bytes);
    parts[REDMCSB_F7063_PART_THING_DATA_0 + 11U].bytes = NULL;
    parts[REDMCSB_F7063_PART_THING_DATA_0 + 11U].byte_count = 0U;
    checksum = 0U;
    zero_stream_size = 0U;
    for (part = 0U; part < REDMCSB_F7063_DUNGEON_PART_COUNT; ++part) {
        if (part != REDMCSB_F7063_PART_THING_DATA_0 + 11U) {
            stream[zero_stream_size++] = part_bytes[part];
            checksum = (uint16_t)(checksum + part_bytes[part]);
        }
    }
    stream[zero_stream_size++] = (uint8_t)checksum;
    stream[zero_stream_size++] = (uint8_t)(checksum >> 8);
    reader.bytes = stream;
    reader.byte_count = zero_stream_size;
    reader.cursor = 0U;
    reader.calls = 0U;
    reader.fail_at_call = 0U;
    CHECK("zero-byte ThingData is not replaced by a synthetic record",
          redmcsb_f0434_load_dungeon_tail_pc34(read_exact, &reader, parts,
                                                &receipt) ==
              REDMCSB_F0434_PC34_RESULT_OK);
    CHECK("source evidence is available",
          strstr(redmcsb_f0434_dungeon_tail_pc34_source_evidence(), "F0421") != NULL);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB F0434 dungeon-tail byte boundary");
    return 0;
}
