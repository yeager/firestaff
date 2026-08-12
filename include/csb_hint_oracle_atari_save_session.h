/*
 * Authenticated Atari MINI.DAT coordinate handoff for the CSB Hint Oracle.
 *
 * ReDMCSB HINTHINT.C C09 reads PartyMapIndex/PartyMapX/PartyMapY from the
 * selected CSB game.  Firestaff's CSB_V1_AtariSaveInfo is the existing
 * checked GAMEBLOCK2 receipt for those native fields.  This adapter is
 * intentionally limited to that receipt; it does not accept generic runtime
 * coordinates, CSBWin saves or a replacement save parser.
 */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_ATARI_SAVE_SESSION_H
#define FIRESTAFF_CSB_HINT_ORACLE_ATARI_SAVE_SESSION_H

#include "csb_hint_oracle_session.h"
#include "csb_v1_atari_save_decode_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_HINT_ORACLE_ATARI_SAVE_SESSION_OK = 0,
    CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_POSE = -2,
    CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ORACLE = -3
} CSB_HintOracleAtariSaveSession_Result;

/* Start a Hint Oracle list from a successful native Atari MINI.DAT decode.
 * The info receipt must remain owned by the caller; only its signed source
 * fields are copied into the session's unsigned HTC lookup request. */
int csb_hint_oracle_atari_save_session_select(
    CSB_HintOracleSession *session,
    const CSB_HintOracleHTC *htc,
    const CSB_V1_AtariSaveInfo *info);

const char *csb_hint_oracle_atari_save_session_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_ATARI_SAVE_SESSION_H */
