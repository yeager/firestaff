#ifndef FIRESTAFF_DM1_V1_CHAMPION_UNPOISON_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_UNPOISON_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The portion of champion state owned by F0323.  Keeping this separate from
 * the host champion record makes the reset usable by every death/revive path. */
typedef struct {
    uint16_t poisonDose;
} DM1_V1_ChampionPoisonStatePc34Compat;

/* ReDMCSB CHAMPION.C F0323_CHAMPION_Unpoison (line 1652).
 * Cancels the accumulated poison payload. */
void F0323_CHAMPION_Unpoison_Compat(
    DM1_V1_ChampionPoisonStatePc34Compat* poisonState);

const char* F0323_CHAMPION_Unpoison_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_UNPOISON_PC34_COMPAT_H */
