#include "theron_v1_track02_file_cabinet.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(strstr(theron_v1_track02_us_no_space(), "FILE CABINET") != NULL);
    assert(strstr(theron_v1_track02_us_choose_delete(), "DELETE") != NULL);
    assert(strstr(theron_v1_track02_us_sure(), "SURE") != NULL);
    assert(strstr(theron_v1_track02_us_thank_you(), "THANK YOU") != NULL);
    assert(strstr(theron_v1_track02_us_not_saved(), "NOT BE SAVED") != NULL);
    assert(strstr(theron_v1_track02_us_boot_attention(), "ATTENTION") != NULL);
    assert(strstr(theron_v1_track02_us_boot_attention(), "SUPER CD-ROM2") != NULL);

    printf("PASS: theron_v1_track02_file_cabinet\n");
    return 0;
}
