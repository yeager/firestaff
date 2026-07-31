#include "dm1_v1_f0433_save_command_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

int main(void)
{
    uint8_t header[DM1_ORIGINAL_SAVE_PC34_HEADER_BYTES];
    uint8_t part0[128], part1[16], part2[32], part3[20], part4[8];
    uint8_t output[1024];
    uint16_t random_words[DM1_ORIGINAL_SAVE_PC34_HEADER_RANDOM_WORD_COUNT];
    DM1_V1_F0433SaveCommandRequestPc34 request;
    DM1_V1_F0433SaveCommandReceiptPc34 receipt;
    uint8_t *parts[] = { part0, part1, part2, part3, part4 };
    size_t sizes[] = { sizeof(part0), sizeof(part1), sizeof(part2),
                       sizeof(part3), sizeof(part4) };
    size_t index;

    memset(header, 0, sizeof(header));
    header[299] = 5u;
    header[306] = 0x44u;
    header[307] = 0x4du;
    write_le16(header + 374u, 1u);
    write_le16(header + 376u, 0u);
    for (index = 0u; index < 5u; ++index) {
        size_t byte;
        write_le16(header + 310u + index * 2u, (uint16_t)(0x1200u + index));
        for (byte = 0u; byte < sizes[index]; ++byte) {
            parts[index][byte] = (uint8_t)(index * 31u + byte);
        }
    }
    for (index = 0u; index < sizeof(random_words) / sizeof(random_words[0]);
         ++index) {
        random_words[index] = (uint16_t)(0x4000u + index * 13u);
    }
    memset(&request, 0, sizeof(request));
    memcpy(request.headerPlain, header, sizeof(header));
    request.sourceHeaderReceiptValid = 1;
    request.sourceRandomWords = random_words;
    request.sourceRandomWordCount = sizeof(random_words) / sizeof(random_words[0]);
    for (index = 0u; index < 5u; ++index) {
        request.parts[index].plainBytes = parts[index];
        request.parts[index].byteCount = sizes[index];
        request.parts[index].key = (uint16_t)(0x1200u + index);
        request.parts[index].sourceReceiptValid = 1;
    }

    CHECK(strstr(dm1_v1_f0433_save_command_source_evidence_pc34(), "F0433") != NULL);
    CHECK(dm1_v1_f0433_save_command_write_pc34(&request, output, sizeof(output),
                                                &receipt));
    CHECK(receipt.accepted && receipt.bytesWritten == 716u &&
          receipt.partOffsets[0] == 512u && receipt.partOffsets[1] == 640u &&
          receipt.partOffsets[4] == 708u && receipt.headerFingerprint != 0u &&
          receipt.partChecksums[0] != 0u && receipt.suppressSyntheticFallback);
    /* The first encrypted source word starts at 512. 0x80 would be the
     * obsolete private 128-byte prefix rather than PC34 save data. */
    CHECK(output[512] == 0x00u && output[513] == 0x13u);

    request.parts[2].sourceReceiptValid = 0;
    CHECK(!dm1_v1_f0433_save_command_write_pc34(&request, output, sizeof(output),
                                                 &receipt));
    request.parts[2].sourceReceiptValid = 1;
    request.sourceRandomWordCount--;
    CHECK(!dm1_v1_f0433_save_command_write_pc34(&request, output, sizeof(output),
                                                 &receipt));
    request.sourceRandomWordCount++;
    write_le16(request.headerPlain + 310u, 0xbeefu);
    CHECK(!dm1_v1_f0433_save_command_write_pc34(&request, output, sizeof(output),
                                                 &receipt));

    printf("test_dm1_v1_f0433_save_command_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
