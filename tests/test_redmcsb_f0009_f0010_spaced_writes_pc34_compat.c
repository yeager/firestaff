#include "redmcsb_f0009_f0010_spaced_writes_pc34_compat.h"

#include <stdio.h>

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    char bytes[] = { 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a' };
    char zero_count_byte = 'a';
    char zero_spacing_byte = 'a';
    int16_t words[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int16_t zero_count_word = 7;
    int16_t zero_spacing_word = 7;
    int ok = 1;

    F0009_MAIN_WriteSpacedBytes(bytes, 3, 'x', 4);
    ok &= check(bytes[0] == 'x' && bytes[4] == 'x' && bytes[8] == 'x' &&
                    bytes[1] == 'a' && bytes[7] == 'a',
                "F0009 writes bytes at the requested stride");

    F0009_MAIN_WriteSpacedBytes(&zero_count_byte, 0, 'x', 1);
    ok &= check(zero_count_byte == 'a', "F0009 zero count leaves the buffer unchanged");

    F0009_MAIN_WriteSpacedBytes(&zero_spacing_byte, 3, 'x', 0);
    ok &= check(zero_spacing_byte == 'x', "F0009 zero spacing repeatedly writes index zero");

    F0010_MAIN_WriteSpacedWords(words, 3, (int16_t)-1, 8);
    ok &= check(words[0] == -1 && words[4] == -1 && words[8] == -1 &&
                    words[1] == 0 && words[7] == 0,
                "F0010 converts byte spacing eight to a four-word stride");

    F0010_MAIN_WriteSpacedWords(&zero_count_word, 0, (int16_t)-1, 2);
    ok &= check(zero_count_word == 7, "F0010 zero count leaves the buffer unchanged");

    F0010_MAIN_WriteSpacedWords(&zero_spacing_word, 3, (int16_t)-1, 0);
    ok &= check(zero_spacing_word == -1,
                "F0010 zero spacing repeatedly writes word index zero");

    if (!ok) return 1;
    puts("PASS redmcsb_f0009_f0010_spaced_writes_pc34_compat");
    return 0;
}
