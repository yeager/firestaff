#include "redmcsb_f0435_save_tail_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t cursor;
    unsigned int calls;
} Reader;

typedef struct {
    unsigned int calls;
    int result;
} Tail;

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static int read_exact(void *context, uint8_t *destination, size_t byte_count)
{
    Reader *reader = (Reader *)context;

    ++reader->calls;
    if (byte_count > reader->byte_count - reader->cursor) {
        return 0;
    }
    memcpy(destination, reader->bytes + reader->cursor, byte_count);
    reader->cursor += byte_count;
    return 1;
}

static int load_tail(void *context)
{
    Tail *tail = (Tail *)context;

    ++tail->calls;
    return tail->result;
}

static void encode_part(uint8_t *bytes, size_t byte_count, uint16_t key,
                        uint16_t *checksum)
{
    *checksum = redmcsb_f7056_saveutil_get_checksum_pc34(bytes, byte_count, key);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        bytes, byte_count, key);
}

static void admitted_initial_receipt(RedmcsbF1918LoadReceiptPc34 *initial,
                                    uint16_t events_key,
                                    uint16_t events_checksum,
                                    uint16_t timeline_key,
                                    uint16_t timeline_checksum)
{
    memset(initial, 0, sizeof(*initial));
    initial->header_valid = 1;
    initial->parts_loaded = REDMCSB_F1918_PC34_PART_COUNT;
    write_le16(initial->header + REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET + 6U,
               events_key);
    write_le16(initial->header +
               REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET + 6U,
               events_checksum);
    write_le16(initial->header + REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET + 8U,
               timeline_key);
    write_le16(initial->header +
               REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET + 8U,
               timeline_checksum);
}

int main(void)
{
    uint8_t events_cipher[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    uint8_t timeline_cipher[4] = { 2U, 0U, 1U, 0U };
    uint8_t events_plain[sizeof(events_cipher)];
    uint8_t timeline_plain[sizeof(timeline_cipher)];
    uint8_t stream[sizeof(events_cipher) + sizeof(timeline_cipher)];
    uint16_t events_checksum;
    uint16_t timeline_checksum;
    RedmcsbF1918LoadReceiptPc34 initial;
    RedmcsbF0435EventTimelineSpansPc34 spans;
    RedmcsbF0435TailLoadReceiptPc34 receipt;
    Reader reader;
    Tail tail;
    int result;

    memcpy(events_plain, events_cipher, sizeof(events_plain));
    memcpy(timeline_plain, timeline_cipher, sizeof(timeline_plain));
    encode_part(events_cipher, sizeof(events_cipher), 0x0404U, &events_checksum);
    encode_part(timeline_cipher, sizeof(timeline_cipher), 0x0505U,
                &timeline_checksum);
    memcpy(stream, events_cipher, sizeof(events_cipher));
    memcpy(stream + sizeof(events_cipher), timeline_cipher, sizeof(timeline_cipher));
    admitted_initial_receipt(&initial, 0x0404U, events_checksum, 0x0505U,
                            timeline_checksum);
    spans.events = events_cipher;
    spans.events_byte_count = sizeof(events_cipher);
    spans.timeline = timeline_cipher;
    spans.timeline_byte_count = sizeof(timeline_cipher);
    spans.event_maximum_count = 2U;
    reader.bytes = stream;
    reader.byte_count = sizeof(stream);
    reader.cursor = 0U;
    reader.calls = 0U;
    tail.calls = 0U;
    tail.result = 1;

    result = redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
        read_exact, &reader, &initial, &spans, load_tail, &tail, &receipt);
    CHECK("F0435 consumes EVENTS, TIMELINE and then F0434 tail",
          result == REDMCSB_F0435_PC34_RESULT_OK && reader.calls == 2U &&
          reader.cursor == sizeof(stream) && tail.calls == 1U);
    CHECK("F0435 restores exact caller-owned source spans",
          memcmp(events_cipher, events_plain, sizeof(events_plain)) == 0 &&
          memcmp(timeline_cipher, timeline_plain, sizeof(timeline_plain)) == 0);
    CHECK("F0435 reads C3/C4 key and checksum words from accepted header",
          receipt.events_loaded == 1U && receipt.timeline_loaded == 1U &&
          receipt.dungeon_tail_loaded == 1U && receipt.events_key == 0x0404U &&
          receipt.events_checksum == events_checksum &&
          receipt.timeline_key == 0x0505U &&
          receipt.timeline_checksum == timeline_checksum);

    memcpy(events_cipher, stream, sizeof(events_cipher));
    memcpy(timeline_cipher, stream + sizeof(events_cipher), sizeof(timeline_cipher));
    stream[0] ^= 0x80U;
    reader.cursor = 0U;
    reader.calls = 0U;
    tail.calls = 0U;
    result = redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
        read_exact, &reader, &initial, &spans, load_tail, &tail, &receipt);
    CHECK("bad EVENTS checksum stops before TIMELINE and dungeon tail",
          result == REDMCSB_F0435_PC34_RESULT_EVENTS_FAILED &&
          reader.calls == 1U && tail.calls == 0U && receipt.events_loaded == 0U);

    stream[0] ^= 0x80U;
    memcpy(events_cipher, stream, sizeof(events_cipher));
    memcpy(timeline_cipher, stream + sizeof(events_cipher), sizeof(timeline_cipher));
    reader.cursor = 0U;
    reader.calls = 0U;
    tail.calls = 0U;
    tail.result = 0;
    result = redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
        read_exact, &reader, &initial, &spans, load_tail, &tail, &receipt);
    CHECK("F0434 failure is propagated after both original parts",
          result == REDMCSB_F0435_PC34_RESULT_DUNGEON_TAIL_FAILED &&
          reader.calls == 2U && tail.calls == 1U && receipt.events_loaded == 1U &&
          receipt.timeline_loaded == 1U && receipt.dungeon_tail_loaded == 0U);

    spans.timeline_byte_count = 2U;
    CHECK("timeline length must be EventMaximumCount times uint16",
          redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
              read_exact, &reader, &initial, &spans, load_tail, &tail,
              &receipt) == REDMCSB_F0435_PC34_RESULT_PRECONDITION_FAILED);
    CHECK("source evidence is available",
          strstr(redmcsb_f0435_save_tail_pc34_source_evidence(), "F0434") != NULL);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB F0435 EVENTS/TIMELINE/dungeon-tail boundary");
    return 0;
}
