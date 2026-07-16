#ifndef CSB_V1_F1918_HINTLOAD_INITIAL_LOAD_PC34_COMPAT_H
#define CSB_V1_F1918_HINTLOAD_INITIAL_LOAD_PC34_COMPAT_H

#include "redmcsb_f1918_hintload_pc34_compat.h"

/* CSB-owned wrapper for the ReDMCSB HINTHTC.C F1918 initial load boundary.
 *
 * F1918 consumes exactly the already-admitted save header plus GLOBAL_DATA,
 * ACTIVE_GROUPS, and PARTY spans. F1919 is represented as the following
 * post boundary: only an accepted F1918 transaction may hand off to later
 * LOADSAVE.C tail processing. No DSA, save-tail, seek table, allocation, or
 * substitute part is inferred here. */

typedef RedmcsbF1910ReadExactPc34 CSB_V1_F1918_ReadExactPc34;
typedef RedmcsbF1918LoadReceiptPc34 CSB_V1_F1918_LoadReceiptPc34;

int csb_v1_f1918_load_game_cpsx_pc34(
    CSB_V1_F1918_ReadExactPc34 read, void *context,
    CSB_V1_F1918_LoadReceiptPc34 *receipt);

int csb_v1_f1919_post_f1918_load_game_cpsx_pc34(
    const CSB_V1_F1918_LoadReceiptPc34 *receipt);

const char *csb_v1_f1918_hintload_initial_load_source_evidence_pc34(void);

#endif
