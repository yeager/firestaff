#include "theron_v1_track02_class_base_stats.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_class_base_word_count() == 16);

    const uint16_t *w = theron_v1_track02_class_base_words();
    assert(w != NULL);

    assert(w[0] == 0);
    assert(w[2] == 60);
    assert(w[3] == 50);
    assert(w[4] == 256);
    assert(w[8] == 3);
    assert(w[12] == 0);
    assert(w[15] == 90);

    printf("PASS: theron_v1_track02_class_base_stats\n");
    return 0;
}
