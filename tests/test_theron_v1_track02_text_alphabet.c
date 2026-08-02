#include "theron_v1_track02_text_alphabet.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *alpha = theron_v1_track02_text_alphabet();
    assert(strlen(alpha) == THERON_TRACK02_ALPHABET_LENGTH);

    /* Index 0 = terminator */
    assert(alpha[0] == '|');

    /* Index 1 = 'a' */
    assert(alpha[1] == 'a');

    /* Index 24 = 'x' */
    assert(alpha[24] == 'x');

    /* Index 25 = '0' */
    assert(alpha[25] == '0');

    /* Index 32 = '7' */
    assert(alpha[32] == '7');

    /* Common-word shortcuts */
    assert(strcmp(theron_v1_track02_text_shortcut_the(), "THE") == 0);
    assert(strcmp(theron_v1_track02_text_shortcut_you(), "YOU") == 0);

    printf("PASS: theron_v1_track02_text_alphabet\n");
    return 0;
}
