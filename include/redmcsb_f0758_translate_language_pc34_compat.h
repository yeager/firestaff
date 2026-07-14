/* ReDMCSB LANGUAGE.C F0758_TranslateLanguage, PC 3.4 text-table route. */
#ifndef FIRESTAFF_REDMCSB_F0758_TRANSLATE_LANGUAGE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0758_TRANSLATE_LANGUAGE_PC34_COMPAT_H

#include <stdint.h>

#include "redmcsb_f0757_load_texts_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns the C700 text-table pointer at string_index.  ReDMCSB uses its
 * global empty string when the signed index is outside the loaded range.
 */
const char *redmcsb_f0758_translate_language_pc34_compat(
    const redmcsb_f0757_texts_pc34_compat *texts, int16_t string_index);

const char *redmcsb_f0758_translate_language_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
