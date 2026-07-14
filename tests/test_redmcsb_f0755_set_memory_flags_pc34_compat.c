#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0755_set_memory_flags_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\\n", message); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    uint8_t profile[] = { 2U, 0xFDU, 3U, 5U, 0xFEU, 0U };
    uint8_t terminal_profile[] = { 1U, 0xFEU, 0xFFU };
    uint8_t rejected_profile[] = { 4U, 0xFFU };
    uint8_t *cursor = profile;
    uint8_t flags[REDMCSB_F0755_MEMORY_FLAG_COUNT_PC34_COMPAT] = {
        9U, 9U, 9U, 9U, 9U, 9U
    };
    int32_t byte_count = -1;

    CHECK(redmcsb_f0755_set_memory_flags_pc34_compat(
              &cursor, &byte_count, flags) == 1,
          "FE followed by non-FF continues with the next profile");
    CHECK(cursor == profile + 5,
          "cursor is stored immediately after the FE sentinel");
    CHECK(byte_count == 3072,
          "FD byte is converted from KiB to bytes");
    CHECK(flags[0] == 0U && flags[1] == 0U && flags[2] == 1U &&
              flags[3] == 0U && flags[4] == 0U && flags[5] == 1U,
          "ordinary profile bytes set only their matching flag");

    cursor = terminal_profile;
    memset(flags, 9, sizeof(flags));
    byte_count = -1;
    CHECK(redmcsb_f0755_set_memory_flags_pc34_compat(
              &cursor, &byte_count, flags) == 0,
          "FE followed by FF rejects a further memory profile");
    CHECK(cursor == terminal_profile + 2,
          "terminal FE still commits the cursor after the sentinel");
    CHECK(byte_count == 0,
          "each call resets the byte count before scanning");
    CHECK(flags[0] == 0U && flags[1] == 1U && flags[2] == 0U &&
              flags[3] == 0U && flags[4] == 0U && flags[5] == 0U,
          "each call clears all six flags before applying its profile");

    cursor = rejected_profile;
    memset(flags, 9, sizeof(flags));
    byte_count = -1;
    CHECK(redmcsb_f0755_set_memory_flags_pc34_compat(
              &cursor, &byte_count, flags) == 0,
          "FF before FE rejects the stream");
    CHECK(cursor == rejected_profile,
          "pre-FE FF leaves the caller cursor unchanged as in the source");
    CHECK(byte_count == 0 && flags[4] == 1U,
          "pre-FE FF keeps mutations already made during this call");
    CHECK(strstr(redmcsb_f0755_set_memory_flags_source_evidence_pc34(),
                 "STARTUP2.C:1054-1079") != NULL,
          "source evidence names the exact ReDMCSB range");

    puts("ok: ReDMCSB F0755 PC 3.4 memory-profile flags");
    return 0;
}
