#include "theron_v1_track02_save_strings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(strstr(theron_v1_track02_us_which_file_play(), "PLAY") != NULL);
    assert(strstr(theron_v1_track02_us_which_file_load(), "LOAD") != NULL);
    assert(strstr(theron_v1_track02_us_file_exists(), "EXISTS") != NULL);

    assert(strcmp(theron_v1_track02_us_save_file_name(0), "FILE_1") == 0);
    assert(strcmp(theron_v1_track02_us_save_file_name(1), "FILE_2") == 0);
    assert(strcmp(theron_v1_track02_us_save_file_name(2), "FILE_3") == 0);
    assert(theron_v1_track02_us_save_file_name(3) == NULL);

    assert(strcmp(theron_v1_track02_us_yes(), "YES") == 0);
    assert(strcmp(theron_v1_track02_us_no(), "NO ") == 0);
    assert(strcmp(theron_v1_track02_us_replace(), "REPLACE") == 0);

    printf("PASS: theron_v1_track02_save_strings\n");
    return 0;
}
