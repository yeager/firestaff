/*
 * ReDMCSB IMAGE.C F0698_InvertBox, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0698_INVERT_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0698_INVERT_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* M704-M707 zone ordering in ReDMCSB: left, right, top, bottom. */
typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} RedmcsbF0698ZonePc34Compat;

typedef void (*RedmcsbF0698InvertBoxPc34Compat)(
    void *context,
    int16_t left,
    int16_t right,
    int16_t top,
    int16_t bottom);

typedef struct {
    RedmcsbF0698InvertBoxPc34Compat invert_box;
    void *context;
} RedmcsbF0698VideoDriverPc34Compat;

/*
 * Calls the PC 3.4 video-driver inversion callback using the zone bounds.
 * Returns false when the required driver, callback, or zone is absent.
 */
bool redmcsb_f0698_invert_box_pc34_compat(
    const RedmcsbF0698VideoDriverPc34Compat *video_driver,
    const RedmcsbF0698ZonePc34Compat *zone);

const char *redmcsb_f0698_invert_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0698_INVERT_BOX_PC34_COMPAT_H */
