#include "f0711_convert_scan_code_to_ascii_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void assert_conversion(int16_t scan_code, int16_t expected)
{
    (void)expected;
    (void)scan_code;
    assert(f0711_convert_scan_code_to_ascii_pc34_compat(scan_code) == expected);
}

int main(void)
{
    /* G8038: unshifted top row, alpha row, punctuation, keypad and unmapped. */
    assert_conversion(0x0001, 0x001B);
    assert_conversion(0x0002, '1');
    assert_conversion(0x0010, 'q');
    assert_conversion(0x001E, 'a');
    assert_conversion(0x002B, '\\');
    assert_conversion(0x0039, ' ');
    assert_conversion(0x004A, '-');
    assert_conversion(0x0053, 0x007F);
    assert_conversion(0x003A, 0x0000);
    assert_conversion(0x007F, 0x0000);

    /* G8039 selected only by the original bit-9 qualifier. */
    assert_conversion(0x0202, '!');
    assert_conversion(0x0210, 'Q');
    assert_conversion(0x021A, '{');
    assert_conversion(0x022B, '|');
    assert_conversion(0x0233, '<');

    /* F8092 indexes with & 0x7F; make/break and unrelated qualifiers do
     * not alter its table selection. */
    assert_conversion((int16_t)0x0082, '1');
    assert_conversion((int16_t)0xFF10, 'Q');
    assert_conversion((int16_t)0x8100, 0x0000);

    assert(strstr(f0711_convert_scan_code_to_ascii_source_evidence_pc34(),
                  "IO2.C:168-177") != 0);
    return 0;
}
