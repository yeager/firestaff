#include "../src/ui/menu_data_root_policy_m12.h"
#include <stdio.h>

#define CHECK(expected, explicitRoot, selected, candidate, requested, selectedGame, candidateGame) \
    do { \
        int actual = M12_MenuDataRoot_ShouldUseCandidate( \
            explicitRoot, selected, candidate, requested, selectedGame, candidateGame); \
        if (actual != expected) { \
            fprintf(stderr, "FAIL: root policy at line %d (got %d, expected %d)\n", \
                    __LINE__, actual, expected); \
            return 1; \
        } \
    } while (0)

int main(void) {
    /* Explicit --data-dir always owns the root, even if it is empty. */
    CHECK(0, 1, 0, 5, 0, 0, 0);
    CHECK(0, 1, 1, 5, 1, 0, 1);
    /* A persisted DM1/CSB/DM2 collection is not replaced by a default root
     * that happens to admit Nexus and Theron as well. */
    CHECK(0, 0, 3, 5, 0, 0, 0);
    CHECK(0, 0, 1, 5, 1, 1, 1);
    /* --menu --game dm2 scans all cards: an admitted DM1 root stays selected
     * and DM2 remains a blocked card until the user changes media. */
    CHECK(0, 0, 1, 5, 0, 0, 1);
    /* Recover an empty persisted root or a missing requested --game. */
    CHECK(1, 0, 0, 3, 0, 0, 0);
    CHECK(1, 0, 1, 1, 1, 0, 1);
    CHECK(0, 0, 1, 1, 1, 0, 0);
    CHECK(0, 0, 0, 0, 0, 0, 0);
    puts("PASS: M12 data-root fallback policy");
    return 0;
}
