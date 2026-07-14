#ifndef FIRESTAFF_REDMCSB_F1067_INIT_AMIGA_STUFF_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1067_INIT_AMIGA_STUFF_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F1067_InitAmigaStuff initializes Amiga OS resources for each source media
 * variant. The source supplies no PC 3.4 branch or portable host behavior.
 */
void redmcsb_f1067_init_amiga_stuff_pc34_compat(void);

const char *redmcsb_f1067_init_amiga_stuff_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1067_INIT_AMIGA_STUFF_PC34_COMPAT_H */
