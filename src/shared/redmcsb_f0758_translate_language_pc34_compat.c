#include "redmcsb_f0758_translate_language_pc34_compat.h"

const char *redmcsb_f0758_translate_language_pc34_compat(
    const redmcsb_f0757_texts_pc34_compat *texts, int16_t string_index)
{
    /* ReDMCSB LANGUAGE.C:43-51, MEDIA736_I34M PC 3.4 route. */
    if (string_index < 0 || string_index >= (int16_t)texts->string_count) {
        return "";
    }
    return texts->strings[string_index];
}

const char *redmcsb_f0758_translate_language_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 LANGUAGE.C:43-51: F0758 returns G2135_ "
           "(the initialized empty string) for a signed index below zero or "
           "at/above G2171_; otherwise it returns G2172_[index] directly.";
}
