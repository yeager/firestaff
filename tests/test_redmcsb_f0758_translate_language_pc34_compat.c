#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0758_translate_language_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

#define REQUIRE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "requirement failed: %s at line %d\n", #condition, \
                    __LINE__); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    char first[] = "READY";
    char second[] = "CLICK";
    char *strings[] = {first, second};
    redmcsb_f0757_texts_pc34_compat texts;
    const char *negative;
    const char *at_count;

    texts.texts = first;
    texts.strings = strings;
    texts.string_count = 2;

    REQUIRE(redmcsb_f0758_translate_language_pc34_compat(&texts, 0) == first);
    REQUIRE(redmcsb_f0758_translate_language_pc34_compat(&texts, 1) == second);

    negative = redmcsb_f0758_translate_language_pc34_compat(&texts, -1);
    at_count = redmcsb_f0758_translate_language_pc34_compat(&texts, 2);
    REQUIRE(strcmp(negative, "") == 0);
    REQUIRE(strcmp(at_count, "") == 0);
    REQUIRE(negative == at_count);
    REQUIRE(strstr(redmcsb_f0758_translate_language_source_evidence_pc34(),
                   "LANGUAGE.C:43-51") != NULL);

    puts("ok: ReDMCSB F0758 PC 3.4 text-table lookup");
    return 0;
}
