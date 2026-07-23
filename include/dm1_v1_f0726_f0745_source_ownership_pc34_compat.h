#ifndef FIRESTAFF_DM1_V1_F0726_F0745_SOURCE_OWNERSHIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0726_F0745_SOURCE_OWNERSHIP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34 = 0,
    DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34 = 1,
    DM1_V1_F0726_F0745_PC34_NOOP_PC34 = 2
} DM1_V1_F0726F0745OwnershipKindPc34;

typedef struct {
    unsigned int functionId;
    DM1_V1_F0726F0745OwnershipKindPc34 kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
} DM1_V1_F0726F0745OwnershipPc34;

const DM1_V1_F0726F0745OwnershipPc34 *
dm1_v1_f0726_f0745_source_ownership_pc34(unsigned int functionId);

/* MUSIC.C:513-524 and :540-557 select no I34E/I34M statements. These
 * functions intentionally have no host-audio callback or generated sound. */
void dm1_v1_f0738_music_continue_pc34_noop(void);
void dm1_v1_f0739_music_stop_pc34_noop(void);

const char *dm1_v1_f0726_f0745_source_ownership_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
