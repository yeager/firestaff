#include "dm1_v1_hint_string_helpers_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    char text[] = "AbC-Z 09 !\x7f";
    (void)text;
    char dst[8];
    char tiny[4];
    char zeroGuard = 'x';
    (void)zeroGuard;
    const char* source;
    (void)source;

    assert(F1984_ConvertCharacterToLowerCase('A') == 'a');
    assert(F1984_ConvertCharacterToLowerCase('Z') == 'z');
    assert(F1984_ConvertCharacterToLowerCase('M') == 'm');
    assert(F1984_ConvertCharacterToLowerCase('a') == 'a');
    assert(F1984_ConvertCharacterToLowerCase('0') == '0');
    assert(F1984_ConvertCharacterToLowerCase(0x80) == 0x80);
    assert(dm1_v1_hint_convert_character_to_lower_case_f1984_pc34('Q') ==
           'q');

    assert(F2014_ConvertStringToLowerCase(text) == text);
    assert(strcmp(text, "abc-z 09 !\x7f") == 0);
    assert(dm1_v1_hint_convert_string_to_lower_case_f2014_pc34(0) == 0);

    memset(dst, '?', sizeof(dst));
    assert(F1909_CopyStringUntilCharacter(dst, sizeof(dst), "abc:def", ':') ==
           3u);
    assert(strcmp(dst, "abc") == 0);

    memset(dst, '?', sizeof(dst));
    assert(F1909_CopyStringUntilCharacter(dst, sizeof(dst), "abcdef", ':') ==
           6u);
    assert(strcmp(dst, "abcdef") == 0);

    memset(tiny, '?', sizeof(tiny));
    assert(dm1_v1_hint_copy_string_until_character_f1909_pc34(
               tiny, sizeof(tiny), "abcdef", ':') == 6u);
    assert(strcmp(tiny, "abc") == 0);

    assert(F1909_CopyStringUntilCharacter(&zeroGuard, 0u, "abc", ':') ==
           3u);
    assert(zeroGuard == 'x');
    assert(F1909_CopyStringUntilCharacter(dst, sizeof(dst), 0, ':') == 0u);
    assert(dst[0] == '\0');

    source = dm1_v1_hint_convert_character_to_lower_case_f1984_source_pc34();
    assert(source != 0);
    assert(strstr(source, "HINTCASE.C:13") != 0);
    assert(strstr(source, "F1984") != 0);

    source = dm1_v1_hint_convert_string_to_lower_case_f2014_source_pc34();
    assert(source != 0);
    assert(strstr(source, "HINT001.C:8") != 0);
    assert(strstr(source, "F2014") != 0);
    assert(strstr(source, "no hint file") != 0);

    source = dm1_v1_hint_copy_string_until_character_f1909_source_pc34();
    assert(source != 0);
    assert(strstr(source, "HINTHINT.C:179") != 0);
    assert(strstr(source, "F1909") != 0);
    assert(strstr(source, "does not synthesize HTC/file/oracle data") != 0);

    return 0;
}
