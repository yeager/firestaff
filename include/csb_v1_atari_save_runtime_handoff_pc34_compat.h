/* Authenticated Atari CSB MINI.DAT -> live runtime handoff. */
#ifndef FIRESTAFF_CSB_V1_ATARI_SAVE_RUNTIME_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_ATARI_SAVE_RUNTIME_HANDOFF_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_atari_save_decode_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_ATARI_RUNTIME_OK = 0,
    CSB_V1_ATARI_RUNTIME_ERR_NULL = -1,
    CSB_V1_ATARI_RUNTIME_ERR_PARTY = -2,
    CSB_V1_ATARI_RUNTIME_ERR_DUNGEON = -3,
    CSB_V1_ATARI_RUNTIME_ERR_POSE = -4
} CSB_V1_AtariRuntimeResult;

/* A decoded F0435 candidate owns its dungeon until it is committed or
 * discarded.  This lets backup recovery validate the complete original save
 * before LOADSAVE.C's BAK -> DAT filesystem transition. */
typedef struct {
    CSB_V1_DungeonData *dungeon;
    CSB_V1_PartyState party;
    CSB_V1_AtariSaveInfo info;
} CSB_V1_AtariSaveHandoffCandidate;

/* Split the fallible F0435 decode/load phase from the one-way runtime
 * ownership transition.  A successfully prepared candidate is safe to
 * commit without further parsing or allocation. */
int csb_v1_atari_save_prepare_runtime_handoff_pc34_compat(
    const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveHandoffCandidate *out_candidate);
void csb_v1_atari_save_discard_runtime_handoff_candidate_pc34_compat(
    CSB_V1_AtariSaveHandoffCandidate *candidate);
int csb_v1_atari_save_commit_runtime_handoff_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_AtariSaveHandoffCandidate *candidate,
    CSB_V1_AtariSaveInfo *out_info);

/* The native GAMEBLOCK2/character sections are decoded before the dungeon
 * ownership transition, so this route does not need a synthetic party. */
int csb_v1_atari_save_handoff_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile, const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveInfo *out_info);

const char *csb_v1_atari_save_runtime_handoff_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
