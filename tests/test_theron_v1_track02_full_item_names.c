#include "theron_v1_track02_full_item_names.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_us_full_item_count() == 79);

    assert(strcmp(theron_v1_track02_us_full_item_name(0), "TORCH") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(4), "FLAMITT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(8), "FALCHION") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(9), "SWORD") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(26), "PLATE OF LYTE") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(40), "SHIELD DEFIANT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(50), "RABBIT'S FOOT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(73), "SCREAMER SLICE") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(77), "RA KEY") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(78), "EMPTY FLASK") == 0);

    assert(theron_v1_track02_us_full_item_name(79) == NULL);

    printf("PASS: theron_v1_track02_full_item_names\n");
    return 0;
}
