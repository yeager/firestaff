#ifndef FIRESTAFF_REDMCSB_F1063_CHECKSUM_EOR_CPSX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1063_CHECKSUM_EOR_CPSX_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * READWRIT.C F1063_ChecksumEor_CPSX is an Amiga copy-protection checksum
 * routine over function addresses, implemented solely as 68k assembly. Its
 * recovered declaration and call-site return contract disagree, so the source
 * supplies no portable callable behavior or PC 3.4 counterpart.
 */
void redmcsb_f1063_checksum_eor_cpsx_pc34_compat(void);

const char *redmcsb_f1063_checksum_eor_cpsx_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1063_CHECKSUM_EOR_CPSX_PC34_COMPAT_H */
