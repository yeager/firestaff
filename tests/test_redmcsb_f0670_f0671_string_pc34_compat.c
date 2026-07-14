#include "redmcsb_f0670_f0671_string_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    char output[64];
    int ok = 1;

    ok &= expect(redmcsb_f0670_replace_character_by_string_pc34_compat(
                     "OUT OF MEMORY. ~K NEEDED.", "512", '~', output) == 1 &&
                     strcmp(output, "OUT OF MEMORY. 512K NEEDED.") == 0,
                 "F0670 replaces only the source first placeholder");
    ok &= expect(redmcsb_f0670_replace_character_by_string_pc34_compat(
                     "A~B~C", "X", '~', output) == 1 &&
                     strcmp(output, "AXB~C") == 0,
                 "F0670 preserves later source characters");
    ok &= expect(redmcsb_f0670_replace_character_by_string_pc34_compat(
                     "SAVE.DAT", "X", '~', output) == 0 &&
                     strcmp(output, "SAVE.DAT") == 0,
                 "F0670 copies unchanged source when character is absent");
    ok &= expect(redmcsb_f0670_replace_character_by_string_pc34_compat(
                     NULL, "X", '~', output) == -1,
                 "F0670 rejects missing caller-owned source");

    redmcsb_f0671_convert_long_to_string_pc34_compat(0, output);
    ok &= expect(strcmp(output, "0") == 0, "F0671 formats zero");
    redmcsb_f0671_convert_long_to_string_pc34_compat(-12345, output);
    ok &= expect(strcmp(output, "-12345") == 0, "F0671 formats negative value");
    redmcsb_f0671_convert_long_to_string_pc34_compat(INT32_MAX, output);
    ok &= expect(strcmp(output, "2147483647") == 0,
                 "F0671 formats PC long maximum");
    redmcsb_f0671_convert_long_to_string_pc34_compat(INT32_MIN, output);
    ok &= expect(strcmp(output, "-2147483648") == 0,
                 "F0671 maintains a bounded host-safe PC long minimum result");
    ok &= expect(strstr(redmcsb_f0670_f0671_string_source_evidence_pc34(),
                        "STRING.C") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
