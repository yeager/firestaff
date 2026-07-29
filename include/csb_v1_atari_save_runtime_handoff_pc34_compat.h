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

/* MINI.DAT does not own the prison-selected party transfer.  Therefore this
 * commits only after a real party already exists in the runtime. */
int csb_v1_atari_save_handoff_runtime_pc34_compat(
    CSB_V1_RuntimeProfile *profile, const uint8_t *bytes, size_t size,
    CSB_V1_AtariSaveInfo *out_info);

const char *csb_v1_atari_save_runtime_handoff_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
