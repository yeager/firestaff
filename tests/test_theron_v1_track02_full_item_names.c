#include "theron_v1_track02_full_item_names.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(theron_v1_track02_us_full_item_count() == 80);

    assert(strcmp(theron_v1_track02_us_full_item_name(0), "COMPASS") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(1), "TORCH") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(5), "FLAMITT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(9), "FALCHION") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(10), "SWORD") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(27), "PLATE OF LYTE") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(41), "SHIELD DEFIANT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(51), "RABBIT'S FOOT") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(74), "SCREAMER SLICE") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(78), "RA KEY") == 0);
    assert(strcmp(theron_v1_track02_us_full_item_name(79), "EMPTY FLASK") == 0);

    assert(theron_v1_track02_us_full_item_name(80) == NULL);

    printf("PASS: theron_v1_track02_full_item_names\n");
    return 0;
}
