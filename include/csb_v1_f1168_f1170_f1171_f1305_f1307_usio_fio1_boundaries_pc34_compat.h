#ifndef FIRESTAFF_CSB_V1_F1168_F1170_F1171_F1305_F1307_USIO_FIO1_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1168_F1170_F1171_F1305_F1307_USIO_FIO1_BOUNDARIES_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

void F1168_USIO_18_Empty(void);
void F1170_USIO_03_Expunge(void);
void F1171_USIO_19_LockDF0(void);
void F1305_OpenFTLLibrary(void);
void F1307_FIO1_03_Expunge(void);

void csb_v1_f1168_usio_18_empty_pc34_compat(void);
void csb_v1_f1170_usio_03_expunge_pc34_compat(void);
void csb_v1_f1171_usio_19_lock_df0_pc34_compat(void);
void csb_v1_f1305_open_ftl_library_pc34_compat(void);
void csb_v1_f1307_fio1_03_expunge_pc34_compat(void);

const char *csb_v1_f1168_usio_18_empty_source_evidence_pc34(void);
const char *csb_v1_f1170_usio_03_expunge_source_evidence_pc34(void);
const char *csb_v1_f1171_usio_19_lock_df0_source_evidence_pc34(void);
const char *csb_v1_f1305_open_ftl_library_source_evidence_pc34(void);
const char *csb_v1_f1307_fio1_03_expunge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1168_F1170_F1171_F1305_F1307_USIO_FIO1_BOUNDARIES_PC34_COMPAT_H */
