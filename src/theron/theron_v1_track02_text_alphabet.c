#include "theron_v1_track02_text_alphabet.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * 5-bit packed text alphabet from UD 0x09D44F.
 * Index 0 = terminator '|', indices 1-24 = 'a'-'x', 25-32 = '0'-'7',
 * index 33 = 'x' (duplicate). Common-word shortcuts: "THE", "YOU". */

const char *theron_v1_track02_text_alphabet(void) {
    return "|abcdefghijklmnopqrstuvwx01234567x";
}

const char *theron_v1_track02_text_shortcut_the(void) {
    return "THE";
}

const char *theron_v1_track02_text_shortcut_you(void) {
    return "YOU";
}
