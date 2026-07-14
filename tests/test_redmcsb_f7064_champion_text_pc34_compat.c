#include "redmcsb_f7064_champion_text_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); failures++; } } while (0)

int main(void)
{
    uint8_t names[2][REDMCSB_F7064_CHAMPION_NAME_BYTES] = {
        {'A', 'L', 0, 'x', 'x', 'x', 'x', 'x'},
        {'F', 'U', 'L', 'L', 'N', 'A', 'M', 'E'}
    };
    uint8_t titles[2][REDMCSB_F7064_CHAMPION_TITLE_BYTES];

    memset(titles, 'x', sizeof(titles));
    titles[0][0] = 'T'; titles[0][1] = 0;
    memcpy(titles[1], "TWENTY-CHARACTER-TEXT", REDMCSB_F7064_CHAMPION_TITLE_BYTES);
    redmcsb_f7064_pad_champion_name_and_title_pc34(names, titles, 2U);
    CHECK(names[0][2] == 0 && names[0][7] == 0, "F7064 zero-fills name after first NUL");
    CHECK(memcmp(names[1], "FULLNAME", 8U) == 0, "F7064 preserves a full 8-byte name");
    CHECK(titles[0][1] == 0 && titles[0][19] == 0, "F7064 zero-fills title after first NUL");
    CHECK(memcmp(titles[1], "TWENTY-CHARACTER-TEXT", 20U) == 0, "F7064 preserves a full 20-byte title");
    CHECK(strcmp(redmcsb_f7064_champion_text_pc34_source_evidence(), "ReDMCSB CEDTINCQ.C F7064 PC34 fixed name/title padding") == 0, "source evidence identifies F7064");
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F7064 champion text padding");
    return 0;
}
