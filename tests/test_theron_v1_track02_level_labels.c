#include "theron_v1_track02_level_labels.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(theron_v1_track02_us_level_label_count() == 16);

    const char *blank = theron_v1_track02_us_level_label(0);
    assert(blank != NULL);
    assert(strlen(blank) == 8);
    assert(strcmp(blank, "        ") == 0);

    assert(strcmp(theron_v1_track02_us_level_label(1), "LEVEL  1") == 0);
    assert(strcmp(theron_v1_track02_us_level_label(5), "LEVEL  5") == 0);
    assert(strcmp(theron_v1_track02_us_level_label(10), "LEVEL 10") == 0);
    assert(strcmp(theron_v1_track02_us_level_label(15), "LEVEL 15") == 0);

    assert(theron_v1_track02_us_level_label(16) == NULL);

    for (unsigned i = 1; i <= 15; i++) {
        const char *l = theron_v1_track02_us_level_label(i);
        assert(l != NULL);
        assert(strlen(l) == 8);
        assert(strncmp(l, "LEVEL", 5) == 0);
    }

    printf("PASS: theron_v1_track02_level_labels\n");
    return 0;
}
