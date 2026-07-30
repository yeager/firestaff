#include "csb_v1_csbwin_dungeon_tail.h"
#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(expr) do { if (!(expr)) { ++failures; \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } } while (0)

static void put_be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8u);
    p[1] = (uint8_t)value;
}

static void check_staged_real_save(void)
{
    const char *path = getenv("FIRESTAFF_CSBWIN_REAL_SAVE");
    FILE *file;
    long length;
    uint8_t *bytes;
    CSB_V1_CSBWinExtendedTailReport extension;
    CSB_V1_CSBWinExtendedDSAReport dsa;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_CSBWin512BodyReport body;
    CSB_V1_CSBWinDungeonTailPrefix prefix;

    if (!path || path[0] == '\0') {
        puts("SKIP: FIRESTAFF_CSBWIN_REAL_SAVE is not staged");
        return;
    }
    file = fopen(path, "rb");
    CHECK(file != NULL);
    if (!file) return;
    CHECK(fseek(file, 0L, SEEK_END) == 0);
    length = ftell(file);
    CHECK(length > 0L);
    CHECK(fseek(file, 0L, SEEK_SET) == 0);
    if (length <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    CHECK(bytes != NULL);
    if (!bytes) {
        fclose(file);
        return;
    }
    CHECK(fread(bytes, 1u, (size_t)length, file) == (size_t)length);
    fclose(file);
    memset(&extension, 0, sizeof(extension));
    memset(&dsa, 0, sizeof(dsa));
    memset(&features, 0, sizeof(features));
    memset(&body, 0, sizeof(body));
    CHECK(csb_v1_csbwin_512_inspect_extended_tail(bytes, (size_t)length,
              &extension, &dsa, &features) == CSB_V1_CSBWIN_EXTENDED_OK);
    CHECK(extension.valid);
    CHECK(csb_v1_csbwin_512_verify_save_body(bytes + extension.next_payload_offset,
              (size_t)length - extension.next_payload_offset, 12u, &body) ==
          CSB_V1_CSBWIN_512_OK);
    CHECK(body.appended_size > CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              bytes + extension.next_payload_offset + body.appended_offset,
              body.appended_size, features.flags, &prefix) == 0);
    CHECK(prefix.valid && prefix.level_count > 0u);
    CHECK(prefix.next_database_offset < body.appended_size);
    free(bytes);
}

int main(void)
{
    uint8_t tail[512];
    CSB_V1_CSBWinDungeonTailPrefix report;
    const size_t levels = 2u;
    const size_t descriptors = CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES +
        levels * CSB_V1_CSBWIN_LEVEL_DESC_BYTES;

    memset(tail, 0, sizeof(tail));
    put_be16(tail + 0u, 13u);
    put_be16(tail + 2u, 128u);
    put_be16(tail + 4u, 0x0200u);
    put_be16(tail + 6u, 3u);
    put_be16(tail + 10u, 5u);
    put_be16(tail + 12u, 4u);
    put_be16(tail + descriptors - 32u + 8u, (uint16_t)(3u << 6u));
    put_be16(tail + descriptors - 16u + 8u, (uint16_t)(1u << 6u));

    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, sizeof(tail),
              CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT, &report) == 0);
    CHECK(report.valid);
    CHECK(report.sentinel == 13u);
    CHECK(report.level_count == 2u);
    CHECK(report.text_word_count == 3u);
    CHECK(report.object_list_length == 5u);
    CHECK(report.database_entries[0] == 4u);
    CHECK(report.level_last_column[0] == 3u);
    CHECK(report.level_last_column[1] == 1u);
    CHECK(report.column_pointer_count == 6u);
    CHECK(report.level_descriptors_offset == 44u);
    CHECK(report.object_list_index_offset == descriptors);
    CHECK(report.object_list_offset == descriptors + 12u);
    CHECK(report.text_offset == descriptors + 12u + 10u);
    CHECK(report.next_database_offset == descriptors + 12u + 10u + 12u);
    CHECK(report.indirect_text);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(tail, 43u, 0u, &report) ==
          CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED);
    put_be16(tail + 4u, 0u);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(tail, sizeof(tail), 0u,
              &report) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT);

    check_staged_real_save();

    if (failures) return 1;
    puts("PASS: CSBWin dungeon-tail prefix framing");
    return 0;
}
