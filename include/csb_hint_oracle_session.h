/*
 * Source-shaped Hint Oracle selection state.
 *
 * ReDMCSB HINTHINT.C C09_SELECT_HINTS walks the HTC location table in
 * file order, accepts an exact coordinate or the 255,255 wildcard and stops
 * after seven entries.  C06_DRAW_HINT_LIST exposes those seven rows; F1940
 * keeps a one-based page number and LAST/NEXT never wrap.  This module keeps
 * exactly that state.  It deliberately has no save-file or launcher I/O: its
 * caller must supply coordinates from an authenticated CSB Atari save/runtime.
 */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_SESSION_H
#define FIRESTAFF_CSB_HINT_ORACLE_SESSION_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS 7u

typedef enum {
    CSB_HINT_ORACLE_SESSION_AWAIT_LOAD = 0,
    CSB_HINT_ORACLE_SESSION_HINT_LIST,
    CSB_HINT_ORACLE_SESSION_HINT_PAGE,
    CSB_HINT_ORACLE_SESSION_NO_CLUE,
    CSB_HINT_ORACLE_SESSION_CLOSED
} CSB_HintOracleSession_State;

typedef enum {
    CSB_HINT_ORACLE_SESSION_OK = 0,
    CSB_HINT_ORACLE_SESSION_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_SESSION_ERR_STATE = -2,
    CSB_HINT_ORACLE_SESSION_ERR_ROW = -3,
    CSB_HINT_ORACLE_SESSION_ERR_PAGE_BOUNDARY = -4,
    CSB_HINT_ORACLE_SESSION_ERR_FORMAT = -5
} CSB_HintOracleSession_Result;

typedef struct {
    const CSB_HintOracleHTC *htc; /* borrowed; must outlive this session */
    uint16_t selected_hint_indices[CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS];
    size_t selected_hint_count;
    size_t selected_row;
    size_t page_number; /* one-based while state is HINT_PAGE */
    CSB_HintOracleSession_State state;
} CSB_HintOracleSession;

void csb_hint_oracle_session_init(CSB_HintOracleSession *session);

/* Begin the source C09 selection phase from an authenticated location. */
int csb_hint_oracle_session_select_location(CSB_HintOracleSession *session,
                                            const CSB_HintOracleHTC *htc,
                                            uint8_t level, uint8_t x, uint8_t y);

/* Select a visible 0..6 row, then show that hint's original first page. */
int csb_hint_oracle_session_open_hint_row(CSB_HintOracleSession *session,
                                          size_t row);
int csb_hint_oracle_session_previous_page(CSB_HintOracleSession *session);
int csb_hint_oracle_session_next_page(CSB_HintOracleSession *session);

/* Source DONE: page -> list; list/no-clue -> prompt for another load. */
int csb_hint_oracle_session_done(CSB_HintOracleSession *session);
void csb_hint_oracle_session_close(CSB_HintOracleSession *session);

const CSB_HintOracleHTC_Hint *
csb_hint_oracle_session_current_hint(const CSB_HintOracleSession *session,
                                     CSB_HintOracleHTC_Hint *out_hint);

const char *csb_hint_oracle_session_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_SESSION_H */
