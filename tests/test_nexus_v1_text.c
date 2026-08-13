#include "nexus_v1_text.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        failures++; \
    } \
} while (0)

static void test_small_output_buffers(void)
{
    static const uint8_t ascii[] = {'A', 'B', 'C'};
    static const uint8_t kana[] = {0xA1};
    char output[4] = {'x', 'x', 'x', 'x'};

    CHECK(nexus_v1_sjis_to_utf8(ascii, 3, output, 0) == 0,
          "zero-capacity output is rejected without a write");
    CHECK(output[0] == 'x', "zero-capacity output remains untouched");
    CHECK(nexus_v1_sjis_to_utf8(ascii, 3, output, 1) == 0 &&
          output[0] == '\0', "one-byte output receives only a terminator");
    CHECK(nexus_v1_sjis_to_utf8(kana, 1, output, 3) == 0 &&
          output[0] == '\0', "short kana output is not partially written");
    CHECK(nexus_v1_sjis_to_utf8(kana, 1, output, 4) == 3,
          "kana fits exactly with its terminator");
}

static void test_extracted_strings_do_not_alias(void)
{
    static const uint8_t data[] = {
        0, 'F', 'I', 'R', 'S', 'T', 0, 0,
        'S', 'E', 'C', 'O', 'N', 'D', 0
    };
    char *strings[2] = {NULL, NULL};

    CHECK(nexus_v1_extract_strings(data + 1, (int)sizeof(data) - 1,
                                   strings, 2) == 2,
          "two ASCII strings are extracted");
    CHECK(strings[0] && strings[1] && strings[0] != strings[1],
          "extracted strings have distinct storage");
    CHECK(strings[0] && strcmp(strings[0], "FIRST") == 0,
          "first extracted string remains stable");
    CHECK(strings[1] && strcmp(strings[1], "SECOND") == 0,
          "second extracted string is correct");
}

int main(void)
{
    test_small_output_buffers();
    test_extracted_strings_do_not_alias();
    if (failures) {
        fprintf(stderr, "test_nexus_v1_text: %d failure(s)\n", failures);
        return 1;
    }
    puts("test_nexus_v1_text: PASS");
    return 0;
}
