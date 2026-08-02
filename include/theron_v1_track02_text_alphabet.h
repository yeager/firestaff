#ifndef THERON_V1_TRACK02_TEXT_ALPHABET_H
#define THERON_V1_TRACK02_TEXT_ALPHABET_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * 5-bit packed text encoding alphabet from UD 0x09D44F.
 *
 * The alphabet string is: "|abcdefghijklmnopqrstuvwx01234567x"
 * where '|' (0x7C) is the terminator/escape character.
 * Common-word shortcuts follow: "THE", "YOU", plus single chars y/z/{/|.
 *
 * This is the same encoding used for champion names at UD 0x09D1D6:
 * 3 characters per 16-bit LE word, each character is a 5-bit index
 * into this alphabet (0 = '|' terminator, 1 = 'a', ... 24 = 'x'). */

#define THERON_TRACK02_ALPHABET_LENGTH  34u

const char *theron_v1_track02_text_alphabet(void);
const char *theron_v1_track02_text_shortcut_the(void);
const char *theron_v1_track02_text_shortcut_you(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_TEXT_ALPHABET_H */
