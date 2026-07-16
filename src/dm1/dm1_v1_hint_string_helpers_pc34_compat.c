#include "dm1_v1_hint_string_helpers_pc34_compat.h"

int F1984_ConvertCharacterToLowerCase(int character) {
    if ((character >= 'A') && (character <= 'Z')) {
        return character + ('a' - 'A');
    }
    return character;
}

int dm1_v1_hint_convert_character_to_lower_case_f1984_pc34(int character) {
    return F1984_ConvertCharacterToLowerCase(character);
}

const char*
dm1_v1_hint_convert_character_to_lower_case_f1984_source_pc34(void) {
    return "ReDMCSB HINTCASE.C:13 F1984_ConvertCharacterToLowerCase: "
           "ASCII A-Z fold used by hint text helpers; non-uppercase bytes "
           "pass through unchanged.";
}

char* F2014_ConvertStringToLowerCase(char* text) {
    char* cursor;

    if (text == 0) {
        return 0;
    }

    for (cursor = text; *cursor != '\0'; ++cursor) {
        *cursor = (char)F1984_ConvertCharacterToLowerCase(
            (unsigned char)*cursor);
    }
    return text;
}

char* dm1_v1_hint_convert_string_to_lower_case_f2014_pc34(char* text) {
    return F2014_ConvertStringToLowerCase(text);
}

const char*
dm1_v1_hint_convert_string_to_lower_case_f2014_source_pc34(void) {
    return "ReDMCSB HINT001.C:8 F2014_ConvertStringToLowerCase: "
           "in-place NUL-terminated hint text fold via F1984; no hint file, "
           "screen, palette, or oracle state is synthesized.";
}

size_t F1909_CopyStringUntilCharacter(char* dst,
                                      size_t dstCapacity,
                                      const char* src,
                                      int stopCharacter) {
    size_t copied;
    size_t written;
    unsigned char stop;

    if (src == 0) {
        if ((dst != 0) && (dstCapacity > 0u)) {
            dst[0] = '\0';
        }
        return 0u;
    }

    copied = 0u;
    written = 0u;
    stop = (unsigned char)stopCharacter;

    while (((unsigned char)src[copied] != stop) &&
           (src[copied] != '\0')) {
        if ((dst != 0) && (written + 1u < dstCapacity)) {
            dst[written] = src[copied];
            ++written;
        }
        ++copied;
    }

    if ((dst != 0) && (dstCapacity > 0u)) {
        dst[written] = '\0';
    }

    return copied;
}

size_t dm1_v1_hint_copy_string_until_character_f1909_pc34(
    char* dst,
    size_t dstCapacity,
    const char* src,
    int stopCharacter) {
    return F1909_CopyStringUntilCharacter(
        dst, dstCapacity, src, stopCharacter);
}

const char*
dm1_v1_hint_copy_string_until_character_f1909_source_pc34(void) {
    return "ReDMCSB HINTHINT.C:179 F1909_CopyStringUntilCharacter: "
           "copies caller-provided hint text until a delimiter; PC34 bounds "
           "the destination and does not synthesize HTC/file/oracle data.";
}
