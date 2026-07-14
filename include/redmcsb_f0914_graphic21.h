#ifndef FIRESTAFF_REDMCSB_F0914_GRAPHIC21_H
#define FIRESTAFF_REDMCSB_F0914_GRAPHIC21_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB GRAPH21.C F0914_Graphic21 source branches. */
#define REDMCSB_F0914_GRAPHIC21_A20E_FIRST_WORD_INDEX UINT16_C(20)
#define REDMCSB_F0914_GRAPHIC21_A20E_LAST_WORD_INDEX UINT16_C(508)
#define REDMCSB_F0914_GRAPHIC21_A31E_FIRST_WORD_INDEX UINT16_C(635)
#define REDMCSB_F0914_GRAPHIC21_A31E_LAST_WORD_INDEX UINT16_C(1123)
#define REDMCSB_F0914_GRAPHIC21_FUZZY_WORD_COUNT UINT16_C(29)
#define REDMCSB_F0914_GRAPHIC21_ANALYZED_VALUE INT16_C(136)
#define REDMCSB_F0914_GRAPHIC21_CHECK_TIME_VALUE INT16_C(512)

/*
 * GRAPH21.C receives this state through an array of int16_t pointers and
 * casts result[2] to long *. The portable boundary makes each destination
 * explicit. The caller owns all pointed-to storage and must provide a sector
 * buffer through the selected branch's last word index and 29 fuzzy words.
 * F0914 performs no null or capacity validation in the original source.
 */
typedef struct {
    uint16_t *fuzzy_bits;
    int16_t *fuzzy_sector_analyzed;
    int32_t *last_event22_time;
    int16_t *check_last_event22_time;
} redmcsb_f0914_graphic21_result;

/* GRAPH21.C:170-204, MEDIA432_A20E: scans sector words 20 through 508. */
int16_t redmcsb_f0914_graphic21_a20e(
    const uint16_t *sector_buffer,
    const redmcsb_f0914_graphic21_result *result);

/* GRAPH21.C:207-241, MEDIA618_A31E: scans sector words 635 through 1123. */
int16_t redmcsb_f0914_graphic21_a31e(
    const uint16_t *sector_buffer,
    const redmcsb_f0914_graphic21_result *result);

const char *redmcsb_f0914_graphic21_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0914_GRAPHIC21_H */
