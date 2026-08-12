#include "csb_hint_oracle_session.h"

#include <string.h>

void csb_hint_oracle_session_init(CSB_HintOracleSession *session)
{
    if (!session) return;
    memset(session, 0, sizeof(*session));
    session->state = CSB_HINT_ORACLE_SESSION_AWAIT_LOAD;
}

int csb_hint_oracle_session_select_location(CSB_HintOracleSession *session,
                                            const CSB_HintOracleHTC *htc,
                                            uint8_t level, uint8_t x, uint8_t y)
{
    size_t source_count = 0u;
    int rc;
    if (!session || !htc) return CSB_HINT_ORACLE_SESSION_ERR_ARGUMENT;
    csb_hint_oracle_session_init(session);
    session->htc = htc;
    rc = csb_hint_oracle_htc_find_hints_for_location(
        htc, level, x, y, session->selected_hint_indices,
        CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS, &source_count);
    /* The parser reports truncation, whereas HINTHINT.C intentionally stops
     * after row seven.  Its first seven source-order values are authoritative. */
    if (rc != CSB_HINT_ORACLE_HTC_OK &&
        rc != CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL) {
        csb_hint_oracle_session_init(session);
        return CSB_HINT_ORACLE_SESSION_ERR_FORMAT;
    }
    session->selected_hint_count = source_count;
    if (session->selected_hint_count > CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS)
        session->selected_hint_count = CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS;
    session->state = session->selected_hint_count == 0u ?
        CSB_HINT_ORACLE_SESSION_NO_CLUE : CSB_HINT_ORACLE_SESSION_HINT_LIST;
    return CSB_HINT_ORACLE_SESSION_OK;
}

int csb_hint_oracle_session_open_hint_row(CSB_HintOracleSession *session,
                                          size_t row)
{
    CSB_HintOracleHTC_Hint hint;
    if (!session || session->state != CSB_HINT_ORACLE_SESSION_HINT_LIST)
        return CSB_HINT_ORACLE_SESSION_ERR_STATE;
    if (row >= session->selected_hint_count) return CSB_HINT_ORACLE_SESSION_ERR_ROW;
    if (csb_hint_oracle_htc_get_hint(session->htc,
                                     session->selected_hint_indices[row], &hint) !=
        CSB_HINT_ORACLE_HTC_OK || hint.page_count == 0u)
        return CSB_HINT_ORACLE_SESSION_ERR_FORMAT;
    session->selected_row = row;
    session->page_number = 1u;
    session->state = CSB_HINT_ORACLE_SESSION_HINT_PAGE;
    return CSB_HINT_ORACLE_SESSION_OK;
}

static int turn_page(CSB_HintOracleSession *session, int delta)
{
    CSB_HintOracleHTC_Hint hint;
    if (!session || session->state != CSB_HINT_ORACLE_SESSION_HINT_PAGE)
        return CSB_HINT_ORACLE_SESSION_ERR_STATE;
    if (csb_hint_oracle_htc_get_hint(session->htc,
                                     session->selected_hint_indices[session->selected_row],
                                     &hint) != CSB_HINT_ORACLE_HTC_OK)
        return CSB_HINT_ORACLE_SESSION_ERR_FORMAT;
    if ((delta < 0 && session->page_number <= 1u) ||
        (delta > 0 && session->page_number >= hint.page_count))
        return CSB_HINT_ORACLE_SESSION_ERR_PAGE_BOUNDARY;
    session->page_number = (size_t)((int)session->page_number + delta);
    return CSB_HINT_ORACLE_SESSION_OK;
}

int csb_hint_oracle_session_previous_page(CSB_HintOracleSession *session)
{ return turn_page(session, -1); }

int csb_hint_oracle_session_next_page(CSB_HintOracleSession *session)
{ return turn_page(session, 1); }

int csb_hint_oracle_session_done(CSB_HintOracleSession *session)
{
    if (!session) return CSB_HINT_ORACLE_SESSION_ERR_ARGUMENT;
    if (session->state == CSB_HINT_ORACLE_SESSION_HINT_PAGE) {
        session->state = CSB_HINT_ORACLE_SESSION_HINT_LIST;
        session->page_number = 0u;
        return CSB_HINT_ORACLE_SESSION_OK;
    }
    if (session->state == CSB_HINT_ORACLE_SESSION_HINT_LIST ||
        session->state == CSB_HINT_ORACLE_SESSION_NO_CLUE) {
        csb_hint_oracle_session_init(session);
        return CSB_HINT_ORACLE_SESSION_OK;
    }
    return CSB_HINT_ORACLE_SESSION_ERR_STATE;
}

void csb_hint_oracle_session_close(CSB_HintOracleSession *session)
{
    if (!session) return;
    csb_hint_oracle_session_init(session);
    session->state = CSB_HINT_ORACLE_SESSION_CLOSED;
}

const CSB_HintOracleHTC_Hint *
csb_hint_oracle_session_current_hint(const CSB_HintOracleSession *session,
                                     CSB_HintOracleHTC_Hint *out_hint)
{
    if (!session || !out_hint || session->state != CSB_HINT_ORACLE_SESSION_HINT_PAGE ||
        session->selected_row >= session->selected_hint_count ||
        csb_hint_oracle_htc_get_hint(session->htc,
                                     session->selected_hint_indices[session->selected_row],
                                     out_hint) != CSB_HINT_ORACLE_HTC_OK)
        return NULL;
    return out_hint;
}

const char *csb_hint_oracle_session_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_SESSION_OK: return "OK";
    case CSB_HINT_ORACLE_SESSION_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_SESSION_ERR_STATE: return "state";
    case CSB_HINT_ORACLE_SESSION_ERR_ROW: return "row";
    case CSB_HINT_ORACLE_SESSION_ERR_PAGE_BOUNDARY: return "page-boundary";
    case CSB_HINT_ORACLE_SESSION_ERR_FORMAT: return "format";
    default: return "unknown";
    }
}
