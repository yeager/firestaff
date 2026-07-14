#ifndef FIRESTAFF_REDMCSB_F1043_NOOP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1043_NOOP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C F1043_ is an empty routine in the Amiga-only MEDIA746 route. Keep the
 * portable boundary empty rather than assigning it an inferred responsibility.
 */
void redmcsb_f1043_noop_pc34_compat(void);

const char *redmcsb_f1043_noop_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1043_NOOP_PC34_COMPAT_H */
