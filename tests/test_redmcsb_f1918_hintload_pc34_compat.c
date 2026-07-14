#include "redmcsb_f1918_hintload_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"
#include "redmcsb_f7061_save_header_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

typedef struct {
    const uint8_t *bytes;
    size_t byte_count;
    size_t cursor;
    int fail_at_call;
    int calls;
} Reader;

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static uint16_t alternating_header_checksum(const uint8_t *header)
{
    uint16_t checksum = 0U;
    size_t word;

    for (word = 0U; word < 128U; word += 4U) {
        uint16_t a = (uint16_t)(header[word * 2U] | (header[word * 2U + 1U] << 8));
        uint16_t b = (uint16_t)(header[(word + 1U) * 2U] | (header[(word + 1U) * 2U + 1U] << 8));
        uint16_t c = (uint16_t)(header[(word + 2U) * 2U] | (header[(word + 2U) * 2U + 1U] << 8));
        uint16_t d = (uint16_t)(header[(word + 3U) * 2U] | (header[(word + 3U) * 2U + 1U] << 8));
        checksum = (uint16_t)(checksum + a);
        checksum = (uint16_t)(checksum ^ b);
        checksum = (uint16_t)(checksum - c);
        checksum = (uint16_t)(checksum ^ d);
    }
    return checksum;
}

static int read_exact(void *context, uint8_t *destination, size_t byte_count)
{
    Reader *reader = (Reader *)context;
    ++reader->calls;
    if (reader->fail_at_call == reader->calls ||
        byte_count > reader->byte_count - reader->cursor) {
        return 0;
    }
    memcpy(destination, reader->bytes + reader->cursor, byte_count);
    reader->cursor += byte_count;
    return 1;
}

static void encode_part(uint8_t *bytes, size_t byte_count, uint16_t key,
                        uint16_t *checksum)
{
    *checksum = redmcsb_f7056_saveutil_get_checksum_pc34(bytes, byte_count, key);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        bytes, byte_count, key);
}

static void build_source_header(uint8_t header[512], uint16_t keys[3],
                                uint16_t checksums[3])
{
    uint8_t decrypted_tail[256];
    uint16_t expected;
    uint16_t tail_sum = 0U;
    unsigned int i;

    memset(header, 0, 512U);
    write_le16(header + 58U, 0x2468U);
    expected = alternating_header_checksum(header);
    memset(decrypted_tail, 0, sizeof(decrypted_tail));
    for (i = 0U; i < 3U; ++i) {
        write_le16(decrypted_tail + (312U - 256U) + i * 2U, keys[i]);
        write_le16(decrypted_tail + (344U - 256U) + i * 2U, checksums[i]);
    }
    for (i = 1U; i < 128U; ++i) {
        tail_sum = (uint16_t)(tail_sum +
            (uint16_t)(decrypted_tail[i * 2U] |
                       (decrypted_tail[i * 2U + 1U] << 8)));
    }
    write_le16(decrypted_tail, (uint16_t)(expected - tail_sum));
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        decrypted_tail, sizeof(decrypted_tail), 0x2468U);
    memcpy(header + 256U, decrypted_tail, sizeof(decrypted_tail));
}

int main(void)
{
    uint8_t header[512];
    uint8_t header_probe[512];
    uint8_t global_cipher[8] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U };
    uint8_t groups_cipher[6] = { 9U, 10U, 11U, 12U, 13U, 14U };
    uint8_t party_cipher[10] = { 15U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 23U, 24U };
    uint8_t global_plain[sizeof(global_cipher)];
    uint8_t groups_plain[sizeof(groups_cipher)];
    uint8_t party_plain[sizeof(party_cipher)];
    uint8_t stream[sizeof(header) + sizeof(global_cipher) + sizeof(groups_cipher) + sizeof(party_cipher)];
    uint16_t keys[3] = { 0x0101U, 0x0202U, 0x0303U };
    uint16_t checksums[3];
    Reader reader;
    RedmcsbF1918LoadReceiptPc34 receipt;
    int result;

    memcpy(global_plain, global_cipher, sizeof(global_plain));
    memcpy(groups_plain, groups_cipher, sizeof(groups_plain));
    memcpy(party_plain, party_cipher, sizeof(party_plain));
    encode_part(global_cipher, sizeof(global_cipher), keys[0], &checksums[0]);
    encode_part(groups_cipher, sizeof(groups_cipher), keys[1], &checksums[1]);
    encode_part(party_cipher, sizeof(party_cipher), keys[2], &checksums[2]);
    build_source_header(header, keys, checksums);
    memcpy(header_probe, header, sizeof(header_probe));
    CHECK("constructed F1914 header validates",
          redmcsb_f7061_is_read_save_header_successful_pc34(
              header_probe, sizeof(header_probe), 29U) == 1);
    memcpy(stream, header, sizeof(header));
    memcpy(stream + sizeof(header), global_cipher, sizeof(global_cipher));
    memcpy(stream + sizeof(header) + sizeof(global_cipher), groups_cipher, sizeof(groups_cipher));
    memcpy(stream + sizeof(header) + sizeof(global_cipher) + sizeof(groups_cipher), party_cipher, sizeof(party_cipher));

    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_cipher;
    receipt.parts[0].byte_count = sizeof(global_cipher);
    receipt.parts[1].bytes = groups_cipher;
    receipt.parts[1].byte_count = sizeof(groups_cipher);
    receipt.parts[2].bytes = party_cipher;
    receipt.parts[2].byte_count = sizeof(party_cipher);
    reader.bytes = stream;
    reader.byte_count = sizeof(stream);
    reader.cursor = 0U;
    reader.fail_at_call = 0;
    reader.calls = 0;

    result = redmcsb_f1918_load_initial_save_parts_pc34(read_exact, &reader, &receipt);
    CHECK("F1918 accepts header and first three source parts", result == REDMCSB_F1918_PC34_RESULT_OK);
    CHECK("F1918 uses sequential original reads", reader.calls == 4 && reader.cursor == sizeof(stream));
    CHECK("F1914 records admitted header", receipt.header_valid == 1 && receipt.parts_loaded == 3U);
    CHECK("F1918 extracts source header keys and checksums",
          memcmp(receipt.keys, keys, sizeof(keys)) == 0 &&
          memcmp(receipt.checksums, checksums, sizeof(checksums)) == 0);
    CHECK("F1913 restores exact source part bytes",
          memcmp(global_cipher, global_plain, sizeof(global_plain)) == 0 &&
          memcmp(groups_cipher, groups_plain, sizeof(groups_plain)) == 0 &&
          memcmp(party_cipher, party_plain, sizeof(party_plain)) == 0);

    memcpy(global_cipher, stream + sizeof(header), sizeof(global_cipher));
    memcpy(groups_cipher, stream + sizeof(header) + sizeof(global_cipher), sizeof(groups_cipher));
    memcpy(party_cipher, stream + sizeof(header) + sizeof(global_cipher) + sizeof(groups_cipher), sizeof(party_cipher));
    memset(&receipt, 0, sizeof(receipt));
    receipt.parts[0].bytes = global_cipher;
    receipt.parts[0].byte_count = sizeof(global_cipher);
    receipt.parts[1].bytes = groups_cipher;
    receipt.parts[1].byte_count = sizeof(groups_cipher);
    receipt.parts[2].bytes = party_cipher;
    receipt.parts[2].byte_count = sizeof(party_cipher);
    reader.cursor = 0U;
    reader.calls = 0;
    reader.fail_at_call = 3;
    CHECK("F1918 preserves HINTLOAD active-group failure stage",
          redmcsb_f1918_load_initial_save_parts_pc34(read_exact, &reader, &receipt) ==
              REDMCSB_F1918_PC34_RESULT_ACTIVE_GROUPS_FAILED &&
          receipt.header_valid == 1 && receipt.parts_loaded == 1U);
    CHECK("zero-byte F1910 does not call reader",
          redmcsb_f1910_load_saved_game_part_pc34(NULL, NULL, NULL, 0U) == 1);
    CHECK("source evidence is available",
          strstr(redmcsb_f1918_hintload_pc34_source_evidence(), "F1918") != NULL);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB HINTLOAD F1910/F1913/F1914/F1918 initial save parts");
    return 0;
}
