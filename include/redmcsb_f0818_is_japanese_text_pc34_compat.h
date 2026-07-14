/*
 * ReDMCSB CEDT030.C F0818_IsJapaneseText, PC-98 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0818_IS_JAPANESE_TEXT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0818_IS_JAPANESE_TEXT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns nonzero when the original PC-98 test classifies string as Japanese:
 * an initial ESC byte, or any nonzero byte with bit 7 set.
 */
int redmcsb_f0818_is_japanese_text_pc34_compat(const uint8_t *string);

const char *redmcsb_f0818_is_japanese_text_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
