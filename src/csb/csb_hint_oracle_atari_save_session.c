#include "csb_hint_oracle_atari_save_session.h"

int csb_hint_oracle_atari_save_session_select(
    CSB_HintOracleSession *session,
    const CSB_HintOracleHTC *htc,
    const CSB_V1_AtariSaveInfo *info)
{
    int rc;
    if (!session || !htc || !info)
        return CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ARGUMENT;
    if (info->party_map_index < 0 || info->party_map_index > 255 ||
        info->party_x < 0 || info->party_x > 255 ||
        info->party_y < 0 || info->party_y > 255)
        return CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_POSE;
    rc = csb_hint_oracle_session_select_location(
        session, htc, (uint8_t)info->party_map_index,
        (uint8_t)info->party_x, (uint8_t)info->party_y);
    return rc == CSB_HINT_ORACLE_SESSION_OK ?
        CSB_HINT_ORACLE_ATARI_SAVE_SESSION_OK :
        CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ORACLE;
}

const char *csb_hint_oracle_atari_save_session_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_ATARI_SAVE_SESSION_OK: return "OK";
    case CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_POSE: return "pose";
    case CSB_HINT_ORACLE_ATARI_SAVE_SESSION_ERR_ORACLE: return "oracle";
    default: return "unknown";
    }
}
